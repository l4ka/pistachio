/*********************************************************************
 *                
 * Copyright (C) 2005,  Karlsruhe University
 *                
 * File path:     region.cc
 * Description:   Generic regions
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
 * $Id: region.cc,v 1.3 2005/06/02 14:11:08 joshua Exp $
 *                
 ********************************************************************/
#include <l4/message.h>
#include <l4/kdebug.h>
#include <l4io.h>

#include "sigma0.h"
#include "region.h"


/**
 * List of free region_t structures.
 */
region_list_t region_list;



/* ================================================================
**
**			region_t
**
*/

region_t::region_t (L4_Word_t l, L4_Word_t h, L4_ThreadId_t o)
{
    low = l;
    high = h;
    owner = o;
}

void * region_t::operator new (L4_Size_t size)
{
    return (void *) region_list.alloc ();
}


/**
 * Swap region_t backing store.  The whole region_t structure is
 * copied and pointers for surrounding region structures are relocated
 * to the new location.
 *
 * @param r	location of region_t to use instead of current memory
 */
void region_t::swap (region_t * r)
{
    *r = *this;
    r->prev->next = r->next->prev = r;
}


/**
 * Remove memory region.  Region is first removed from the pool which
 * it is allocated to before its memory is freed.  Region must not be
 * accessed after it has been removed.
 */
void region_t::remove (void)
{
    prev->next = next;
    next->prev = prev;
    region_list.free (this);
}


/**
 * Check if supplied memory region is adjacent to current one.
 * @param r	memory region to check against
 * @return true it memory regions are adjacent, false otherwise
 */
bool region_t::is_adjacent (const region_t & r)
{
    return (low == r.high + 1 && low != 0) ||
	(high + 1 == r.low && r.low != 0);
}


/**
 * Concatenate supplied memory region with current one.
 *
 * @param r	memory region to concatenate with
 *
 * @return true if concatenation was successful, false otherwise
 */
bool region_t::concatenate (region_t * r)
{
    if (owner != r->owner)
	return false;

    if (low == r->high + 1 && low != 0)
	low = r->low;
    else if (high + 1 == r->low && r->low != 0)
	high = r->high;
    else
	return false;

    return true;
}


/**
 * Try allocating part from memory region.  If allocation is
 * successful, current region_t might be deleted or split up into
 * multiple regions.
 *
 * @param log2size	size of region to allocate
 * @param tid		thread id to use for allocation
 * @param make_fpage	function for creating fpage
 *
 * @return fpage for allocated region if successful, nilpage otherwise
 */
L4_Fpage_t region_t::allocate (L4_Word_t log2size, L4_ThreadId_t tid,
			       L4_Fpage_t (*make_fpage) (L4_Word_t, int))
{
    L4_Word_t size = 1UL << log2size;
    L4_Fpage_t ret;

    // Low and high address of region within mwmregion when they are
    // aligned according to log2size.  Note that these values might
    // overflow and must as such be handled with care.
    L4_Word_t low_a = (low + size - 1) & ~(size-1);
    L4_Word_t high_a = ((high + 1) & ~(size-1)) - 1;

    if (low_a > high_a			// Low rounded up to above high
	|| low > low_a			// Low wrapped around
	|| high < size-1		// High wrapper around
	|| (high_a - low_a) < size-1	// Not enough space in region
	|| (owner != tid && owner != L4_anythread))
    {
	// Allocation failed
	ret = L4_Nilpage;
    }
    else if (low_a == low)
    {
	// Allocate from start of region
	ret = make_fpage (low, log2size) + L4_FullyAccessible;
	if (low + size == high + 1)
	    remove ();
	else
	    low += size;
    }
    else if (high_a == high)
    {
	// Allocate from end of region
	ret = make_fpage (high_a - size + 1, log2size) + L4_FullyAccessible;
	high -= size;
    }
    else
    {
	// Allocate from middle of region
	ret = make_fpage (low_a, log2size) + L4_FullyAccessible;
	region_t * r = new region_t (low_a + size, high, owner);
	r->next = next;
	r->prev = this;
	r->next->prev = next = r;
	high = low_a - 1;
    }

    return ret;
}


/**
 * Try allocating part from memory region.  If allocation is
 * successful, current region_t might be deleted or split up into
 * multiple regions.
 *
 * @param addr		location of region to allocate
 * @param log2size	size of region to allocate
 * @param tid		thread id to use for allocation
 * @param make_fpage	function for creating fpage
 *
 * @return fpage for allocated region if successful, nilpage otherwise
 */
