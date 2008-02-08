/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     generic/linear_ptab_walker.cc
 * Description:   Linear page table manipulation
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
 * $Id: linear_ptab_walker.cc,v 1.66 2006/11/17 17:29:31 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GENERIC__LINEAR_PTAB_WALKER_CC__
#define __GENERIC__LINEAR_PTAB_WALKER_CC__

#include INC_ARCH(pgent.h)
#include INC_API(fpage.h)
#include INC_API(tcb.h)
#include INC_API(space.h)

#include <kdb/tracepoints.h>
#include <linear_ptab.h>
#include <mdb.h>

#if !defined(CONFIG_NEW_MDB)
#include <mapping.h>
#else
#include <mdb_mem.h>
#endif


DECLARE_TRACEPOINT (FPAGE_MAP);
DECLARE_TRACEPOINT (FPAGE_OVERMAP);
DECLARE_TRACEPOINT (FPAGE_MAPCTRL);
#if !defined(CONFIG_NEW_MDB)
DECLARE_TRACEPOINT (MDB_MAP);
DECLARE_TRACEPOINT (MDB_UNMAP);
#endif

DECLARE_KMEM_GROUP (kmem_pgtab);

word_t hw_pgshifts[] = HW_PGSHIFTS;

#if defined(CONFIG_NEW_MDB)
#define mapnode_t mdb_node_t
#endif

class mapnode_t;


/*
 * Helper functions.
 */

static inline word_t dbg_pgsize (word_t sz)
{
    return (sz >= GB (1) ? sz >> 30 : sz >= MB (1) ? sz >> 20 : sz >> 10);
}

static inline char dbg_szname (word_t sz)
{
    return (sz >= GB (1) ? 'G' : sz >= MB (1) ? 'M' : 'K');
}


/**
 * Map fpage to another address space.  If mapping is a grant
 * operation the mappings in the current address space are
 * subsequently removed.  The access bits in the send fpage indicate
 * which access rights the mappings in the destination address space
 * should have (access bits in the send fpage are first bitwise and'ed
 * with access bits of existing source mappings).
 *
 * @param snd_fp	fpage to map
 * @param base		send base
 * @param t_space	destination address space
 * @param rcv_fp	receive window in destination space
 * @param grant		is mapping a grant operation
 */
