/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/space.cc
 * Description:   VAS implementation 
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
 * $Id: space.cc,v 1.27 2006/11/17 17:14:30 skoglund Exp $
 *                
 ********************************************************************/

#include <kmemory.h>

#include INC_API(space.h)		/* space_t		*/
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)

#include INC_ARCH(pgent.h)
#include <linear_ptab.h>

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

#define PGSIZE_KTCB	(pgent_t::size_4k)
#define PGSIZE_KIP	(pgent_t::size_4k)
#define PGSIZE_UTCB	(pgent_t::size_4k)

asid_cache_t asid_cache UNIT("cpulocal");
space_t * kernel_space = NULL;
tcb_t *dummy_tcb = NULL;

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    int ktcb_idx = page_table_index(pgent_t::size_max,
		    (addr_t) (KTCB_AREA_START | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1))));

    /* Set up ktcb area (allocate page) */
    pgent(ktcb_idx)->make_subtree(this, pgent_t::size_max, true);
}

INLINE word_t pagedir_idx (addr_t addr)
{
    return page_table_index (pgent_t::size_max, addr);
}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space);
    kernel_space = allocate_space();
    ASSERT(kernel_space);

    kernel_space->get_asid()->init();
    kernel_space->init_kernel_mappings();

    /* Allocate dummy tcb */
    ASSERT(!dummy_tcb);
    dummy_tcb = (tcb_t *) kmem.alloc(kmem_tcb, MIPS64_PAGE_SIZE);
    ASSERT(dummy_tcb);

    TRACE_INIT ("Allocated kernel space of size 0x%x @ %p\n",
		sizeof (space_t), kernel_space);
}

/**
 * initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    int ktcb_idx = page_table_index(pgent_t::size_max,
		    (addr_t) (KTCB_AREA_START | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1))));

    this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    get_asid()->init();

    /* Copy top level entries for the ktcb areas */
    pgent(ktcb_idx)->set_entry(this, pgent_t::size_max,
		    *get_kernel_space()->pgent(ktcb_idx));

    add_mapping(kip_area.get_base(), virt_to_phys((addr_t)get_kip ()),
		PGSIZE_KIP, false, false);
}

/**
 * Release mappings that belong to the kernel (UTCB, KIP)
 * @param vaddr		virtual address in the space
 * @param paddr		physical address the mapping refers to
 * @param log2size	log2(size of mapping)
 */
void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    /* Free up memory used for UTCBs */
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}

/**
 * establish a mapping in sigma0's space
 * @param addr	the fault address in sigma0
 *
 * This function should install a mapping that allows sigma0 to make
 * progress. Sigma0's space is available as this.
 */
void space_t::map_sigma0(addr_t addr)
{
    //TRACEF("%p\n", addr);
    add_mapping(addr, addr, pgent_t::size_4k, true, false);
}

/**
 * Try to copy a mapping from kernel space into the current space
 * @param addr the address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 * Synchronization must happen at the highest level, allowing sharing.
 */
bool space_t::sync_kernel_space(addr_t addr)
{
    /* We set everything up at initialisation time */
    return false;
}

/**
 * Install a dummy TCB
 * @param addr	address where the dummy TCB should be installed
 *
 * The dummy TCB must be read-only and fail all validity tests.
 */
void space_t::map_dummy_tcb (addr_t addr)
{
    //TRACEF("%p %p\n", addr, (addr_t)dummy_tcb);
    add_mapping(addr, virt_to_phys((addr_t)dummy_tcb), PGSIZE_KTCB, false, true);
}

/**
 * Allocate a UTCB
 * @param tcb	Owner of the utcb
 *
 */
