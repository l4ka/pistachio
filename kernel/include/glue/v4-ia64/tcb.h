/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/tcb.h
 * Description:   TCB/thread related functions for Version 4, IA-64
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
 * $Id: tcb.h,v 1.73 2006/10/20 21:31:45 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__TCB_H__
#define __GLUE__V4_IA64__TCB_H__

#include <debug.h>

#include INC_GLUE(context.h)
#include INC_GLUE(registers.h)
#include INC_GLUE(resource_functions.h)
#include INC_ARCH(runconv.h)
#include INC_API(syscalls.h)

/**
 * Translate arbitrary address to TCB pointer.
 * @param addr		address to translate
 * @return TCB in which ADDR is located
 */
INLINE tcb_t * addr_to_tcb (addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}


/**
 * Locate current TCB by using current stack pointer.
 * @return current TCB
 */
INLINE tcb_t * get_current_tcb (void)
{
    register addr_t sp asm ("sp");
    return addr_to_tcb (sp);
}


#if defined(CONFIG_SMP)
INLINE cpuid_t get_current_cpu (void)
{
    return get_idle_tcb ()->get_cpu ();
}
#endif


/**
 * Perform thread swith from current thread.
 * @param dest		thread to switch to
 */
INLINE void tcb_t::switch_to (tcb_t * dest)
{
    word_t counter;

    if (EXPECT_FALSE (resource_bits.have_resources ()))
	resources.save (this, dest);

    __asm__ __volatile__ ("mov %0 = " MKSTR (r_KERNEL_STACK_COUNTER)
			  :"=r" (counter));

    __asm__ __volatile__ (
	"							\n"
	"r_bsp		= r14					\n"
	"r_ip		= r15					\n"
	"r_rp		= r16					\n"
	"r_cfm		= r17					\n"
	"r_pfs		= r18					\n"
	"r_rnat		= r19					\n"
	"r_unat		= r20					\n"
	"r_pr		= r21					\n"
	"r_psr		= r22					\n"
	"new_stack	= r31					\n"
	"sp1		= r10					\n"
	"sp2		= r11					\n"
	"							\n"
#if 0
	"{.mlx							\n"
	"	break.m	0x3					\n"
	"	movl r0 = 9f ;;					\n"
	"}							\n"
	"	.rodata						\n"
	"9:	stringz \"switch to\"				\n"
	"	.previous					\n"
#endif
	"	// Move context into general registers		\n"
	"	mov 	r_pfs = ar.pfs				\n"
	"	mov 	r_rp = rp				\n"
	"	movl	r_ip = 2f				\n"
	"							\n"
	"	// Make sure that stacked reg is not used	\n"
	"	mov	new_stack = %[dest_stack]		\n"
	"							\n"
	"	// Allocate for switch frame			\n"
	"	add	sp = -%[sizeof_ctx],sp ;;		\n"
	"	;;						\n"
	"	mov	ar.rsc = 0				\n"
	"	add	sp1 = %[offset_pfs], sp			\n"
	"	add	sp2 = %[offset_pfs]+8,sp		\n"
	"	st8	[%[this_stack_ptr]] = sp		\n"
	"							\n"
	"	// Set thread ids and ksp for new thread	\n"
	"	mov	"MKSTR(r_GLOBAL_ID)" = %[dest_gid]	\n"
	"	mov	"MKSTR(r_LOCAL_ID) " = %[dest_lid]	\n"
	"	mov	"MKSTR(r_KERNEL_SP)" = %[dest_stack_top]\n"
	"	mov	"MKSTR(r_PHYS_TCB_ADDR)" = %[dest_tcb_phys]\n"
	"							\n"
	"	// Set region id				\n"
	"	mov	rr[r0] = %[dest_rid]			\n"
	"							\n"
	"	// Make a call so that we can get CFM		\n"
	"	br.call.sptk.many rp = 1f			\n"
	"							\n"
	"	// Store context into switch frame		\n"
	"1:	alloc	r_cfm = ar.pfs, 0, 0, 0, 0		\n"
	"	;;						\n"
	"	mov 	r_bsp = ar.bsp				\n"
	"	flushrs						\n"
	"	;;						\n"
	"							\n"
	"	st8	[sp1] = r_pfs, 16			\n"
	"	st8	[sp2] = r_cfm, 16			\n"
	"	mov	r_unat = ar.unat			\n"
	"	mov	r_rnat = ar.rnat			\n"
	"	mov	r_pr = pr				\n"
	"	mov	r_psr = psr				\n"
	"	;;						\n"
	"	st8	[sp1] = r_ip, 16			\n"
	"	st8	[sp2] = r_bsp, 16			\n"
	"	;;						\n"
	"	st8	[sp1] = r_rnat, 16			\n"
	"	st8	[sp2] = r_unat, 16			\n"
	"	;;						\n"
	"	st8	[sp1] = r_pr				\n"
	"	st8	[sp2] = r_psr, 16			\n"
	"	invala						\n"
	"	loadrs						\n"
	"	;;						\n"
	"	st8	[sp2] = r_rp				\n"
	"							\n"
	"	// Get new switch frame				\n"
	"	add	sp1 = %[offset_pfs], new_stack		\n"
	"	add	sp2 = %[offset_pfs]+8,new_stack		\n"
	"	;;						\n"
	"							\n"
	"	// Load context from new frame			\n"
	"	ld8	r_pfs = [sp1], 16			\n"
	"	ld8	r_cfm = [sp2], 16			\n"
	"	;;						\n"
	"	ld8	r_ip = [sp1], 16			\n"
	"	ld8	r_bsp = [sp2], 16			\n"
	"	mov	ar.pfs = r_cfm				\n"
	"	;;						\n"
	"	ld8	r_rnat = [sp1], 16			\n"
	"	ld8	r_unat = [sp2], 16			\n"
	"	mov	ar.bspstore = r_bsp			\n"
	"	;;						\n"
	"	ld8	r_pr = [sp1]				\n"
	"	ld8	r_psr = [sp2], 16			\n"
	"	mov	ar.rnat = r_rnat			\n"
	"	mov	ar.unat = r_unat			\n"
	"	;;						\n"
	"	ld8	r_rp  = [sp2]				\n"
	"	mov	rp = r_ip				\n"
	"	add	sp = %[sizeof_ctx],new_stack		\n"
	"	mov	pr = r_pr, 0x1ffff			\n"
	"	mov	psr.l = r_psr				\n"
	"	;;						\n"
	"	srlz.d						\n"
	"	;;						\n"
#if 0
	"{.mlx							\n"
	"	break.m	0x3					\n"
	"	movl r0 = 9f ;;					\n"
	"}							\n"
	"	.rodata						\n"
	"9:	stringz \"switched to\"				\n"
	"	.previous					\n"
#endif
	"	br.ret.sptk.many rp				\n"
	"							\n"
	"2:	// Restore non-clobberable registers		\n"
	"	mov	ar.pfs = r_pfs				\n"
	"	mov	rp = r_rp				\n"
	"	mov	ar.rsc = 3				\n"
	"	;;						\n"
	:
	:
	[this_stack_ptr] "r" (&this->stack),
	[dest_stack]	 "r" (dest->stack),
	[sizeof_ctx]	 "i" (sizeof (ia64_switch_context_t)),
	[offset_pfs]	 "i" (offsetof (ia64_switch_context_t, pfs)),
	[dest_gid]	 "r" (dest->get_global_id ().get_raw ()),
	[dest_lid]	 "r" (dest->get_local_id ().get_raw ()),
	[dest_stack_top] "r" (dest->get_stack_top ()),
	[dest_tcb_phys]	 "r" (dest->arch.phys_addr),
	[dest_rid]	 "r" ((dest->space->get_region_id () << 8) + (12 << 2))
	:
	CALLER_SAVED_REGS, CALLEE_SAVED_REGS, "memory");

    __asm__ __volatile__ ("mov " MKSTR (r_KERNEL_STACK_COUNTER) "= %0"
			  ::"r" (counter));

    if (EXPECT_FALSE (resource_bits.have_resources ()))
	resources.load (this);
}


