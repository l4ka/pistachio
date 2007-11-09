/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/except_handlers.cc
 * Description:	PowerPC exception handlers.
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
 * $Id: except_handlers.cc,v 1.56 2003/12/30 09:00:54 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>

#include INC_ARCH(phys.h)
#include INC_ARCH(except.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(frame.h)

#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(interrupt.h)
#include INC_API(kernelinterface.h)

#include INC_GLUE(exception.h)


DECLARE_TRACEPOINT(except_isi_cnt);
DECLARE_TRACEPOINT(except_dsi_cnt);
DECLARE_TRACEPOINT(except_prog_cnt);
DECLARE_TRACEPOINT(except_decr_cnt);

/*  EXCDEF is a macro which helps consistantly declare exception handlers,
 *  while reducing typing :)
 */
#define EXCDEF(n,params...) extern "C" __attribute__((noreturn)) void except_##n (word_t srr0 , word_t srr1 , except_regs_t *frame , ## params )

/* except_return() short circuits the C code return path.
 * We declare the exception handlers as noreturn, to avoid
 * the C prolog (which redundantly spills registers which the assembler
 * path already spills).
 */
#define except_return()			\
do {					\
    asm volatile (			\
	    "mtlr %0 ;"			\
	    "mr %%r1, %1 ;"		\
	    "blr ;"			\
	    :				\
	    : "r" (__builtin_return_address(0)), \
	      "b" (__builtin_frame_address(1)) \
	    );				\
    while(1);				\
} while(0)



INLINE void halt_user_thread( void )
{
    tcb_t *current = get_current_tcb();

    current->set_state( thread_state_t::halted );
    current->switch_to_idle();
}

static bool send_exception_ipc( word_t exc_no, word_t exc_code )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
    {
	printf( "Unable to deliver user exception: no exception handler.\n" );
	return false;
    }

    // Setup exception IPC.
#define EXC_IPC_SAVED_REGISTERS (GENERIC_EXC_MR_MAX+1)
    word_t saved_mr[EXC_IPC_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < EXC_IPC_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );
    current->set_saved_state( current->get_state() );

    // Create the message tag.
    tag.set( 0, GENERIC_EXC_MR_MAX, GENERIC_EXC_LABEL );
    current->set_tag( tag );

    // Create the message.
    current->set_mr( GENERIC_EXC_MR_UIP,      (word_t)current->get_user_ip() );
    current->set_mr( GENERIC_EXC_MR_USP,      (word_t)current->get_user_sp() );
    current->set_mr( GENERIC_EXC_MR_UFLAGS,   (word_t)current->get_user_flags() );
    current->set_mr( GENERIC_EXC_MR_NO,       exc_no );
    current->set_mr( GENERIC_EXC_MR_CODE,     exc_code );
    current->set_mr( GENERIC_EXC_MR_LOCAL_ID, current->get_local_id().get_raw() );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr(GENERIC_EXC_MR_UIP) );
	current->set_user_sp( (addr_t)current->get_mr(GENERIC_EXC_MR_USP) );
	current->set_user_flags( current->get_mr(GENERIC_EXC_MR_UFLAGS) );
    }
    else
	printf( "Unable to deliver user exception: IPC error.\n" );

    // Clean-up.
    for( int i = 0; i < EXC_IPC_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );
    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );
    current->set_state( current->get_saved_state() );
    current->set_saved_state( thread_state_t::aborted );

    return !tag.is_error();
}

INLINE void try_to_debug( except_regs_t *regs, word_t exc_no, word_t dar=0, word_t dsisr=0 )
{
    if( EXPECT_TRUE(get_kip()->kdebug_entry == NULL) )
	return;

    except_info_t info;
    info.exc_no = exc_no;
    info.regs = regs;
    info.dar = dar;
    info.dsisr = dsisr;

    get_kip()->kdebug_entry( (void *)&info );
}

static void dispatch_exception( except_regs_t *regs, word_t exc_no )
{
    if( get_kip()->kdebug_entry )
    {
	// If the debugger exists, let it have the first try at handling
	// the exception.
	word_t start_ip = regs->srr0_ip;
	word_t start_flags = regs->srr1_flags;

	try_to_debug( regs, exc_no );

	if( (regs->srr0_ip != start_ip) || (regs->srr1_flags != start_flags) )
	    return;	// The kernel debugger handled the exception.
    }

    if( ppc_is_kernel_mode(regs->srr1_flags) )
	panic( "exception in kernel thread.\n" );

    // Try to send the exception to the user's exception handler.
    if( !send_exception_ipc(exc_no, regs->srr1_flags) )
    {
	enter_kdebug( "unhandled user exception, halting thread" );
	halt_user_thread();
    }
}


