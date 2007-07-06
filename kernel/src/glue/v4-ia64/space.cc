/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/space.cc
 * Description:   IA-64 V4 specific space management
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
 * $Id: space.cc,v 1.48 2006/11/17 17:14:30 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kmemory.h>
#include <kdb/tracepoints.h>
#include <generic/lib.h>

#include INC_GLUE(space.h)
#include INC_GLUE(context.h)
#include INC_GLUE(memory.h)
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(pgent.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include <linear_ptab.h>

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

/**
 * kernel_space: global space used for holding kernel mappings
 */
space_t * kernel_space = NULL;


static tcb_t * dummy_tcb = NULL;

static tcb_t * get_dummy_tcb (void)
{
    if (!dummy_tcb)
    {
	dummy_tcb = (tcb_t *) kmem.alloc (kmem_tcb,
					  page_size (KTCB_ALLOC_SIZE));
	ASSERT (dummy_tcb);
	dummy_tcb = virt_to_phys (dummy_tcb);
    }

    return dummy_tcb;
}


INLINE word_t pagedir_idx (addr_t addr)
{
    return page_table_index (pgent_t::size_max, addr);
}



/**
 * space_t::init initializes the space_t
 *
 * Maps the kernel area and initializes shadow ptabs etc.
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    add_mapping (kip_area.get_base(),
		 virt_to_phys ((addr_t) get_kip ()),
		 pgent_t::size_4k, false,
		 translation_t::write_back,
		 translation_t::ro,
		 translation_t::data,
		 0);
}


extern "C" void * memcpy (void *, const void *, unsigned int);

void space_t::init_cpu_mappings (cpuid_t cpuid)
{
    word_t cpu_local_size = (word_t) end_cpu_local - (word_t) start_cpu_local;
    word_t mapsize = matching_pgsize (cpu_local_size);
    if (mapsize == 0)
	panic ("Size of CPU local memory too large (0x%x)", cpu_local_size);

    if (addr_align (start_cpu_local, (1UL << mapsize)) != start_cpu_local)
    {
	panic ("Virtual location of CPU local memory is not properly "
	       "aligned (location=%p size=%p)\n",
	       start_cpu_local, (1UL << mapsize));
    }

    // Size of allocated CPU local memory need not match the mapsize.
    word_t alloc_size = (1 << 12);
    while (alloc_size < cpu_local_size)
	alloc_size <<= 1;

    EXTERN_KMEM_GROUP (kmem_misc);

    addr_t cpu_local_memory;
    if (cpuid == 0)
    {
	if (addr_align (start_cpu_local_mem, (1UL << mapsize)) !=
	    start_cpu_local_mem)
	{
	    // Memory in kernel image not properly aligned.
	    TRACE_INIT ("CPU local memory in kernel image not properly "
			"aligned.  Creating a copy.\n");

	    cpu_local_memory = kmem.alloc (kmem_misc, (1UL << mapsize));
	    memcpy (cpu_local_memory, start_cpu_local_mem, cpu_local_size);
	}
	else
	    // Use memory from kernel image
	    cpu_local_memory = start_cpu_local_mem;
    }
    else
	// Allocate memory for CPU local data
	cpu_local_memory = kmem.alloc (kmem_misc, (1UL << mapsize));


    if (cpu_local_memory != start_cpu_local_mem)
	// Free unused part of allocated CPU local memory
	for (addr_t ptr = addr_offset (cpu_local_memory, alloc_size);
	     ptr < addr_offset (cpu_local_memory, (1UL << mapsize));
	     ptr = addr_offset (ptr, alloc_size))
	{
	    kmem.free (kmem_misc, ptr, alloc_size);
	}

    TRACE_INIT ("CPU %d: Map CPU local memory from %p (%dKB)\n",
		cpuid, cpu_local_memory, (1UL << mapsize) >> 10);

    translation_t tr (true, translation_t::write_back, true, true, 0,
		      translation_t::rwx, virt_to_phys (cpu_local_memory), 0);

    static word_t cpu_local_tr = 0;
    if (cpuid == 0)
    {
	// Insert new translation
	cpu_local_tr = dtrmap.add_map (tr, start_cpu_local, mapsize, 0);
    }
    else
    {
	// Purge existing translation and insert new one
	purge_dtr (start_cpu_local, mapsize);
	tr.put_dtr (cpu_local_tr, start_cpu_local, mapsize, 0);
    }


    // Calculate physical location of idle TCBs
    word_t offset = (word_t) get_idle_tcb () - (word_t) start_cpu_local;
    get_idle_tcb ()->arch.phys_addr =
	addr_offset (virt_to_phys (cpu_local_memory), offset);
}

utcb_t * space_t::allocate_utcb (tcb_t * tcb)
{
    ASSERT (tcb);
    addr_t utcb = (addr_t) tcb->get_utcb_location ();

    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (lookup_mapping ((addr_t) utcb, &pg, &pgsize))
    {
	addr_t kaddr = addr_mask (pg->translation ()->phys_addr (),
				  ~page_mask (pgsize));
	return (utcb_t *) phys_to_virt
	    (addr_offset (kaddr, (word_t) utcb & page_mask (pgsize)));
    }

    addr_t page = kmem.alloc (kmem_utcb, page_size (UTCB_ALLOC_SIZE));

    add_mapping ((addr_t) utcb, virt_to_phys (page),
		 UTCB_ALLOC_SIZE, false,
		 translation_t::write_back,
		 translation_t::rw,
		 translation_t::data,
		 0);

    return (utcb_t *)
	addr_offset (page, addr_mask (utcb, page_size (UTCB_ALLOC_SIZE) - 1));
}

void space_t::allocate_tcb (addr_t addr)
{
    addr_t page = kmem.alloc (kmem_tcb, page_size (KTCB_ALLOC_SIZE));

    // Store physical location of TCB
    for (tcb_t * vtcb = addr_to_tcb (page);
	 vtcb < addr_offset (page, page_size (KTCB_ALLOC_SIZE));
	 vtcb = (tcb_t *) addr_offset (vtcb, KTCB_SIZE))
    {
	vtcb->arch.phys_addr = virt_to_phys (vtcb);
    }

    kernel_space->add_mapping (addr, virt_to_phys (page),
			       KTCB_ALLOC_SIZE, true,
			       translation_t::write_back,
			       translation_t::rw,
			       translation_t::data,
			       0);
}

void space_t::map_dummy_tcb (addr_t addr)
{
    kernel_space->add_mapping (addr, get_dummy_tcb (),
			       KTCB_ALLOC_SIZE, true,
			       translation_t::write_back,
			       translation_t::ro,
			       translation_t::data,
			       0);
}

void space_t::map_sigma0 (addr_t addr)
{
    // Insert 256MB mapping
    add_mapping (addr, addr, pgent_t::size_256m, false,
		 translation_t::write_back,
		 translation_t::rwx,
		 translation_t::both,
		 0);
}

void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}

void SECTION (".init") init_kernel_space (void)
{
    kernel_space = (space_t *) kmem.alloc (kmem_space, sizeof (space_t));

    get_idle_tcb ()->set_space (kernel_space);

    TRACE_INIT ("Allocated kernel space of size 0x%x @ %p\n",
		sizeof (space_t), kernel_space);

    // Set up region register for TCB area.
    rr_t rr;
    rr.set (false, kernel_space->get_region_id (), KTCB_ALLOC_SIZE_SHIFT);
    rr.put (5);
    ia64_srlz_d ();
}


void space_t::add_mapping (addr_t vaddr, addr_t paddr,
			   pgent_t::pgsize_e size, bool kernel,
			   translation_t::memattrib_e memattrib,
			   translation_t::access_rights_e access_rights,
			   translation_t::type_e type,
			   word_t key)
{
    pgent_t * pg = this->pgent (pagedir_idx (vaddr), 0);
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    /*
     * Sanity checking on page size
     */

    if (! is_page_size_valid (size))
    {
	printf ("Mapping invalid pagesize (%dKB)\n", page_size (pgsize) >> 10);
	enter_kdebug ("invalid page size");
	return;
    }

    /*
     * Lookup mapping
     */

    while (pgsize > size)
    {
	if (pg->is_valid (this, pgsize))
	{
	    // Sanity check
	    if (! pg->is_subtree (this, pgsize))
	    {
		printf ("%dKB mapping @ %p space %p already exists.\n",
			page_size (pgsize) >> 10, vaddr, this);
		enter_kdebug ("mapping exists");
		return;
	    }
	}
	else
	    // Create subtree
	    pg->make_subtree (this, pgsize, kernel);

	pg = pg->subtree (this, pgsize)->next
	    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
	pgsize--;
    }

    /*
     * Modify page table
     */

    pg->set_entry (this, pgsize, paddr, access_rights, memattrib, kernel);
}


