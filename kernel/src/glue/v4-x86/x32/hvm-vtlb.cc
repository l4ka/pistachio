/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     glue/v4-ia32/hvm/vtlb.cc
 * Description:   Full Virtualization Extensions - Generic VTLB
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
 * $Id$
 *
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(hvm.h)
#include INC_GLUE(hvm-vtlb.h)

#include <kmemory.h>
#include <linear_ptab.h>


DECLARE_KMEM_GROUP (kmem_vtlb);

DECLARE_TRACEPOINT(X86_HVM_VTLB);
DECLARE_TRACEPOINT(X86_HVM_VTLB_MISS);
DECLARE_TRACEPOINT(X86_HVM_VTLB_FLUSH);

bool x86_hvm_vtlb_t::alloc (space_t *space)
{

    /* Allocate a single host pdir for paged mode */
    hpdir_paged = (pgent_t *) kmem.alloc (kmem_vtlb, X86_PAGE_SIZE);;
    if (!hpdir_paged)
	return false;

    for (word_t i = 0; i < 1024; i++)
	hpdir_paged[i].pgent.clear();
    

    /* Allocate a single host pdir for unpaged mode */
    hpdir_nonpaged = (pgent_t *) kmem.alloc (kmem_vtlb, X86_PAGE_SIZE);;
    if (!hpdir_nonpaged)
	return false;

    for (word_t i = 0; i < 1024; i++)
	hpdir_nonpaged[i].pgent.clear();
    
    hpdir = hpdir_nonpaged;
    
    this->space = space;
    flags.pe = flags.wp = false;

    
    return true;
}

void x86_hvm_vtlb_t::free()
{
    if (hpdir_paged)
	kmem.free (kmem_vtlb, hpdir_paged, X86_PAGE_SIZE);
    
    if (hpdir_nonpaged)
	kmem.free (kmem_vtlb, hpdir_nonpaged, X86_PAGE_SIZE);

    this->space = NULL;
}


void x86_hvm_vtlb_t::flush_hpdir(pgent_t *pdir)
{
    TRACEPOINT(X86_HVM_VTLB_FLUSH, "VTLB (%x:%x) flush %x", gpdir, hpdir, pdir);
    ASSERT(pdir);
    
    for (word_t i = 0; i < 1024; i++)
    {
	bool pdir_global = false;
	
	if (pdir[i].is_valid(space, pgent_t::size_4m) && 
	    pdir[i].is_subtree(space, pgent_t::size_4m) &&
	    (pdir[i].reference_bits(space, pgent_t::size_4m, 0) & 0x6))
	{
	    pgent_t *hptab = pdir[i].subtree(space, pgent_t::size_4m);
	    bool ptab_global = false;
	    
	    //If any pte is global, leave in place
	    for (word_t j = 0; j < 1024; j++)
	    {
		if (hptab[j].is_valid(space, pgent_t::size_4k) && 
		    hptab[j].is_global(space, pgent_t::size_4k))
		    ptab_global = true;
		else
		    hptab[j].pgent.clear();
	    }
	    if (!ptab_global || !flags.pg)
		kmem.free (kmem_vtlb, hptab, X86_PAGE_SIZE);
	    else
		pdir_global = true;
	}
	if (!pdir_global || !flags.pg)
	    pdir[i].pgent.clear();
    }

}


void x86_hvm_vtlb_t::flush_hpdir(pgent_t *pdir, addr_t gvaddr)
{
    TRACEPOINT(X86_HVM_VTLB_FLUSH, "VTLB (%x:%x) flush %x single %x", gpdir, hpdir, pdir, gvaddr);
    ASSERT(pdir);
    
    pgent_t *pg = pdir->next(space, pgent_t::size_4m, page_table_index(pgent_t::size_4m, gvaddr));
    /* We clear only single entries and never remove page tables. */

    if (EXPECT_TRUE (!pg->pgent.is_valid()))
	return;

    if (!pg->is_subtree(space, pgent_t::size_4m))
	pg->pgent.clear();
    else
    {
	pg = pg->subtree(space, pgent_t::size_4m)->
	    next(space, pgent_t::size_4k, page_table_index(pgent_t::size_4k, (addr_t)  gvaddr));
	
	pg->pgent.clear();
    }

}



