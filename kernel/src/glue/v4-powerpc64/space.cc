/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/space.cc
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
 * $Id: space.cc,v 1.13 2006/11/17 17:04:18 skoglund Exp $
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
//#include INC_ARCH(phys.h)

#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)
#include INC_GLUE(pgent_inline.h)

DECLARE_TRACEPOINT(hash_miss_cnt);
DECLARE_TRACEPOINT(hash_insert_cnt);
//DECLARE_TRACEPOINT(ptab_4k_map_cnt);

DECLARE_KMEM_GROUP(kmem_utcb);
DECLARE_KMEM_GROUP(kmem_tcb);
EXTERN_KMEM_GROUP(kmem_pgtab);
EXTERN_KMEM_GROUP(kmem_space);

#define TRACE_SPACE(x...)
//#define TRACE_SPACE(x...)	TRACEF(x)

/* The kernel space is statically defined beause it is needed
 * before the virtual memory has been setup and the kernel
 * memory allocator.
 */
char kernel_space_object[sizeof(space_t)] __attribute__((aligned(POWERPC64_PAGE_SIZE)));
#if CONFIG_POWERPC64_STAB
char kernel_space_segment_table[POWERPC64_STAB_SIZE] __attribute__((aligned(POWERPC64_STAB_SIZE)));
#endif

space_t *kernel_space = (space_t*)&kernel_space_object;
tcb_t *dummy_tcb = NULL;

INLINE word_t pagedir_idx (addr_t addr)
{
    return page_table_index (pgent_t::size_max, addr);
}

INLINE tcb_t * get_dummy_tcb()
{
    return dummy_tcb;
}

bool space_t::handle_hash_miss( addr_t vaddr )
{
    pgent_t *pg;
    pgent_t::pgsize_e pgsize;

    TRACEPOINT( hash_miss_cnt,
	printf ( "hash miss @ %p (current=%p, space=%p)\n",
	vaddr, get_current_tcb(), this ));

    if ( this->lookup_mapping( vaddr, &pg, &pgsize ) )
    {
	TRACEPOINT( hash_insert_cnt );

	get_pghash()->insert_mapping( this, vaddr, pg, pgsize );

	return true;
    }

    return false;
}

bool space_t::handle_protection_fault( addr_t vaddr, bool dsi )
{
    pgent_t *pg;
    pgent_t::pgsize_e pgsize;

    // Check if mapping exist in page table
    if ( this->lookup_mapping ( vaddr, &pg, &pgsize) )
    {
	/* If data exception - check if write access denied */
	if (dsi)
	{
	    // Is it writeable (rights have changed)
	    if (pg->is_writable (this, pgsize))
	    {
		get_pghash()->update_mapping( this, vaddr, pg, pgsize );
		return true;
	    }
	} else {    /* Instruction access */
	    // Check if rights have been updated
	    if (pg->is_executable( this, pgsize ))
	    {
		get_pghash()->update_mapping( this, vaddr, pg, pgsize );
		return true;
	    }
	}
    }

    return false;
}

void space_t::add_mapping( addr_t vaddr, addr_t paddr,
		    bool writable, bool executable,
		    bool kernel, pgent_t::pgsize_e size )
{
    pgent_t * pg = this->pgent( pagedir_idx (vaddr), 0 );
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    //TRACEF("space %p: vaddr = %p, pgent = %p, size = %d\n", this, vaddr, pg, size);
    /*
     * Sanity check size
     */
#ifdef CONFIG_POWERPC64_LARGE_PAGES
    ASSERT((size == pgent_t::size_4k) || (size == pgent_t::size_16m));
#else
    ASSERT((size == pgent_t::size_4k));
#endif

    /*
     * Lookup mapping
     */
    while (1) {
	if ( pg->is_valid( this, pgsize ) )
	{
	    // Sanity check
	    if ( !pg->is_subtree(this, pgsize) )
	    {
		printf ("%dKB mapping @ %p space %p already exists.\n",
			page_size (pgsize) >> 10, vaddr, this);
		enter_kdebug ("mapping exists");
		return;
	    }
	}

	if ( pgsize == size )
	    break;

	// Create subtree
	if ( !pg->is_valid( this, pgsize ) )
	    pg->make_subtree( this, pgsize, kernel );

	pg = pg->subtree( this, pgsize )->next
	    ( this, pgsize-1, page_table_index( pgsize-1, vaddr ) );
	pgsize--;
    }

    /*
     * Modify page table
     */
    pg->set_entry (this, pgsize, paddr,
		   4 | (writable ? 2 : 0) | (executable ? 1 : 0),
		   pgent_t::l4default, kernel );

    get_pghash()->insert_mapping( this, vaddr, pg, size );
}

