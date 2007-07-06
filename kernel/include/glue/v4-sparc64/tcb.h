/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/tcb.h
 * Description:  TCB related functions for Version 4, Sparc64
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
 * $Id: tcb.h,v 1.25 2006/10/20 21:32:38 reichelt Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__TCB_H__
#define __GLUE__V4_SPARC64__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif

#include INC_ARCH(asm.h)
#include INC_ARCH(types.h)
#include INC_ARCH(frame.h)
#include INC_API(syscalls.h)	/* for sys_ipc */
#include <linear_ptab.h>

#ifndef MKASMSYM /* prevent trying to include asmsyms.h while generating it */
#include <asmsyms.h>
#endif

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    myself_local.set_raw (utcb_location);
}

INLINE word_t tcb_t::get_utcb_location()
{
    return myself_local.get_raw();
}

INLINE void tcb_t::set_cpu(cpuid_t cpu)
{
    this->cpu = cpu;
    get_utcb()->processor_no = cpu;
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    return get_utcb()->mr[index];
}

/**
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    get_utcb()->mr[index] = value;
}


/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);

    for (word_t idx = start; idx < start + count; idx++)
	dest->set_mr(idx, this->get_mr(idx));
}


/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    return get_utcb()->br[index];
}

/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    get_utcb()->br[index] = value;
}


/**
 * allocate the tcb
 * The tcb pointed to by this will be allocated.
 */
INLINE void tcb_t::allocate()
{
    // Ensure that a page is allocated for this tcb, by writing to it. If no
    // page is allocated yet, the page fault handler will allocate one.
    this->kernel_stack[0] = 0;
}


/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    // sometimes it might be desirable to use a pdir cache,
    // like in cases where it's not cheap to derive the page
    // directory from the space
    //this->pdir_cache = (word_t)space->get_pdir();
}

/**
 * Short circuit a return path from an IPC system call.  The error
 * code TCR and message registers are already set properly.  The
 * function only needs to restore the appropriate user context and
 * return execution to the instruction directly following the IPC
 * system call.
 */
INLINE void tcb_t::return_from_ipc (void)
{
    register trap_frame_t* trap_frame asm ("g1") =
	((trap_frame_t*) this->arch.pinned_stack_top) - 1;
    cwp_t cwp = this->arch.tstate.get_cwp();

    asm volatile (
	/* switch to the thread's stack */
	"wrpr	%[trap_cwp], 0, %%cwp\n\t"
	"sub    %[trap_frame], "STR(STACK_BIAS_64BIT)", %%sp\n\t"

	/* jump to return_from_ipc, which will load MRs into local registers,
	 * restore %sp and %o7 and return */
	"ba,pt  %%xcc, return_from_ipc\n\t"
	 "nop"
	::
	[trap_frame] "r" (trap_frame),
	[trap_cwp] "r" (cwp.cwp)
    );
}


/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption (void)
{
    register trap_frame_t* trap_frame asm("g1") =
	((trap_frame_t*) this->arch.pinned_stack_top) - 1;
    cwp_t cwp = this->arch.tstate.get_cwp();

    asm volatile (
	/* make sure we're at TL=1 */
	"wrpr   %%g0, 1, %%tl\n\t"

	/* switch to the thread's stack */
	"wrpr   %[trap_cwp], 0, %%cwp\n\t"
	"sub    %[trap_frame], "STR(STACK_BIAS_64BIT)", %%sp\n\t"

	/* restore in and local registers */
	/* XXX this assumes a flushw was done on context switch */
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L0)" ], %%l0\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L1)" ], %%l1\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L2)" ], %%l2\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L3)" ], %%l3\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L4)" ], %%l4\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L5)" ], %%l5\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L6)" ], %%l6\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_L7)" ], %%l7\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I0)" ], %%i0\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I1)" ], %%i1\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I2)" ], %%i2\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I3)" ], %%i3\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I4)" ], %%i4\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I5)" ], %%i5\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I6)" ], %%fp\n\t"
	"ldx	[ %%sp + "STR(STACK_BIAS_64BIT)" + "STR(WINDOW_FRAME_I7)" ], %%i7\n\t"

	/* jump to otrap_retry which will restore out registers and return */
	"ba,pt  %%xcc, otrap_retry\n\t"
	 "nop\n\t"
	::
	[trap_frame] "r" (trap_frame),
	[trap_cwp] "r" (cwp.cwp)
    );
}