/**
 * Initializes context of initial thread and switches to it.  The
 * context (e.g., instruction pointer) has been generated by inserting
 * a notify procedure context on the stack.  We simply restore this
 * context.
 *
 * @param tcb		TCB of initial thread
 */
INLINE void initial_switch_to (tcb_t * tcb) __attribute__ ((noreturn));
INLINE void initial_switch_to (tcb_t * tcb) 
{
    // Do not invoke notify procedure.  Only invoke restore code.
    __asm __volatile (
	"	mov	"MKSTR(r_KERNEL_SP)" = %[dest_stack_top]	\n"
	"	mov	"MKSTR(r_PHYS_TCB_ADDR)" = %[dest_tcb_phys]	\n"
	"	mov	sp = %[stack]					\n"
	"	br.sptk.many ia64_notify_return_trampoline		\n"
	:
	:
	[stack]		 "r" (tcb->stack),
	[dest_stack_top] "r" (tcb->get_stack_top ()),
	[dest_tcb_phys]	 "r" (tcb->get_arch ()->phys_addr));

    /* NOTREACHED */
}


/**
 * Calculate the stack top (i.e., stack pointer for an empty stack) of
 * current thread.
 *
 * @return stack top of current thread
 */
INLINE word_t * tcb_t::get_stack_top (void)
{
    return &kernel_stack[(KTCB_SIZE - sizeof (tcb_t)) / 
			 sizeof (kernel_stack[0])];
}