void space_t::map_fpage (fpage_t snd_fp, word_t base,
			 space_t * t_space, fpage_t rcv_fp,
			 bool grant)
{
    word_t offset, f_num, t_num, f_off;
    pgent_t *fpg, *tpg;
    pgent_t::pgsize_e f_size, t_size, pgsize;
    mapnode_t *newmap, *map = NULL;
    addr_t f_addr, t_addr;

    pgent_t * r_fpg[pgent_t::size_max];
    pgent_t * r_tpg[pgent_t::size_max];
    word_t r_fnum[pgent_t::size_max];
    word_t r_tnum[pgent_t::size_max];


    /*
     *        SO, HOW DOES THIS MAPPING STUFF WORK ANYWAYS?
     *
     * The code below that deals with mapping of arbitrary addresses
     * and page sizes from a source address space to a destination
     * address space is probably the single most algorithmically
     * complex part of the kernel.  The following short description
     * might help shed some light on how this works.
     *
     * Due to the recursive nature of multi-level forward mapped page
     * tables, the algorithm does rely on performing recursion.
     * However, since recursion can be preetty deep (12+ levels on an
     * Itanium), doing function based recursion is not an option.
     * Such a scheme would require too much stack space, which would
     * in turn require larger TCBs in order to avoid stack overflows.
     * Instead of performing pure function based recursion, we just
     * keep four arrays and push/pop the necessary variables to/from
     * these arrays when we recurse.  Since we know how many page
     * sizes are supported by the architecture we can limit the length
     * of the arrays during compile time.
     *
     * Once the idea of performing recursion without doing recursive
     * function calls is clear, we can proceed with a step by step
     * explanation of how the mapping algorithm works.
     *
     *  1. Use SND_FP and RCV_FP to decide what the mapped range in
     *     the mapper's and mappee's address space will be.  If the
     *     receiver window is larger, then BASE will indicate what
     *     part of the receive window to use.  If the send window is
     *     larger, then BASE will indicate what part of the send
     *     window to use.
     * 
     *  2. Calculate the hardware page size to map (PGSIZE).  This
     *     page size need not necessarily be a valid one.  It can be
     *     the size of a page directory entry (e.g., 4MB on ia32)
     *     which is not a valid page size.  The algorithm will later
     *     on detect that the page size is not valid, and recurse into
     *     the subtrees to perform the mapping.
     * 
     *  3. Caclulate number of pages of size PGSIZE to map; F_NUM and
     *     T_NUM respectively for the sender and receiver size.  At
     *     the start of the algorithm F_NUM and T_NUM will of course
     *     be equal.
     *
     *  4. F_SIZE and T_SIZE indicates the page sizes that is
     *     currently being operated on on the sender side (from) and
     *     receiver side (to).  They are initialized to the maximum
     *     page size.
     * 
     *  5. Start recursing down into the page table at the sender side
     *     until F_SIZE equals PGSIZE.  If we are unable to recurse
     *     down because there is no valid mapping, no mapping can be
     *     performed and the map algorithm terminates.  At this point
     *     FPG will point to a valid page table entry (or subtree).
     *   
     *  7. Start recursing down into the page table at the receiver
     *     side.  During recursion, two things may happen:
     *  
     *       - T_SIZE is larger than PGSIZE and an invalid page table
     *         entry is encountered.  In this case we create a subtree
     *         and continue recursion.
     *       - T_SIZE is larger than PGSIZE and a valid page table
     *         entry indicates a superpage mapping.  In this case the
     *         mapping fails (and kernel enters the debugger).  Note
     *         that there has been discussion about chaning the
     *         behaviour of the specification so that the larger
     *         mapping is removed before proceeding the recursion.
     *   
     *     At some stage (unless mapping failed), T_SIZE will be equal
     *     to PGSIZE.
     *
     *  8. If current page table entry on the sender size is a
     *     subtree, we recurse further down into page table until we
     *     get to a page table entry that does not branch off into a
     *     subtree.  At a later stage, we will recurse up again and
     *     perform the remaining mappings (if any).
     * 
     *  9. If F_SIZE is smaller than T_SIZE it indicates that we need
     *     to map smaller page sizes.  At this point three things may
     *     happen:
     *  
     *         - TPG points to an invalid page table entry.  We create
     *           a subtree and recurse down into it.  At a later stage
     *           we will recurse up again and perform the remaining
     *           mappings (if any).
     *         - TPG points to a subtree.  We recurse into the subtree
     *           and will at a later stage recurse up again and
     *           perform the remaining mappings (if any).
     *         - TPG points to a valid mapping.  In this case, the
     *           mapping fails (and enters kernel debugger).  As
     *           mentioned above (step 7) there has been talks on
     *           changing the specs so that the existing mapping is
     *           removed instead.
     *  
     * 10. FPG will at this point indicate a valid mapping, and TPG
     *     will indicate a page table entry of same size for holding
     *     the mapping.  At this stage, three things may happen:
     *     
     *         - TPG points to an invalid page table entry.  The
     *           mapping is simply inserted.
     *         - TPG points to a valid mapping.  The existing mapping
     *           is unmapped via and the new mapping is inserted.
     *         - TPG points to a subtree.  We recurse into the subtree
     *           and remove all the mappings.  We also remove all the
                 subtrees.  The new mapping is then inserted.
     * 
     * 11. Decrease T_NUM.  If T_NUM is still non-nil we step to the
     *     next page table entry (receiver side) and jump back to step
     *     8.  If T_NUM is nil and F_SIZE is less than T_SIZE it means
     *     that we need to recurse up again and continue with the next
     *     mapping in step 8.  If neither of the above, we have mapped
     *     the complete FPG and continue on the next step.
     *  
     * 12. Decrease F_NUM.  If F_NUM is still non-nil we step to the
     *     next page table entry (sender side) and jump back to step
     *     9.  If F_NUM is nil and F_SIZE is less than PGSIZE it means
     *     that we need to recurse up again and continue with the next
     *     mapping in step 9.  If neiter of the above, the complete
     *     map operation has completed.
     *
     * Note that the above algorithm is something like a gross
     * oversimplification.  There are a number of other variables
     * (e.g., current source and receive address) that needs to be
     * kept track off, certain regions (e.g., UTCBs) can not be mapped
     * or overwritten, etc.  In addition sigma0 is assumed to have no
     * page table but is rather defined to automatically own all
     * physical memory.
     *
     */


    TRACEPOINT (FPAGE_MAP,"%s_fpage (f_spc=%p  f_fp=%p  t_spc=%p  t_fp=%p)\n",
		grant ? "grant" : "map", this, snd_fp.raw, t_space, rcv_fp.raw);

    /* Since get_size returns 1 for nil fpages, and get_size_log2 cannot
       return a useful value anyway, check for nil fpages at first. */
    if (snd_fp.is_nil_fpage () || rcv_fp.is_nil_fpage ())
	return;

    /*
     * Calculate the actual send and receive address to use.
     */

    if (snd_fp.get_size_log2 () <= rcv_fp.get_size_log2 ())
    {
	f_num = snd_fp.get_size_log2 ();
	base &= base_mask (rcv_fp, f_num);
	f_addr = address (snd_fp, f_num);
	t_addr = addr_offset (addr_mask (address (rcv_fp, f_num),
					 ~base_mask (rcv_fp, f_num)), base);
    }
    else
    {
	f_num = t_num = rcv_fp.get_size_log2 ();
	base &= base_mask (snd_fp, t_num);
	f_addr = addr_offset (addr_mask (address (snd_fp, t_num),
					 ~base_mask (snd_fp, t_num)), base);
	t_addr = address (rcv_fp, t_num);
    }

    if (f_num < hw_pgshifts[0])
    {
	if (f_num != 0)
	    enter_kdebug ("map_fpage(): invalid fpage size");
	return;
    }


    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max; hw_pgshifts[pgsize] > f_num; pgsize--) {}

    f_num = t_num = 1UL << (f_num - hw_pgshifts[pgsize]);
    f_size = t_size = pgent_t::size_max;
    f_off = 0;

    fpg = this->pgent (page_table_index (f_size, f_addr));
    tpg = t_space->pgent (page_table_index (t_size, t_addr));

    space_t::begin_update ();

    while (f_num > 0 || t_num > 0)
    {
	if ((! is_user_area (f_addr) && ! is_sigma0_space (this)) ||
	    (! is_user_area (t_addr)))
	    /* Do not mess with kernel area. */
	    break;

	if (is_sigma0_space (this))
	{
	    /*
	     * When mapping from sigma0 we bypass the page table lookup.
	     */
	    f_size = pgsize;
	}
	else
	{
	    if (! fpg->is_valid (this, f_size))
	    {
		while (t_size < f_size)
		{
		    /* Recurse up. */
		    f_off += page_size (t_size);
		    tpg = r_tpg[t_size];
		    t_num = r_tnum[t_size];
		    t_size++;
		}

		if (t_size == f_size)
		    goto Next_receiver_entry;

		/* t_size > f_size */
		goto Next_sender_entry;
	    }

	    if ((f_size > pgsize) && fpg->is_subtree (this, f_size))
	    {
		/*
		 * We are currently working on too large page sizes.
		 */
		f_size--;
		fpg = fpg->subtree (this, f_size+1)->next
		    (this, f_size, page_table_index (f_size, f_addr));
		continue;
	    }
	    else if (fpg->is_subtree (this, f_size))
	    {
		/*
		 * The mappings in the senders address space are too
		 * small.  We have to map each single entry in the
		 * subtree.
		 */
		f_size--;
		r_fpg[f_size] = fpg->next (this, f_size+1, 1);
		r_fnum[f_size] = f_num - 1;

		fpg = fpg->subtree (this, f_size+1);
		f_num = page_table_size (f_size);
		continue;
	    }
	    else if (f_size > pgsize)
	    {
		/*
		 * We'll be mapping smaller page sizes out of a single
		 * larger page size.
		 */
		f_num = 1;
	    }
	}

	/*
	 * If we get here `fpg' is a valid mapping in the senders
	 * address space.
	 */

	if ((t_size > f_size) || (t_size > pgsize))
	{
	    /*
	     * We are currently working on too large receive pages.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;

	    if (! tpg->is_valid (t_space, t_size+1))
	    {
		/*
		 * Subtree does not exist.  Create one.
		 */
		tpg->make_subtree (t_space, t_size+1, false);
	    }
	    else if (! tpg->is_subtree (t_space, t_size+1))
	    {
		/*
		 * There alredy exists a larger mapping.  Just
		 * continue.  BEWARE: This may cause extension of
		 * access rights to be refused even though they are
		 * perfectly legal.  I.e. if all the mappings in the
		 * subtree of the sender's address space are valid.
		 *
		 * NOTE: There have been discussions about changing
		 * the specifications so that the larger mapping is
		 * removed.
		 */
		printf ("map_fpage(): Larger mapping already exists.\n");
		enter_kdebug ("warning: larger mapping exists");
		t_size++;
		goto Next_receiver_entry;
	    }

	    if (t_size >= pgsize)
	    {
		tpg = tpg->subtree (t_space, t_size+1)->next
		    (t_space, t_size, page_table_index (t_size, t_addr));
		continue;
	    }
	    else
	    {
		tpg = tpg->subtree (t_space, t_size+1);
		t_num = page_table_size (t_size);
	    }

	    /* Adjust destination according to where source is located. */
	    tpg = tpg->next (t_space, t_size,
			     page_table_index (t_size, f_addr));
	    t_num -= page_table_index (t_size, f_addr);
	    t_addr = addr_offset (t_addr, page_table_index (t_size, f_addr) <<
				  hw_pgshifts[t_size]);
	    continue;
	}
	else if (tpg->is_valid (t_space, t_size) &&
		 t_size == f_size && f_size <= pgsize &&

		 /* Make sure that we do not overmap KIP and UTCB area */
		 (! t_space->get_kip_page_area ().is_range_overlapping
		  (t_addr, addr_offset (t_addr, page_size (t_size)))) &&
		 (! t_space->get_utcb_page_area ().is_range_overlapping
		  (t_addr, addr_offset (t_addr, page_size (t_size)))) &&

		 /* Check if we're simply extending access rights */
		 (tpg->is_subtree (t_space, t_size) ||
		  (is_sigma0_space (this) ?
		   (tpg->address (t_space, t_size) != f_addr) :
		   (tpg->address (t_space, t_size) !=
		    fpg->address (this, f_size)))
#if defined(CONFIG_NEW_MDB)
		  || 
		  (tpg->mapnode (t_space, t_size,
				addr_mask (t_addr, ~page_mask (t_size)))->
		   get_parent ()
		   !=
		   fpg->mapnode (this, f_size,
				 addr_mask (f_addr, ~page_mask (f_size))))
#endif
		     ))
	{
	    /*
	     * We are doing overmapping.  Need to remove existing
	     * mapping or subtree from destination space.
	     */

	    TRACEPOINT (FPAGE_OVERMAP,"overmapping: faddr=%p fsz=%d%cB  taddr=%p tsz=%d%cB (%s)\n",
			f_addr, dbg_pgsize (page_size (f_size)), dbg_szname (page_size (f_size)),
			t_addr, dbg_pgsize (page_size (t_size)), dbg_szname (page_size (t_size)),
			tpg->is_subtree (t_space, t_size) ? "subtree" : "single map");

	    word_t num = 1;
	    addr_t vaddr = t_addr;
	    rcv_fp.set ((word_t) vaddr, page_shift (t_size), true, true, true);

	    while (num > 0)
	    {
		if (! tpg->is_valid (t_space, t_size))
		{
		    /* Skip invalid entries. */
		}
		else if (tpg->is_subtree (t_space, t_size))
		{
		    /* We have to flush each single page in the subtree. */
		    t_size--;
		    r_tpg[t_size] = tpg;
		    r_tnum[t_size] = num - 1;

		    tpg = tpg->subtree (t_space, t_size+1);
		    num = page_table_size (t_size);
		    continue;
		}
		else if (t_space->is_mappable (vaddr))
		{
		    mapnode_t * map = tpg->mapnode
			(t_space, t_size,
			 addr_mask (vaddr, ~page_mask (t_size)));

#if !defined(CONFIG_NEW_MDB)
		    TRACEPOINT
			(MDB_UNMAP, "mdb_flush (spc=%p pg=%p map=%p vaddr=%p rwx tsz=%d%cB)  paddr=%p\n",
			 t_space, tpg, map, vaddr, dbg_pgsize (page_size (t_size)), 
			 dbg_szname (page_size (t_size)), tpg->address (t_space, t_size));
#endif

#if defined(CONFIG_NEW_MDB)
		    mdb_mem.flush (map);
#else
		    mdb_flush (map, tpg, t_size, vaddr, pgsize, rcv_fp, true);
#endif
		}

		if (t_size < f_size)
		{
		    /* Skip to next entry. */
		    vaddr = addr_offset (vaddr, page_size (t_size));
		    tpg = tpg->next (t_space, t_size, 1);
		}

		num--;
		if (num == 0 && t_size < f_size)
		{
		    do {
			/* Recurse up and remove subtree. */
			tpg = r_tpg[t_size];
			num = r_tnum[t_size];
			t_size++;
			tpg->remove_subtree (t_space, t_size, false);
			if (t_size < f_size)
			    tpg = tpg->next (t_space, t_size, 1);
		    } while (num == 0 && t_size < f_size);
		}
	    }

	    /* We might have to flush the TLB after removing mappings. */
	    if (space_t::does_tlbflush_pay (rcv_fp.get_size_log2 ()))
		flush_tlb (get_current_space ());

	    /*
	     * We might have invalidated the source mapping during the
	     * unmap operation above.  If so, we have to skip it.
	     */
	    if (! fpg->is_valid (this, f_size))
		goto Next_receiver_entry;
	}
	else if (tpg->is_valid (t_space, t_size) &&
		 tpg->is_subtree (t_space, t_size))
	{
	    /*
	     * Target mappings are of smaller page size.  We have to
	     * map each single entry in the subtree.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;

	    tpg = tpg->subtree (t_space, t_size+1);
	    t_num = page_table_size (t_size);
	}
	else if (! is_page_size_valid (t_size))
	{
	    /*
	     * Pagesize is ok but is not a valid hardware pagesize.
	     * Need to create mappings of smaller size.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;
	    tpg->make_subtree (t_space, t_size+1, false);

	    tpg = tpg->subtree (t_space, t_size+1);
	    t_num = page_table_size (t_size);
	    continue;
	}

	if ((! is_mappable (f_addr) && ! is_sigma0_space (this)) ||
	    (! t_space->is_mappable (t_addr)))
	    goto Next_receiver_entry;

	/*
	 * If we get here `tpg' will be the page table entry that we
	 * are going to change.
	 */

	offset = (word_t) f_addr & page_mask (f_size) & ~page_mask (t_size);
	
	if (tpg->is_valid (t_space, t_size))
	{
	    /*
	     * If a mapping already exists, it might be that we are
	     * just extending the current access rights.
	     */
	    if (is_sigma0_space (this) ?
		(tpg->address (t_space, t_size) != f_addr) :
		(tpg->address (t_space, t_size) !=
		 addr_offset (fpg->address (this, f_size), offset)))
	    {
		addr_t a UNUSED = is_sigma0_space (this) ? f_addr :
		    addr_offset (fpg->address (this, f_size), offset);
		printf ("map_fpage(from=%p  to=%p  base=%p  "
			"sndfp=%p  rcvfp=%p)  paddr %p != %p\n",
			this, t_space, base, snd_fp.raw, rcv_fp.raw,
			tpg->address (t_space, t_size), a);
		enter_kdebug ("map_fpage(): Mapping already exists.");
		goto Next_receiver_entry;
	    }

	    /* Extend access rights. */
	    word_t rights = 0;
    
	    if (snd_fp.is_execute () && fpg->is_executable (this, f_size))
		rights += 1;
	    if (snd_fp.is_write () && fpg->is_writable (this, f_size))
		rights += 2;
	    if (snd_fp.is_read () && fpg->is_readable (this, f_size))
		rights += 4;
	    
	    tpg->update_rights (t_space, t_size, rights);
	}
	else
	{
	    /*
	     * This is where the real work is done.
	     */

	    if  (is_sigma0_space (this))
	    {
		/*
		 * If mapping from sigma0, fpg will not be a valid
		 * page table entry.
		 */

#if !defined(CONFIG_NEW_MDB)
		TRACEPOINT (MDB_MAP, "mdb_map (from {sigma0 pg=%p addr=%p %d%cB} to {"
				    "spc=%p pg=%p addr=%p %d%cB})  paddr=%p\n",
			    fpg, addr_offset (f_addr, offset),
			    dbg_pgsize (page_size (f_size)), dbg_szname (page_size (f_size)),
			    t_space, tpg, t_addr,
			    dbg_pgsize (page_size(t_size)), dbg_szname (page_size(t_size)),
			    addr_offset (f_addr, offset + f_off));
#endif

#if defined(CONFIG_NEW_MDB)
		newmap = mdb_mem.map (sigma0_memnode, tpg, page_shift (t_size),
				      (word_t) f_addr + offset + f_off,
				       snd_fp.get_rwx (), ~0UL);
		newmap->set_misc (mdb_mem_misc (t_space, t_size));
#else
		newmap = mdb_map (sigma0_mapnode, fpg, pgent_t::size_max+1,
				  addr_offset (f_addr, offset + f_off),
				  tpg, t_size, t_space, grant);
#endif

#ifdef CONFIG_HAVE_MEMORY_CONTROL
		if (lookup_mapping (f_addr, &fpg, &f_size))
		    tpg->set_entry (t_space, t_size,
		       		    addr_offset (f_addr, offset + f_off),
				    snd_fp.get_rwx (), 
				    fpg->attributes(this, f_size),
				    false);
		else
#endif
		    tpg->set_entry (t_space, t_size,
				    addr_offset (f_addr, offset + f_off),
				    snd_fp.get_rwx (), 0, false);

		tpg->set_linknode (t_space, t_size, newmap, t_addr);
	    }
	    else
	    {
		map = fpg->mapnode (this, f_size,
				    addr_mask (f_addr, ~page_mask (f_size)));

#if !defined(CONFIG_NEW_MDB)
		TRACEPOINT (MDB_MAP, "mdb_map (node=%p from {spc=%p pg=%p addr=%p %d%cB} to {"
				    "spc=%p pg=%p addr=%p %d%cB})  paddr=%p\n",
				      map, this, fpg, f_addr, 
				      dbg_pgsize (page_size(f_size)), dbg_szname (page_size(f_size)),  
				      t_space, tpg, t_addr,
				      dbg_pgsize (page_size(t_size)), dbg_szname (page_size(t_size)),
				      addr_offset (fpg->address (this, f_size),  offset + f_off));
#endif

#if defined(CONFIG_NEW_MDB)
		mdb_node_t * smap = grant ? map->get_parent () : map;
		newmap = mdb_mem.map (smap, tpg, page_shift (t_size),
				      (word_t) fpg->address (this, f_size) +
				      offset + f_off,
				      snd_fp.get_rwx (), ~0UL);
		newmap->set_misc (mdb_mem_misc (t_space, t_size));
#else
		newmap = mdb_map (map, fpg, f_size,
				  addr_offset (f_addr, offset + f_off),
				  tpg, t_size, t_space, grant);
#endif

		tpg->set_entry
		    (t_space, t_size,
		     addr_offset (fpg->address (this, f_size), offset+f_off),
		     snd_fp.get_rwx (), fpg->attributes (this, f_size),
		     false);
		tpg->set_linknode (t_space, t_size, newmap, t_addr);

#if !defined(CONFIG_NEW_MDB)
		if (grant)
		{
		    /* Grant operation.  Remove mapping from current space. */
		    fpg->clear (this, f_size, false, f_addr);
		    flush_tlbent (get_current_space (), f_addr,
				  page_shift (f_size));
		}
#endif
	    }
	}

    Next_receiver_entry:

	t_addr = addr_offset (t_addr, page_size (t_size));
	t_num--;

	if (t_num > 0)
	{
	    /* Go to next entry */
	    tpg = tpg->next (t_space, t_size, 1);
	    if (t_size < f_size)
	    {
		f_off += page_size (t_size);
		continue;
	    }
	}
	else if (t_size < f_size && f_size <= pgsize)
	{
	    do {
		/* Recurse up */
		f_off += page_size (t_size);
		tpg = r_tpg[t_size];
		t_num = r_tnum[t_size];
		t_size++;
	    } while (t_num == 0 && t_size < pgsize);
	    if (t_size < f_size)
		continue;
	}
	else if (t_size > f_size)
	{
	    /* Skip to next fpg entry.  Happens if tpg is already mapped. */
	    f_addr = addr_offset (f_addr,
				  page_size (t_size) - page_size (f_size));
	    f_num = 1;
	}

    Next_sender_entry:

#if defined(CONFIG_NEW_MDB)
	if (grant && map != NULL)
	{
	    mdb_mem.flush (map);
	    map = NULL;
	}
#endif
	f_addr = addr_offset (f_addr, page_size (f_size));
	f_off = 0;
	f_num--;

	if (f_num > 0)
	{
	    /* Go to next entry */
	    if (! is_sigma0_space (this))
		fpg = fpg->next (this, f_size, 1);
	    continue;
	}
	else if (f_size < pgsize)
	{
	    do {
		/* Recurse up */
		fpg = r_fpg[f_size];
		f_num = r_fnum[f_size];
		f_size++;
	    } while (f_size < pgsize && f_num == 0);

	    /* We may now also need to recurse up in receiver space. */
	    if (t_size < f_size && t_num == 0)
	    {
		do {
		    /* Recurse up */
		    tpg = r_tpg[t_size];
		    t_num = r_tnum[t_size];
		    t_size++;
		} while (t_num == 0 && t_size < pgsize);
	    }
	}
	else
	{
	    /* Finished */
	    t_num = 0;
	}
    }

    space_t::end_update ();
}


