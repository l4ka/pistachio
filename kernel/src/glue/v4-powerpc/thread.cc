/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/thread.cc
 * Description:	Misc thread stuff.
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
 * $Id: thread.cc,v 1.36 2003/11/01 17:32:18 joshua Exp $
 *
 ***************************************************************************/

#include INC_ARCH(msr.h)
#include INC_ARCH(frame.h)
#include INC_API(tcb.h)
#include INC_GLUE(tracepoints.h)

//#define TRACE_THREAD(x...)	TRACEF(x)
#define TRACE_THREAD(x...)

__attribute__ ((noreturn)) static void notify_trampoline()
{
    tcb_t *tcb;
    notify_frame_t *notify_frame;

    tcb = get_current_tcb();
    ASSERT(tcb);
    ASSERT(tcb == get_sprg_tcb());

    // Locate the notify frame.
    notify_frame = (notify_frame_t *)
	addr_offset( tcb->stack, sizeof(tswitch_frame_t) );

    // Call the notify function.
    notify_frame->func( notify_frame->arg1, notify_frame->arg2 );

    // Restore the stack to its pre-notification position.
    // The value of tcb->stack may be invalid if the notify callback function
    // caused a thread switch.
    tcb->stack = (word_t *) addr_offset( notify_frame, sizeof(notify_frame_t) );

    //  Resume this tcb's original thread of execution prior to the notify.
    //  This code is modeled after the tcb_t switch_to() functions.
    asm volatile (
	    "addi %%r1, %0, 16 ;"	// Install the new stack.
	    "lwz  %%r3, -16(%%r1) ;"	// Grab the old instruction pointer.
	    "lwz  %%r30, -4(%%r1) ;"	// Restore r30.
	    "lwz  %%r31, -8(%%r1) ;"	// Restore r31.
	    "mtctr %%r3 ;"		// Prepare to branch.
	    "bctr ;"			// Branch to the old instr pointer.
	    : /* outputs */
	    : /* inputs */
	      "b" (tcb->stack)
	    );

    enter_kdebug( "notify" );
    while( 1 );
}

void tcb_t::notify( void (*func)(word_t, word_t), word_t arg1, word_t arg2 )
{
    /*  Create the stack frame seen by the notify trampoline.
     *  An old thread switch record will precede this info if the tcb
     *  is already a live thread.
     */
    notify_frame_t *notify_frame = (notify_frame_t *)
	addr_offset( this->stack,  -sizeof(notify_frame_t) );
    this->stack = (word_t *)notify_frame;

    tswitch_frame_t *tswitch_frame = (tswitch_frame_t *)
	addr_offset( this->stack, -sizeof(tswitch_frame_t) );
    this->stack = (word_t *)tswitch_frame;

    notify_frame->func = func;
    notify_frame->arg1 = arg1;
    notify_frame->arg2 = arg2;
    notify_frame->lr_save = 0;
    notify_frame->back_chain = 0;

    tswitch_frame->ip = (word_t)notify_trampoline;
    tswitch_frame->r30 = 0;
    tswitch_frame->r31 = 0;
}

__attribute__ ((noreturn)) static void enter_user_thread( tcb_t *tcb, 
	void (*func)() )
{
    /* Initialize the user thread. */
    func();

    /* Jump to user mode. */
    word_t user_ip = (word_t)tcb->get_user_ip();
    word_t user_sp = (word_t)tcb->get_user_sp();
    word_t user_utcb = tcb->get_local_id().get_raw();

    TRACE_THREAD( "\nenter user thread: tcb %p ip %p, sp %p, utcb %p\n", 
	          tcb, user_ip, user_sp, user_utcb );

    word_t msr = MSR_USER;
    asm volatile (
	    "mr " MKSTR(ABI_LOCAL_ID) ", %3 ; "	// Install the local ID.
	    "mr %%r1, %2 ; "		// Stick the stack in r1.
	    "mtspr 27, %0 ; "		// Stick the MSR in srr1.
	    "mtspr 26, %1 ; "		// Stick the ip in srr0.
	    "rfi ; "
	    : /* no outputs */
	    : "r" (msr), "r" (user_ip), "b" (user_sp), "b" (user_utcb)
	    );

    ASSERT(0);
    spin_forever();
}

void tcb_t::create_startup_stack( void (*func)() )
{
    /* Allocate the space for the user exception frame. */
    syscall_regs_t *regs = get_user_syscall_regs(this);
    this->stack = (word_t *)regs;

    /* Put sentinels in some of the user exception frame. */
    regs->r1_stack = 0x12345678;
    regs->srr0_ip  = 0x87654321;
    regs->lr = 0;

    /* Init the user flags so that the kernel doesn't mistake
     * this thread for an interrupted kernel thread.
     */
    regs->srr1_flags = MSR_USER;

    /* Init the user's local ID, so that an exchange registers
     * on an inactive thread will succeed. */
    ASSERT( this->utcb );
#if (ABI_LOCAL_ID != 2)
# error "Expected register R2 to store the user's local ID."
#endif
    regs->r2_local_id = this->get_local_id().get_raw();

    /* Create the thread switch context record for starting this
     * kernel thread. */
    this->notify( (void (*)(word_t,word_t))enter_user_thread, 
	    (word_t)this, (word_t)func );
}