/**
 * Calculate bottom of register stack.  Register stack is aligned so
 * that a NaT collection will be stored after 63 register stores.
 *
 * @return bottom of register stack
 */
INLINE word_t * tcb_t::get_reg_stack_bottom (void)
{
    return (word_t *) (((word_t) &kernel_stack + 511) & ~(512 - 1));
}


/**
 * Initialize stack space.
 */
INLINE void tcb_t::init_stack (void)
{
    stack = get_stack_top ();
}


extern word_t ia64_notify_trampoline;

/**
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 *
 * @param func		notify procedure to invoke
 */
INLINE void tcb_t::notify (void (*func)())
{
    bool empty_stack = false;
    if (stack == get_stack_top ())
    {
	// Make room for 2 scratch words (C calling convention)
	stack -= 2;
	empty_stack = true;
    }

    ia64_switch_context_t * old_context = (ia64_switch_context_t *) stack;

    // Create new context switch frame
    stack -= sizeof (ia64_switch_context_t) / sizeof (*stack);
    ia64_switch_context_t * context = (ia64_switch_context_t *) stack;

    // Make sure that we have a valid BSP
    if (empty_stack)
	context->bspstore = &kernel_stack[0];
    else
	context->bspstore = old_context->bspstore;


    // Align BSP so that we start with a new RSE NaT collection
    context->bspstore = (word_t *)
	(((word_t) context->bspstore + 511) & ~(512 - 1));

    // Create a valid context frame
    context->ip = (addr_t) &ia64_notify_trampoline;
    context->rp = *(addr_t *) func;
    context->unat = 0;	// Don't care about NaTs
    context->rnat = 0;
    context->cfm = 0;	// in + loc = 0, out = 0, rot = 0
    context->pfs = 0;
    context->psr.raw = 0;
    context->psr.ic = 1;
    context->psr.it = 1;
    context->psr.dt = 1;
    context->psr.rt = 1;
}


/**
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 *
 * @param func		notify procedure to invoke
 * @param arg1		1st argument to notify procedure
 */
INLINE void tcb_t::notify (void (*func)(word_t), word_t arg1)
{
    notify ((void (*)(void)) func);

    ia64_switch_context_t * context = (ia64_switch_context_t *) stack;

    if ((word_t) context->bspstore & 0x1f8 == 0x1f8)
	*(context->bspstore++) = 0;

    *(context->bspstore++) = arg1; // Push argument on register stack
    context->cfm = 1 + (1 << 7); // in + loc = 1, out = 0, rot = 0
}


/**
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 *
 * @param func		notify procedure to invoke
 * @param arg1		1st argument to notify procedure
 * @param arg2		2nd argument to notify procedure
 */
INLINE void tcb_t::notify (void (*func)(word_t, word_t),
			   word_t arg1, word_t arg2)
{
    notify ((void (*)(word_t)) func, arg1);

    ia64_switch_context_t * context = (ia64_switch_context_t *) stack;

    if ((word_t) context->bspstore & 0x1f8 == 0x1f8)
	*(context->bspstore++) = 0;

    *(context->bspstore++) = arg2; // Push argument on register stack
    context->cfm = 2 + (2 << 7); // in + loc = 1, out = 0, rot = 0
}


INLINE void tcb_t::set_utcb_location (word_t utcb_location)
{
    myself_local.set_raw (utcb_location);
}

INLINE word_t tcb_t::get_utcb_location (void)
{
    return myself_local.get_raw ();
}

INLINE word_t tcb_t::get_user_flags (void)
{
//    UNIMPLEMENTED ();
    return 0;
}

INLINE void tcb_t::set_user_flags (const word_t flags)
{
//    UNIMPLEMENTED ();
}

INLINE word_t tcb_t::get_mr (word_t index)
{
    return get_utcb ()->mr[index];
}

INLINE void tcb_t::set_mr (word_t index, word_t value)
{
    get_utcb ()->mr[index] = value;
}

INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);

    word_t * ws = &get_utcb ()->mr[start];
    word_t * wd = &dest->get_utcb ()->mr[start];
    while (count--)
	*wd++ = *ws++;
}

INLINE word_t tcb_t::get_br (word_t index)
{
    return get_utcb ()->br[index];
}

