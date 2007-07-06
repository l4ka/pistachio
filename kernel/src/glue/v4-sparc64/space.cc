/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/space.cc
 * Description:  space_t implementation for SPARC v9
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
 * $Id: space.cc,v 1.5 2004/05/21 02:34:56 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>			/* for UNIMPLEMENTED */
#include <kmemory.h>

#include INC_API(tcb.h)

#include INC_GLUE_API_ARCH(space.h)	/* space_t           */

DECLARE_KMEM_GROUP(kmem_utcb);
DECLARE_KMEM_GROUP(kmem_tcb);

space_t kernel_space;

/*************************
* space_t implementation *
*************************/

word_t space_t::space_control(word_t ctrl)
{
    /* The control field of space control is currently unused in sparc64 */
    return 0;

} // space_t::space_control()

/**
 * Release mappings that belong to the kernel (UTCB, KIP)
 * @param vaddr		virtual address in the space
 * @param paddr		physical address the mapping refers to
 * @param log2size	log2(size of mapping)
 */
void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}

utcb_t * space_t::allocate_utcb(tcb_t *tcb)
{
    ASSERT(tcb != NULL);
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    addr_t page;
    pgent_t::pgsize_e size;
    pgent_t *pgent;

    if (this->lookup_mapping(utcb, &pgent, &size)) {
	// there's already a mapping at the UTCB address
	// philipd (10/12/03): the mask is only right if it's an 8k page
	return (utcb_t*)
	    phys_to_virt(addr_offset(pgent->address(this, size),
				     (word_t)utcb & (~SPARC64_PAGE_MASK)));
    }

    // allocate a new UTCB page
    page = kmem.alloc_aligned(kmem_utcb, SPARC64_PAGE_SIZE,
			      (word_t)utcb, SPARC64_CACHE_ALIGN);
    ASSERT(page != NULL);
    // XXX size_8k is ultrasparc-specific
    add_mapping((addr_t)utcb, virt_to_phys(page), pgent_t::size_8k,
		tlb_t::cache_vir, true, false, false);

    return (utcb_t*) addr_offset(page, (word_t)utcb & (~SPARC64_PAGE_MASK));
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
    // philipd (10/12/03) XXX: use a superpage once they are working
    this->add_mapping(addr, addr, pgent_t::size_8k, tlb_t::cache_vir,
		      true, true, false);
}

/**
 * Try to copy a mapping from kernel space into the current space
 * @param addr the address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 * Synchronization must happen at the highest level, allowing sharing.
 */
bool space_t::sync_kernel_space(addr_t addr)
{
    // philipd (10/12/03) XXX: is this necessary?
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
    static tcb_t* dummy_tcb = NULL;

    if(dummy_tcb == NULL) {
	dummy_tcb = (tcb_t *) kmem.alloc(kmem_tcb, SPARC64_PAGE_SIZE);
    }

    add_mapping(addr, virt_to_phys((addr_t)dummy_tcb), PGSIZE_KTCB,
		tlb_t::cache_phy, false, false, true);
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
    addr_t page = kmem.alloc_aligned(kmem_tcb, SPARC64_PAGE_SIZE,
				     (word_t)addr, SPARC64_CACHE_ALIGN);
    kernel_space.add_mapping(addr, virt_to_phys(page), PGSIZE_KTCB,
			     tlb_t::cache_vir, true, false, true);
}
