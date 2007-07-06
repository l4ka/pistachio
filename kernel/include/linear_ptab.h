/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     linear_ptab.h
 * Description:   Helper functions for generic linear page table
 *                manipulation.
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
 * $Id: linear_ptab.h,v 1.11 2004/04/28 16:37:45 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __LINEAR_PTAB_H__
#define __LINEAR_PTAB_H__

#include INC_ARCH(pgent.h)
#include INC_GLUE(space.h)


/**
 * Array containing the actual page sizes (as bit-shifts) for the
 * various page table numbers.  Array is indexed by page size number
 * (i.e., pgent_t::pgsize_e).  Last entry must be the bit-shift for the
 * complete address space.
 */
extern word_t hw_pgshifts[];


/*
 * Define some operators on the pgsize enum to make code more
 * readable.
 */

INLINE pgent_t::pgsize_e operator-- (pgent_t::pgsize_e & l, int)
{
    pgent_t::pgsize_e ret = l;
    l = (pgent_t::pgsize_e) ((word_t) l - 1);
    return ret;
}

INLINE pgent_t::pgsize_e operator++ (pgent_t::pgsize_e & l, int)
{
    pgent_t::pgsize_e ret = l;
    l = (pgent_t::pgsize_e) ((word_t) l + 1);
    return ret;
}

INLINE pgent_t::pgsize_e operator+ (pgent_t::pgsize_e l, int r)
{
    return (pgent_t::pgsize_e) ((word_t) l + r);
}

INLINE pgent_t::pgsize_e operator- (pgent_t::pgsize_e l, int r)
{
    return (pgent_t::pgsize_e) ((word_t) l - r);
}



/**
 * Get page size in bytes for given page size number.
 *
 * @param pgsize	page size number
 *
 * @return number of bytes for given page size
 */
INLINE word_t page_size (pgent_t::pgsize_e pgsize)
{
    return 1UL << hw_pgshifts[pgsize];
}


/**
 * Get page size in shift length for given page size.
 *
 * @param pgsize	page size number
 *
 * @return page shift for given page size
 */
INLINE word_t page_shift (pgent_t::pgsize_e pgsize)
{
    return hw_pgshifts[pgsize];
}


/**
 * Get page mask for a given page size (i.e., bitmask which mask out
 * least significant bits).
 *
 * @param pgsize	page size number
 *
 * @return address mask for given page size number
 */
INLINE word_t page_mask (pgent_t::pgsize_e pgsize)
{
    return ((1UL << hw_pgshifts[pgsize]) - 1);
}


/**
 * Get number of entries for page table of a given page size.
 *
 * @param pgsize	page size number 
 *
 * @return page size (number of entries) for given page size
 */
INLINE word_t page_table_size (pgent_t::pgsize_e pgsize)
{
    return 1UL << (hw_pgshifts[pgsize+1] - hw_pgshifts[pgsize]);
}


/**
 * Get page table index of virtual address for a particular page size.
 *
 * @param pgsize	page size number
 * @param vaddr		virtual address
 *
 * @return page table index for a table of the given page size
 */
INLINE word_t page_table_index (pgent_t::pgsize_e pgsize, addr_t vaddr)
{
    return ((word_t) vaddr >> hw_pgshifts[pgsize]) & 
	(page_table_size (pgsize) - 1);
}


/**
 * Check if page size is supported by hardware.
 *
 * @param pgsize	page size number
 *
 * @return true if page size is supported by hardware, false otherwise
 */
INLINE bool is_page_size_valid (pgent_t::pgsize_e pgsize)
{
    return (1UL << hw_pgshifts[pgsize]) & HW_VALID_PGSIZES;
}


/**
 * Safely read from memory.  If memory is a user address, the page
 * tables are parsed to find the physical address to read from.  If
 * memory is not a user-address, the memory is accessed directly.
 *
 * @param space		space to read from
 * @param vaddr		virtual memory location to read from
 * @param v		returned value
 *
 * @return true if memory could be read, false otherwise
 */
template<typename T>
INLINE bool readmem (space_t * space, addr_t vaddr, T * v)
{
    if (! space->is_user_area (vaddr))
    {
	// We are not reading user memory.  Just access it directly
	*v = *(T *) vaddr;
	return true;
    }

    word_t w;

    // Check if memory is accessible
    if (! space->readmem (vaddr, &w))
	return false;

#if defined(CONFIG_BIGENDIAN)
    w >>= sizeof (word_t) - sizeof (T);
#endif

    switch (sizeof (T))
    {
    case 1: *v = (T) (w & 0xff); break;
    case 2: *v = (T) (w & 0xffff); break;
    case 4: *v = (T) (w & 0xffffffff); break;
    case 8: *v = (T) (w); break;
    }

    return true;
}


#endif /* !__LINEAR_PTAB_H__ */
