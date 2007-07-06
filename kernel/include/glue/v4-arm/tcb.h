/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-arm/tcb.h
 * Description:   TCB related functions for Version 4, ARM
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
 * $Id: tcb.h,v 1.40 2006/10/20 21:31:33 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_ARM__TCB_H__
#define __GLUE__V4_ARM__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif

#include INC_API(syscalls.h)		/* for sys_ipc */
#include INC_ARCH(thread.h)
#include INC_ARCH(fass.h)
#include INC_ARCH(fass_inline.h)
#include INC_CPU(cache.h)
#include INC_GLUE(resource_functions.h)


INLINE word_t *tcb_t::get_stack_top(void)
{
    word_t *st = (word_t *) ((char *)this + KTCB_SIZE);

    return st;
}

/**
 * Locate current TCB by using current stack pointer and return it.
 */
INLINE tcb_t * get_current_tcb (void)
{
        register word_t stack_var asm("sp");

        return (tcb_t *) (stack_var & KTCB_MASK);
}

/* FIXME - might save significant # cycles on IPC path with FASS by optimising 
 * this further 
 */
INLINE utcb_t * tcb_t::get_utcb() 
{
#ifdef CONFIG_ENABLE_FASS
    if (!this->space)
	return this->utcb;

    arm_domain_t domain = this->space->get_domain();
    word_t utcb_location = get_utcb_location();
    word_t utcb_section = utcb_location >> 20;

    if ((domain != INVALID_DOMAIN && arm_fass.set_member(domain, utcb_section))) {
        return (utcb_t *)utcb_location;
    } else {
        /* FIXME - this belongs in functions like set_mr etc. - if it is
         * here then the UTCB is being marked dirty too conservatively. However,
         * some of these functions are in generic code...
         */
        SET_BIT_WORD(utcb_dirty, domain);

        //*(volatile word_t *)(this->utcb);

        return this->utcb;
    }

#else
    tcb_t *current_tcb = get_current_tcb();

    if (EXPECT_TRUE(this->space && (current_tcb->space == this->space)))
        return (utcb_t *)get_utcb_location();
    else
        return this->utcb;
#endif
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
    word_t *dest_mr = dest->get_utcb()->mr;
    word_t *src_mr = get_utcb()->mr;

    /* FIXME - optimise for ARM */

    ASSERT(start + count <= IPC_NUM_MR);

    for (word_t idx = start; idx < start + count; idx++)
        dest_mr[idx] = src_mr[idx];
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
    this->kernel_stack[0] = 0;
}


/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
}

/* 
 * Return back to user_land when an IPC is aborted
 * We short circuit the restoration of any saved registers/state
 */
INLINE void tcb_t::return_from_ipc (void)
{
    char * context =
	    (char *) get_stack_top () - ARM_IPC_STACK_SIZE; 
    extern void * ipc_syscall_return;
    word_t ret = ((word_t)&ipc_syscall_return & 0x0fff) | ARM_HIGH_VECTOR_VADDR;

    do {
	__asm__ __volatile__ (
	    "mov	sp,	%0		\n"
	    "mov	pc,	%1		\n"
	    : : "r" (context), "r" (ret)
	);
    } while (1);
}


/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption(void)
{
    arm_irq_context_t * context =
	    (arm_irq_context_t *) get_stack_top () - 1;
    extern void * arm_abort_return;
    word_t ret = ((word_t)&arm_abort_return & 0x0fff) | ARM_HIGH_VECTOR_VADDR;

    do {
	__asm__ __volatile__ (
	    "mov	sp,	%0		\n"
	    "mov	pc,	%1		\n"
	    : : "r" (context), "r" (ret)
	);
    } while (1);
    // NOT REACHED
}