/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

/**
 * switch to initial thread
 * @param tcb TCB of initial thread
 *
 * Initializes context of initial thread and switches to it.  The
 * context (e.g., instruction pointer) has been generated by inserting
 * a notify procedure context on the stack.  We simply restore this
 * context.
 */
INLINE void NORETURN initial_switch_to (tcb_t * tcb)
{
    asm volatile (
	/* Make sure we're at trap level 1 */
	"wrpr   %%g0, 1, %%tl\n\t"

	/* Set the initial register window state */
	"wrpr	%%g0, %%canrestore\n\t"
	"wrpr	%%g0, %%cleanwin\n\t"
	"wrpr	%%g0, %%otherwin\n\t"
	"wrpr	("STR(NWINDOWS)"-2), %%cansave\n\t"

	".register %%g7,kstack\n\t"
	/* Load the new thread's alternate %g1, %g2 and %g7. */
	"rdpr	%%pstate, %%o0\n\t"
	"wrpr	%%o0, "STR(PSTATE_AG)", %%pstate\n\t"
	".register %%g2,#scratch\n\t"
	"mov	%[new_window_save_area], %%g2\n\t"
	"mov	%[new_pinned_stack_top], %%g7\n\t"
	"mov	%[new_saved_windows], %%g1\n\t"
	"wrpr	%%o0, %%pstate\n\t"

	/* Set the new stack and restore registers */
	"sub	%[stack], "STR(STACK_BIAS_64BIT)", %%sp\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O7)" + "STR(STACK_BIAS_64BIT)" ], %%o7\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O0)" + "STR(STACK_BIAS_64BIT)" ], %%o0\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O1)" + "STR(STACK_BIAS_64BIT)" ], %%o1\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O2)" + "STR(STACK_BIAS_64BIT)" ], %%o2\n\t"
	"add	%%sp, "STR(SWITCH_FRAME_SIZE)", %%sp\n\t"

	/* Now jump to sparc64_do_notify */
	"jmp	%%o7\n\t"
	"nop\n\t"
	::
	[stack] "r" (tcb->stack),
	[new_pinned_stack_top] "r" (tcb->arch.pinned_stack_top),
	[new_saved_windows] "r" (tcb->arch.saved_windows),
	[new_window_save_area] "r" (&tcb->get_utcb()->reg_win[0])
	: "l0", "o0"
    );

    ASSERT(!"initial_switch_to should not return");
    while (1);
}

