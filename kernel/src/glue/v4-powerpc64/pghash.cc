/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/pghash.cc
 * Description:	PowerPC64 page hash handler.
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
 * $Id: pghash.cc,v 1.9 2006/11/17 17:04:18 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <linear_ptab.h>

#include INC_API(kernelinterface.h)

#include INC_GLUE(pghash.h)
#include INC_GLUE(space.h)
#include INC_GLUE(pgent_inline.h)
#include INC_PLAT(prom.h)

pghash_t pghash;

/* If it exists, modify an entry in the Hash Table */
void pghash_t::update_mapping( space_t *s, addr_t vaddr, pgent_t *pgent, pgent_t::pgsize_e size )
{
#ifdef CONFIG_POWERPC64_LARGE_PAGES
    ASSERT((size == pgent_t::size_4k) || (size == pgent_t::size_16m));
#else
    ASSERT((size == pgent_t::size_4k));
#endif
    bool large = (size == pgent_t::size_4k) ? false : true;
    word_t vsid = s->get_vsid( vaddr );

    ppc64_pte_t *pte = get_htab()->locate_pte( (word_t)vaddr,
	    vsid, pgent->map.pteg_slot,
	    pgent->map.second_hash, large );

    if( pte )
    {
	pte->create( (word_t)vaddr, pgent->get_pte(s), vsid,
			pgent->map.second_hash, large , pte->x.bolted );
    }
    ppc64_invalidate_tlbe( vaddr, large );
}

void pghash_t::flush_mapping( space_t *s, addr_t vaddr, pgent_t *pgent, pgent_t::pgsize_e size )
{
#ifdef CONFIG_POWERPC64_LARGE_PAGES
    ASSERT((size == pgent_t::size_4k) || (size == pgent_t::size_16m));
#else
    ASSERT((size == pgent_t::size_4k));
#endif

    ppc64_pte_t *pte = get_htab()->locate_pte( (word_t)vaddr,
	    s->get_vsid(vaddr), pgent->map.pteg_slot,
	    pgent->map.second_hash, (size == pgent_t::size_4k) ? 0 : 1);

    if( pte )
	pte->raw.word0 = 0;
}

void pghash_t::insert_mapping( space_t *s, addr_t vaddr, pgent_t *pgent, pgent_t::pgsize_e size, bool bolted )
{
    word_t pteg_slot, is_second_hash;
    ppc64_pte_t *pte;
    word_t vsid = s->get_vsid( vaddr );
    bool large = (size == pgent_t::size_4k) ? false : true;

#ifdef CONFIG_POWERPC64_LARGE_PAGES
    ASSERT((size == pgent_t::size_4k) || (size == pgent_t::size_16m));
#else
    ASSERT((size == pgent_t::size_4k));
#endif

    pte = get_htab()->find_insertion( (word_t)vaddr, vsid,
	    &pteg_slot, &is_second_hash, large);

    // Check for a pre-existing, valid translation in the page hash.
    if( pte->x.v == 1 )
    {
	space_t *evict_space = space_t::lookup_space( pte->x.vsid );
	addr_t evict_addr = (addr_t)get_htab()->reverse_hash( pte );

	TRACEF( "pghash eviction: vaddr %x, space %x\n", 
		evict_addr, evict_space );

	// TODO: lock the evict space, to prevent collisions in the data
	// structures (SMP).
	pgent_t *evict_pgent;
	pgent_t::pgsize_e evict_size;

	// Flush in-flight updates to the translation.
	sync();

	// Update the page table's dirty + referenced bits.
	ASSERT( evict_space->lookup_mapping( evict_addr, &evict_pgent, &evict_size ) );

#ifdef CONFIG_POWERPC64_LARGE_PAGES
	evict_pgent->set_accessed( evict_space, pte->x.l ? pgent_t::size_16m : pgent_t::size_4k, pte->x.r );
	evict_pgent->set_dirty( evict_space, pte->x.l ? pgent_t::size_16m : pgent_t::size_4k, pte->x.c );
#else
	evict_pgent->set_accessed( evict_space, pgent_t::size_4k, pte->x.r );
	evict_pgent->set_dirty( evict_space,  pgent_t::size_4k, pte->x.c );
#endif
	
	/* Clear the PTE */
	pte->raw.word0 = 0;

	ppc64_invalidate_tlbe( evict_addr, pte->x.l );
    }

    // Insert a new translation.
    pte->create( (word_t)vaddr, pgent->get_pte(s), vsid, is_second_hash, large, bolted );
    pgent->map.pteg_slot = pteg_slot;
    pgent->map.second_hash = is_second_hash;
}