/**
 * Check if translation exists in page table, and if entry permits
 * indicated access type, insert translation into TLB.
 *
 * @param vaddr		virtual address to check for
 * @param access	access rights to match agains
 *
 * @return true if insertion was successful, false otherwise
 */
bool space_t::insert_translation (addr_t vaddr, space_t::access_e access)
{
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (lookup_mapping (vaddr, &pg, &pgsize))
    {
	// Check access rights
	if ((is_execute (access) && pg->is_executable (this, pgsize)) ||
	    (is_read (access)    && pg->is_readable (this, pgsize)) ||
	    (is_write (access)   && pg->is_writable (this, pgsize)))
	{
	    // Insert translation into TLB.  We also set the
	    // appropriate reference bits to avoid faulting
	    // immediately (optimization).
	    if (is_execute (access))
	    {
		pg->set_executed ();
		pgent_t pgx = *pg;
		pgx.set_referenced ();
		pgx.translation ()->put_tc (translation_t::code, vaddr,
					    page_shift (pgsize), 0);
	    }
	    else
	    {
		pg->set_referenced ();
		if (is_write (access))
		{
		    // A read-only TLB entry may already exist
		    purge_tc (vaddr, page_shift (pgsize), get_region_id ());
		    pg->set_written ();
		}
		pg->translation ()->put_tc (translation_t::data, vaddr,
					    page_shift (pgsize), 0);
	    }

	    return true;
	}
    }

    return false;
}


