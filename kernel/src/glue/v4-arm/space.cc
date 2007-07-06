/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-arm/space.cc
 * Description:	ARM space_t implementation.
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
 * $Id: space.cc,v 1.26 2006/11/17 17:14:30 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>			/* for UNIMPLEMENTED	*/
#include <linear_ptab.h>
#include INC_API(space.h)		/* space_t		*/
#include INC_API(tcb.h)
#include INC_ARCH(pgent.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(config.h)
#include INC_ARCH(fass.h)
#include INC_CPU(syscon.h)

DECLARE_KMEM_GROUP(kmem_utcb);
DECLARE_KMEM_GROUP(kmem_tcb);
EXTERN_KMEM_GROUP (kmem_space);

tcb_t * dummy_tcb = NULL;

/* The kernel space is statically defined beause it is needed
 * before the virtual memory has been setup or the kernel
 * memory allocator.
 */
char UNIT("kspace") kernel_space_object[sizeof(space_t)] __attribute__((aligned(ARM_L1_SIZE)));

space_t *kernel_space = (space_t*)((word_t)&kernel_space_object);

#ifdef CONFIG_ENABLE_FASS
space_t *cpd;
#endif

void SECTION(".init") init_kernel_space()
{
    TRACE_INIT("Initialising kernel space @ %p...\n\r", kernel_space);

    kernel_space->init_kernel_mappings();

    dummy_tcb = (tcb_t *)kmem.alloc(kmem_tcb, KTCB_SIZE);
 
#ifdef CONFIG_ENABLE_FASS
    cpd = (space_t *)virt_to_page_table(kernel_space);
    arm_fass.init();
#endif 
}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init") space_t::init_kernel_mappings()
{
    /* Kernel space's mappings already setup in initial root page table */
}

/**
 * initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    word_t i;
    pgent_t *pg_to = pgent(USER_AREA_SECTIONS);
    pgent_t *pg_from = get_kernel_space()->pgent(USER_AREA_SECTIONS);

    for (i=0; i < (KTCB_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;
    for (i=0; i < (KERNEL_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;
    for (i=0; i < (UNCACHE_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;
    for (i=0; i < (COPY_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;

    pg_to += VAR_AREA_SECTIONS;
    pg_from += VAR_AREA_SECTIONS;

    for (i=0; i < (IO_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;
    for (i=0; i < (MISC_AREA_SECTIONS); i++)
	*pg_to++ = *pg_from++;

    *pg_to = *pg_from;	/* high_int_vector */

#ifdef CONFIG_ENABLE_FASS
    set_domain(INVALID_DOMAIN);
#endif 

    utcb_area.mem.x.execute = utcb_area.mem.x.write = 0;
    kip_area.mem.x.execute = kip_area.mem.x.write = 0;
    pt.pd_split.utcb_area = utcb_area;
    pt.pd_split.kip_area = kip_area;

    /* Map the kip (user read only) */
    add_mapping(kip_area.get_base(), 
            virt_to_phys((addr_t) get_kip()), KIP_KIP_PGSIZE, false, false);
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
 * Allocate a UTCB
 * @param tcb   Owner of the utcb
 */
utcb_t * space_t::allocate_utcb(tcb_t * tcb) 
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

    addr_t page = kmem.alloc (kmem_utcb, page_size (UTCB_AREA_PGSIZE)); 
    arm_cache::flush_dcache_ent(page, PAGE_BITS_4K);

    add_mapping((addr_t) utcb, virt_to_phys(page),
		UTCB_AREA_PGSIZE, true, false);

    return (utcb_t *)
	addr_offset (page, addr_mask (utcb, page_size (UTCB_AREA_PGSIZE) - 1));
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
    kernel_space->add_mapping(addr, virt_to_phys((addr_t)dummy_tcb),
		    KTCB_PGSIZE, false, true);
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
    addr_t page = kmem.alloc(kmem_tcb, KTCB_SIZE);
//    arm_cache::flush_dcache_ent(page, KTCB_BITS);
    arm_cache::flush_dcache_ent(page, KTCB_BITS*2);

    kernel_space->add_mapping(addr, virt_to_phys(page), KTCB_PGSIZE, true, 
            true);
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
    /* (ht) appears unused */
    UNIMPLEMENTED();

