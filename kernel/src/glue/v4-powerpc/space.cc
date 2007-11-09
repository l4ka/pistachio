/*********************************************************************
 *                
 * Copyright (C) 2002, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/space.cc
 * Description:   address space management
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
 * $Id: space.cc,v 1.53 2007/01/14 21:36:19 uhlig Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(page.h)
#include INC_ARCH(pghash.h)
#include INC_ARCH(pgtab.h)
#include INC_ARCH(phys.h)

#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)
#include INC_GLUE(pgent_inline.h)

#include INC_PLAT(ofppc.h)

DECLARE_TRACEPOINT(hash_miss_cnt);
DECLARE_TRACEPOINT(hash_insert_cnt);
DECLARE_TRACEPOINT(ptab_4k_map_cnt);

DECLARE_KMEM_GROUP(kmem_utcb);
DECLARE_KMEM_GROUP(kmem_tcb);
EXTERN_KMEM_GROUP(kmem_pgtab);
EXTERN_KMEM_GROUP(kmem_space);

#define TRACE_SPACE(x...)
//#define TRACE_SPACE(x...)	TRACEF(x)

space_t *kernel_space = NULL;
tcb_t *dummy_tcb = NULL;

static tcb_t * get_dummy_tcb()
{
    if (!dummy_tcb)
    {
	dummy_tcb = (tcb_t*)kmem.alloc( kmem_tcb, POWERPC_PAGE_SIZE );
	ASSERT(dummy_tcb);
	dummy_tcb = virt_to_phys(dummy_tcb);
    }
    return dummy_tcb;
}

bool space_t::handle_hash_miss( addr_t vaddr )
{
    TRACEPOINT(hash_miss_cnt, "hash_miss_cnt");

    pgent_t *pgent = this->page_lookup( vaddr );
    if( !pgent || !pgent->is_valid(this, pgent_t::size_4k) )
	return false;

    TRACEPOINT(hash_insert_cnt, "hash_insert_cnt");

    get_pghash()->insert_4k_mapping( this, vaddr, pgent );
    return true;
}

void space_t::add_4k_mapping( addr_t vaddr, addr_t paddr, 
	bool writable, bool kernel, bool cacheable )
{
    pgent_t *pgent = this->pgent( page_table_index(pgent_t::size_4m, vaddr) );
    if( !pgent->is_valid(this, pgent_t::size_4m))
	pgent->make_subtree( this, pgent_t::size_4m, kernel );

    pgent = pgent->subtree( this, pgent_t::size_4m )->next( this, 
	    pgent_t::size_4k, page_table_index(pgent_t::size_4k, vaddr) );

    pgent->set_entry( this, pgent_t::size_4k, paddr, writable ? 7 : 5, 
		      cacheable, kernel);

    get_pghash()->insert_4k_mapping( this, vaddr, pgent);
}

void space_t::flush_4k_mapping( addr_t vaddr, pgent_t *pgent )
{
    get_pghash()->flush_4k_mapping( this, vaddr, pgent );
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
 *
 *                         System initialization 
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != POWERPC_PAGE_SIZE)
# error invalid kernel page size - please adapt
#endif

void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space);

    ASSERT(sizeof(space_t) <= POWERPC_PAGE_SIZE);
    kernel_space = (space_t *)kmem.alloc( kmem_space, sizeof(space_t) );
    ASSERT(kernel_space);

    TRACE_SPACE( "allocated space of size %x @ %p\n",
	         sizeof(space_t), kernel_space);

    ASSERT(!dummy_tcb);

    kernel_space->init_kernel_mappings();
}

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
#if !defined(CONFIG_PPC_BAT_SYSCALLS)
    mem_region_t syscall_region;

    /* Figure out the range of the syscall code to be mapped into the user
     * space.
     */
    syscall_region.low = addr_align( (addr_t)ofppc_syscall_start(), 
	    POWERPC_PAGE_SIZE );
    syscall_region.high = addr_align_up( (addr_t)ofppc_syscall_end(), 
	    POWERPC_PAGE_SIZE );

    /* Create mappings for the system calls, so user space can access them
     * (after a sync_kernel_space()).
     */
    addr_t page = syscall_region.low;
    while( page < syscall_region.high ) 
    {
	add_4k_mapping( page, virt_to_phys(page), false, true, true );
	page = addr_offset( page, POWERPC_PAGE_SIZE );
    }