utcb_t * space_t::allocate_utcb (tcb_t * tcb)
{
    ASSERT (tcb);
    addr_t utcb = (addr_t) tcb->get_utcb_location ();

    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (lookup_mapping ((addr_t) utcb, &pg, &pgsize))
    {
        addr_t kaddr = addr_mask (pg->address(this, pgsize),
                                  ~page_mask (pgsize));
        return (utcb_t *) phys_to_virt
            (addr_offset (kaddr, (word_t) utcb & page_mask (pgsize)));
    }

#define MIPS64_ALIGN    ((ICACHE_SIZE/MIPS64_PAGE_SIZE/CACHE_WAYS - 1ul) << MIPS64_PAGE_BITS)

    addr_t page = kmem.alloc_aligned (kmem_utcb, page_size (PGSIZE_UTCB),
    					(word_t) utcb, MIPS64_ALIGN);

    add_mapping((addr_t) utcb, virt_to_phys(page), 
		PGSIZE_UTCB, true, false);

    return (utcb_t *)
        addr_offset (page, addr_mask (utcb, page_size (PGSIZE_UTCB) - 1));
}

/**
 * Map memory usable for TCB
 * @param addr address of the TCB that should be made usable
 *
 * This function is called when a TCB should be made usable the first
 * time. Usually, this happens when a) no page is mapped at the TCB
 * address at all, or b) a read-only page is mapped and now a write
 * access to the TCB occured.
 *
 * @see space_t::map_dummy_tcb
 */
void space_t::allocate_tcb(addr_t addr)
{
    addr_t page = kmem.alloc (kmem_tcb, MIPS64_PAGE_SIZE);
    //TRACEF("page = %p\n", page);

    kernel_space->add_mapping(addr, virt_to_phys(page), PGSIZE_KTCB, true, true);
}

/**
 * Translate a user accessible UTCB address to a kernel accessible one
 * @param utcb	user accessible address of UTCB
 * @returns kernel accessible address of UTCB
 *
 * The returned address must be accessible in the current address
 * space. This is required for checking values in the UTCB of a thread
 * in a different address space.
 */
utcb_t * space_t::utcb_to_kernel_space(utcb_t * utcb)
{
    /* (cvs) check this */
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if (lookup_mapping ((addr_t) utcb, &pg, &pgsize))
    {
	addr_t kaddr = pg->translation ()->phys_addr ();
	return (utcb_t *)phys_to_virt(kaddr);
    }

    return NULL;
}

DEFINE_SPINLOCK (tlb_lock);	// XXX
/**
 * Handle a XTLB Refill
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
extern "C" void handle_xtlb_miss (addr_t faddr, mips64_irq_context_t * frame)
{
    space_t * space = get_saved_pagetable();
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;
    bool tcb = false;

    if(space == NULL)
	space = get_kernel_space();

    if ((word_t)faddr & KTCB_AREA_START)
    {
	faddr = (addr_t)((word_t)faddr | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1)));
	tcb = true;
    }

    // TRACEF("(%016lx, %p) %d\n", faddr, space, get_current_cpu());

    // Check if mapping exist in page table
    if (space->lookup_mapping (faddr, &pg, &pgsize))
    {
	/* matthewc HACK: fill out both halves of the pair straight away
	 * or things can get nasty with nested faults (TCB case).
         */
	word_t temp, mapsize = (1 << MIPS64_PAGE_BITS);
	bool odd = (word_t)faddr & mapsize;
	pgent_t * buddy, * pg1, * pg2;
	if (space->lookup_mapping (addr_offset(faddr, odd ? -mapsize : mapsize), &buddy, &pgsize))
	{
	    pg1 = odd ? buddy : pg;
	    pg2 = odd ? pg : buddy;
	    faddr = (addr_t)((word_t)faddr & ~(1UL << (hw_pgshifts[pgent_t::size_max+1]-1)));
	    __asm__ __volatile__ (
		"dmfc0  %0,"STR(CP0_ENTRYHI)"\n\t"
		"nop;\n\t"
		"dmtc0  %1,"STR(CP0_ENTRYHI)"\n\t"
		"dmtc0  %2,"STR(CP0_ENTRYLO0)"\n\t"
		"dmtc0  %3,"STR(CP0_ENTRYLO1)"\n\t"
		"nop;nop;nop;\n\t"
		"tlbwr\n\t"
		"nop;nop;nop;\n\t"
		"dmtc0  %0,"STR(CP0_ENTRYHI)"\n\t"
		: "=r" (temp) : "r" ((((word_t)faddr>>13)<<13) | space->get_asid()->get()),
		  "r" (pg1->translation()->get_raw()), "r" (pg2->translation()->get_raw())
	    );
	    return;
	}
	pg->translation ()->put_tc (faddr, page_shift (pgsize),
					space->get_asid()->get());
	return;
    }

    while (1) {
	__asm__ __volatile__ (
	    "move	$29, %0	    \n\r"
	    "j	_mips64_xtlb_fall_through \n\r"
	    :: "r" (frame)
	);
    }
}