#if 0
    pgent_t::pgsize_e pgsize;
    pgent_t *pg;

    if (!lookup_mapping((addr_t) utcb, &pg, &pgsize))
    {
        return NULL;
    }

    return (utcb_t *) (((word_t) utcb & ARM_SMALL_MASK) + 
            (word_t) phys_to_virt(pg->address(this, pgsize)));
#endif
}

/**
 * reads a word from a given physical address, uses a remap window and
 * maps a 4MB page for the access
 *
 * @param paddr		physical address to read from
 * @return the value at the given address
 */
word_t space_t::readmem_phys (addr_t paddr)
{
    /* XXX this needs to be optimized */
    word_t data;

    space_t *space = (space_t *)read_cp15_register(C15_ttbase,
				C15_CRm_default, C15_OP2_default);
    space = phys_to_virt(space);

    arm_cache::cache_flush(); /* Only really used in kdb afaict */

    space->add_mapping((addr_t) PHYSMAPPING_VADDR,
		    (addr_t)((word_t)paddr & (~(PAGE_SIZE_1M-1))),
		    pgent_t::size_1m, false, true, true);

    data = *(word_t*)(PHYSMAPPING_VADDR + ((word_t)paddr & (PAGE_SIZE_1M-1)));

    space->remove_mapping((addr_t) PHYSMAPPING_VADDR, pgent_t::size_1m, true);

    return data;
}

void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
                bool writeable, bool kernel, bool uncached)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    pgent_t *pg = this->pgent(page_table_index(pgsize, vaddr));

    //TRACEF("vaddr = %p paddr = %p %d\n", vaddr, paddr, size);

    if (!is_page_size_valid(size)) 
    {
        enter_kdebug("invalid page size");
        return;
    }

    while (pgsize > size)
    {
        if (pg->is_valid(this, pgsize))
        {
            if (!pg->is_subtree(this, pgsize))
            {
                ASSERT(!"mapping exists");
                enter_kdebug("mapping exists");
                return;
            }
        }
        else {
                pg->make_subtree(this, pgsize, kernel);
        }

        pg = pg->subtree(this, pgsize)->next(this, pgsize-1, 
                page_table_index(pgsize-1, vaddr));

        pgsize--;
    }

    pg->set_entry(this, pgsize, paddr, writable ? 7 : 5,
		  uncached ? 1 : 0, kernel);

    flush_tlbent(this, vaddr, page_shift(size));
}

void space_t::remove_mapping(addr_t vaddr, pgent_t::pgsize_e size, bool kernel)
{
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    pgent_t *pg = this->pgent(page_table_index(pgsize, vaddr));

    if (!is_page_size_valid(size)) 
    {
        enter_kdebug("invalid page size");
        return;
    }

    while (pgsize > size)
    {
        if (pg->is_valid(this, pgsize))
        {
            if (!pg->is_subtree(this, pgsize))
            {
                ASSERT(!"mapping >size exists");
                enter_kdebug("larger mapping exists");
                return;
            }
        }
        else {
	    ASSERT(!"mapping not present");
        }

        pg = pg->subtree(this, pgsize)->next(this, pgsize-1, 
                page_table_index(pgsize-1, vaddr));

        pgsize--;
    }

    pg->clear(this, pgsize, kernel);

    flush_tlbent(this, vaddr, page_shift(size));
}

space_t * allocate_space()
{
    space_t * space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t));

    if (!space)
	return NULL;
 
    /* kmem.alloc zeros out the page, in cached memory. Since we'll be using
     * this for uncached accesses, need to flush this out now.
     */
    arm_cache::flush_dcache_ent(space, ARM_SECTION_BITS+2);

    return space;
}

void free_space(space_t * space)
{
    ASSERT(space);

#ifdef CONFIG_ENABLE_FASS
    if (space->get_domain() != INVALID_DOMAIN)
        arm_fass.set_space(space->get_domain(), NULL);
#endif
    /* srXXX: This function does not exist (any more?). */
    //space->dequeue_spaces();

    kmem.free(kmem_space, (addr_t)space, sizeof(space_t));
}

void space_t::flush_tlb(space_t *curspace)
{
    arm_cache::tlb_flush();
}

void space_t::flush_tlbent(space_t *curspace, addr_t vaddr, word_t log2size)
{
    arm_cache::tlb_flush_ent(vaddr, log2size);
}
