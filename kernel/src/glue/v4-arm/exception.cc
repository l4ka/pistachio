/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/exception.cc
 * Description:   ARM syscall and page fault handlers
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: exception.cc,v 1.35 2006/10/27 18:00:51 reichelt Exp $
 *
 ********************************************************************/

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)
#include INC_API(syscalls.h)
#include INC_API(kernelinterface.h)
#include INC_ARCH(thread.h)
#include INC_GLUE(exception.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(intctrl.h)
#include INC_CPU(cpu.h)
#include INC_PLAT(timer.h)
#include INC_API(schedule.h)
#include <kdb/tracepoints.h>
#ifdef CONFIG_DEBUG
#include <kdb/console.h>
#endif

DECLARE_TRACEPOINT(ARM_PAGE_FAULT);
DECLARE_TRACEPOINT(EXCEPTION_IPC_SYSCALL);
DECLARE_TRACEPOINT(EXCEPTION_IPC_GENERAL);

INLINE void halt_user_thread( void )
{
    tcb_t *current = get_current_tcb();

    current->set_state( thread_state_t::halted );
    current->switch_to_idle();
}

/* As on MIPS */

#define SYSCALL_SAVED_REGISTERS (EXCEPT_IPC_SYS_MR_NUM+1)

static bool send_exception_ipc( word_t exc_no, word_t exc_code, arm_irq_context_t *context );

static bool send_syscall_ipc(arm_irq_context_t *context)
{
    tcb_t *current = get_current_tcb();

    if (current->get_exception_handler().is_nilthread()) {
        printf( "Unable to deliver user exception: no exception handler.\n" );
        return false;
    }

    TRACEPOINT (EXCEPTION_IPC_SYSCALL,
		printf ("EXCEPTION_IPC_SYSCALL: (%p) IP = %p\n",
			current, (word_t)current->get_user_ip()));

    // Save message registers on the stack
    word_t saved_mr[SYSCALL_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for (int i = 0; i < SYSCALL_SAVED_REGISTERS; i++)
        saved_mr[i] = current->get_mr(i);

    current->set_saved_partner(current->get_partner());
    current->set_saved_state(current->get_state());
 
    // Create the message tag.
    tag.set(0, EXCEPT_IPC_SYS_MR_NUM, EXCEPT_IPC_SYS_LABEL);
    current->set_tag(tag);

    // Create the message.
    current->set_mr(EXCEPT_IPC_SYS_MR_PC, (word_t)current->get_user_ip()); 
    current->set_mr(EXCEPT_IPC_SYS_MR_R0, context->r0);
    current->set_mr(EXCEPT_IPC_SYS_MR_R1, context->r1);
    current->set_mr(EXCEPT_IPC_SYS_MR_R2, context->r2);
    current->set_mr(EXCEPT_IPC_SYS_MR_R3, context->r3);
    current->set_mr(EXCEPT_IPC_SYS_MR_R4, context->r4);
    current->set_mr(EXCEPT_IPC_SYS_MR_R5, context->r5);
    current->set_mr(EXCEPT_IPC_SYS_MR_R6, context->r6);
    current->set_mr(EXCEPT_IPC_SYS_MR_R7, context->r7);
    current->set_mr(EXCEPT_IPC_SYS_MR_SP, (word_t)current->get_user_sp());
    current->set_mr(EXCEPT_IPC_SYS_MR_LR, context->lr);
    current->set_mr(EXCEPT_IPC_SYS_MR_SYSCALL, *(word_t *)((word_t)current->get_user_ip()-4));
    current->set_mr(EXCEPT_IPC_SYS_MR_FLAGS, current->get_user_flags());
 

    // Deliver the exception IPC.
    tag = current->do_ipc(current->get_exception_handler(),
            current->get_exception_handler(), timeout_t::never());

    // Alter the user context if necessary.
    if (!tag.is_error()) {
        current->set_user_ip((addr_t)current->get_mr(EXCEPT_IPC_SYS_MR_PC));
        current->set_user_sp((addr_t)current->get_mr(EXCEPT_IPC_SYS_MR_SP));
        current->set_user_flags(current->get_mr(EXCEPT_IPC_SYS_MR_FLAGS));

	context->r0 = current->get_mr(EXCEPT_IPC_SYS_MR_R0);
	context->r1 = current->get_mr(EXCEPT_IPC_SYS_MR_R1);
	context->r2 = current->get_mr(EXCEPT_IPC_SYS_MR_R2);
	context->r3 = current->get_mr(EXCEPT_IPC_SYS_MR_R3);
	context->r4 = current->get_mr(EXCEPT_IPC_SYS_MR_R4);
	context->r5 = current->get_mr(EXCEPT_IPC_SYS_MR_R5);
	context->r6 = current->get_mr(EXCEPT_IPC_SYS_MR_R6);
	context->r7 = current->get_mr(EXCEPT_IPC_SYS_MR_R7);
	context->lr = current->get_mr(EXCEPT_IPC_SYS_MR_LR);
    } else {
        printf("Unable to deliver user exception: IPC error.\n");
    }

    // Clean-up.
    for (int i = 0; i < SYSCALL_SAVED_REGISTERS; i++)
        current->set_mr(i, saved_mr[i]);
    
    current->set_partner(current->get_saved_partner());
    current->set_saved_partner(NILTHREAD);
    current->set_state(current->get_saved_state());
    current->set_saved_state(thread_state_t::aborted);
    
    return !tag.is_error();
}

extern "C" void syscall_exception(arm_irq_context_t *context)
{  
    if (!send_syscall_ipc(context)) {
        printf(TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, 
                "Unhandled User SYSCALL");  

	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);

	halt_user_thread();
    }
}