/**
 * switches to another tcb thereby switching address spaces if needed
 * @param dest tcb to switch to
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    //TRACEF("this: %p dest: %p dest->stack: %p\n", this, dest, dest->stack);
    space_t *newspace = dest->get_space();
    space_t *oldspace = get_space();

    if(newspace == NULL) {
	newspace = get_kernel_space();
    }
    if(oldspace == NULL) {
	oldspace = get_kernel_space();
    }

    asm volatile (
	/* Flush the register windows to the stack / UTCB. */
	"flushw\n\t"

	/* Save the trap state */
	"rdpr	%%tstate, %%o2\n\t"
	"stx	%%o2, [ %[from_tcb] + "STR(TCB_TSTATE)" ]\n\t"
	"rdpr	%%tpc, %%o3\n\t"
	"stx	%%o3, [ %[from_tcb] + "STR(TCB_TPC)" ]\n\t"
	"rdpr	%%tnpc, %%o4\n\t"
	"stx	%%o4, [ %[from_tcb] + "STR(TCB_TNPC)" ]\n\t"
	"rdpr	%%pil, %%g1\n\t"
	"stb	%%g1, [ %[from_tcb] + "STR(TCB_PIL)" ]\n\t"
	"rdpr	%%tl, %%o5\n\t"
	"stb	%%o5, [ %[from_tcb] + "STR(TCB_TL)" ]\n\t"

	/* Restore the destination thread's trap state */
	"ldub	[ %[to_tcb] + "STR(TCB_TL)" ], %%o5\n\t"
	"wrpr	%%o5, %%tl\n\t"
	"ldx	[ %[to_tcb] + "STR(TCB_TSTATE)" ], %%o2\n\t"
	"wrpr	%%o2, %%tstate\n\t"
	"ldx	[ %[to_tcb] + "STR(TCB_TPC)" ], %%o3\n\t"
	"wrpr	%%o3, %%tpc\n\t"
	"ldx	[ %[to_tcb] + "STR(TCB_TNPC)" ], %%o4\n\t"
	"wrpr	%%o4, %%tnpc\n\t"
	"ldub	[ %[to_tcb] + "STR(TCB_PIL)" ], %%g1\n\t"
	"wrpr	%%g1, %%pil\n\t"

	/* Save some registers on the stack */
	"sub	%%sp, "STR(SWITCH_FRAME_SIZE)", %%sp\n\t"
	"setx	1f, %%g1, %%o7\n\t"
	"stx	%%o7, [ %%sp + "STR(SWITCH_FRAME_O7)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"stx	%%i6, [ %%sp + "STR(SWITCH_FRAME_I6)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"stx	%%i7, [ %%sp + "STR(SWITCH_FRAME_I7)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"stx	%%o0, [ %%sp + "STR(SWITCH_FRAME_O0)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"stx	%%o1, [ %%sp + "STR(SWITCH_FRAME_O1)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"stx	%%o2, [ %%sp + "STR(SWITCH_FRAME_O2)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"mov	%%y, %%g1\n\t"
	"stx	%%g1, [ %%sp + "STR(SWITCH_FRAME_Y)" + "STR(STACK_BIAS_64BIT)" ]\n\t"
	"rdpr	%%cwp, %%g1\n\t"
	"stb	%%g1, [ %[from_tcb] + "STR(TCB_SAVED_CWP)" ]\n\t"

	/* Save the current thread's stack pointer */
	"add	%%sp, "STR(STACK_BIAS_64BIT)", %%g1\n\t"
	"stx	%%g1, [ %[stack_save] ]\n\t"

	/* Set the new primary context */
	"set	"STR(PRIMARY_CONTEXT)", %%g1\n\t"
	"stxa	%[new_context], [ %%g1 ] "STR(ASI_DMMU)"\n\t"
	/* Note: The secondary context will be kept invalid except in the fast
	 * path.  So we don't need to touch it here. */

	/* Save the old thread's alternate %g1, and load the new thread's
	 * alternate %g1, %g2 and %g7. */
	"rdpr	%%pstate, %%o0\n\t"
	"wrpr	%%o0, "STR(PSTATE_AG)", %%pstate\n\t"
	"stx	%%g1, [ %[old_saved_windows] ]\n\t"
	".register %%g2,#scratch\n\t"
	"mov	%[new_window_save_area], %%g2\n\t"
	".register %%g7,kstack\n\t"
	"mov	%[new_pinned_stack_top], %%g7\n\t"
	"mov	%[new_saved_windows], %%g1\n\t"
	"wrpr	%%o0, %%pstate\n\t"

	/* Store the new thread's local thread ID in main %g7 */
	"mov	%[new_local_id], %%g7\n\t"

	/* Set up the register window state */
	"wrpr       %%g0, %%cleanwin\n\t"

	/* Set the new stack and cwp, and load registers. Note that cwp may
	 * change at this point, so all parameters which might be in windowed
	 * registers become invalid. */
	"sub	%[new_stack], "STR(STACK_BIAS_64BIT)", %%g1\n\t"
	"ldub	[ %[to_tcb] + "STR(TCB_SAVED_CWP)" ], %%l0\n\t"
	"wrpr	%%l0, 0, %%cwp\n\t"
	"mov	%%g1, %%sp\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_Y)" + "STR(STACK_BIAS_64BIT)" ], %%g1\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O7)" + "STR(STACK_BIAS_64BIT)" ], %%o7\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_I6)" + "STR(STACK_BIAS_64BIT)" ], %%i6\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_I7)" + "STR(STACK_BIAS_64BIT)" ], %%i7\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O0)" + "STR(STACK_BIAS_64BIT)" ], %%o0\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O1)" + "STR(STACK_BIAS_64BIT)" ], %%o1\n\t"
	"ldx	[ %%sp + "STR(SWITCH_FRAME_O2)" + "STR(STACK_BIAS_64BIT)" ], %%o2\n\t"
	"mov	%%g1, %%y\n\t"
	"add	%%sp, "STR(SWITCH_FRAME_SIZE)", %%sp\n\t"

	/* jump to the return address */
	"jmp	%%o7\n\t"
	"1:  nop\n\t"
	::
	[from_tcb] "r" (this),
	[to_tcb] "r" (dest),
	[new_stack] "r" (dest->stack),
	[stack_save] "r" (&this->stack),
	[new_context] "r" (newspace->get_context()),
	[new_pinned_stack_top] "r" (dest->arch.pinned_stack_top),
	[new_saved_windows] "r" (dest->arch.saved_windows),
	[old_saved_windows] "r" (&this->arch.saved_windows),
	[new_window_save_area] "r" (&dest->get_utcb()->reg_win[0]),
	[new_local_id] "r" (dest->get_utcb_location())
	: "g1", "g2", "g3", "g4", "g5", "o0", "o1", "o2", "o3",
	  "o4", "o5", "o7", "memory", "cc"
    );

    /* Trash the other registers, to make sure the compiler saves them if
     * necessary. */
    asm volatile ("" ::: 
		  "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
		  "i0", "i1", "i2", "i3", "i4", "i5");
}