INLINE void tcb_t::set_br (word_t index, word_t value)
{
    get_utcb ()->br[index] = value;
}

INLINE void tcb_t::set_cpu (cpuid_t cpu) 
{ 
    this->cpu = cpu;
    get_utcb ()->processor_no = cpu;
}


/**
 * tcb_t::allocate: allocate memory for TCB
 *
 * Allocate memory for the given TCB.  We do this by generating a
 * write to the TCB area.  If TCB area is not backed by writable
 * memory (i.e., already allocated) the pagefault handler will
 * allocate the memory and map it.
 */
INLINE void tcb_t::allocate (void)
{
    arch.scratch = 0;
    ia64_mf ();
}

INLINE void tcb_t::set_space (space_t * space)
{
    this->space = space;
}

INLINE addr_t tcb_t::get_user_ip (void)
{
    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;

    return context->iip;
}

INLINE addr_t tcb_t::get_user_sp (void)
{
    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;

    return context->r12;
}

INLINE void tcb_t::set_user_ip (addr_t ip)
{
    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;

    context->iip = ip;
    context->ipsr.ri = 0;
}

INLINE void tcb_t::set_user_sp (addr_t sp)
{
    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;

    context->r12 = sp;
}

INLINE msg_tag_t tcb_t::do_ipc (threadid_t to_tid, threadid_t from_tid,
				timeout_t timeout)
{
    sys_ipc (to_tid, from_tid, timeout);
    return get_tag ();
}

INLINE void tcb_t::return_from_ipc (void)
{
    extern long sys_ipc_return;

    // Calculate location of register stack.
    word_t * regframe = get_reg_stack_bottom () +
	(arch.num_dirty / sizeof (word_t));
    word_t * regframe_top = (regframe + 8);

    // Check for NaT collection.
    if (addr_align (regframe, 512) != addr_align (regframe_top, 512))
	regframe_top++;

    // Calculate stack pointer
    word_t sp = (word_t) get_stack_top ();
    sp -= (sizeof (ia64_switch_context_t) + 16);

    cr_set_tpr (cr_tpr_t::some_enabled (14));
    ia64_srlz_d ();

    // Return to the location where sys_ipc was invoked.  Restore the
    // stack pointers.
    asm volatile (
	"	rsm	psr.i					\n"
	"	;;						\n"
	"	mov	ar.rsc = 0				\n"
	"	mov	"MKSTR(r_KERNEL_STACK_COUNTER)" = 2	\n"
	"	;;						\n"
	"	loadrs						\n"
	"	;;						\n"
	"	mov	ar.pfs = %[pfs]				\n"
	"	mov	ar.bspstore = %[bspstore]		\n"
	"	mov	r9 = %[from]				\n"
	"	mov	rp = %[ipc_return]			\n"
	"	mov	sp = %[sp]				\n"
	"	;;						\n"
	"	ssm	psr.i					\n"
	"	;;						\n"
	"	srlz.d						\n"
	"	;;						\n"
	"	br.ret.sptk.many rp				\n"
	:
	:
	[ipc_return]	"r" (&sys_ipc_return),
	[sp]		"r" (sp),
	[bspstore]	"r" (regframe_top),
	[pfs]		"r" (FRAME_MARKER (0, 3, 7)),
	[from]		"r" (get_partner ().get_raw ()));

    /* NOTREACHED */
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    ia64_exception_context_t * context =
	(ia64_exception_context_t *) get_stack_top () - 1;
    word_t * bspstore = get_reg_stack_bottom () +
	(context->num_dirty / sizeof (word_t));

    // Restore context from the last exception frame and return to
    // user-level.  We must also ensure that the bsp points to the
    // location where the flushed user-level registers reside.
    asm volatile (
	"	flushrs					\n"
	"	mov	ar.rsc = 0			\n"
	"	;;					\n"
	"	mov	ar.bspstore = %[bspstore]	\n"
	"	mov	sp = %[context]			\n"

	"	br.sptk.many load_context		\n"
	:
	:
	[context]	"r" (context),
	[bspstore]	"r" (bspstore));

    /* NOTREACHED */
}

INLINE void tcb_t::adjust_for_copy_area (tcb_t * dst, addr_t * s, addr_t * d)
{
    resources.enable_copy_area (this, dst);
    *d = ia64_phys_to_rr (4, *d);
}

INLINE void tcb_t::release_copy_area (void)
{
    resources.disable_copy_area (this, true);
}

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    return ia64_phys_to_rr (0, addr);
}

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

#endif /* !__GLUE__V4_IA64__TCB_H__ */