extern "C" void undefined_exception(arm_irq_context_t *context)
{  
    word_t instr = *(word_t *)
	    ((word_t)get_current_tcb()->get_user_ip()-4);

    if (!send_exception_ipc(1, instr, context)) {
	printf(TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, 
		    "Unhandled User Undefined Instruction");  

	if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    get_kip()->kdebug_entry(context);

	halt_user_thread();
    }
}

extern "C" void reset_exception(arm_irq_context_t *context)
{  
    printf(TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, 
		"Unhandled Reset Exception");  

    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	get_kip()->kdebug_entry(context);

    halt_user_thread();
}

extern "C" void fiq_exception(arm_irq_context_t *context)
{  
    printf(TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, 
		"FIQ Exception");  

    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	get_kip()->kdebug_entry(context);

    halt_user_thread();
}

extern "C" void arm_misc_l4_syscall(arm_irq_context_t *context)
{
#ifdef CONFIG_DEBUG
    switch (context->pc & 0xff) {
    case L4_TRAP_KIP: 
	{
	    space_t *space = get_current_space();

	    context->r0 = (word_t)space->get_kip_page_area().get_base();
	    context->r1 = get_kip()->api_version;
	    context->r2 = get_kip()->api_flags;
	    context->r3 = get_kip()->kernel_desc_ptr ?
		    *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr)
		    : 0;

	    return;
	}
    case L4_TRAP_KPUTC:
	putc((char)context->r0);
	return;
    case L4_TRAP_KGETC:
	context->r0 = (s32_t)(s8_t)getc(true);
	return;
    case L4_TRAP_KGETC_NB:
	context->r0 = (s32_t)(s8_t)getc(false);
	return;
    case L4_TRAP_KDEBUG:
	printf("User trap to kdebug\n");
	kdebug_entry(context);
	return;
    default:
	break;
    }
#endif

    printf("Illegal misc syscall\n");
    halt_user_thread();
}

extern "C" u64_t sys_clock(void)
{
    return get_current_scheduler()->get_current_time();
}