INLINE void tcb_t::set_cpu(unsigned short foo) {}

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
#ifdef CONFIG_ENABLE_FASS
    addr_t new_pt = page_table_to_phys(cpd);

    /* Load CPD with kernel's PD */
    for (int i = 0; i < (1 << ARM_SECTION_BITS); ++i)
        cpd->pt.pdir[i] = get_kernel_space()->pt.pdir[i];

    arm_fass.activate_domain(tcb->get_space());
#else
    addr_t new_pt = tcb->get_space();

    if (new_pt == NULL)
        new_pt = get_kernel_space();

    new_pt = virt_to_phys(new_pt);
#endif

    arm_cache::cache_flush();

    USER_UTCB_REF = tcb->get_utcb_location();

    __asm__ __volatile__ (
        "    mcr     p15, 0, r0, c7, c10, 4  \n" /* drain write buffer */   
        "    mcr     p15, 0, %0, c2, c0, 0   \n" /* install new PT */      
        "    mcr     p15, 0, r0, c8, c7, 0   \n" /* flush TLB */            
        "                                    \n"               
        "    mov     sp,     %1              \n" /* load new stack ptr */   
        "                                    \n"               
        "    ldmfd   sp!,    {r4,r5,r11,lr}  \n" /* load notify context */
        "                                    \n"               
        "    mov     pc,     lr              \n" /* load new PC */  
        : 
        : "r" (new_pt), "r" (tcb->stack)
        : "r0", "memory");

    ASSERT(!"We shouldn't get here!");
    while(true) {}
}

/**
 * switches to another tcb thereby switching address spaces if needed
 * Note: current_tcb may not be in dest->space, be careful
 * @param dest tcb to switch to
 */

INLINE void tcb_t::switch_to(tcb_t * dest)
{
    if (EXPECT_FALSE(resource_bits))
        resources.save(this);

#ifdef CONFIG_ENABLE_FASS
    if (EXPECT_TRUE(dest->get_space() != get_space())) 
        arm_fass.activate_domain(dest->get_space());

    USER_UTCB_REF = dest->get_utcb_location();

    __asm__ __volatile__ (
	"   adr	    r12,    1f			\n"
	"   stmfd   sp!,    {r4, r5, r11, r12}	\n"
	"   str	    sp,	    [%0]		\n"
	"   mov	    sp,	    %1			\n"
	"   ldmfd   sp!,    {r4, r5, r11, pc}	\n"
	"1:					\n"
	:
	: "r" (&stack), "r" (dest->stack)
	: "r12", "memory"
    );
    __asm__ __volatile__ ("" ::: "r0","r1","r2","r3","r6","r7" );
    __asm__ __volatile__ ("" ::: "r8","r9","r10","r11","memory" );

#else
    addr_t new_pt;

    if (EXPECT_FALSE(dest->get_space() != get_space()))
    {
    	new_pt = dest->get_space();

        if (new_pt == NULL) 
            new_pt = get_kernel_space();

        new_pt = virt_to_phys(new_pt);

        arm_cache::cache_flush();
        arm_cache::tlb_flush();
    } else {
	new_pt = NULL;
    }

    __asm__ __volatile__ (
	"   adr	    r12,    1f			\n"
	"   stmfd   sp!,    {r4, r5, r11, r12}	\n"
	"   str	    sp,	    [%0]		\n"
	"   mov	    sp,	    %1			\n"
	"   cmp	    %2,	    #0			\n"
	"   mcrne   p15, 0, %2, c2, c0, 0	\n" /* Set new page table */
	"   str	    %4,	    [%3]		\n" /* set UTCB address */
	"   ldmfd   sp!,    {r4, r5, r11, pc}	\n"
	"1:					\n"
	:
	: "r" (&stack), "r" (dest->stack), "r" (new_pt),
	  "r" (USER_UTCB_REF_PAGE), "r" (dest->get_utcb_location())
	: "r12", "memory"
    );
    __asm__ __volatile__ ("" ::: "r0","r1","r2","r3","r6","r7" );
    __asm__ __volatile__ ("" ::: "r8","r9","r10","r11","memory" );
#endif

    if (EXPECT_FALSE(resource_bits))
        resources.load(this);
}


