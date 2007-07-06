/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     arch/ia64/trmap.h
 * Description:   Strcutures for keeping track of TR mappings
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
 * $Id: trmap.h,v 1.6 2004/06/04 19:40:05 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__TRMAP_H__
#define __ARCH__IA64__TRMAP_H__

#include INC_ARCH(tlb.h)
#include <debug.h>


/**
 * A list of all present mappings in kernel TRs.
 *
 * @param is_itr	true if map is for ITRs, false otherwise
 * @param num_trs	number of TRs in map
 */
template<bool is_itr, word_t num_trs> class tr_map_t
{
public:
    word_t	vaddr[num_trs];
    word_t	size[num_trs];
    bool	is_free[num_trs];
    translation_t tr[num_trs];

    void init (void)
	{ for (word_t i = 0; i < num_trs; i++) is_free[i] = true; }

    word_t num_entries (void)
	{ return num_trs; }

    bool match (int i, addr_t addr);
    bool is_mapped (addr_t addr);
    int add_map (translation_t tr, addr_t addr,
		 word_t page_size, word_t key, bool do_map = true);
    bool free_map (addr_t addr);

    void put (word_t n, word_t key)
	{
	    if (is_itr)
		tr[n].put_itr (n, (addr_t) vaddr[n], size[n], key);
	    else
		tr[n].put_dtr (n, (addr_t) vaddr[n], size[n], key);
	}

    void dump (void);
};


/**
 * Check if indicated TR is mapped at indicated address.
 *
 * @param i		TR index
 * @param addr		virtual address
 *
 * @return true if TR is mapped, false otherwise
 */
template<bool is_itr, word_t num_trs>
INLINE bool tr_map_t<is_itr, num_trs>::match (int i, addr_t addr)
{
    return (! is_free[i]) &&
	(vaddr[i] >> size[i]) == ((word_t) addr >> size[i]);
}


/**
 * Query whether given address is already backed by a kernel TR.
 *
 * @param addr		virtual address
 *
 * @return true if address is backed by TR, false otherwise
 */
template<bool is_itr, word_t num_trs>
INLINE bool tr_map_t<is_itr, num_trs>::is_mapped (addr_t addr)
{
    for (word_t i = 0; i < num_trs; i++)
	if (match (i, addr))
	    return true;
    return false;
}


/**
 * Allocate a new TR and map it.
 *
 * @param tr		translation entry
 * @param addr		virtual address
 * @param page_size	page size of new mapping (log2)
 * @param key		protection key for new mapping
 * @param do_map	whether to add mapping to TR or not
 *
 * @return number of the allocated TR, or -1 if map was full
 */
template<bool is_itr, word_t num_trs>
INLINE int tr_map_t<is_itr, num_trs>::add_map (translation_t tr, addr_t addr,
					       word_t page_size, word_t key,
					       bool do_map)
{
    for (word_t i = 0; i < num_trs; i++)
    {
	if (! is_free[i])
	    continue;

	vaddr[i] = (word_t) addr;
	size[i] = page_size;
	is_free[i] = false;
	this->tr[i] = tr;

	if (do_map)
	{
	    if (is_itr)
		tr.put_itr (i, addr, page_size, key);
	    else
		tr.put_dtr (i, addr, page_size, key);
	}

	return i;
    }

    return -1;
}


/**
 * Remove indicated mapping from TR map.
 *
 * @param addr		virtual address to free
 *
 * @return true if mapping removed, false otherwise
 */
template<bool is_itr, word_t num_trs>
INLINE bool tr_map_t<is_itr, num_trs>::free_map (addr_t addr)
{
    for (word_t i = 0; i < num_trs; i++)
    {
	if (match(i, addr))
	{
	    is_free[i] = true;
	    if (is_itr)
		purge_itr (addr, size[i]);
	    else
		purge_dtr (addr, size[i]);
	    return true;
	}
    }
    return false;
}


/**
 * Dump TR map.
 */
template<bool is_itr, word_t num_trs>
INLINE void tr_map_t<is_itr, num_trs>::dump (void)
{
    for (word_t i = 0; i < num_trs; i++)
    {
	if (is_free[i])
	    printf ("  %ctr[%d]%s (free)\n",
		    is_itr ? 'i' : 'd', i, i < 10 ? " " : "");
	else
	{
	    word_t sz = size[i];
	    char xB;

	    if (sz >= 30)	sz = (1UL << (sz-30)), xB = 'G';
	    else if (sz >= 20)	sz = (1UL << (sz-20)), xB = 'M';
	    else		sz = (1UL << (sz-10)), xB = 'K';

	    printf ("  %ctr[%d]%s vaddr=%p  paddr=%p  size=%d%cB\n",
		    is_itr ? 'i' : 'd', i, i < 10 ? " " : "",
		    vaddr[i], tr[i].phys_addr (), sz, xB);
	}
    }
}


typedef tr_map_t<true, 8>	itr_map_t;
typedef tr_map_t<false, 16>	dtr_map_t;


/**
 * Mappings for instruction translation registers.
 */
extern itr_map_t itrmap;

/**
 * Mappings for data translation registers.
 */
extern dtr_map_t dtrmap;


#endif /* !__ARCH__IA64__TRMAP_H__ */