DECLARE_TRACEPOINT (VHPT_MISS);
DECLARE_TRACEPOINT (ACCESS_RIGHTS_FAULT);

extern "C" void
handle_vhpt_miss (ia64_exception_context_t * frame)
{
    space_t * fspace;
    space_t * space = get_current_space ();
    space_t::access_e ftype = (space_t::access_e) frame->isr.rwx;
    addr_t faddr = frame->ifa;
    bool is_kernel = frame->ipsr.cpl == 0;

    // If a kernel access is not backed by a TR we insert a huge TLB
    // entry for it.  This is necessary when, e.g., accessing
    // user-level memory by looking up the physical address through
    // the page tables.
    if (is_kernel && (IA64_RR_NUM (faddr) == 6 || IA64_RR_NUM (faddr) == 7))
    {
	word_t rr = IA64_RR_NUM (faddr);
	translation_t tr (true, (rr == 7 ? translation_t::write_back :
				 translation_t::uncacheable),
			  true, true, 0, translation_t::rwx,
			  virt_to_phys (faddr), 0);
	tr.put_tc (ftype == (space_t::execute) ?
		   translation_t::code : translation_t::data,
		   faddr, HUGE_PGSIZE, 0);
	return;
    }

    // Use kernel space if we have kernel fault in TCB area or
    // when no space is set (e.g., running on the idler)
    if ((is_kernel && space->is_tcb_area (faddr)) || space == NULL)
	space = get_kernel_space ();

    // If there was a miss in the copy area, we should try resolving
    // the fault using the partner's address space.
    if (is_kernel && space->is_copy_area (faddr))
    {
	tcb_t * partner = space->get_tcb (get_current_tcb ()->get_partner ());
	ASSERT (partner != NULL);
	fspace = partner->get_space ();
    }
    else
	fspace = space;

    TRACEPOINT (VHPT_MISS,
		printf ("VHPT miss @ %p, ip=%p (frame=%p) type=%s %s\n",
			faddr, addr_offset
			(frame->iip, frame->isr.instruction_slot * 6), frame,
			is_kernel ? "kernel" : "user",
			ftype == (space_t::execute) ? "execute" :
			ftype == (space_t::read) ? "read" :
			ftype == (space_t::write) ? "write" :
			ftype == (space_t::readwrite) ? "read/write" :
			"unknown"));

    for (word_t tries = 0; tries <= 1; tries++)
    {
	if (fspace->insert_translation (faddr, ftype))
	{
 	    // Mapping existed in page table
	    return;
	}

	if (tries == 0)
	    // Try to resolve pagefault and check mapping again
	    space->handle_pagefault (faddr, frame->iip, ftype, is_kernel);
    }
}

