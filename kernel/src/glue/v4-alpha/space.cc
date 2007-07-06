/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/space.cc
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
 * $Id: space.cc,v 1.21 2005/06/03 15:54:06 joshua Exp $
 *                
 ********************************************************************/

#include <debug.h>			/* for UNIMPLEMENTED	*/
#include <linear_ptab.h>
#include <kdb/tracepoints.h>
#include <generic/lib.h>
#include <kmemory.h>

#include INC_API(space.h)		/* space_t		*/
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(pal.h)

#define PGSIZE_KTCB	(pgent_t::size_base)
#define PGSIZE_UTCB	(pgent_t::size_base)
#define PGSIZE_KERNEL	(pgent_t::size_base)
#define PGSIZE_KIP	(pgent_t::size_base)

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

asid_cache_t asid_cache;
space_t *kernel_space = NULL;
tcb_t *dummy_tcb = NULL;

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    int ktcb_idx = page_table_index(pgent_t::size_max, (addr_t) KTCB_AREA_START),
	vlpt_idx = page_table_index(pgent_t::size_max, (addr_t) VLPT_AREA_START);

    /* Set up ktcb area (allocate page) */
    pgent(ktcb_idx)->make_subtree(this, pgent_t::size_max, true);

    /* Set up self-mapping */
    /* sjw (06/08/2002): This is somewhat evil */
    pgent_t tmp = pgent(vlpt_idx)->create_entry(get_kernel_space(), pgent_t::size_max, 
						virt_to_phys((addr_t) pgent(0)), true, true, false, false, false);

    pgent(vlpt_idx)->set_entry(this, pgent_t::size_max, tmp);
}

void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space);

    kernel_space = allocate_space();
    ASSERT(kernel_space);

    kernel_space->get_asid()->init();
    kernel_space->init_kernel_mappings();

    /* now allocate dummy tcb */
    ASSERT(!dummy_tcb);
    dummy_tcb = (tcb_t *) kmem.alloc(kmem_tcb, ALPHA_PAGE_SIZE);
    ASSERT(dummy_tcb);
}

space_t *allocate_space(void)
{
    addr_t page = kmem.alloc(kmem_space, ALPHA_PAGE_SIZE);
    ASSERT(page);
    return space_t::ptbr_to_space(page);
}

void free_space(space_t *space)
{
    addr_t page = (addr_t) space->get_ptbr();
    /* sjw (28/07/2002): What about the page tables? */
    kmem.free(kmem_space, page, ALPHA_PAGE_SIZE);
}

/**
 * initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
    int console_idx = page_table_index(pgent_t::size_max, (addr_t) CONSOLE_AREA_START),
	ktcb_idx = page_table_index(pgent_t::size_max, (addr_t) KTCB_AREA_START),
	vlpt_idx = page_table_index(pgent_t::size_max, (addr_t) VLPT_AREA_START);

    this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    get_asid()->init();

    /* Copy top level entries for the console and ktcb areas */
    pgent(console_idx)->set_entry(this, pgent_t::size_max, *get_kernel_space()->pgent(console_idx));
    pgent(ktcb_idx)->set_entry(this, pgent_t::size_max, *get_kernel_space()->pgent(ktcb_idx));

    /* Self-map the VPT */
    pgent_t tmp = pgent(vlpt_idx)->create_entry(this, pgent_t::size_max, virt_to_phys((addr_t) pgent(0)),
						true, true, false, false, false);
    
    pgent(vlpt_idx)->set_entry(this, pgent_t::size_max, tmp);
    
    /* map kip read-only to user */
    add_mapping(kip_area.get_base(), virt_to_phys((addr_t) get_kip()), PGSIZE_KIP, false, false);
}

/**
 * add a kernel mapping for a UTCB
 * @param utcb the user-visible address of the UTCB to add the mapping for
 * @return the kernel-accessible address of the UTCB
 */
utcb_t * space_t::map_utcb(utcb_t * utcb)
{
    /* sjw (25/07/2002):  Is this correct?  Should we not init it at least? */
    addr_t page = kmem.alloc(kmem_utcb, ALPHA_PAGE_SIZE);
    ASSERT(page);
    
    add_mapping((addr_t) utcb, virt_to_phys(page), PGSIZE_UTCB, true, false);

    return (utcb_t *) addr_offset(page, (word_t) utcb & (~ALPHA_PAGE_MASK));
}

/**
 * Release mappings that belong to the kernel (UTCB, KIP)
 * @param vaddr		virtual address in the space
 * @param paddr		physical address the mapping refers to
 * @param log2size	log2(size of mapping)
 */
void space_t::release_kernel_mapping(addr_t vaddr, addr_t paddr,
				     word_t log2size)
{
    if (get_utcb_page_area().is_addr_in_fpage (vaddr))
	kmem.free(kmem_utcb, phys_to_virt(paddr), 1UL << log2size);
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
    add_mapping(addr, addr, pgent_t::size_base, true, false);
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
    kernel_space->add_mapping(addr, virt_to_phys((addr_t) dummy_tcb), PGSIZE_KTCB, false, true);
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

    addr_t page = kmem.alloc (kmem_utcb, page_size (PGSIZE_UTCB));

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
    addr_t page = kmem.alloc(kmem_tcb, ALPHA_PAGE_SIZE);
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
    pgent_t *pg;
    pgent_t::pgsize_e pgsize;

    if(!lookup_mapping((addr_t) utcb, &pg, &pgsize)) {
	return NULL;
    }
    
    return (utcb_t *) (((word_t) utcb & ALPHA_OFFSET_MASK) + (word_t) phys_to_virt(pg->address(this, pgsize)));
}



void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    pgent_t * pg = this->pgent(page_table_index(pgsize, vaddr));

    /*
     * Sanity checking on page size
     */
    if (!is_page_size_valid(size))
    {
	printf("Mapping invalid pagesize (%dKB)\n", page_size(pgsize) >> 10);
	enter_kdebug("invalid page size");
	return;
    }

    /*
     * Lookup mapping
     */
    while (pgsize > size)
    {
//	pg->print(this, pgsize, (word_t) vaddr); 

	if (pg->is_valid(this, pgsize))
	{
	    // Sanity check
	    if (!pg->is_subtree(this, pgsize))
	    {
		printf("%dKB mapping @ %p space %p already exists.\n",
			page_size (pgsize) >> 10, vaddr, this);
		enter_kdebug("mapping exists");
		return;
	    } 
	}
	else
	    // Create subtree
	    pg->make_subtree(this, pgsize, kernel);

//	pg->print(this, pgsize, (word_t) vaddr);

	pg = pg->subtree(this, pgsize)->next(this, pgsize-1, page_table_index(pgsize-1, vaddr));
	pgsize--;
    }

    /*
     * Modify page table
     */
    
    /* XXX benjl: This is huge overkill. There are some specific times, where
     *  some specific TLB entries should be flushed. But I don't want to code
     * that yet. So for now we flush all entries, all the time
     */

    PAL::tbia();

    pg->set_entry(this, pgsize, paddr, true, writable, true, kernel);

//    printf("Final: ");	pg->print(this, pgsize, (word_t) vaddr);
}

extern "C" void handle_mm(addr_t va, word_t mmcsr, space_t::access_e access, alpha_context_t *ctx)
{
    tcb_t *current = get_current_tcb();
    space_t *space = current->get_space();

    if(space == NULL)
	space = get_kernel_space();

    /* sjw (30/07/2002): Do dirty and reference stuff here */
    space->handle_pagefault(va, (addr_t) ctx->pc, access, (ctx->ps & PAL_PS_USER) ? false : true);    
}