/**
 * Unmaps, flushes, or otherwise operate on indicated fpage.  The CTRL
 * parameter specifies the exact operation to perform; unmapping,
 * reading access bits, setting page attributes, etc.  The UNMAP_ALL
 * parameter if true indicates that kernel mappings should also be
 * removed from the address space.
 *
 * @param fpage		fpage to operate on
 * @param ctrl		specifies operation to perform
 * @param attribute	new page attribute
 * @param unmap_all	also unmap kernel mappings (i.e., UTCB and KIP)
 *
 * @returns the original fpage (with access bits set if specified)
 */
fpage_t space_t::mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
			  word_t attribute, bool unmap_all)
{
    pgent_t::pgsize_e size, pgsize;
    pgent_t * pg;
    mapnode_t * map;
    addr_t vaddr;
    word_t num, rwx = 0;

    pgent_t *r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];

    TRACEPOINT (FPAGE_MAPCTRL,
		"<spc=%p>::mapctrl ([%x, %d %x] [%x/%s], %x, %c)\n",
		this, fpage.get_address (), 
		fpage.get_size_log2 (), fpage.get_rwx (),
		ctrl.raw, ctrl.string(), attribute,
		unmap_all ? 't' : 'f');
    
#if !defined(CONFIG_NEW_MDB)
    // Fpage access bits semantics are reversed for old MDB
    fpage.set_rwx (~fpage.get_rwx ());