/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    /* Find the top of the stack in pinned memory and allocate space for
     * a trap frame on it */
    arch.pinned_stack_top = (word_t)get_stack_top();
    trap_frame_t* frame = (trap_frame_t*)arch.pinned_stack_top - 1;
    stack = (word_t*)frame;

    /* clear the trap frame */
    for (word_t * t = stack; t < (word_t*)arch.pinned_stack_top; t++)
	*t = 0;

    utcb_t* utcb = get_utcb();
    if(utcb != NULL) {
	/* since the trap frame only saves out registers, we must also clean a
	 * UTCB window that can be restored to clear the in and local registers,
	 * and ensure that it is the window that is active when the thread
	 * starts. */
	cwp_t cwp;
	cwp.cwp = 0;
	arch.tstate.set_cwp(cwp);
	arch.saved_windows = 1;
	word_t* w = (word_t*)&utcb->reg_win[0];
	for(int i = 0; i < 16; i++)
	    w[i] = 0;
	
    }

    arch.tl.tl = 1;
}

INLINE word_t * tcb_t::get_stack_top()
{
    /* The stack must be pinned in the TLB to avoid infinite recursion in the
     * page fault handler. So we must look up this ktcb's physical address and
     * translate it into an address in the kernel pinned region. */
    extern word_t _start_text[];
    if((word_t*)this > _start_text) {
	return (word_t*)((char*)this + KTCB_SIZE);
    }

    pgent_t* pg;
    pgent_t::pgsize_e size;
    if(!get_kernel_space()->lookup_mapping(this, &pg, &size)) {
	ASSERT(0);
    }

    addr_t phys_addr = (addr_t)((word_t)pg->address(get_kernel_space(), size) +
				((word_t)this % page_size(size)) + KTCB_SIZE);
    return (word_t*)phys_to_virt(phys_addr);
}

/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)())
{
    ((switch_frame_t *)stack)--;
    ((switch_frame_t *)stack)->o7 = (word_t)sparc64_do_notify;
    ((switch_frame_t *)stack)->o2 = (word_t)func;
}

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)(word_t), word_t arg1)
{
    ((switch_frame_t *)stack)--;
    ((switch_frame_t *)stack)->o7 = (word_t)sparc64_do_notify;
    ((switch_frame_t *)stack)->o2 = (word_t)func;
    ((switch_frame_t *)stack)->o0 = arg1;
}

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 * @param arg2 2nd argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)(word_t, word_t), word_t arg1, word_t arg2)
{
    ((switch_frame_t *)stack)--;
    ((switch_frame_t *)stack)->o7 = (word_t)sparc64_do_notify;
    ((switch_frame_t *)stack)->o2 = (word_t)func;
    ((switch_frame_t *)stack)->o0 = arg1;
    ((switch_frame_t *)stack)->o1 = arg2;
}

/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

/**
 * Enable copy area for current thread.
 *
 * @param dst		destination TCB for IPC copy operation
 * @param s		source address
 * @param d		destination address
 */
