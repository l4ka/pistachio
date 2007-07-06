/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 * Copyright (C) 2003,  National ICT Australia
 *                
 * File path:     glue/v4-mips64/exception.cc
 * Description:   Mips64 exception handling
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
 * $Id: exception.cc,v 1.37 2005/07/12 07:35:45 cgray Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(kernelinterface.h)
#include INC_API(syscalls.h)
#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(exception.h)

#include <kdb/tracepoints.h>
#ifdef CONFIG_DEBUG
#include <kdb/console.h>
#endif

DECLARE_TRACEPOINT(EXCEPTION_IPC_SYSCALL);
DECLARE_TRACEPOINT(EXCEPTION_IPC_GENERAL);

extern "C" void mips64_exception(mips64_irq_context_t *context);

#define GENERIC_SAVED_REGISTERS (EXCEPT_IPC_GEN_MR_NUM+1)

static bool send_exception_ipc( word_t exc_no, word_t exc_code, mips64_irq_context_t *context )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
    {
	return false;
    }

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
    current->set_mr( EXCEPT_IPC_GEN_MR_FLAGS, (word_t)current->get_user_flags() |
			    (context->cause & CAUSE_BD ? 1 : 0) );
    current->set_mr( EXCEPT_IPC_GEN_MR_EXCEPTNO, exc_no );
    current->set_mr( EXCEPT_IPC_GEN_MR_ERRORCODE, exc_code );
    current->set_mr( EXCEPT_IPC_GEN_MR_LOCALID, current->get_local_id().get_raw() );

    // For fast path, we need to indicate that we are doing exception ipc
    current->resources.set_exception_ipc( current );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    current->resources.clear_exception_ipc( current );

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

extern "C" word_t sys_clock(void)
{
    return get_current_scheduler()->get_current_time();
}

extern "C" bool mips64_break(mips64_irq_context_t *context)
{
    switch (context->at) {
    /* Debug functions */
#if defined(CONFIG_DEBUG)
    case L4_TRAP_KPUTC:	    putc((char)context->a0);	break;
    case L4_TRAP_KGETC:	    context->v0 = getc(true);	break;
    case L4_TRAP_KGETC_NB:  context->v0 = getc(false);	break;
    case L4_TRAP_KDEBUG: {
	pgent_t * pg;
	pgent_t::pgsize_e pgsize;
	space_t * space = get_current_tcb ()->get_space();
	if (space == NULL)
	    space = get_kernel_space();

	if (EXPECT_TRUE(get_kip()->kdebug_entry == NULL))
	    return false;

	printf( TXT_BRIGHT "--- KD# " );
	if (context->status & ST_KSU)
	    printf( "<User> " );

	if ( (!(context->status & ST_KSU)) ||
	    (space->lookup_mapping((addr_t)context->v0, &pg, &pgsize)) )
	{
	    printf( "%s ", (char*)context->v0 );
	}

	printf( "---\n" TXT_NORMAL );

	get_kip()->kdebug_entry(context);
	break;
	}
#ifdef CONFIG_CPU_MIPS64_SB1
    case L4_TRAP_READ_PERF:
	context->a0 = mips_cpu::get_cp0_perf_control0();
	context->a1 = mips_cpu::get_cp0_perf_counter0();
	context->a2 = mips_cpu::get_cp0_perf_control1();
	context->a3 = mips_cpu::get_cp0_perf_counter1();
	context->t0 = mips_cpu::get_cp0_perf_control2();
	context->t1 = mips_cpu::get_cp0_perf_counter2();
	context->t2 = mips_cpu::get_cp0_perf_control3();
	context->t3 = mips_cpu::get_cp0_perf_counter3();
	break;
    case L4_TRAP_WRITE_PERF:
	switch(context->a0) {
	case 0: mips_cpu::set_cp0_perf_control0(context->a1); break;
	case 1: mips_cpu::set_cp0_perf_counter0(context->a1); break;
	case 2: mips_cpu::set_cp0_perf_control1(context->a1); break;
	case 3: mips_cpu::set_cp0_perf_counter1(context->a1); break;
	case 4: mips_cpu::set_cp0_perf_control2(context->a1); break;
	case 5: mips_cpu::set_cp0_perf_counter2(context->a1); break;
	case 6: mips_cpu::set_cp0_perf_control3(context->a1); break;
	case 7: mips_cpu::set_cp0_perf_counter3(context->a1); break;
	default:
	    return false;
	}
	break;
#endif
#endif
    default:
	return false;
    }
    ASSERT(!(context->cause & CAUSE_BD));

    context->epc += 4;
    return true;	    // Succesfully handled
}