#endif

    num = fpage.get_size_log2 ();
    vaddr = address (fpage, num);

    if (num < hw_pgshifts[0])
    {
	WARNING ("mapctrl_fpage(): too small fpage size (%d) - fixed\n", num);
	num = hw_pgshifts[0];
    }

    /*
     * Some architectures may not support a complete virtual address
     * space.  Enforce unmaps to only cover the supported space.
     */

    if (num > hw_pgshifts[pgent_t::size_max+1])
	num = hw_pgshifts[pgent_t::size_max+1];

    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max; hw_pgshifts[pgsize] > num; pgsize--) {}

    num = 1UL << (num - hw_pgshifts[pgsize]);
    size = pgent_t::size_max;
    pg = this->pgent (page_table_index (size, vaddr));

    space_t::begin_update ();

    while (num)
    {
	if (! is_user_area (vaddr))
	    /* Do not mess with kernel area. */
	    break;

	if (size > pgsize)
	{
	    /* We are operating on too large page sizes. */
	    if (! pg->is_valid (this, size))
		break;
	    else if (pg->is_subtree (this, size))
	    {
		size--;
		pg = pg->subtree (this, size+1)->next
		    (this, size, page_table_index (size, vaddr));
		continue;
	    }
	    else if (ctrl.mapctrl_self)
	    {
		/* Fpage too small -- expand to current size */
		num = 1;
		pgsize = size;
	    }
	}

	if (! pg->is_valid (this, size))
	    goto Next_entry;

	if (pg->is_subtree (this, size))
	{
	    /* We have to flush each single page in the subtree. */
	    size--;
	    r_pg[size] = pg;
	    r_num[size] = num - 1;

	    pg = pg->subtree (this, size+1);
	    num = page_table_size (size);
	    continue;
	}

	/* Only unmap from mapping database if user-mapping. */
	if (is_mappable (vaddr))
	{
	    map = pg->mapnode (this, size,
			       addr_mask (vaddr, ~page_mask (size)));

#if !defined(CONFIG_NEW_MDB)
	    TRACEPOINT (MDB_UNMAP, "mdb_%s (spc=%p pg=%p map=%p vaddr=%p %c%c%c "
			"fsz=%d%cB tsz=%d%cB)  paddr=%p\n",
			ctrl.mapctrl_self ? "flush" : "unmap",
			this, pg, map, vaddr,
			fpage.is_read () ? 'r' : '~',
			fpage.is_write () ? 'w' : '~',
			fpage.is_execute () ? 'x' : '~',
			dbg_pgsize (page_size(size)), dbg_szname (page_size(size)),
			dbg_pgsize (page_size(pgsize)), dbg_szname (page_size(pgsize)),
			pg->address (this, size));
#endif

#if defined(CONFIG_NEW_MDB)
	    rwx |= mdb_mem.mapctrl (map,
				    mdb_t::range_t (fpage.get_base (),
						    fpage.get_size_log2 ()),
				    ctrl, fpage.get_rwx (), attribute);
#else
	    rwx |= mdb_flush (map, pg, size, vaddr, pgsize,
			      fpage, ctrl.mapctrl_self);
#endif
	}
	else if (unmap_all)
	{
	    release_kernel_mapping (vaddr, pg->address (this, size),
				    page_shift (size));

	    pg->clear (this, size, true, vaddr);
	    if (! space_t::does_tlbflush_pay (fpage.get_size_log2 ()))
		flush_tlbent (get_current_space (), vaddr, page_shift (size));
	}

    Next_entry:

	pg = pg->next (this, size, 1);
	vaddr = addr_offset (vaddr, page_size (size));
	num--;

	while (num == 0 && size < pgsize)
	{
	    /* Recurse up */
	    pg = r_pg[size];
	    num = r_num[size];
	    size++;

	    fpage_t fp;
	    fp.set ((word_t) vaddr, page_size (size), false, false, false);

	    if (ctrl.mapctrl_self && ctrl.unmap	&&
		(unmap_all || is_mappable (fp)))
		pg->remove_subtree (this, size, false);

	    pg = pg->next (this, size, 1);	
	}
    }

    if (space_t::does_tlbflush_pay (fpage.get_size_log2 ()))
	flush_tlb (get_current_space ());

    space_t::end_update ();

    fpage.set_rwx (rwx);
    return fpage;
}


