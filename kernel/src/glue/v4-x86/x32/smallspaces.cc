/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/smallspaces.cc
 * Description:   Handling of small address spaces
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
 * $Id: smallspaces.cc,v 1.8 2004/03/10 18:33:22 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(smp.h)
#include INC_GLUE(space.h)


FEATURESTRING ("smallspaces");

DECLARE_TRACEPOINT (SMALLSPACE_CREATE);
DECLARE_TRACEPOINT (SMALLSPACE_ENLARGE);
DECLARE_TRACEPOINT (SMALLSPACE_SYNC);


/**
 * Array containing the owners of small space slots.
 */
x86_space_t * small_space_owner[SMALLSPACE_AREA_SIZE >> X86_X32_PDIR_BITS];


/**
 * Spinlock protecting access to the small space owners array.
 */
DEFINE_SPINLOCK (small_space_owner_lock);


/**
 * Linked list of spaces that are polluted with small space mappings.
 */
static x86_space_t * polluted_spaces = NULL;


/**
 * Spinlock protecting the polluted spaces list.
 */
DEFINE_SPINLOCK (polluted_spaces_lock);



/**
 * Set to non-nil if currently running in a small space.
 */
word_t __is_small UNIT ("cpulocal");


#if defined(CONFIG_X86_X32_SMALL_SPACES_GLOBAL)
/**
 * Modify global bits in the page tables of indicated space.
 *
 * @param space		space to modify global bits in
 * @param pg		pgent to start modification in
 * @param size		size of memory region to modify (in bytes)
 * @param onoff		whether to set (true) or clear (false) global bit
 */
static void modify_global_bits (space_t * space, pgent_t * pg,
				int size, bool onoff)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    while (size > 0)
    {
	if (pg->is_valid (space, pgsize))
	{
	    pg->set_global (space, pgsize, onoff);

	    if (pg->is_subtree (space, pgsize))
	    {
		pgent_t * pg2 = pg->subtree (space, pgsize--);
		for (int i = 1024; i > 0; i--)
		{
		    if (pg2->is_valid (space, pgsize))
			pg2->set_global (space, pgsize, onoff);
		    pg2 = pg2->next (space, pgsize, 1);
		}
		pgsize++;
	    }
	}

	size -= page_size (pgsize);
	pg = pg->next (space, pgsize, 1);
    }
}
#endif /* CONFIG_X86_X32_SMALL_SPACES_GLOBAL */


/*
 * When global small spaces is enabled we must do a global TLB flush
 * to get rid of the small space TLB entries.
 */
#if defined(CONFIG_X86_X32_SMALL_SPACES_GLOBAL)
#define FLUSH_GLOBAL true
#else
#define FLUSH_GLOBAL false
#endif


#if defined(CONFIG_SMP)
static void do_xcpu_flush_tlb(cpu_mb_entry_t * entry)
{
    spin(60, get_current_cpu());
    x86_mmu_t::flush_tlb (FLUSH_GLOBAL);
}
#endif /* CONFIG_SMP */



/**
 * Turn address space into a small space.  If address space is already
 * small we first turn the space into a large space before turning it
 * into a small space again.
 *
 * @param id		small space id
 *
 * @return true if conversion succeeded, false otherwise
 */
bool x86_space_t::make_small (smallspace_id_t id)
{
    const word_t max_idx = SMALLSPACE_AREA_SIZE >> X86_X32_PDIR_BITS;

    word_t size = id.size () >> 22;
    word_t offset = id.offset () >> 22;

    TRACEPOINT (SMALLSPACE_CREATE,
		"make_small: space=%p  size=%dMB  offset=%dMB\n", this, size*4, offset*4);

    if (offset + size > max_idx)
	return false;

    // Grab lock and make sure that we are not already holding a small
    // space area.
    for (;;)
    {
	small_space_owner_lock.lock ();

	// Verify that space is not already small.  If so, we enlarge
	// space and try again.
	for (word_t i = 0; i < max_idx; i++)
	    if (small_space_owner[i] == this)
	    {
		small_space_owner_lock.unlock ();
		make_large ();
		continue;
	    }

	break;
    }

    word_t i;

    // Try allocation small space slots.
    for (i = 0; i < size; i++)
    {
	if (small_space_owner[offset + i] == NULL)
	    small_space_owner[offset + i] = this;
	else
	    break;
    }

    // If allocation failed, free up the allocated slots and return
    // error.
    if (i < size)
    {
	while (i > 0)
	    small_space_owner[offset + --i] = NULL;

	small_space_owner_lock.unlock ();
	return false;
    }

    // Small space area has now been allocated.
    *smallid () = id;
    
    segdesc ()->set_seg (smallspace_offset (), smallspace_size () - 1,
			 3, x86_segdesc_t::data);

    small_space_owner_lock.unlock ();

#if defined(CONFIG_X86_X32_SMALL_SPACES_GLOBAL)
    // Set global bits for all pages in small space area.
    modify_global_bits (this, pgent (0), smallspace_size (), true);
#endif

    if (this == get_current_space () || get_current_tcb () == get_idle_tcb ())
    {
	// Reset GDT entries to have proper limits.
	extern x86_segdesc_t gdt[];

	gdt[X86_UCS >> 3].set_seg (smallspace_offset (), smallspace_size ()-1,
				    3, x86_segdesc_t::code);
	gdt[X86_UDS >> 3].set_seg (smallspace_offset (), smallspace_size ()-1,
				    3, x86_segdesc_t::data);

	reload_user_segregs ();

	// Inform thread switch code that we run in a small space.
	__is_small = 1;
    }

    return true;
}


/**
 * Turn address space into a large space.  This is an expensive
 * operation since all stale pagedir entries in the small space area
 * of other page directories must be purged.
 */