SECTION(".init") bool pghash_t::init( word_t tot_phys_mem )
{
    word_t size;
    word_t phys_start;

    // Try allocating memory for the page hash, starting with the optimal
    // size, and then by reducing the size by half.
    for( size = get_htab()->optimal_size(tot_phys_mem); 
	    size >= get_htab()->min_size();
	    size = size >> 1 )
    {
	// Search through phys memory for a location that fits the page
	// hash of a given size, and aligned to the size.
	for( phys_start = size; 
		phys_start < (tot_phys_mem - size); 
		phys_start += size )
	{
	    if( this->try_location(phys_start, size) )
	       	return this->finish_init( phys_start, size );
	}
    }

    return false;
}

SECTION(".init") bool pghash_t::try_location( word_t phys_start, word_t size )
{
    // We are relocating everything
    kernel_interface_page_t *kip = PTRRELOC(get_kip());
    word_t phys_end = phys_start + size;

    if ((word_t)kip->sigma0.mem_region.high > phys_start)
	return false;
    if ((word_t)kip->sigma1.mem_region.high > phys_start)
	return false;
    if ((word_t)kip->root_server.mem_region.high > phys_start)
	return false;

    // Walk through the KIP's memory descriptors and search for any
    // reserved memory regions that collide with our intended memory
    // allocation.
    for( word_t i = 0; i < kip->memory_info.get_num_descriptors(); i++ )
    {
	memdesc_t *mdesc = kip->memory_info.get_memdesc( i );
	if( (mdesc->type() == memdesc_t::conventional) || mdesc->is_virtual() )
	    continue;

	word_t low = (word_t)mdesc->low();
	word_t high = (word_t)mdesc->high();

	if( (phys_start < low) && (phys_end > high) )
	    return false;
	if( (phys_start >= low) && (phys_start < high) )
	    return false;
	if( (phys_end > low) && (phys_end <= high) )
	    return false;
    }

    // No console setup yet - go via prom
    prom_print_hex( "Installing hash table at", phys_start );
    prom_print_hex( ", size", size );
    prom_puts("\n\r");
    return true;
}

SECTION(".init") bool pghash_t::finish_init( word_t phys_start, word_t size )
{
    kernel_interface_page_t *kip = PTRRELOC(get_kip());
    addr_t virt_start = addr_align_up( (addr_t)PGHASH_AREA_START, size );

    // Insert a KIP memory descriptor to protect the page hash.
    kip->memory_info.insert( memdesc_t::reserved, false,
	    (addr_t)phys_start, (addr_t)(phys_start + size) );

    // Initialize the page hash at the given location.
    PTRRELOC(get_htab())->init( phys_start, (word_t)virt_start, size );

    prom_print_hex( "Setup hash table at virtual", (word_t)virt_start | phys_start );
    prom_print_hex( ", physical", phys_start );
    prom_puts("\n\r");

    pgent_t pg;

    prom_puts( "Inserting bolted hash table mappings\n\r" );

#ifdef CONFIG_POWERPC64_LARGE_PAGES
    pgent_t::pgsize_e pgsize = pgent_t::size_16m;
#else
    pgent_t::pgsize_e pgsize = pgent_t::size_4k;
#endif

    /* Insert mappings for hash page table */
    for ( word_t i = 0; i < (size); i += page_size(pgsize))
    {
	/* Create a dummy page table entry */
	pg.set_entry( get_kernel_space(), pgsize,
			(addr_t)(phys_start + i),
		      6, pgent_t::l4default, true );

	insert_mapping( get_kernel_space(),
			(addr_t)(((word_t)virt_start | phys_start) + i),
			&pg, pgsize, true );
    }

    return true;
}