EXCDEF( dsi_handler, word_t dar, word_t dsisr )
{
    TRACEPOINT(except_dsi_cnt, "except_dsi_cnt");

    // Let the debugger have a first shot at inspecting the data fault.
    try_to_debug( frame, EXCEPT_ID(DSI), dar, dsisr );

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    // Do we have a page hash miss?
    if( EXCEPT_IS_DSI_MISS(dsisr) )
    {

	// Is the page hash miss in the copy area?
	if( EXPECT_FALSE(space->is_copy_area((addr_t)dar)) )
	{
	    // Resolve the fault using the partner's address space!
	    tcb_t *partner = space->get_tcb( tcb->get_partner() );
	    if( partner )
	    {
		addr_t real_fault = tcb->copy_area_real_address( (addr_t)dar );
		if( partner->get_space()->handle_hash_miss(real_fault) )
	    	    except_return();
	    }
	}

	// Normal page hash miss.
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)dar)) )
	    except_return();
    }

    space->handle_pagefault( (addr_t)dar, (addr_t)srr0, 
	    EXCEPT_IS_DSI_WRITE(dsisr) ?  space_t::write : space_t::read,
	    ppc_is_kernel_mode(srr1) );

    except_return();
}

EXCDEF( unknown_handler )
{
    try_to_debug( frame, 0 );
    except_return();
}

EXCDEF( sys_reset_handler )
{
    try_to_debug( frame, EXCEPT_ID(SYSTEM_RESET) );
    except_return();
}

EXCDEF( machine_check_handler )
{
    try_to_debug( frame, EXCEPT_ID(MACHINE_CHECK) );
    except_return();
}

EXCDEF( isi_handler )
{
    TRACEPOINT(except_isi_cnt, "except_isi_cnt");

    // Let the debugger have a first shot at inspecting the instr fault.
    try_to_debug( frame, EXCEPT_ID(ISI) );

    space_t *space = get_current_tcb()->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    if( EXCEPT_IS_ISI_MISS(srr1) ) 
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)srr0)) ) 
	    except_return();

    space->handle_pagefault( (addr_t)srr0, (addr_t)srr0, 
	    space_t::execute, ppc_is_kernel_mode(srr1) );

    except_return();
}

EXCDEF( extern_int_handler )
{
    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) ) {
	srr1 = processor_wake( srr1 );
	frame->srr1_flags = srr1;
    }

    get_interrupt_ctrl()->handle_irq( 0 );

    except_return();
}

EXCDEF( alignment_handler )
{
    dispatch_exception( frame, EXCEPT_ID(ALIGNMENT) );
    except_return();
}

EXCDEF( program_handler )
{
    TRACEPOINT(except_prog_cnt, "except_prog_cnt");

    space_t *space = get_current_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    word_t instr = space->get_from_user( (addr_t)srr0 );
    if( instr == KIP_EXCEPT_INSTR ) {
	frame->r3 = (word_t)space->get_kip_page_area().get_base();
	frame->r4 = get_kip()->api_version;
	frame->r5 = get_kip()->api_flags;
       	frame->r6 = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();
	frame->srr0_ip += 4;
	except_return();
    }

    dispatch_exception( frame, EXCEPT_ID(PROGRAM) );
    except_return();
}

EXCDEF( fp_unavail_handler )
{
    tcb_t *current_tcb = get_current_tcb();
    current_tcb->resources.fpu_unavail_exception( current_tcb );

    except_return();
}

EXCDEF( decrementer_handler )
{
    extern word_t decrementer_interval;

    /* Don't go back to sleep if the thread was in power savings mode.
     * We will only see timer interrupts from user mode, or from the sleep
     * function in kernel mode.
     */
    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) ) {
	srr1 = processor_wake( srr1 );
	frame->srr1_flags = srr1;
    }

    TRACEPOINT(except_decr_cnt, "except_decr_cnt");

    ppc_set_dec( decrementer_interval );
    get_current_scheduler()->handle_timer_interrupt();

    except_return();
}

EXCDEF( trace_handler )
{
    dispatch_exception( frame, EXCEPT_ID(TRACE) );
    except_return();
}

EXCDEF( fp_assist_handler )
{
    dispatch_exception( frame, EXCEPT_ID(FP_ASSIST) );
    except_return();
}