/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    /* Create space for an exception context */
    arm_irq_context_t *context = (arm_irq_context_t *)get_stack_top() - 1;

    stack = (word_t *)context; /* Update new stack position */

    /* clear whole context */
    for (word_t *t = (word_t *)context; t < get_stack_top(); t++)
        *t = 0;
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
extern "C" void arm_return_from_notify0(void);
INLINE void tcb_t::notify (void (*func)())
{
    arm_switch_stack_t *arm_switch = (arm_switch_stack_t *)stack;
    arm_switch--;
    stack = (word_t*) arm_switch;
    arm_switch->r4 = (word_t)func;
    arm_switch->lr = (word_t)arm_return_from_notify0; 
}


    
/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
extern "C" void arm_return_from_notify1(void);
INLINE void tcb_t::notify (void (*func)(word_t), word_t arg1)
{
    arm_switch_stack_t *arm_switch = (arm_switch_stack_t *)stack;
    arm_switch--;
    stack = (word_t*) arm_switch;

    arm_switch->r4 = (word_t) func;
    arm_switch->r5 = arg1;
    arm_switch->lr = (word_t)arm_return_from_notify1; 
}

extern "C" void arm_return_from_notify2(void);
INLINE void tcb_t::notify (void (*func)(word_t, word_t), word_t arg1, 
        word_t arg2)
{
    arm_switch_stack_t *arm_switch = (arm_switch_stack_t *)stack;
    arm_switch--;
    stack = (word_t*) arm_switch;

    arm_switch->r4 = (word_t) func;
    arm_switch->r5 = arg1;
    arm_switch->r11 = arg2;
    arm_switch->lr = (word_t)arm_return_from_notify2; 
}

/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level instruction pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    arm_irq_context_t * context =
        (arm_irq_context_t *) get_stack_top () - 1;

    return (addr_t) (context)->pc;
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    arm_irq_context_t * context =
        (arm_irq_context_t *) get_stack_top () - 1;

    return (addr_t) (context)->sp;
}

/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    arm_irq_context_t *context = (arm_irq_context_t *)get_stack_top() -1;

    context->pc = (word_t)ip;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    arm_irq_context_t *context = (arm_irq_context_t *)get_stack_top() -1;

    context->sp = (word_t)sp;
}


INLINE word_t tcb_t::get_utcb_location()
{
    return myself_local.get_raw();
}

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    myself_local.set_raw (utcb_location);
}


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    arm_irq_context_t * context =
        (arm_irq_context_t *) get_stack_top () - 1;

    return (word_t) (context)->cpsr & ARM_USER_FLAGS_MASK;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    arm_irq_context_t *context = (arm_irq_context_t *)get_stack_top() -1;

    context->cpsr = (context->cpsr & ~ARM_USER_FLAGS_MASK) |
            ((word_t)flags & ARM_USER_FLAGS_MASK);
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
    //UNIMPLEMENTED();
    *d = resources.enable_copy_area (this, dst, *d);
}

/**
 * Release copy area(s) for current thread.
 */
INLINE void tcb_t::release_copy_area (void)
{
    resources.release_copy_area (this, true);
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
    ASSERT (space->is_copy_area (addr));

    return resources.copy_area_real_address(addr);
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

#ifdef CONFIG_IPC_FASTPATH
    tcb_t *current = get_current_tcb();
    // For fast path, we need to indicate that we are doing ipc from the kernel
    current->resources.set_kernel_ipc( current );
#endif

    sys_ipc (to_tid, from_tid, timeout);
    tag.raw = get_mr (0);

#ifdef CONFIG_IPC_FASTPATH
    current->resources.clear_kernel_ipc( current );
#endif

    return tag;
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


#endif /* !__GLUE__V4_ARM__TCB_H__ */