static word_t get_reg(mips64_irq_context_t *context, word_t num)
{
    switch (num)
    {
    case 0: return 0;
    case 1: return context->at; case 2: return context->v0;
    case 3: return context->v1; case 4: return context->a0;
    case 5: return context->a1; case 6: return context->a2;
    case 7: return context->a3; case 8: return context->t0;
    case 9: return context->t1; case 10: return context->t2;
    case 11: return context->t3; case 12: return context->t4;
    case 13: return context->t5; case 14: return context->t6;
    case 15: return context->t7; case 16: return context->s0;
    case 17: return context->s1; case 18: return context->s2;
    case 19: return context->s3; case 20: return context->s4;
    case 21: return context->s5; case 22: return context->s6;
    case 23: return context->s7; case 24: return context->t8;
    case 25: return context->t9; case 28: return context->gp;
    case 29: return context->sp; case 30: return context->s8;
    case 31: return context->ra;
    default:
	   printf("Read k-register\n"); 
    }
    return 0;
}

static void set_reg(mips64_irq_context_t *context, word_t num, word_t val)
{
    switch (num)
    {
    case 0: return;
    case 1: context->at = val; break; case 2: context->v0 = val; break;
    case 3: context->v1 = val; break; case 4: context->a0 = val; break;
    case 5: context->a1 = val; break; case 6: context->a2 = val; break;
    case 7: context->a3 = val; break; case 8: context->t0 = val; break;
    case 9: context->t1 = val; break; case 10: context->t2 = val; break;
    case 11: context->t3 = val; break; case 12: context->t4 = val; break;
    case 13: context->t5 = val; break; case 14: context->t6 = val; break;
    case 15: context->t7 = val; break; case 16: context->s0 = val; break;
    case 17: context->s1 = val; break; case 18: context->s2 = val; break;
    case 19: context->s3 = val; break; case 20: context->s4 = val; break;
    case 21: context->s5 = val; break; case 22: context->s6 = val; break;
    case 23: context->s7 = val; break; case 24: context->t8 = val; break;
    case 25: context->t9 = val; break; case 28: context->gp = val; break;
    case 29: context->sp = val; break; case 30: context->s8 = val; break;
    case 31: context->ra = val; break;
    default:
	printf("Write k-register\n");
    }
}

typedef struct {
    union {
	u32_t raw;
	struct {
	    s32_t offset : 16;
	    u32_t rt : 5;
	    u32_t base : 5;
	    u32_t op : 6;
	} x;
    };
} load_instr;

static struct {
    word_t address;
    word_t space;
} llval;

