/*********************************************************************
 *                
 * Copyright (C) 2002, 2005,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-powerpc/prepost.cc
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/console.h>

#include INC_ARCH(debug.h)
#include INC_ARCH(frame.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(phys.h)

#include INC_API(tcb.h)

static spinlock_t kdb_lock;
static bool user_io = true;
static bool user_kdb_enter = true;

bool kdb_t::pre() 
{ 
    bool enter_kdb = false;
    kdb_lock.lock();

    tcb_t *current = get_current_tcb();
    except_info_t *frame = (except_info_t *)kdb_param;

    if( EXCEPT_ID(TRACE) == frame->exc_no )
    {
	word_t flags = frame->regs->srr1_flags;
	flags = MSR_CLR( flags, MSR_SE );	// Disable single stepping
	flags = MSR_CLR( flags, MSR_BE );	// Disable branch tracing
	frame->regs->srr1_flags = flags;
	enter_kdb = true;
    }

    else if( EXCEPT_ID(PROGRAM) == frame->exc_no )
    {
	if( (user_kdb_enter || ppc_is_kernel_mode(frame->regs->srr1_flags)) 
		    && (frame->regs->r0 == L4_TRAP_KDEBUG) )
	{
	    // Synchronous request to enter the kernel debugger.
	    // It can be executed from user or kernel code.
	    word_t instr;
	    char *debug_msg;

	    space_t *space = get_current_space();
	    if( EXPECT_FALSE(space == NULL) )
		space = get_kernel_space();

	    instr = space->get_from_user( (addr_t)(frame->regs->srr0_ip + 4) );
	    if( DEBUG_IS_MAGIC(instr) )
		debug_msg = (char *)(frame->regs->srr0_ip + 8);
	    else
		debug_msg = "debug request";
	    printf( ">> %s\n", debug_msg );
	    frame->regs->srr0_ip += 4;
	    enter_kdb = true;
	}

	else if( user_io && (L4_TRAP_KPUTC == frame->regs->r0) )
	{
	    // User request to print a character.
	    putc( (char)frame->regs->r3 );
	    frame->regs->srr0_ip += 4;
	}

	else if( user_io && (L4_TRAP_KGETC == frame->regs->r0) )
	{
	    // User request to get a character, while blocking.
	    frame->regs->r3 = (word_t)getc();
	    frame->regs->srr0_ip += 4;
	}
    }

    else if( EXCEPT_ID(DSI) == frame->exc_no )
    {
	// Data address exception.  Look for typical symptoms of bugs.
	if( frame->dar == 0 )
	{
	    printf( ">> %s data null dereference\n",
		ppc_is_kernel_mode(frame->regs->srr1_flags) ? "kernel":"user" );
	    enter_kdb = true;
	}
    }

    else if( EXCEPT_ID(ISI) == frame->exc_no )
    {
	// Instruction address exception.  Look for typical symptoms of bugs.
	if( 0 == frame->regs->srr0_ip )
	{
	    printf( ">> %s code null dereference\n",
		ppc_is_kernel_mode(frame->regs->srr1_flags) ? "kernel":"user" );
	    enter_kdb = true;
	}
	else if( ppc_is_kernel_mode(frame->regs->srr1_flags) )
	{
	    if( frame->regs->srr0_ip < USER_AREA_END )
	    {
		printf( ">> kernel execution in user area: "
			"0x%08x (lr 0x%08x)\n",
			frame->regs->srr0_ip, frame->regs->lr );
		enter_kdb = true;
	    }
	    else if( (frame->regs->srr0_ip < KERNEL_AREA_START) ||
		     (frame->regs->srr0_ip >= KERNEL_AREA_END) )
	    {
		printf( ">> kernel execution in kernel data area: "
			"0x%08x (lr 0x%08x)\n",
			frame->regs->srr0_ip, frame->regs->lr );
		enter_kdb = true;
	    }
	}
	else if( frame->regs->srr0_ip >= USER_AREA_END )
	{
	    printf( ">> user attempt to execute in kernel area: "
		    "0x%08x (lr 0x%08x)\n",
	    	    frame->regs->srr0_ip, frame->regs->lr );
	    enter_kdb = true;
	}
    }

    else if( 0 == frame->exc_no )
    {
	printf( ">> Unknown processor exception.\n" );
	enter_kdb = true;
    }

    else if( EXCEPT_ID(SYSTEM_RESET) == frame->exc_no )
    {
	printf( ">> Processor reset exception.\n" );
	enter_kdb = true;
    }

    else if( EXCEPT_ID(MACHINE_CHECK) == frame->exc_no )
    {
	printf( ">> Processor machine check exception.\n" );
	enter_kdb = true;
    }

    if( enter_kdb )
    {
	// We are entering the debugger, so print some extra info.
#if defined(CONFIG_SMP)
	printf( "CPU%d, ", get_current_cpu() );
#endif
	printf( "IP: 0x%08x, MSR: 0x%08x",
		frame->regs->srr0_ip, frame->regs->srr1_flags );
	except_regs_t *user_regs = get_user_except_regs(current);
	if( (user_regs->srr0_ip != frame->regs->srr0_ip) ||
	    (user_regs->srr1_flags != frame->regs->srr1_flags) )
	{
	    printf( ", user IP: 0x%08x, user MSR: 0x%08x",
		    user_regs->srr0_ip, user_regs->srr1_flags );
	}
	printf( "\n" );
    }
    else
	kdb_lock.unlock();

    return enter_kdb; 
}

void kdb_t::post() 
{ 
    kdb_lock.unlock();
}