L4_Fpage_t region_t::allocate (L4_Word_t addr, L4_Word_t log2size,
			       L4_ThreadId_t tid,
			       L4_Fpage_t (*make_fpage) (L4_Word_t, int))
{
    L4_Word_t size = 1UL << log2size;
    L4_Fpage_t ret;

    // Low and high address of region within mwmregion when they are
    // aligned according to log2size.  Note that these values might
    // overflow and must as such be handled with care.
    L4_Word_t low_a = (low + size - 1) & ~(size-1);
    L4_Word_t high_a = ((high + 1) & ~(size-1)) - 1;

    if (addr < low_a			// Address range below low
	|| (addr + size - 1) > high_a	// Address range above high
	|| (high_a - low_a) < size-1	// Not enough space in region
	|| low > low_a			// Low wrapped around
	|| high < size-1		// High wrapper around
	|| (owner != tid && owner != L4_anythread))
    {
	// Allocation failed
	ret = L4_Nilpage;
    }
    else if (low_a == low && addr == low)
    {
	// Allocate from start of region
	ret = make_fpage (low, log2size) + L4_FullyAccessible;
	if (low + size == high + 1)
	    remove ();
	else
	    low += size;
    }
    else if (high_a == high && (addr + size - 1) == high)
    {
	// Allocate from end of region
	ret = make_fpage (high_a - size + 1, log2size) + L4_FullyAccessible;
	high -= size;
    }
    else
    {
	// Allocate from middle of region
	ret = make_fpage (addr, log2size) + L4_FullyAccessible;
	region_t * r = new region_t (addr + size, high, owner);
	r->next = next;
	r->prev = this;
	r->next->prev = next = r;
	high = addr - 1;
    }

    return ret;
}


/**
 * Check if it is possible to allocate from memory region.
 *
 * @param addr		location of region to allocate
 * @param log2size	size of region to allocate
 * @param tid		thread id to use for allocation
 *
 * @return true if allocation is possible, false otherwise
 */
bool region_t::can_allocate (L4_Word_t addr, L4_Word_t log2size,
				L4_ThreadId_t tid)
{
    L4_Word_t size = 1UL << log2size;

    // Low and high address of region within mwmregion when they are
    // aligned according to log2size.  Note that these values might
    // overflow and must as such be handled with care.
    L4_Word_t low_a = (low + size - 1) & ~(size-1);
    L4_Word_t high_a = ((high + 1) & ~(size-1)) - 1;

    if (addr < low_a			// Address range below low
	|| (addr + size - 1) > high_a	// Address range above high
	|| (high_a - low_a) < size-1	// Not enough space in region
	|| low > low_a			// Low wrapped around
	|| high < size-1		// High wrapper around
	|| (owner != tid && owner != L4_anythread))
	return false;
    else
	return true;
}



/* ================================================================
**
**			region_list_t
**
*/


/**
 * Add more memory to be used for region_t structures.
 *
 * @param addr	location of memory to add
 * @param size	amount of memory to add
 */
void region_list_t::add (L4_Word_t addr, L4_Word_t size)
{
    if (addr == 0)
    {
	// Avoid inserting a NULL pointer into the list.
	addr += sizeof (region_listent_t);
	size -= sizeof (region_listent_t);
    }

    region_listent_t * m = (region_listent_t *) addr;

    for (; (L4_Word_t) (m+1) < addr + size; m++)
	m->set_next (m+1);

    m->set_next (list);
    list = (region_listent_t *) addr;
}


/**
 * @return number of region_t structures in pool
 */
L4_Word_t region_list_t::contents (void)
{
    L4_Word_t n = 0;
    for (region_listent_t * m = list; m != NULL; m = m->next ())
	n++;
    return n;
}


/**
 * Allocate a region_t structure.
 * @return newly allocated structure
 */