INLINE void tcb_t::adjust_for_copy_area (tcb_t * dst, addr_t * s, addr_t * d)
{
    UNIMPLEMENTED ();
}

/**
 * Release copy area(s) for current thread.
 */
INLINE void tcb_t::release_copy_area (void)
{
    // No need for this as long as get_copy_area is unimplemented & unused
    //UNIMPLEMENTED ();
}

/**
 * Retrieve the real address associated with a copy area address.
 *
 * @param addr		address within copy area
 *
 * @return address translated into a regular user-level address
 */
INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    UNIMPLEMENTED ();

    return addr;
}

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

INLINE tcb_t * addr_to_tcb(addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}

/**
 * Locate current TCB by using current stack pointer and return it.
 */
INLINE tcb_t * get_current_tcb (void)
{
    register word_t stack_var asm("sp");

    tcb_t* pinned_tcb = addr_to_tcb((addr_t*)(stack_var + STACK_BIAS_64BIT));

    if(pinned_tcb == get_idle_tcb()) {
	return pinned_tcb;
    } else {
	return get_kernel_space()->get_tcb(pinned_tcb->myself_global);
    }
}


/**
 * invoke an IPC from within the kernel
 *
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc (threadid_t to_tid, threadid_t from_tid,
				timeout_t timeout)
{
    msg_tag_t tag;
    sys_ipc (to_tid, from_tid, timeout);
    tag.raw = get_mr (0);

    return tag;
}

/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level instruction pointer
 * @return	the user-level instruction pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    if(this == get_current_tcb()) {
	tl_t tl, saved_tl;
	tpc_t tpc;

	saved_tl.get();
	tl.tl = 1;
	tl.set();
	tpc.get();
	saved_tl.set();

	return (addr_t)tpc.tpc;
    }

    return (addr_t) arch.tpc.tpc;
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    trap_frame_t * trapframe =
	(trap_frame_t *) arch.pinned_stack_top - 1;

    return (addr_t) trapframe->o6;
}

/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    if(this == get_current_tcb()) {
	tl_t tl, saved_tl;
	saved_tl.get();
	tl.tl = 1;
	tl.set();

	tpc_t tpc;
	tpc.tpc = (word_t)ip;
	tpc.set();

	tnpc_t tnpc;
	tnpc.tnpc = (word_t)ip + 4;
	tnpc.set();

	saved_tl.set();
	return;
    }

    arch.tpc.tpc = (word_t)ip;
    arch.tnpc.tnpc = (word_t)ip + 4;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    trap_frame_t * trapframe =
	(trap_frame_t *) arch.pinned_stack_top - 1;

    /* adjust for stack bias if necessary */
    if(!((word_t)sp & 1)) {
	sp = (addr_t)((word_t)sp - STACK_BIAS_64BIT);
    }

    trapframe->o6 = (word_t)sp;
}


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    if(this == get_current_tcb()) {
	tl_t tl, saved_tl;
	saved_tl.get();
	tl.tl = 1;
	tl.set();

	tstate_t tstate;
	tstate.get();

	saved_tl.set();
	return tstate.get_pstate().raw;
    }

    return arch.tstate.get_pstate().raw;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    pstate_t pstate;

    pstate.raw = (get_user_flags() & (~PSTATE_USER_MASK)) |
	(flags & PSTATE_USER_MASK);

    if(this == get_current_tcb()) {
	tl_t tl, saved_tl;
	saved_tl.get();
	tl.tl = 1;
	tl.set();

	tstate_t tstate;
	tstate.set_pstate(pstate);
	tstate.set();

	saved_tl.set();
	return;
    }

    arch.tstate.set_pstate(pstate);
}

/**********************************************************************
 *
 *                  architecture-specific functions
 *
 **********************************************************************/

/**
 * initialize architecture-dependent root server properties based on
 * values passed via KIP
 * @param space the address space this server will run in   
 * @param ip the initial instruction pointer           
 * @param sp the initial stack pointer
 */
INLINE void tcb_t::arch_init_root_server (space_t * space, word_t ip, word_t sp)
{ 
}

#endif /* !__GLUE__V4_SPARC64__TCB_H__ */
