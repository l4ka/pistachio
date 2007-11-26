/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/tcb.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__TCB_H__
#define __GLUE__V4_X86__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif

#include INC_API(syscalls.h)			/* for sys_ipc */
#include INC_GLUE(resource_functions.h)		/* for thread_resources_t */
#include INC_GLUE_SA(tcb.h)

/* forward declaration */
tcb_t * get_idle_tcb();

/**********************************************************************
 * 
 *            generic tcb functions
 *
 **********************************************************************/


INLINE void tcb_t::allocate()
{
    __asm__ __volatile__(
	"or $0, %0\n"
	: 
	: "m"(*this));

}

INLINE word_t * tcb_t::get_stack_top()
{
    return (word_t*)addr_offset(this, KTCB_SIZE);
}

INLINE void tcb_t::init_stack()
{
    stack = get_stack_top();
    //TRACE("stack = %p\n", stack);
}


#if !defined(CONFIG_X86_COMPATIBILITY_MODE)

INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    this->pdir_cache = space ? (word_t)space->get_top_pdir_phys(get_cpu()) : NULL;

}

INLINE void tcb_t::set_cpu(cpuid_t cpu) 
{ 
    // update the pdir cache on migration
    if (space && !this->pdir_cache) {
	this->pdir_cache = (word_t)space->get_top_pdir_phys(cpu);
	ASSERT(this->pdir_cache);
    }

    // only update UTCB if there is one
    if (get_utcb())
	get_utcb()->processor_no = cpu;
    
    this->cpu = cpu;
}


/**********************************************************************
 * 
 *            utcb state manipulation
 *
 **********************************************************************/

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    utcb_t * dummy = (utcb_t*)NULL;
    myself_local.set_raw (utcb_location + ((word_t)&dummy->mr[0]));
}

INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t * dummy = (utcb_t*)NULL;
    return myself_local.get_raw() - ((word_t)&dummy->mr[0]);
}


/**
 * tcb_t::get_mr: returns value of message register
 * @index: number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    ASSERT(index < IPC_NUM_MR);
    return get_utcb()->mr[index];
}

/**
 * tcb_t::set_mr: sets the value of a message register
 * @index: number of message register
 * @value: value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    ASSERT(index < IPC_NUM_MR);
    get_utcb()->mr[index] = value;
}



/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    return get_utcb()->br[32U-index];
}


/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    get_utcb()->br[32U-index] = value;
}

#endif /* CONFIG_X86_COMPATIBILITY_MODE */

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
    return (addr_t)get_stack_top()[KSTACK_UIP];
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    return (addr_t)get_stack_top()[KSTACK_USP];
}

/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    return get_stack_top()[KSTACK_UFLAGS];
}

/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    get_stack_top()[KSTACK_UIP] = (word_t)ip;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    get_stack_top()[KSTACK_USP] = (word_t)sp;
}


/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    get_stack_top()[KSTACK_UFLAGS] = (get_user_flags() & (~X86_USER_FLAGMASK)) | (flags & X86_USER_FLAGMASK);
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
 * @param saddr		source address
 * @param daddr		destination address
 */

INLINE void tcb_t::adjust_for_copy_area (tcb_t * dst,
					 addr_t * saddr, addr_t * daddr)
{
    resources.enable_copy_area (this, saddr, dst, daddr);
}

/**
 * Release copy area(s) for current thread.
 */

INLINE void tcb_t::release_copy_area (void)
{
    resources.release_copy_area (this, true);
}

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

#ifdef CONFIG_SMP
extern cpuid_t current_cpu;
INLINE cpuid_t get_current_cpu()
{
    return current_cpu;
}
#endif

INLINE tcb_t * addr_to_tcb (addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}



#endif /* !__GLUE__V4_X86__TCB_H__ */