#endif

    /* Create user-visible mappings for the KIP, so that the SystemClock 
     * system-call can read the processor speed from user-level.  This 
     * mapping obviously is not intended to be directly accessed by 
     * applications.
     */
    for( addr_t page = get_kip(); 
	    page != ofppc_kip_end();
	    page = addr_offset(page, POWERPC_PAGE_SIZE) )
    {
	add_4k_mapping( page, virt_to_phys(page), false, true, true );
    }
}

SECTION(".init.memory") addr_t space_t::map_device( addr_t paddr, word_t size, 
	bool cacheable )
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
	this->add_4k_mapping( addr_offset(start_addr, page),
		addr_offset(paddr, page), true, true, cacheable );
    }

    return start_addr;
}

/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/

bool space_t::sync_kernel_space(addr_t addr)
{
    if( this == kernel_space )
	return false;

    word_t pdir_idx = page_table_index( pgent_t::size_4m, addr );
    pgent_t *our_pgent = this->pgent( pdir_idx );
    pgent_t *kernel_pgent = get_kernel_space()->pgent( pdir_idx );

    /* If the target space already has a page entry for this address,
     * or if it is an invalid kernel page description, then
     * return false.
     */
    if( our_pgent->is_valid(this, pgent_t::size_4m) || 
	    !kernel_pgent->is_valid(get_kernel_space(), pgent_t::size_4m) )
	return false;

    /* Copy the kernel mapping to the target space.
     */
    our_pgent->raw = kernel_pgent->raw;
    return true;
}

/**
 * space_t::init initializes the space_t
 *
 * maps the kernel area and initializes shadow ptabs etc.
 */
void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
    /* Copy the kernel area's page directory into the user's page directory.
     * This is an optimization.  It could be done lazily instead.
     * TODO: how much should we precopy?  (this also preloads the page hash).
     */
    addr_t addr = (addr_t)KTCB_AREA_START;
    while( addr < (addr_t)KTCB_AREA_END ) {
	this->sync_kernel_space( addr );
	addr = addr_offset( addr, PPC_PAGEDIR_SIZE );
    }

    this->x.utcb_area = utcb_area;
    this->x.kip_area = kip_area;
    this->add_4k_mapping( kip_area.get_base(), virt_to_phys(get_kip()), 
	    false, false );
}

void space_t::allocate_tcb(addr_t addr)
{
    /* Remove the dummy tcb mapping. We could have a dummy tcb
     * mapping due to someone sending invalid parameters to system
     * calls
     */
    pgent_t *pgent = kernel_space->page_lookup( addr );
    if( pgent && pgent->is_valid(this, pgent_t::size_4k) )
	kernel_space->flush_4k_mapping( addr, pgent );

    addr_t page = kmem.alloc( kmem_tcb, POWERPC_PAGE_SIZE );
    ASSERT(page);

    TRACE_SPACE( "new tcb, kmem virt %p, phys %p, tcb virt %p\n", page, 
	         virt_to_phys(page), addr);
//    pgent = kernel_space->add_4k_mapping( pgent, addr, virt_to_phys(page), space_t::tcb );
    kernel_space->add_4k_mapping( addr, virt_to_phys(page), true, true);

    sync_kernel_space( addr );
}

void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}

utcb_t *space_t::allocate_utcb( tcb_t *tcb )
{
    ASSERT(tcb);
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    addr_t page;

    pgent_t *pgent = this->page_lookup( utcb );
    if( pgent && pgent->is_valid(this, pgent_t::size_4k) )
	// Already a valid page mapped at the UTCB address.
	page = phys_to_virt( pgent->address(this, pgent_t::size_4k) );
    else
    {
	// Allocate a new UTCB page.
	page = kmem.alloc( kmem_utcb, POWERPC_PAGE_SIZE );
	if( page == NULL )
	{
	    WARNING( "out of memory!\n" );
	    return NULL;
	}
	add_4k_mapping( utcb, virt_to_phys(page), true, false );
    }

    return (utcb_t *)addr_offset( page, (word_t)utcb & ~POWERPC_PAGE_MASK );
}

void space_t::map_dummy_tcb(addr_t addr)
{
//    add_4k_mapping( NULL, addr, (addr_t)get_dummy_tcb(), space_t::dummytcb );
    add_4k_mapping( addr, (addr_t)get_dummy_tcb(), false, true );
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

//    add_4k_mapping( NULL, addr, addr, space_t::sigma0 );
    add_4k_mapping( addr, addr, true, false );
}