/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!dummy_tcb);
    dummy_tcb = (tcb_t*)kmem.alloc( kmem_tcb, POWERPC64_PAGE_SIZE );
    ASSERT(dummy_tcb);
    dummy_tcb = virt_to_phys(dummy_tcb);

    TRACE_SPACE( "initialised kernel space of size %x @ %p\n",
	         sizeof(space_t), kernel_space);

    kernel_space->init_kernel_mappings();
}

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    /* Initialize the ASID cache */
    get_vsid_asid_cache()->init( this );

    /* XXX Insert kernel into linear page table?
     * we should never fault on it. */
}

/* We need to map the kernel early.
 * This mapping is bolted(not replaceable).
 * This is because the kernel memory allocator has not started.
 */
void SECTION(".init.memory") early_kernel_map()
{
    pgent_t pg;

    // XXX - use a block zero function
    for ( word_t i = 0; i < sizeof(space_t); i+= 8 )
	*(word_t *)((word_t) kernel_space + i) = 0;

#if CONFIG_POWERPC64_STAB
    /* Setup the kernel segment table */
    ppc64_stab_t *stab = kernel_space->get_seg_table();
    /* Set the ASR directly, with the valid bit */
    *(word_t *)stab = (word_t)virt_to_phys(&kernel_space_segment_table) | 1;

    // XXX - use a block zero function
    for ( word_t i = 0; i < POWERPC64_STAB_SIZE; i+= 8 )
	*(word_t *)((word_t)&kernel_space_segment_table + i) = 0;
#else
    /* Set the initial Address Space ASID to 0
     * as we are running on the kernel ASID
     */
    asm volatile ( "mtasr   %0;" :: "r" (0) );
#endif

#ifdef CONFIG_POWERPC64_LARGE_PAGES
    /* Create a dummy page table entry */
    pg.set_entry( kernel_space, pgent_t::size_16m, 0,
		  7, pgent_t;:l4default, true );
    /* Insert the kernel mapping, bolted */
    get_pghash()->insert_mapping( kernel_space, (addr_t) KERNEL_OFFSET,
			&pg, pgent_t::size_16m, true );

#else
    /* Insert mappings for kernel (16MB area) */
    for ( word_t i = 0; i < (MB(16)); i += page_size(pgent_t::size_4k) )
    {
	/* Create a dummy page table entry */
	pg.set_entry( kernel_space, pgent_t::size_4k,
		      (addr_t)(i), 7, pgent_t::l4default, true );

	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( kernel_space,
			(addr_t)(KERNEL_OFFSET + i), &pg, pgent_t::size_4k, true );
    }
#endif
}

/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/

/**
 * space_t::init initializes the space_t
 *
 * maps the kernel area and initializes shadow ptabs etc.
 */
void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
    //TRACEF("uctb %p, kip %p\n", utcb_area,kip_area);

    this->x.utcb_area = utcb_area;
    this->x.kip_area = kip_area;
    this->x.vsid_asid.init();

#if CONFIG_POWERPC64_STAB
    this->x.segment_table.init();
#endif

    /* Map the kip. Not writeable, user */
    this->add_4k_mapping( kip_area.get_base(), virt_to_phys(get_kip()), 
	    false, false );
}

void space_t::allocate_tcb(addr_t addr)
{
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    ASSERT(this == kernel_space);

    addr_t page = kmem.alloc (kmem_tcb, POWERPC64_PAGE_SIZE);

    // Check if mapping exist in page table (dummy page)
    if ( this->lookup_mapping ( addr, &pg, &pgsize) )
    {
	pg->clear(this, pgsize, false, addr);
	flush_tlbent( this, addr, POWERPC64_PAGE_BITS );
    }

    add_4k_mapping_noexecute(addr, virt_to_phys(page), true, true);
}

void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    /* Free up memory used for UTCBs */
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}

utcb_t *space_t::allocate_utcb( tcb_t *tcb )
{
    ASSERT (tcb);
    addr_t utcb = (addr_t) tcb->get_utcb_location ();

    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if( lookup_mapping ((addr_t) utcb, &pg, &pgsize) )
    {
        addr_t kaddr = addr_mask( pg->address(this, pgsize),
                                  ~page_mask (pgsize) );
        return (utcb_t *)phys_to_virt
            ( addr_offset( kaddr, (word_t) utcb & page_mask( pgsize )) );
    }

    addr_t page = kmem.alloc( kmem_utcb, page_size( pgent_t::size_4k ) );

    this->add_4k_mapping((addr_t) utcb, virt_to_phys(page), 
		 true, false);

    return (utcb_t *)
        addr_offset (page, addr_mask (utcb, page_size (pgent_t::size_4k) - 1));
}

void space_t::map_dummy_tcb(addr_t addr)
{
    ASSERT(this == kernel_space);
    /* XXX We map the dummy page (kernel/user read-only) since PPC has no
     * support for super-readonly,user-noaccess
     */
    add_4k_mapping( addr, (addr_t)get_dummy_tcb(), false, false );
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

    add_4k_mapping( addr, addr, true, false );
}

