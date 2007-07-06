/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *
 * File path:    glue/v4-sparc64/space.h
 * Description:  space_t implmentation for SPARC v9. Includes CPU
 *               specific space.h where space_t is defined and
 *               provides SPARC v9 generic implmenetations of space_t
 *               methods.
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
 * $Id: space.h,v 1.7 2005/06/03 15:54:01 joshua Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__SPACE_H__
#define __GLUE__V4_SPARC64__SPACE_H__

#include INC_GLUE_API_CPU(space.h)

/* space_t: tcb management. */
  
INLINE tcb_t * space_t::get_tcb(threadid_t tid)
{
    return (tcb_t*)((KTCB_AREA_START) + 
		    ((tid.get_threadno() & THREADNO_MASK) << KTCB_BITS));
}

INLINE tcb_t * space_t::get_tcb(void * ptr)
{
    return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}

/* space_t: address ranges. */

/**
 *  space_t::is_user_area()
 *  User spaces get the full 64-bits of the address space that are implemented.
 */
INLINE bool space_t::is_user_area(addr_t addr)
{
    if(this == get_kernel_space()) {
	return false;
    }

#if (SPARC64_VIRTUAL_ADDRESS_BITS < 64)

    return ((addr <  (addr_t)USER_AREA_LOWER_LIMIT) ||
	    (addr >= (addr_t)USER_AREA_UPPER_START));

#else

    return true;

#endif /* (SPARC64_VIRTUAL_ADDRESS_BITS < 64) */

}

/**
 *  space_t::is_tcb_area()
 *  KTCBs are mapped from KTCB_AREA_START to KTCB_AREA_END in the kernel_space
 *  only.
 */
INLINE bool space_t::is_tcb_area(addr_t addr)
{
    if(this == get_kernel_space()) {

	return (addr < (addr_t)KTCB_AREA_END && addr >= (addr_t)KTCB_AREA_START);

    } else {

	return false;
    }
}

/* space_t: copy area related methods. */

/**
 *  space_t::is_copy_area()
 *  Copy areas are not required on SPARC due to load/store alternative space
 *  instructions. Target AS's context is loaded into the secondary context
 *  register.
 */
INLINE bool space_t::is_copy_area(addr_t addr)
{
    return false;
}

/**
 *  space_t::get_copy_limit()
 *  unused, see above.
 */
INLINE word_t space_t::get_copy_limit(addr_t addr, word_t limit)
{
    return (word_t)-1L;
}

/* space_t: kip and utcb handling. */

INLINE fpage_t space_t::get_kip_page_area(void)
{
    return kip_area;
}

INLINE fpage_t space_t::get_utcb_page_area(void)
{
    return utcb_area;
}

/* space_t: reference counting. */

INLINE void space_t::add_tcb(tcb_t * tcb)
{
    thread_count++;
}

INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    ASSERT(thread_count != 0);
    thread_count--;

    return (thread_count == 0);
}

/* space_t: space control. */

// nothing so far

/* space_t: SPARC v9 specific functions. */

INLINE void space_t::begin_update(void)
{
}

INLINE void space_t::end_update(void)
{
}

INLINE pgent_t * space_t::pgent(word_t num, word_t cpu)
{
    return (this->get_pdir())->next(this, pgent_t::size_8k, num);
}

INLINE word_t space_t::readmem_phys(addr_t paddr)
{
    word_t result;
    asm volatile ("ldxa [ %1 ] %2, %0"
		  : "=r" (result)
		  : "r" (paddr), "i" (ASI_PHYS_USE_EC));
    return result;
}


#endif /* !__GLUE__V4_SPARC64__SPACE_H__ */