extern "C" void
handle_access_rights_fault (ia64_exception_context_t * frame)
{
    space_t * fspace;
    space_t * space = get_current_space ();
    space_t::access_e ftype = (space_t::access_e) frame->isr.rwx;
    addr_t faddr = frame->ifa;
    bool is_kernel = frame->ipsr.cpl == 0;

    // Use kernel space if we have kernel fault in TCB area or
    // when no space is set (e.g., running on the idler)
    if ((is_kernel && space->is_tcb_area (faddr)) || space == NULL)
	space = get_kernel_space ();

    // If there was a miss in the copy area, we should try resolving
    // the fault using the partner's address space.
    if (is_kernel && space->is_copy_area (faddr))
    {
	tcb_t * partner = space->get_tcb (get_current_tcb ()->get_partner ());
	ASSERT (partner != NULL);
	fspace = partner->get_space ();
    }
    else
	fspace = space;

    TRACEPOINT (ACCESS_RIGHTS_FAULT,
		printf ("Access rights fault @ %p, ip=%p "
			"(frame=%p) type=%s %s\n",
			faddr, addr_offset
			(frame->iip, frame->isr.instruction_slot * 6), frame,
			is_kernel ? "kernel" : "user",
			ftype == (space_t::execute) ? "execute" :
			ftype == (space_t::read) ? "read" :
			ftype == (space_t::write) ? "write" :
			ftype == (space_t::readwrite) ? "read/write" :
			"unknown"));

    // Try to resolve pagefault and insert mapping
    space->handle_pagefault (faddr, frame->iip, ftype, is_kernel);
    fspace->insert_translation (faddr, ftype);
}

extern "C" void
handle_reference_bits (ia64_exception_context_t * frame)
{
    space_t * space = get_current_space ();
    addr_t faddr = frame->ifa;
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;
    bool is_kernel = frame->ipsr.cpl == 0;

    // If there was a miss in the copy area, we should update the
    // reference bits of the partner's space.
    if (is_kernel && space->is_copy_area (faddr))
    {
	tcb_t * partner = space->get_tcb (get_current_tcb ()->get_partner ());
	ASSERT (partner != NULL);
	space = partner->get_space ();
    }
    
    if (space->lookup_mapping (faddr, &pg, &pgsize))
    {
	switch (frame->exception_num)
	{
	case 9:
	{
	    // execute
	    pg->set_executed ();
	    pgent_t pgx = *pg;
	    pgx.set_referenced ();
	    pgx.translation ()->put_tc (translation_t::code,
					faddr, page_shift (pgsize), 0);
	    return;
	}

	case 8:
	    // write
	    pg->set_written ();
	    /* FALLTHROUGH */

	case 10:
	    // read
	    pg->set_referenced ();
	    pg->translation ()->put_tc (translation_t::data,
					faddr, page_shift (pgsize), 0);
	    return;

	default:
	    break;
	}
    }

    printf ("Bogus rwx reference (frame=%p)\n", frame);
    enter_kdebug ("handle reference bits");
}

/**
 * ACPI memory handling
 */
addr_t acpi_remap(addr_t addr)
{
    //TRACE_INIT("ACPI remap: %p\n", addr); 
    return addr;
}

void acpi_unmap(addr_t addr)
{
    /* empty right now */
}

