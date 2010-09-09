/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/space.cc
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
 * $Id$
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <generic/lib.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)
#include INC_ARCH(swtlb.h)

DECLARE_TRACEPOINT(hash_miss_cnt);
DECLARE_TRACEPOINT(hash_insert_cnt);
DECLARE_TRACEPOINT(ptab_4k_map_cnt);

DECLARE_KMEM_GROUP(kmem_utcb);
EXTERN_KMEM_GROUP(kmem_pgtab);
EXTERN_KMEM_GROUP(kmem_space);

#define TRACE_SPACE(x...)
//#define TRACE_SPACE(x...)	TRACEF(x)

space_t *kernel_space = NULL;

void space_t::allocate_tcb(addr_t addr)
{
#if !defined(CONFIG_STATIC_TCBS)
    /* Remove the dummy tcb mapping. We could have a dummy tcb
     * mapping due to someone sending invalid parameters to system
     * calls
     */
    pgent_t *pgent = kernel_space->page_lookup( addr );
    if( pgent && pgent->is_valid(this, pgent_t::size_4k) )
	kernel_space->flush_mapping( addr, pgent_t::size_4k, pgent );

    addr_t page = kmem.alloc( kmem_tcb, POWERPC_PAGE_SIZE );
    ASSERT(page);

    TRACE_SPACE( "new tcb, kmem virt %p, phys %p, tcb virt %p\n", page, 
	         virt_to_phys(page), addr);
    kernel_space->add_mapping( addr, virt_to_phys(page), pgent_t::size_4k, true, true);

    sync_kernel_space( addr );
#endif
}

void space_t::map_dummy_tcb(addr_t addr)
{
#if !defined(CONFIG_STATIC_TCBS)
    add_mapping( addr, (addr_t)virt_to_phys(get_dummy_tcb()), pgent_t::size_4k, false, true );
#endif
}

void space_t::add_mapping( addr_t vaddr, paddr_t paddr, pgent_t::pgsize_e size, 
			   bool writable, bool kernel, word_t attrib )
{
    pgent_t * pg = this->pgent (page_table_index (pgent_t::size_max, vaddr), 0);
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    ASSERT(is_page_size_valid(size));

    /* Lookup mapping */
    while (pgsize > size)
    {
	if (!pg->is_valid (this, pgsize))
	    pg->make_subtree (this, pgsize, kernel);
	else
	{
	    ASSERT(pg->is_subtree(this, pgsize) );
	}

	pg = pg->subtree (this, pgsize)->next
	    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
	pgsize--;
    }

    /* Modify page table */
    pg->set_entry( this, pgsize, paddr, writable ? 7 : 5, attrib, kernel);

#ifdef CONFIG_PPC_MMU_SEGMENT
    ASSERT(pgsize == pgent_t::size_4k);
    get_pghash()->insert_4k_mapping( this, vaddr, pgent);
#endif
}

#if 0
void space_t::add_4k_mapping( addr_t vaddr, paddr_t paddr, 
	bool writable, bool kernel, word_t attrib )
{
    pgent_t *pgent = this->pgent( page_table_index(pgent_t::size_4m, vaddr) );
    if( !pgent->is_valid(this, pgent_t::size_4m))
	pgent->make_subtree( this, pgent_t::size_4m, kernel );

    pgent = pgent->subtree( this, pgent_t::size_4m )->next( this, 
	    pgent_t::size_4k, page_table_index(pgent_t::size_4k, vaddr) );

    pgent->set_entry( this, pgent_t::size_4k, paddr, writable ? 7 : 5, 
		      attrib, kernel);

#ifdef CONFIG_PPC_MMU_SEGMENT
    get_pghash()->insert_4k_mapping( this, vaddr, pgent);
#endif
}
#endif

void space_t::flush_mapping( addr_t vaddr, pgent_t::pgsize_e pgsize, pgent_t *pgent )
{
#ifdef CONFIG_PPC_MMU_SEGMENT
    ASSERT(pgsize == pgent_t::size_4k);
    get_pghash()->flush_4k_mapping( this, vaddr, pgent );
#endif
    ppc_invalidate_tlbe( vaddr );
}

pgent_t * space_t::page_lookup( addr_t vaddr )
{
    pgent_t *pgent = this->pgent( page_table_index(pgent_t::size_4m, vaddr) );
    if( !pgent->is_valid(this, pgent_t::size_4m) )
	return NULL;

    pgent = pgent->subtree( this, pgent_t::size_4m );
    pgent = pgent->next( this, pgent_t::size_4k, 
	    page_table_index(pgent_t::size_4k, vaddr) );
    return pgent;
}

/**********************************************************************
 *                               Allocation
 **********************************************************************/