region_t * region_list_t::alloc (void)
{
    if (! list)
    {
	// We might need some memory for allocating memory.
	region_t tmp;
	add ((L4_Word_t) &tmp, sizeof (tmp));

	// Allocate some memory to sigma0.
	L4_MapItem_t dummy;
	if (! allocate_page (sigma0_id, min_pgsize, dummy))
	{
	    printf ("s0: Unable to allocate memory.\n");
	    for (;;)
		L4_KDB_Enter ("s0: out of memory");
	}

	bool was_alloced = (list == NULL);
	if (! was_alloced)
	    list = (region_listent_t *) NULL;

	// Add newly allocated memory to pool.
	add (L4_Address (L4_SndFpage (dummy)), (1UL << min_pgsize));

	if (was_alloced)
	    // Swap temorary structure with a newly allocated one.
	    tmp.swap (alloc ());
    }

    // Remove first item from free list.
    region_listent_t * r = list;
    list = r->next ();

    return r->region ();
}


/**
 * Free a region_t structure.
 * @param r	region structure to free
 */
void region_list_t::free (region_t * r)
{
    region_listent_t * e = (region_listent_t *) r;
    e->set_next (list);
    list = e;
}


/* ================================================================
**
**			region_pool_t
**
*/


/**
 * Initialize the memory pool structure.  Must be done prior to any
 * insertions into the pool.
 */
void region_pool_t::init (void)
{
    first.next = first.prev = &last;
    last.next = last.prev = &first;
    first.low = first.high = 0;
    last.low = last.high = ~0UL;
    first.owner = last.owner = L4_nilthread;
    ptr = &last;
}


/**
 * Insert region into region pool.  Concatenate region with existing
 * regions if possible.
 *
 * @param r	region to insert into pool
 */
void region_pool_t::insert (region_t * r)
{
    region_t * p = &first;
    region_t * n = first.next;

    // Find correct insert location
    while (r->low > n->high)
    {
	p = n;
	n = n->next;
    }

    if (p->concatenate (r))
    {
	// Region concatenated previous one
	region_list.free (r);
	if (p->concatenate (n))
	    remove (n);
    }
    else if (n->concatenate (r))
    {
	// Region concatenated to next one
	region_list.free (r);
    }
    else
    {
	// No concatenation possible.  Insert into list
	r->next = n;
	r->prev = p;
	p->next = n->prev = r;
    }
}


/**
 * Remove region from region pool.  It is assumed that the region is
 * indeed contained in the pool.  Region must not be accessed after it
 * has been removed from pool.
 */
void region_pool_t::remove (region_t * r)
{
    r->remove ();
}


/**
 * Insert specified region into memory pool.  Concatenate with
 * existing regions if possible.
 *
 * @param low		lower limit of memory region
 * @param high		upper limit of memory region
 * @param owner		owner of memory region
 */
void region_pool_t::insert (L4_Word_t low, L4_Word_t high,
			       L4_ThreadId_t owner)
{
    insert (new region_t (low, high, owner));
}


/**
 * Remove specified region from memory pool.
 *
 * @param low		lower limit of memory region to remove
 * @param high		upper limit of memory region to remove
 */
void region_pool_t::remove (L4_Word_t low, L4_Word_t high)
{
    region_t * n = first.next;

    while (low > n->high)
	n = n->next;

    while (n != &last)
    {
	if (low <= n->low && high >= n->high)
	{
	    // Remove whole region node.
	    n = n->next;
	    remove (n->prev);
	}
	else if (low <= n->low && high >= n->low)
	{
	    // Only need to modify lower limit.
	    n->low = high + 1;
	    break;
	}
	else if (low > n->low && low <= n->high)
	{
	    // Need to modify upper limit.
	    L4_Word_t old_high = n->high;
	    n->high = low - 1;
	    if (high < old_high)
	    {
		// Must split region into two separate regions.
		insert (high + 1, old_high, n->owner);
		break;
	    }
	    n = n->next;
	}
	else
	    n = n->next;
    }
}


/**
 * Dump contents of memory region pool.
 */
void region_pool_t::dump (void)
{
    region_t * r;
    reset ();
    while ((r = next ()) != NULL)
    {
	printf ("s0:  %p-%p   %p %s\n",
		(void *) r->low, (void *) r->high,
		(void *) r->owner.raw,
		r->owner == sigma0_id ? "(sigma0)" :
		r->owner == sigma1_id ? "(sigma1)" :
		r->owner == rootserver_id ? "(root server)" :
		is_kernel_thread (r->owner) ? "(kernel)" :
		r->owner == L4_anythread ? "(anythread)" :
		"");
    }
}

void region_pool_t::reset (void)
{
    ptr = first.next;
}

region_t * region_pool_t::next (void)
{
    if (ptr == &last)
	return (region_t *) NULL;
    region_t * ret = ptr;
    ptr = ptr->next;
    return ret;
}