extern "C" void arm_page_fault(word_t fault_status, addr_t fault_addr,
        arm_irq_context_t *context, word_t data_abt)
{
    pgent_t::pgsize_e pgsize;
    pgent_t *pg;
    word_t fs = fault_status & 0xf;

    TRACEPOINT(ARM_PAGE_FAULT,
	printf("pf @ %p, pc = %p, tcb = %p, fs = %x\n", fault_addr, 
	    (addr_t)context->pc, get_current_tcb(), fs));

    /* See ARM ARM B3-19 - only deal with prefetch aborts, translation,
     * domain and permission data aborts. Alignment and external aborts are
     * not currently recoverable.
     */
    if (fault_status)
    {
	switch(fs) {
	    case 7: case 5: case 15: case 13: case 9: case 11:
		break;
	    case 2:	/* Terminal exception, not recoverable */
	    default:
		if (!send_exception_ipc(0x100 + fs, (word_t)fault_addr, context)) {
		    printf(TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, 
				"Unhandled Memory Abort");  

		    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
			get_kip()->kdebug_entry(context);

		    halt_user_thread();
		}
		return;
	}
    }

    space_t *space = get_current_tcb()->get_space();
                                                                                
    if (space == NULL)
	space = get_kernel_space();
                                                                                
    ASSERT(space);
                                     
    bool is_valid = space->lookup_mapping(fault_addr, &pg, &pgsize);
    bool is_writable = is_valid && pg->is_writable(space, pgsize);

    word_t fault_section = (word_t)fault_addr >> 20;

    /* Special case when TCB sections are copied in for the first time */
    if (space->is_tcb_area(fault_addr) && *space->pgent(fault_section) !=
		    *get_kernel_space()->pgent(fault_section)) {
	*space->pgent(fault_section) =
		*get_kernel_space()->pgent(fault_section);
        return;
    }


#ifdef CONFIG_ENABLE_FASS
    /* Does the faulter's space's section match that in the CPD for the fault
     * address? 
     */

    pgent_t spg = *space->pgent(fault_section);
    pgent_t cpg = cpd->pt.pdir[fault_section];

    if (cpg != spg)
    {
loop:
	bool cpd_section_valid = cpg.is_valid(cpd, pgent_t::size_1m); 
	arm_domain_t cpd_section_domain = cpg.get_domain();

	/* Clean the kernel's UTCB synonym if necessary */
	bool is_utcb_section = 
		((word_t)get_current_tcb()->get_utcb_location() >> 20)
		== fault_section;
	bool flush_utcb = is_utcb_section && 
		TEST_BIT_WORD(utcb_dirty, current_domain);

	bool need_flush = flush_utcb || 
		(cpd_section_valid &&  
		    TEST_BIT_WORD(domain_dirty, cpd_section_domain));

	/* If there is an existing mapping in the CPD for this section,
	 * remove it from the owner domain's set of sections in the CPD 
	 */
	if (cpd_section_valid)
	    arm_fass.remove_set(cpd_section_domain, fault_section);

	/* Update the faulter domain's set of sections in the CPD */
	arm_fass.add_set(current_domain, fault_section);

	/* Update the CPD */
	if (spg.is_valid(space, pgent_t::size_1m)) 
	    space->pgent(fault_section)->set_domain(current_domain);
	cpd->pt.pdir[fault_section] = *space->pgent(fault_section);

	if (need_flush) {
	    arm_fass.clean_all();
	    SET_BIT_WORD(domain_dirty, current_domain);
	} else {
	    // need to remove all entries corresponding to fault section
	    // from TLB. Cheaper to flush than to iterate over section
	    // on SA-1100 (FIXME - check other archs' MMU).
	    arm_cache::tlb_flush();
	}

	if (is_valid)
	    return;
    }
#endif

/*printf("v=%x w=%x %p-%p, %p-%p\n", is_valid, is_writable,
		&cpd->pt.pdir[fault_section], cpd->pt.pdir[fault_section],
		space->pgent(fault_section), *space->pgent(fault_section));
*/
    space->handle_pagefault(fault_addr, (addr_t)context->pc,
		    data_abt ? is_valid && !is_writable ?
		    space_t::write : space_t::read : space_t::execute,
		    ((context->cpsr & 0x1f) != 0x10));

    if (fs == 0xf)
	arm_cache::tlb_flush_ent(fault_addr, ARM_PAGE_BITS);
    else if (fs == 0xd)
	arm_cache::tlb_flush();

#ifdef CONFIG_ENABLE_FASS
    if (!is_valid)
	return;

    spg = *space->pgent(fault_section);
    cpg = cpd->pt.pdir[fault_section];

    if (cpg != spg)
	goto loop;
#endif
}

#define GENERIC_SAVED_REGISTERS (EXCEPT_IPC_GEN_MR_NUM+1)

static bool send_exception_ipc( word_t exc_no, word_t exc_code, arm_irq_context_t *context )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
	return false;

    TRACEPOINT (EXCEPTION_IPC_GENERAL,
		printf ("EXCEPTION_IPC_GENERAL: (%p) exc_no %d, exc_code %016lx, IP = %p\n",
			current, exc_no, exc_code, (word_t)current->get_user_ip()));

    // Save message registers on the stack
    word_t saved_mr[GENERIC_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, EXCEPT_IPC_GEN_MR_NUM, EXCEPT_IPC_GEN_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_GEN_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_GEN_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_GEN_MR_FLAGS, (word_t)current->get_user_flags() );
    current->set_mr( EXCEPT_IPC_GEN_MR_EXCEPTNO, exc_no );
    current->set_mr( EXCEPT_IPC_GEN_MR_ERRORCODE, exc_code );
    current->set_mr( EXCEPT_IPC_GEN_MR_LOCALID, current->get_local_id().get_raw() );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_IP) );
	current->set_user_sp( (addr_t)current->get_mr(EXCEPT_IPC_GEN_MR_SP) );
	current->set_user_flags( current->get_mr(EXCEPT_IPC_GEN_MR_FLAGS) );
    }
    else
	printf( "Unable to deliver user exception: IPC error.\n" );

    // Clean-up.
    for( int i = 0; i < GENERIC_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}