void x86_space_t::make_large (void)
{
    smallspace_id_t id = *smallid ();

    // Ignore if already running in a small space.
    if (! id.is_small ())
	return;

    word_t size = id.size () >> 22;
    word_t offset = id.offset () >> 22;
    
    //ENABLE_TRACEPOINT(SMALLSPACE_ENLARGE,~0,~0);

    TRACEPOINT (SMALLSPACE_ENLARGE, "make_large: space=%p (current size=%dMB  offset=%dMB)\n",
		this, size*4, offset*4);

    small_space_owner_lock.lock ();

    // Release allocated slots
    for (word_t i = 0; i < size; i++)
    {
	ASSERT (small_space_owner[offset + i] == this);
	small_space_owner[offset + i] = NULL;
    }

    smallid ()->set_large ();

#if defined(CONFIG_X86_X32_SMALL_SPACES_GLOBAL)
    // Clear global bits for all pages in small space area.
    modify_global_bits (this, pgent (0), id.size (), false);
#endif

    // Remove any stale pdir entries in other page tables
    polluted_spaces_lock.lock ();

    if (polluted_spaces)
    {
	x86_space_t * s = polluted_spaces;
	x86_space_t * b = s;
	do {
	    for (word_t cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	    {
		if (this->data.cpu_ptab[cpu].top_pdir) 
		    for (word_t i = 0; i < size; i++)
			s->data.cpu_ptab[cpu].top_pdir->small[offset + i].clear ();
	    }
	    s = s->get_next ();
	} while (s != b);
    }

    polluted_spaces_lock.unlock ();

    small_space_owner_lock.unlock ();

    if (get_current_space () == this)
    {
	// Reset GDT entries to 3GB limit.
	extern x86_segdesc_t gdt[];

	gdt[X86_UCS >> 3].set_seg (0, USER_AREA_END-1,
				    3, x86_segdesc_t::code);
	gdt[X86_UDS >> 3].set_seg (0, USER_AREA_END-1,
				    3, x86_segdesc_t::data);

	reload_user_segregs ();

	// Make sure that we run on our own page table.
	x86_mmu_t::set_active_pagetable
	    ((u32_t) get_current_space ()->get_top_pdir_phys (get_current_tcb ()->get_cpu ()));

	// Make sure that there are no stale TLB entries.
	x86_mmu_t::flush_tlb (FLUSH_GLOBAL);

	// Inform thread switch code that we run in a large space.
	__is_small = 0;
    }

#if defined(CONFIG_SMP)
    // Perform TLB shootdown on remote CPUs.
    for (word_t cpu = 0;
	 cpu < get_kip ()->processor_info.get_num_processors ();
	 cpu++)
    {
	if (cpu == get_current_cpu ())
	    continue;
	xcpu_request (cpu, do_xcpu_flush_tlb);
    }
#endif
}


/**
 * Dequeue space from the list of polluted spaces.
 */
void x86_space_t::dequeue_polluted (void)
{
    polluted_spaces_lock.lock ();

    if (get_next () == NULL)
    {
	// Space is not in list.
    }
    else if (get_next () == this)
    {
	// Space is only member of list.
	polluted_spaces = NULL;
    }
    else
    {
	// Fixup pointers of neighbor spaces.
	x86_space_t * n = get_next ();
	x86_space_t * p = get_prev ();
	n->set_prev (p);
	p->set_next (n);
	if (polluted_spaces == this)
	    polluted_spaces = n;
    }

    set_prev (NULL);
    set_next (NULL);

    polluted_spaces_lock.unlock ();
}


/**
 * Enqueue into list of polluted spaces.
 */
void x86_space_t::enqueue_polluted (void)
{
    polluted_spaces_lock.lock ();

    if (get_next () != NULL)
    {
	// Space already in list.
    }
    else if (polluted_spaces != NULL)
    {
	// Insert into list.
	x86_space_t * p = polluted_spaces->get_prev ();
	x86_space_t * n = polluted_spaces;

	n->set_prev (this);
	p->set_next (this);
	set_prev (p);
	set_next (n);
    }
    else
    {
	// Space is first one in list.
	set_prev (this);
	set_next (this);
	polluted_spaces = this;
    }

    polluted_spaces_lock.unlock ();
}


/**
 * Synchronize page table of small space with page table of current
 * space.  That is, copy page directory entry from original page table
 * into small space area of current page table.
 *
 * @param faddr		fault address (in small space area)
 *
 * @return true if a valid page directory entry was copied
 */
bool x86_space_t::sync_smallspace (addr_t faddr)
{
    pgent_t::pgsize_e size = pgent_t::size_max;

    // Get space which fault occured in.
    space_t * sspace = (space_t *) this;
    space_t * fspace = space_t::top_pdir_to_space(x86_mmu_t::get_active_pagetable());
	
    // Calculate real fault address.
    addr_t addr = addr_offset (faddr, 0 - smallspace_offset ());

    pgent_t * pgent_s = sspace->pgent (page_table_index (size, addr));
    pgent_t * pgent_f = fspace->pgent (page_table_index (size, faddr));

    // Copy page directory entry if it is valid.
    if (pgent_s->is_valid (sspace, size) && ! pgent_f->is_valid (fspace, size))
    {
	TRACEPOINT(SMALLSPACE_SYNC,
		   "smallspace_sync (%t, cr3 %p): s=%p:v=%p (p=%p) => s=%p:v=%p (p=%p)\n", 
		   get_current_tcb(), x86_mmu_t::get_active_pagetable(),
		   this, addr, pgent_s, fspace, faddr, pgent_f);
	
	*pgent_f = *pgent_s;
	pgent_f->sync(fspace, size);

	// Mark fault space as polluted with small space entries.
	fspace->enqueue_polluted ();
	return true;
    }

    return false;
}