bool x86_hvm_vtlb_t::lookup_gphys_addr (addr_t gvaddr, addr_t *gpaddr)
{
    if (!flags.pe)
    {
	*gpaddr = gvaddr;
	return true;
    }

    /* Read guest page table. */
    pgent_t::pgsize_e gvpgsz = pgent_t::size_max;
    pgent_t *gvpgent = gpdir->next(space, pgent_t::size_4m, page_table_index(pgent_t::size_4m, (addr_t) gvaddr));
    pgent_t gvpgent_phys;
    
    for (;;)
    {
	ASSERT(space);

	if (! space->readmem ((addr_t) gvpgent, (word_t *) &gvpgent_phys))
	    return false;
	
	gvpgent = &gvpgent_phys; 
	
	if (gvpgent->is_valid (space, gvpgsz))
	{
	    if (gvpgent->is_subtree (space, gvpgsz))
	    {
		// Recurse into subtree
		if (gvpgsz == 0)
		    return false;
		
		gvpgent = virt_to_phys(gvpgent->subtree (space, gvpgsz))->next
				  (space, gvpgsz-1, page_table_index (gvpgsz-1, gvaddr));
		gvpgsz--;
	    }
	    else
	    {
		// Return address
		*gpaddr = addr_offset(gvpgent->address(space, gvpgsz), 
				     addr_mask(gvaddr, page_mask(gvpgsz)));

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


bool x86_hvm_vtlb_t::dump_ptab_entry (addr_t gvaddr)
{
    if (!flags.pe)
    {
	printf("[gvirt] %p -> [gphys] %p (no paging)\n", gvaddr, gvaddr);
	return true;
    }

    /* Read guest page table. */
    pgent_t::pgsize_e gvpgsz = pgent_t::size_max;
    pgent_t *gvpgent = gpdir->next(space, pgent_t::size_4m, page_table_index(pgent_t::size_4m, (addr_t) gvaddr));
    pgent_t gvpgent_phys;
    
    for (;;)
    {
	ASSERT(space);

	if (! space->readmem ((addr_t) gvpgent, (word_t *) &gvpgent_phys))
	    return false;
	
	gvpgent = &gvpgent_phys; 
	
	if (gvpgent->is_valid (space, gvpgsz))
	{
	    if (gvpgent->is_subtree (space, gvpgsz))
	    {
		// Recurse into subtree
		if (gvpgsz == 0)
		    return false;
		
		gvpgent = virt_to_phys(gvpgent->subtree (space, gvpgsz))->next
				  (space, gvpgsz-1, page_table_index (gvpgsz-1, gvaddr));
		gvpgsz--;
	    }
	    else
	    {
		addr_t gpaddr = addr_offset(gvpgent->address(space, gvpgsz), addr_mask(gvaddr, page_mask(gvpgsz)));
		printf("[gvirt] %p -> [gphys] %p ", gvaddr, gpaddr );
		word_t pgsz = page_size (gvpgsz);
		word_t rwx = gvpgent->reference_bits (space, gvpgsz, gvaddr);
		printf("%3d%cB %c%c%c (%c%c%c) %s ",
		       (pgsz >= GB (1) ? pgsz >> 30 :
			pgsz >= MB (1) ? pgsz >> 20 : pgsz >> 10),
		       pgsz >= GB (1) ? 'G' : pgsz >= MB (1) ? 'M' : 'K',
		       gvpgent->is_readable (space, gvpgsz)   ? 'r' : '~',
		       gvpgent->is_writable (space, gvpgsz)   ? 'w' : '~',
		       gvpgent->is_executable (space, gvpgsz) ? 'x' : '~',
		       rwx & 4 ? 'R' : '~',
		       rwx & 2 ? 'W' : '~',
		       rwx & 1 ? 'X' : '~',
		       gvpgent->is_kernel (space, gvpgsz) ? "kernel" : "user");
		gvpgent->dump_misc (space, gvpgsz);
		printf("\n");
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


/* Insert a GV->GP mapping into the VTLB.
   Note: This function is generic, not IA-32 specific. */
void x86_hvm_vtlb_t::set_gphys_entry (addr_t gvaddr, addr_t gpaddr, pgent_t::pgsize_e gvpgsz, 
				      word_t rwx, word_t attrib, bool kernel, bool global,
				      word_t access)
{
    pgent_t *gppgent = NULL;
    pgent_t::pgsize_e gppgsz;
    
    tcb_t *current = get_current_tcb();

    /* Lookup/request a mapping from monitor. */
    while (!(space->lookup_mapping(gpaddr, &gppgent, &gppgsz)) ||
	   (!(space->is_user_area(gpaddr))) || 
	   ((access & X86_PAGE_WRITABLE) && !gppgent->is_writable (space, gppgsz)))
        
    {
        
        TRACEPOINT(X86_HVM_VTLB_MISS, "VTLB (%x:%x) gp %08x miss", gpdir, hpdir, gpaddr);
        
        addr_t gpaddr_base = addr_align(gpaddr, page_size(gvpgsz));
	
	// Check if access to KIP/UTCB page area
	if ((access & X86_PAGE_WRITABLE) && 
	    ((space->get_kip_page_area ().is_range_overlapping
	     (gpaddr_base, addr_offset (gpaddr_base, page_size (gvpgsz)-1))) ||
             (space->get_utcb_page_area ().is_range_overlapping
              (gpaddr_base, addr_offset (gpaddr_base, page_size (gvpgsz)-1)))))
	
	{
	    // If unpaged access, try  a smaller size
	    if (gvpgsz == pgent_t::size_max)
	    {
		gvpgsz--;
		continue;
	    }
	    else
	    {
		printf("VTLB (%x:%x) gp %08x base %08x vpgsz %d access %x KIP area %x %x UTCB area %x %x\n", 
		       gpdir, hpdir, gpaddr, gpaddr_base, page_size(gvpgsz), access,
		       space->get_utcb_page_area().get_address(), space->get_utcb_page_area().get_size(),
		       space->get_kip_page_area().get_address(), space->get_kip_page_area().get_size());
		enter_kdebug("HVM thread writes KIP/UTCB area");
	    }
	}

        /*
	 * The space does not have a mapping.
	 * Send page fault message to monitor.
	 */
	current->send_pagefault_ipc((addr_t) gpaddr, current->get_user_ip(), 
				    (space_t::access_e) (access & X86_PAGEFAULT_BITS));

	memory_barrier();
#warning Nilpage handling needs proper implementation
	/* Still no mapping after pagefault, assume someone send a Nilpage */
	if (!(space->lookup_mapping(gpaddr, &gppgent, &gppgsz)) ||
	    (!(space->is_user_area(gpaddr))) || 
	    ((access & X86_PAGE_WRITABLE) && !gppgent->is_writable (space, gppgsz)))
	    return;
    }

    addr_t hpaddr = addr_offset(gppgent->address(space, gppgsz), 
				addr_mask(gpaddr, page_mask(gppgsz)));
    
    
    /* Use the smaller of guest virtual and guest physical page size. */
    pgent_t::pgsize_e hppgsz = min(gvpgsz, gppgsz);
    
    /* Merge bits */
    rwx &= gppgent->rights(space, gppgsz);
    attrib |= gppgent->attributes(space, gppgsz);
    
    /* Insert the VTLB entry. */
    set_hphys_entry (gvaddr, hpaddr, hppgsz, rwx, attrib, kernel, global);
    

}

void x86_hvm_vtlb_t::set_hphys_entry (addr_t gvaddr, addr_t hpaddr, pgent_t::pgsize_e hppgsz, 
				      word_t rwx, word_t attrib, bool kernel, bool global)
{
    pgent_t *hppgent = hpdir->next(space, pgent_t::size_4m, page_table_index(pgent_t::size_4m, gvaddr));
    
   
    TRACEPOINT(X86_HVM_VTLB, "VTLB (%x:%x) gv %08x -> hp %08x sz %d rwx %x attr %x %c %c", 
		gpdir, hpdir, gvaddr, hpaddr, page_size(hppgsz), rwx, attrib, 
		(kernel ? 'k' : 'u'), (global ? 'g' : ' '));
    
    /* Check whether we are mapping a superpage. */
    if (hppgsz == pgent_t::size_4m)
    {
	/* If there is a page table in place, remove it. */
	if (hppgent->is_valid (space, hppgsz) && hppgent->is_subtree(space, hppgsz))
	{
	    pgent_t *pt = (pgent_t *) hppgent->subtree(space, pgent_t::size_4m);
	    //printf( "VTLB (%x:%x) gv %08x -> hp %08x sz %d flush subtree %x",
	    //       gpdir, hpdir, gvaddr, hpaddr, page_size(hppgsz), pt);
	    kmem.free (kmem_vtlb, pt, X86_PAGE_SIZE);
	}
    }
    else
    {
	/* If there is no page table, allocate a new one. */
	if (!hppgent->is_valid (space, hppgsz+1) || !hppgent->is_subtree(space, hppgsz+1))
	{
	    /* Allocate memory. */
	    pgent_t *pt = (pgent_t *) kmem.alloc (kmem_vtlb, X86_PAGE_SIZE);
	    if (!pt)
	    {
		printf("VTLB (%x:%x) not enough memory, flush", gpdir, hpdir);
		enter_kdebug("UNTESTED");
		/* Try to free up some space. */
		flush_gphys();
		/* This will cause a retry. */
		return;
	    }
	    
	    //printf( "VTLB (%x:%x) gv %08x -> hp %08x sz %d alloc subtree %x",
	    //       gpdir, hpdir, gvaddr, hpaddr, page_size(hppgsz), pt);
	    
	    /* Clear new page table. */
	    for (word_t i = 0; i < 1024; i++)
		pt[i].pgent.clear();

	    /* Set page table. */
	    hppgent->pgent.set_ptab_entry (virt_to_phys (pt), X86_PAGE_USER | X86_PAGE_WRITABLE);
	    
	}
	//printf( "VTLB (%x:%x) hppgent %08x", gpdir, hpdir, hppgent->raw);

	hppgent = hppgent->subtree(space, pgent_t::size_4m)->
	    next(space, pgent_t::size_4k, page_table_index(pgent_t::size_4k, gvaddr));
    }
    pgent_t s = *hppgent;
    
    /* Set mapping. Don't use set_entry(), since it syncs entries */
    hppgent->pgent.set_entry (hpaddr, (x86_pgent_t::pagesize_e) hppgsz,
			     (kernel ? X86_PAGE_KERNEL : X86_PAGE_USER) |
			      (attrib & 1 ? X86_PAGE_WRITE_THROUGH : 0) |
			      (attrib & 2 ? X86_PAGE_CACHE_DISABLE : 0) |
			      (attrib & 4 ? (1UL << (hppgsz == pgent_t::size_4k ? 7 : 12)) : 0) |
			      (rwx & 2 ? X86_PAGE_WRITABLE : 0) |
			      (global ? X86_PAGE_GLOBAL : 0) |
			      X86_PAGE_VALID);
    
    //printf( "VTLB (%x:%x) hppgent %08x\n", gpdir, hpdir, hppgent->raw);
    
    
}


/* Handle a VTLB miss. */
bool x86_hvm_vtlb_t::handle_vtlb_miss (addr_t gvaddr, word_t access)
{
    ASSERT(get_current_space() == space);
    
    if (!flags.pe)
    {
	set_gphys_entry (gvaddr, gvaddr, pgent_t::size_max, 0xf, 0, false, true, access);
	return true;
    }
    else 
    {
	/* Parse the page table hierarchy. */
	pgent_t::pgsize_e gvpgsz = pgent_t::size_max;
	pgent_t *gvpgent = gpdir->next(space, gvpgsz, page_table_index(gvpgsz, gvaddr));
	word_t rwx = 0, attrib = 0;
	bool kernel = false, global = false;
	addr_t gpaddr = 0;
	
	if (!gvpgent->is_valid(space, gvpgsz))
	    goto access_fault;
	
	rwx = gvpgent->rights(space, gvpgsz);
	attrib = gvpgent->attributes(space, gvpgsz);
	kernel = gvpgent->is_kernel(space, gvpgsz);
	global = gvpgent->is_global(space, gvpgsz);

	while (gvpgent->is_subtree (space, gvpgsz))
	{
	    //printf( "VTLB (%x:%x) gvpgent %08x %d", 
	    //       gpdir, hpdir, gvpgent->raw, page_table_index(gvpgsz, gvaddr));
	    gvpgent = virt_to_phys(gvpgent->subtree(space, gvpgsz--));
	    gvpgent = gvpgent->next(space, gvpgsz, page_table_index(gvpgsz, gvaddr));

	    if (!gvpgent->is_valid(space, gvpgsz))
		goto access_fault;

	    rwx &= gvpgent->rights(space, gvpgsz);
 	    attrib = gvpgent->attributes(space, gvpgsz);
	    kernel = (kernel || gvpgent->is_kernel(space, gvpgsz));
	    global = (global || gvpgent->is_global(space, gvpgsz));
	    
	}

	//printf( "VTLB (%x:%x) gvpgent %08x %d", 
	//   gpdir, hpdir, gvpgent->raw, page_table_index(gvpgsz, gvaddr));

	if (kernel && (access & X86_PAGE_USER))
	    goto access_fault;

	if (!(rwx & 2) && (access & X86_PAGE_WRITABLE) && 
	    ((access & X86_PAGE_USER) || flags.wp))
	    goto access_fault;
		

	gpaddr = addr_offset(gvpgent->address(space, gvpgsz), 
			     addr_mask(gvaddr, page_mask(gvpgsz)));


	/* Notify guest about page access. */
	gvpgent->update_reference_bits (space, gvpgsz, (access & X86_PAGE_WRITABLE) ? 6 : 5);
	
	
	//printf( "VTLB (%x:%x) gv %08x -> gp %08x sz %d rwx %x attr %x %c %c access %x", 
	//   gpdir, hpdir, gvaddr, gpaddr, page_size(gvpgsz), rwx, attrib,
	//   (kernel ? 'k' : 'u'), (global ? 'g' : ' '), access);
	
	/*
	 * The page must be marked as dirty before it can be written to.
	 * Otherwise the dirty bit would stay unset when the guest writes to it later.
	 * We never set the dirty bit on page directories, so don't check for it here.
	 *
	 * SR: What are the hardware semantics if a page is marked dirty but not
	 *     writable (i.e., the guest removed write privileges)?
	 *     We don't remove it from the VTLB since we don't notice the privilege
	 *     removal, but if we get a VTLB miss, we will map it read-only.
	 *     Every sane guest OS will cause a TLB flush, of course.
	 */
	if (!(gvpgent->reference_bits(space, gvpgsz, gvaddr) & 0x2))
	    rwx &= ~2;
	
	set_gphys_entry (gvaddr, gpaddr, gvpgsz, rwx, attrib, kernel, global, access);
	return true;
	
    access_fault:
	TRACEPOINT(X86_HVM_VTLB_MISS, "VTLB (%x:%x) gv %08x -> gp %08x sz %d rwx %x %c %c access fault %x", 
		   gpdir, hpdir, gvaddr, gpaddr, 
		   page_size(gvpgsz), gvpgent->rights(space, gvpgsz), 
		   (kernel ? 'k' : 'u'), (global ? 'g' : ' '), access);

	return false;
    }
    
    TRACEPOINT(X86_HVM_VTLB_MISS, "VTLB (%x:%x) gv %08x -> ### access %x miss", 
	       gpdir, hpdir, gvaddr, access);
   
    return false;
    /*
     * No guest page table entry.
     * Caller has to inject page fault into guest.
     */

}
