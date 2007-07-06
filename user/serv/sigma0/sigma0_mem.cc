/*********************************************************************
 *                
 * Copyright (C) 2005, 2007,  Karlsruhe University
 *                
 * File path:     sigma0_mem.cc
 * Description:   Sigma0 memory
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
 * $Id: sigma0_mem.cc,v 1.2 2007/06/01 11:45:27 stoess Exp $
 *                
 ********************************************************************/
#include <l4/kip.h>
#include <l4/message.h>
#include <l4/kdebug.h>
#include <l4io.h>

#include "sigma0.h"
#include "region.h"

/**
 * Minimum page size (log2) supported by architecture/kernel
 * (initialized from kernel interface page).
 */
L4_Word_t min_pgsize = 10;


/**
 * Memory regions of conventional memory available for allocation.
 */
static region_pool_t conv_memory_pool;


/**
 * Memory regions of non-conventional memory available for allocation.
 */
static region_pool_t memory_pool;


/**
 * Memory regions already allocated.
 */
static region_pool_t alloc_pool;


/**
 * Helpers for rounding pages up and down according to min_pgsize
 */
L4_INLINE L4_Word_t page_start (L4_Word_t page)
{
    return page & ~((1UL << min_pgsize) - 1);
}

L4_INLINE L4_Word_t page_end (L4_Word_t page)
{
    return (page + ((1UL << min_pgsize) - 1)) & ~((1UL << min_pgsize) - 1);
}



void register_memory (L4_Word_t low, L4_Word_t high, L4_ThreadId_t t)
{
    // Align to smallest page size.
    low = page_start (low);
    high = page_end (high);

    L4_MapItem_t map;
    L4_Word_t addr = low;
    while (addr < high)
    {
	if (! allocate_page (t, addr, min_pgsize, map))
	    dprintf (1, "s0: alloc <%p,%p> to %p failed.\n",
		     (void *) low, (void *) high, (void *) t.raw);
	addr += (1UL << min_pgsize);
    }
}

#if defined(OLD_STYLE_MEMORY_REGIONS)

/*
 * Initialization for old style memory regions.
 */