/**
 * Handle a TLB and Software TLB Cache miss
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
extern "C" void handle_stlb_miss (addr_t faddr, mips64_irq_context_t * frame)
{
    space_t * space = get_current_tcb ()->get_space();
    pgent_t::pgsize_e pgsize;
    space_t::access_e access;
    pgent_t * pg;
    bool kernel, twice;

    if(space == NULL)
	space = get_kernel_space();

    if ((word_t)faddr & KTCB_AREA_START)
    {
	faddr = (addr_t)((word_t)faddr | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1)));
    }

    // TRACEF("(%016lx, %p) %d\n", faddr, space, get_current_cpu());

    access = (frame->cause & CAUSE_EXCCODE) == (3<<2) ? /* TLBS (write) */
	    space_t::write : space_t::read;

    twice = false;

    while (1)
    {
	// Check if mapping exist in page table
	if (space->lookup_mapping (faddr, &pg, &pgsize))
	{
	    if (((access == space_t::write) && pg->is_writable(space, pgsize)) ||
		((access == space_t::read) && pg->is_readable(space, pgsize)) )
	    {
		pg->translation ()->put_tc (faddr, page_shift (pgsize),
			    space->get_asid()->get());
		return;
	    }
	}
	if (twice) return;

	if (space->is_user_area(faddr))
	    kernel = frame->status & ST_KSU ? false : true;
	else
	    kernel = true;	/* User-space will cause address error */

	space->handle_pagefault (faddr, (addr_t)frame->epc, access, kernel);

	twice = true;
    }
}

/**
 * Handle a TLB MOD exception
 * @param faddr  faulting address
 * @param frame  context frame of saved process
 */
extern "C" void handle_tlb_mod (addr_t faddr, mips64_irq_context_t * frame)
{
    space_t * space = get_current_tcb ()->get_space();
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if(space == NULL)
	space = get_kernel_space();

    if ((word_t)faddr & KTCB_AREA_START)
    {
	faddr = (addr_t)((word_t)faddr | (1UL << (hw_pgshifts[pgent_t::size_max+1]-1)));
    }

    // Check if mapping exist in page table
    if (space->lookup_mapping (faddr, &pg, &pgsize))
    {
	if (pg->is_writable (space, pgsize))
	{
	    pg->translation ()->put_tc (faddr, page_shift (pgsize),
					space->get_asid()->get());
	    return;
	}
    }

    // TRACEF("(%016lx, %p) %d\n", faddr, space, get_current_cpu());

    space->handle_pagefault (faddr, (addr_t)frame->epc, space_t::write,
			frame->status & ST_KSU ? false : true);
}

/**
 * Add a mapping into this address space
 */
void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel)
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

    tlb_lock.lock();
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

		tlb_lock.unlock();
		return;
	    }
	}
	else
	{
	    // Create subtree
	    pg->make_subtree (this, pgsize, kernel);
	}

	pg = pg->subtree (this, pgsize)->next
	    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
	pgsize--;
    }

    /*
     * Modify page table
     */

    pg->set_entry (this, pgsize, paddr, 7, kernel);
    pg->translation ()->set (
		    translation_t::l4default,
		    true, writable, kernel, paddr);
    tlb_lock.unlock();

    /*
     * Insert translation into TLB
     */

    pg->translation ()->put_tc (vaddr, page_shift (pgsize), this->get_asid()->get());
}
