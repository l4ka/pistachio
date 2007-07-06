/*********************************************************************
 *                
 * Copyright (C) 2003, 2006, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/ultrasparc/space.cc
 * Description:   
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
 * $Id: space.cc,v 1.7 2006/11/17 17:01:04 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <linear_ptab.h>

#include INC_GLUE_API_ARCH(space.h)
#include INC_CPU(tsb.h)
#include INC_API(kernelinterface.h)

EXTERN_KMEM_GROUP(kmem_space);

tsbarrays_t tsb_t::arrays SECTION(".tsb");
asid_cache_t asid_cache UNIT("cpulocal");

/************************
* System initialisation *
************************/

/**
 *  space_t::init_kernel_space()
 *  Initialize kernel space_t.
 *  Notes: The space_t fields kip_area, utcb_area and thread_count unused by
 *  kernel space_t.
 */
void SECTION(".init.memory")
space_t::init_kernel_space()
{
    extern space_t kernel_space;

    mmu_t::init();

    /* Allocate kernel page directory. */

    kernel_space.pgdir =
	(u64_t)kmem.alloc(kmem_space, sizeof(pgent_t) * PT_LEVEL1ENTRIES) >>
	SPARC64_PAGE_BITS;
    ASSERT(kernel_space.pgdir);

    /* Initialise TSB and pinned TSB kernel mappings for physical memory. */
    tsb_t tsb;

    tsb.init();

    /**
     *  Allocate asids and asid cache and kernel asid.
     *  Notes: The UltraSPARC implements 13 bits of ASID tag.
     *  context 0 is the hardware defined asid of the NUCLEUS or kernel context.
     *  context (NUM_CONTEXTS-1) is used to mark invalid TSB entries.
     */

    word_t last_asid;

    if(CONFIG_MAX_NUM_ASIDS < NUM_CONTEXTS - 1) {
	last_asid = CONFIG_MAX_NUM_ASIDS;
    } else {
	last_asid = NUM_CONTEXTS - 2;
    }

    get_asid_cache()->init();
    get_asid_cache()->set_valid(2, last_asid);

    kernel_space.asid.init_kernel(NUCLEUS_CONTEXT);
    kernel_space.context = NUCLEUS_CONTEXT;

    kernel_space->enqueue_spaces();
} // space_t::init_kernel_space()

/*************************
 * space_t implementation *
 *************************/

/**
 * initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    /* Allocate page directory. */
    pgdir =
	(u64_t)kmem.alloc(kmem_space, sizeof(pgent_t) * PT_LEVEL1ENTRIES) >>
	SPARC64_PAGE_BITS;
    ASSERT(pgdir);

    asid.init();

    /* Map the KIP */
    add_mapping(kip_area.get_base(), virt_to_phys((addr_t)get_kip()), 
		pgent_t::size_8k, tlb_t::cache_phy, false, false, false);
}


/**
 * Add a mapping to this address space
 */
void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
			  tlb_t::cache_attrib_e cache, bool writable,
			  bool executable, bool kernel)
{
    pgent_t * pg = this->pgent(page_table_index(pgent_t::size_max, vaddr), 0);
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    /* 
     * Sanity check page size 
     */
    // philipd (11/12/03) XXX: 8k only at first
    ASSERT(size == pgent_t::size_8k);

    /*
     * Lookup mapping
     */
    while (pgsize > size)
    {
	if (pg->is_valid (this, pgsize))
	{
	    if (! pg->is_subtree (this, pgsize))
	    {
		printf ("%dKB mapping @ %p space %p already exists.\n",
			page_size (pgsize) >> 10, vaddr, this);
		enter_kdebug ("mapping exists");

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
     * Modify page table and set TSB entries
     */
    pg->set_entry (this, pgsize, paddr,
		   4 | (writable ? 2 : 0) | (executable ? 1 : 0),
		   cache, kernel);
    if(executable) {
	pg->insert(this, pgsize, vaddr,
		   pgsize == pgent_t::size_8k ?
		   	tsb_t::i8k_tsb : tsb_t::i64k_tsb);
    }
    pg->insert(this, pgsize, vaddr,
	       pgsize == pgent_t::size_8k ? tsb_t::d8k_tsb : tsb_t::d64k_tsb);
}