static void init_mempool_from_kip (void)
{
    // Insert conventional memory.
    conv_memory_pool.insert (kip->MainMem.low, kip->MainMem.high - 1,
			     L4_anythread);

    // Insert rest of memory into non-conventional memory pool.
    if (kip->MainMem.low != 0)
	memory_pool.insert (0, kip->MainMem.low, L4_anythread);
    memory_pool.insert (kip->MainMem.high, ~0UL, L4_anythread);

    dprintf (0, "s0: KIP: %-16s = [%p - %p]\n", "MainMem",
	     (void *) kip->MainMem.low, (void *) kip->MainMem.high);

#define reserve_memory(name, t)						\
    if (kip->name.high)							\
    {									\
	dprintf (0, "s0: KIP: %-16s = [%p - %p]\n", #name,		\
		 (void *) kip->name.low, (void *) kip->name.high);	\
	register_memory (kip->name.low, kip->name.high, t);		\
    }									\
    else 								\
	dprintf (0, "s0: KIP: %-16s = [uninitialized]\n", #name)

#define alloc_memory(name, idx, t)				\
    if (((L4_Word_t *) kip)[idx + 1])				\
    {								\
	L4_Word_t * desc = &((L4_Word_t *) kip)[idx];		\
	dprintf (0, "s0: KIP: %-16s = [%p - %p]\n", #name,	\
		 (void *) desc[0], (void *) desc[1]);		\
	register_memory (desc[0], desc[1], t);			\
    }								\
    else 							\
	dprintf (0, "s0: KIP: %-16s = [uninitialized]\n", #name)

    reserve_memory (DedicatedMem[0], L4_anythread);
    reserve_memory (DedicatedMem[1], L4_anythread);
    reserve_memory (DedicatedMem[2], L4_anythread);
    reserve_memory (DedicatedMem[3], L4_anythread);
    reserve_memory (DedicatedMem[4], L4_anythread);

    reserve_memory (ReservedMem[0], kernel_id);
    reserve_memory (ReservedMem[1], kernel_id);

    alloc_memory (Kdebug, 6, kernel_id);
    alloc_memory (Sigma0, 10, sigma0_id);
    alloc_memory (Sigma1, 14, sigma1_id);
    alloc_memory (RootServer, 18, rootserver_id);
}
#else

/*
 * Inialization for new style memory regions.
 */

static void init_mempool_from_kip (void)
{
    L4_MemoryDesc_t * md;

    // Initialize memory pool with complete physical address space.
    memory_pool.insert (0, ~0UL, L4_anythread);

    // Parse through all memory descriptors in kip.
    for (L4_Word_t n = 0; (md = L4_MemoryDesc (kip, n)); n++)
    {
	if (L4_IsVirtual (md))
	    continue;

	L4_Word_t low = page_start (L4_MemoryDescLow (md));
	L4_Word_t high = page_end (L4_MemoryDescHigh (md)) - 1;

	conv_memory_pool.remove (low, high);
	memory_pool.remove (low, high);
	alloc_pool.remove (low, high);

	if ((L4_MemoryDescType (md) &0xf) == L4_BootLoaderSpecificMemoryType ||
	    (L4_MemoryDescType (md) &0xf) == L4_ArchitectureSpecificMemoryType)
	{
	    /*
	     * Boot loader or architecture dependent memory.  Remove
	     * from conventional memory pool and insert into
	     * non-conventional memory pool.
	     */
	    memory_pool.insert (low, high, L4_anythread);
	}
	else
	{
	    switch (L4_MemoryDescType (md))
	    {
	    case L4_UndefinedMemoryType:
		break;
	    case L4_ConventionalMemoryType:
	    {
		conv_memory_pool.insert (low, high, L4_anythread);
	    }
	    break;
	    case L4_ReservedMemoryType:
	    {
		alloc_pool.insert (low, high, kernel_id);
	    }
	    break;
	    case L4_DedicatedMemoryType:
	    {
		memory_pool.insert (low, high, L4_anythread);
	    }
	    break;
	    case L4_SharedMemoryType:
	    {
		alloc_pool.insert (low, high, L4_anythread);
	    }
	    break;
	    default:
	    { 
		dprintf (0, "s0: Unknown memory type (0x%x)\n",
			 (int) L4_MemoryDescType (md));
	    }
		break;
	    }
	}
    }

#define alloc_memory(name, idx, t)				\
    if (((L4_Word_t *) kip)[idx + 1])				\
    {								\
	L4_Word_t * desc = &((L4_Word_t *) kip)[idx];		\
	register_memory (desc[0], desc[1], t);			\
    }

    // Allocate memory to initial servers.
    alloc_memory (Kdebug, 6, kernel_id);
    alloc_memory (Sigma0, 10, sigma0_id);
    alloc_memory (Sigma1, 14, sigma1_id);
    alloc_memory (RootServer, 18, rootserver_id);
}
#endif


/**
 * Initialze the various memory pools.
 */
void init_mempool (void)
{
    L4_Word_t psmask = L4_PageSizeMask (kip);

    if (psmask == 0)
    {
	printf ("s0: Page-size mask in KIP is empty!\n");
	for (;;)
	    L4_KDB_Enter ("s0: no page-size mask");
    }

    // Determine minimum page size
    for (L4_Word_t m = (1UL << min_pgsize);
	 (m & psmask) == 0;
	 m <<= 1, min_pgsize++)
	;

    // Initialize memory pools
    conv_memory_pool.init ();
    memory_pool.init ();
    alloc_pool.init ();

    init_mempool_from_kip ();
}


/**
 * Dump contents of memory pools.
 */
void dump_mempools (void)
{
    printf ("s0:\ns0: Free pool (conventional memory):\n");
    conv_memory_pool.dump ();
    printf ("s0:\ns0: Free pool (non-conventional memory):\n");
    memory_pool.dump ();
    printf ("s0:\ns0: Alloc pool:\n");
    alloc_pool.dump ();
}


/**
 * Allocate a page located at a specific address.  Address need not be
 * within conventional memory range.
 *
 * @param tid		id of thread doing allocation
 * @param addr		location of page to allocate
 * @param log2size	size of page to allocate
 * @param map		genrated map item corresponding to alloced page
 * @param only_conv	only try to allocation from conventional memory pool
 *
 * @return true upon success, false otherwise
 */
bool allocate_page (L4_ThreadId_t tid, L4_Word_t addr, L4_Word_t log2size,
		    L4_MapItem_t & map, bool only_conventional)
{
    region_t * r;
    L4_Fpage_t fp;

    map = L4_MapItem (L4_Nilpage, 0);

    dprintf (2, "s0: allocate_page (tid: 0x%lx, addr: %lx, log2size: %ld)\n",
	     (long) tid.raw, (long) addr, (long) log2size);

    if (log2size < min_pgsize)
	return false;

    // Make sure that addr is properly aligned.
    addr &= ~((1UL << log2size) - 1);
    L4_Word_t addr_high = addr + (1UL << log2size) - 1;

    region_pool_t * pools[] = { &conv_memory_pool, &memory_pool,
				(region_pool_t *) NULL };

    // Check if we only want to try the conventional memory pool
    if (only_conventional)
	pools[1] = (region_pool_t *) NULL;

    // Try allocating from one of the memory pools.
    for (L4_Word_t i = 0; pools[i] != NULL; i++)
    {
	pools[i]->reset ();
	while ((r = pools[i]->next ()) != NULL)
	{
	    if (r->low > addr_high || r->high < addr)
		continue;

	    fp = r->allocate (addr, log2size, tid, L4_FpageLog2);
	    if (! L4_IsNilFpage (fp))
	    {
		map = L4_MapItem (fp, addr);
		alloc_pool.insert (new region_t (addr, addr_high, tid));
		return true;
	    }
	}
    }

    // Check if memory has already been allocated.
    alloc_pool.reset ();
    while ((r = alloc_pool.next ()) != NULL)
    {
	if (r->can_allocate (addr, log2size, tid))
	{
	    map = L4_MapItem
		(L4_FpageLog2 (addr, log2size) + L4_FullyAccessible, addr);
	    return true;
	}
    }

    // If all the above failed we have to try the slow way of
    // allocating parts of memory from different pools.
    region_pool_t * allpools[] = { &conv_memory_pool,
				   &alloc_pool,
				   &memory_pool,
				   (region_pool_t *) NULL };

    // Check if we only want to try the conventional memory pool
    if (only_conventional)
	allpools[2] = (region_pool_t *) NULL;

    // Loop once for checking followed by once for allocating
    for (L4_Word_t phase = 0; phase < 2; phase++)
    {

	// Use smallest page size as stepping
	for (L4_Word_t a = addr; a < addr_high; a += (1UL << min_pgsize))
	{
	    L4_Word_t a_end = a + (1UL << min_pgsize);
	    bool failed = true;

	    // Try the different pools
	    for (L4_Word_t i = 0; failed && allpools[i] != NULL; i++)
	    {
		allpools[i]->reset ();
		while ((r = allpools[i]->next ()) != NULL)
		{
		    if (r->low > a_end || r->high < a)
			continue;

		    // Test if allocation is possible
		    if (r->can_allocate (a, min_pgsize, tid))
		    {
			failed = false;
			if (phase == 1 && allpools[i] != &alloc_pool)
			{
			    // Allocation phase
			    r->allocate (a, min_pgsize, tid, L4_FpageLog2);
			    alloc_pool.insert 
				(new region_t (a, a_end - 1, tid));
			}
		    }
		}
	    }

	    if (failed)
		return false;
	}
    }

    map = L4_MapItem (L4_FpageLog2 (addr, log2size) + L4_FullyAccessible,
		      addr);

    return true;
}


/**
 * Allocate a page of conventional memory.  Page may be allocated from
 * arbitrary address.
 *
 * @param tid		id of thread doing allocation
 * @param log2size	size of page to allocate
 * @param map		genrated map item corresponding to alloced page
 *
 * @return true upon success, false otherwise
 */
bool allocate_page (L4_ThreadId_t tid, L4_Word_t log2size, L4_MapItem_t & map)
{
    region_t * r;
    L4_Fpage_t fp;

    map = L4_MapItem (L4_Nilpage, 0);

    dprintf (2, "s0: allocate_page (tid: 0x%lx, log2size: %ld)\n",
	     (long) tid.raw, (long) log2size);

    if (log2size < min_pgsize)
	return false;

    // Try allocating memory from pool of real memory.
    conv_memory_pool.reset ();
    while ((r = conv_memory_pool.next ()) != NULL)
    {
	fp = r->allocate (log2size, tid, L4_FpageLog2);
	if (! L4_IsNilFpage (fp))
	{
	    map = L4_MapItem (fp, L4_Address (fp));
	    alloc_pool.insert
		(new region_t (L4_Address (fp), L4_Address (fp) +
			       (1UL << log2size) - 1, tid));
	    return true;
	}
    }

    return false;
}