/**
 * Read word from address space.  Parses page tables to find physical
 * address of mapping and reads the indicated word directly from
 * kernel-mapped physical memory.
 *
 * @param vaddr		virtual address to read
 * @param contents	returned contents in given virtual location
 *
 * @return true if mapping existed, false otherwise
 */
bool space_t::readmem (addr_t vaddr, word_t * contents)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;

    if (! lookup_mapping (vaddr, &pg, &pgsize))
	return false;

    addr_t paddr = pg->address (this, pgsize);
    paddr = addr_offset (paddr, (word_t) vaddr & page_mask (pgsize));
    addr_t paddr1 = addr_mask (paddr, ~(sizeof (word_t) - 1));

    if (paddr1 == paddr)
    {
	// Word access is properly aligned.
	*contents = readmem_phys (paddr);
    }
    else
    {
	// Word access not properly aligned.  Need to perform two
	// separate accesses.
	addr_t paddr2 = addr_offset (paddr1, sizeof (word_t));
	word_t mask = ~page_mask (pgsize);

	if (addr_mask (paddr1, mask) != addr_mask (paddr2, mask))
	{
	    // Word access crosses page boundary.
	    vaddr = addr_offset (vaddr, sizeof (word_t));
	    if (! lookup_mapping (vaddr, &pg, &pgsize))
		return false;
	    paddr2 = pg->address (this, pgsize);
	    paddr2 = addr_offset (paddr2, (word_t) vaddr & page_mask (pgsize));
	    paddr2 = addr_mask (paddr2, ~(sizeof (word_t) - 1));
	}

	word_t idx = ((word_t) vaddr) & (sizeof (word_t) - 1);

#if defined(CONFIG_BIGENDIAN)
	*contents = 
	    (readmem_phys (paddr1) << (idx * 8)) |
	    (readmem_phys (paddr2) >> ((sizeof (word_t) - idx) * 8));
#else
	*contents =
	    (readmem_phys (paddr1) >> (idx * 8)) |
	    (readmem_phys (paddr2) << ((sizeof (word_t) - idx) * 8));
#endif
    }

    return true;
}