space_t * space_t::allocate_space() 
{
    space_t * space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(space);
    return space;
}

void space_t::free_space(space_t *space) 
{
    kmem.free(kmem_space, (addr_t)space, sizeof(space_t));
}

/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != POWERPC_PAGE_SIZE)
# error invalid kernel page size - please adapt
#endif

void SECTION(".init.memory") space_t::init_kernel_space()
{
    kernel_space = space_t::allocate_space();
    kernel_space->init_kernel_mappings();
}

addr_t space_t::map_device( paddr_t paddr, word_t size, bool kernel, word_t attrib)
{
    addr_t start_addr = (addr_t)(DEVICE_AREA_START + DEVICE_AREA_BAT_SIZE);

    /* Ensure that the size is a multiple of the page size. */
    if( size % POWERPC_PAGE_SIZE )
	size = (size + POWERPC_PAGE_SIZE) & POWERPC_PAGE_MASK;

    /* Search for the first available page.
     */
    while( 1 )
    {
	// Look for a 2nd level page table.
	word_t pdir_idx = page_table_index( pgent_t::size_4m, start_addr );
	pgent_t *pgent = this->pgent( pdir_idx );
	if( !pgent->is_valid(this, pgent_t::size_4m) )
	    goto found;

	// Move to the starting position in the 2nd level page table.
	pgent = pgent->subtree( this, pgent_t::size_4m );
	word_t ptab_idx = page_table_index( pgent_t::size_4k, start_addr );
	pgent = pgent->next( this, pgent_t::size_4k, ptab_idx );

	// Search the page table for an unused entry.
	do {
	    if( !pgent->is_valid(this, pgent_t::size_4k) )
		goto found;

	    // Increment the address to the next page.
	    start_addr = addr_offset( start_addr, POWERPC_PAGE_SIZE );
	    if( ((word_t)start_addr + size) > DEVICE_AREA_END )
		return NULL;

	    // Move to the next page table entry.
	    pgent = pgent->next( this, pgent_t::size_4k, 1 );
	    ptab_idx++;
	} while( ptab_idx < POWERPC_PAGE_SIZE/sizeof(pgent_t) );
    }

found:

    /* TODO: flush the cache for the mapped region (to avoid cache paradoxes)!
     */

    // Add 4k device mappings for the entire region.
    TRACE_SPACE( "device mapping 0x%x --> 0x%x, size 0x%x\n", 
	          paddr, start_addr, size );

    for( word_t page = 0; page < size; page += POWERPC_PAGE_SIZE ) 
    {
	this->add_mapping( addr_offset(start_addr, page),
		paddr + page, pgent_t::size_4k, true, kernel, attrib );
    }

    return start_addr;
}

/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/


void space_t::release_kernel_mapping (addr_t vaddr, paddr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, (addr_t) phys_to_virt (paddr), 1UL << log2size);
}

utcb_t *space_t::allocate_utcb( tcb_t *tcb )
{
    ASSERT(tcb);
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    addr_t page;

    pgent_t *pgent = this->page_lookup( utcb );
    if( pgent && pgent->is_valid(this, pgent_t::size_4k) )
	// Already a valid page mapped at the UTCB address.
	page = (addr_t) phys_to_virt( pgent->address(this, pgent_t::size_4k) );
    else
    {
	// Allocate a new UTCB page.
	page = kmem.alloc( kmem_utcb, POWERPC_PAGE_SIZE );
	if( page == NULL )
	{
	    WARNING( "out of memory!\n" );
	    return NULL;
	}
	add_mapping( utcb, (paddr_t)virt_to_phys(page), pgent_t::size_4k, true, false );
    }

    return (utcb_t *)addr_offset( page, (word_t)utcb & ~POWERPC_PAGE_MASK );
}

void space_t::map_sigma0(addr_t addr)
{
    ASSERT( 
	    ((addr < get_kip()->reserved_mem0.low) 
	     || (addr >= get_kip()->reserved_mem0.high))
    	    && 
	    ((addr < get_kip()->reserved_mem1.low) 
	     || (addr >= get_kip()->reserved_mem1.high)) 
	    );

    add_mapping( addr, (paddr_t)addr, pgent_t::size_4k, true, false );
}

word_t space_t::space_control (word_t ctrl)
{
    word_t oldctrl = 0;
#ifdef CONFIG_X_PPC_SOFTHVM
    oldctrl |= this->hvm_mode ? 1 : 0;

    /* XXX: HVM mode can only be changed when space is uninitialized */
    if ((ctrl & 1) && !this->hvm_mode) 
    {
	TRACEF("Enabling HVM mode\n");
	this->hvm_mode = true;
    }
#endif
    return oldctrl;
}