extern "C" bool mips64_illegal(mips64_irq_context_t *context, word_t* code)
{
    if (context->at == MAGIC_KIP_REQUEST)
    {
	//TRACEF("KernelInterface() at %p\n", context->epc);
        space_t * space = get_current_space ();
    
	context->t0 = (u64_t) space->get_kip_page_area ().get_base ();
	context->t1 = get_kip ()->api_version;
	context->t2 = get_kip ()->api_flags;
	context->t3 = (NULL != get_kip()->kernel_desc_ptr) ?
		    *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr) : 0;
    }
    else
    {
	pgent_t * pg;
	pgent_t::pgsize_e pgsize;

	space_t::access_e access = space_t::read;

	/* Check for emulated instructions */
	load_instr op = *(load_instr *)context->epc;
	*code = *(u32_t *)context->epc;

	switch(op.x.op)    /* Get instruction Op */
	{
	case 0x38:  /* SC   Store Conditional Word	*/
	case 0x3c:  /* SCD  Store Conditional Double	*/
	    access = space_t::write;
	case 0x30:  /* LL   Load Linked Word		*/
	case 0x34:  /* LLD  Load Linked Double		*/
	    break;
	default:
	    return false;
	}

	word_t address = get_reg(context, op.x.base) + op.x.offset;

	space_t * space = get_current_tcb ()->get_space();
	if(space == NULL)
	    space = get_kernel_space();

	// Check if address exists in page table and is writeable if needed
	while (!space->lookup_mapping ((addr_t)address, &pg, &pgsize) ||
	      ((access == space_t::write) && (!pg->is_writable(space, pgsize))))
	{
	    space->handle_pagefault ((addr_t)address, (addr_t)context->epc, access, false);
	}

	addr_t paddr = pg->address (space, pgsize);
	paddr = addr_offset (paddr, address & page_mask (pgsize));

	word_t align = 0x3;
	// Word access is properly aligned?
	switch(op.x.op)    /* Get instruction Op */
	{
	case 0x34:  /* LLD  Load Linked Double		*/
	case 0x3c:  /* SCD  Store Conditional Double	*/
	    align = 0x7;
	    break;
	}
	if (address & align)
	{
	    /* Fake an exception frame */
	    mips64_irq_context_t *old = context;
	    context--;
	    context->cause = (old->cause & ~CAUSE_EXCCODE) | (space_t::write ? (5<<2) : (4<<2));
	    context->epc = old->epc;
	    context->badvaddr = address;
	    mips64_exception(context);
	    context++;
	}

	switch(op.x.op)    /* Get instruction Op */
	{
	case 0x30:  /* LL   Load Linked Word		*/
	    llval.address = (word_t)paddr; llval.space = (word_t)space;
	    set_reg(context, op.x.rt, *(s32_t *)address); break;
	case 0x34:  /* LLD  Load Linked Double		*/
	    llval.address = (word_t)paddr; llval.space = (word_t)space;
	    set_reg(context, op.x.rt, *(word_t *)address); break;
	case 0x38:  /* SC   Store Conditional Word	*/
	    if ((llval.address == (word_t)paddr)&&(llval.space == (word_t)space))
	    {
		*(u32_t *)address = (u32_t)get_reg(context, op.x.rt);
		set_reg(context, op.x.rt, 1);
	    } else
	    {
		set_reg(context, op.x.rt, 0);
	    }
	    break;
	case 0x3c:  /* SCD  Store Conditional Double	*/
	    if ((llval.address == (word_t)paddr)&&(llval.space == (word_t)space))
	    {
		*(word_t *)address = get_reg(context, op.x.rt);
		set_reg(context, op.x.rt, 1);
	    } else
	    {
		set_reg(context, op.x.rt, 0);
	    }
	    break;
	}
    }
    ASSERT(!(context->cause & CAUSE_BD));

    context->epc += 4;
    return true;	    // Succesfully handled
}

extern "C" bool mips64_watch(mips64_irq_context_t *context, word_t *code)
{
    /* Kernel should check if kdb wants this exception */
    return false;
}

extern "C" bool mips64_cpu_unavail(mips64_irq_context_t *context, word_t *code)
{
    /* Check if it is a Floating Point unavailable exception */
    if (CAUSE_CE_NUM(context->cause) == 1)
    {
        tcb_t *current_tcb = get_current_tcb();
	ASSERT(context->status & ST_KSU);

	current_tcb->resources.mips64_fpu_unavail_exception( current_tcb, context );
        return true;
    }

    *code = CAUSE_CE_NUM(context->cause);
    return false;
}

INLINE void halt_user_thread( void )
{
    tcb_t *current = get_current_tcb();

    current->set_state( thread_state_t::halted );
    current->switch_to_idle();
}

extern "C" void mips64_exception(mips64_irq_context_t *context)
{
    char * exception = NULL;
    bool result = false;
    word_t exc_no = CAUSE_EXCCODE_NUM(context->cause);
    word_t exc_code = 0;

    switch (exc_no) {
	case 4:  exception = "Address error (load/execute)";
		 exc_code = context->badvaddr;			break;
	case 5:  exception = "Address error (store)";
		 exc_code = context->badvaddr;			break;
	case 6:	 exception = "Bus error (instruction fetch)";	break;
	case 7:	 exception = "Bus error (data load/store)";	break;
	case 9:  exception = "Breakpoint";
		 result = mips64_break(context);		break;
	case 10: exception = "Illegal instruction";
		 result = mips64_illegal(context, &exc_code);	break;
	case 11: exception = "Coprocessor unavailable";
		 result = mips64_cpu_unavail(context, &exc_code); break;
	case 12: exception = "Arithmetic overflow";		break;
	case 13: exception = "Trap exception";			break;
	case 14: exception = "Virtual coherency exception (instruction)"; break;
	case 15: exception = "Floating point exception";	break;
	case 18: exception = "Coprocessor 2 exception";		break;
	case 22: exception = "MDMX Unusable";			break;
	case 23: exception = "Watchpoint";
		 result = mips64_watch(context, &exc_code);	break;
	case 24: exception = "Machine check";			break;
	case 30: exception = "Cache error in debug mode";	break;
	case 31: exception = "Virtual coherency exception (data)"; break;
	default: break;
    }

    if (result == false) {
	if (!send_exception_ipc(exc_no, exc_code, context)) {

	    if( EXPECT_FALSE(get_kip()->kdebug_entry != NULL) )
	    {
		printf( TXT_BRIGHT "--- KD# Unhandled Exception [");
		if (exception)
		    printf( "%s", exception );
		else
		    printf( "%d", exc_no );
		printf( "] ---\n" TXT_NORMAL );

		get_kip()->kdebug_entry(context);
	    }

	    halt_user_thread();
	}
    }
}