/**
 * Lookup mapping in address space.  Parses the page table to find a
 * mapping for the indicated virtual address.  Also returns located
 * page table entry and page size in R_PG and R_SIZE (if non-nil).
 *
 * @param vaddr		virtual address
 * @param r_pg		returned page table entry for mapping
 * @param r_size	returned page size for mapping
 *
 * @return true if mapping exists, false otherwise
 */
bool space_t::lookup_mapping (addr_t vaddr, pgent_t ** r_pg,
			      pgent_t::pgsize_e * r_size, cpuid_t cpu)
{
    pgent_t * pg = this->pgent (page_table_index (pgent_t::size_max, vaddr), cpu);
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    for (;;)
    {
	if (!pg) 
	    return false;
	if (pg->is_valid (this, pgsize))
	{
	    if (pg->is_subtree (this, pgsize))
	    {
		// Recurse into subtree
		if (pgsize == 0)
		    return false;

		pg = pg->subtree (this, pgsize)->next
		    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
		pgsize--;
	    }
	    else
	    {
		// Return mapping
		if (r_pg)
		    *r_pg = pg;
		if (r_size)
		    *r_size = pgsize;
		return true;
	    }
	}
	else
	    // No valid mapping or subtree
	    return false;
    }

    /* NOTREACHED */
    return false;
}

#endif /* !__GENERIC__LINEAR_PTAB_WALKER_CC__ */