#define SYSCALL_SAVED_REGISTERS (EXCEPT_IPC_SYS_MR_NUM+1)

static bool send_syscall_ipc( mips64_irq_context_t *context )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
    {
	printf( "Unable to deliver user exception: no exception handler.\n" );
	return false;
    }

    TRACEPOINT (EXCEPTION_IPC_SYSCALL,
		printf ("EXCEPTION_IPC_SYSCALL: (%p) IP = %p, v0 = 0x%016lx\n",
			current, (word_t)current->get_user_ip(), context->v0));

    // Save message registers on the stack
    word_t saved_mr[SYSCALL_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, EXCEPT_IPC_SYS_MR_NUM, EXCEPT_IPC_SYS_LABEL);
    current->set_tag( tag );

    // Create the message.
    current->set_mr( EXCEPT_IPC_SYS_MR_V0, context->v0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_V1, context->v1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A0, context->a0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A1, context->a1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A2, context->a2 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A3, context->a3 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A4, context->t0 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A5, context->t1 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A6, context->t2 );
    current->set_mr( EXCEPT_IPC_SYS_MR_A7, context->t3 );

    current->set_mr( EXCEPT_IPC_SYS_MR_IP, (word_t)current->get_user_ip() );
    current->set_mr( EXCEPT_IPC_SYS_MR_SP, (word_t)current->get_user_sp() );
    current->set_mr( EXCEPT_IPC_SYS_MR_FLAGS, (word_t)current->get_user_flags() |
			    (context->cause & CAUSE_BD ? 1 : 0));

    // For fast path, we need to indicate that we are doing exception ipc
    current->resources.set_exception_ipc( current );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    current->resources.clear_exception_ipc( current );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_IP ) );
	current->set_user_sp( (addr_t)current->get_mr( EXCEPT_IPC_SYS_MR_SP ) );
	current->set_user_flags( current->get_mr(EXCEPT_IPC_SYS_MR_FLAGS) );

	context->v0 = current->get_mr( EXCEPT_IPC_SYS_MR_V0 );
	context->v1 = current->get_mr( EXCEPT_IPC_SYS_MR_V1 );
	context->a0 = current->get_mr( EXCEPT_IPC_SYS_MR_A0 );
	context->a1 = current->get_mr( EXCEPT_IPC_SYS_MR_A1 );
	context->a2 = current->get_mr( EXCEPT_IPC_SYS_MR_A2 );
	context->a3 = current->get_mr( EXCEPT_IPC_SYS_MR_A3 );
	context->t0 = current->get_mr( EXCEPT_IPC_SYS_MR_A4 );
	context->t1 = current->get_mr( EXCEPT_IPC_SYS_MR_A5 );
	context->t2 = current->get_mr( EXCEPT_IPC_SYS_MR_A6 );
	context->t3 = current->get_mr( EXCEPT_IPC_SYS_MR_A7 );
    }
    else {
	printf( "Unable to deliver user exception: IPC error.\n" );
    }

    // Clean-up.
    for( int i = 0; i < SYSCALL_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );

    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}

extern "C" void syscall_exception(mips64_irq_context_t *context)
{
    if (!send_syscall_ipc(context))
    {
	if (EXPECT_FALSE(get_kip()->kdebug_entry != NULL))
	{
	    printf( TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, "Unhandled User SYSCALL" );

	    get_kip()->kdebug_entry(context);
	}
	halt_user_thread();
    }
}
