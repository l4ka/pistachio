/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/space.cc
 * Description:   Address space handling for MIPS32
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
 * $Id: space.cc,v 1.2 2006/11/17 17:14:30 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>		

#include <kmemory.h>
#include <linear_ptab.h>

#include INC_API(space.h)
#include INC_GLUE(config.h)
#include INC_GLUE(context.h)
#include INC_API(tcb.h)
#include INC_ARCH(cp0regs.h)

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);


asid_cache_t asid_cache;

space_t* kernel_space = 0;
static tcb_t * dummy_tcb = 0;


/**
 * Initialize THE kernel space
 *
 * @see get_kernel_space()
 */
void SECTION(".init.memory") init_kernel_space() {
	
    kernel_space = allocate_space();

    ASSERT( ((word_t)kernel_space & 0xfff) == 0 && "unexpected malloc alignment in init_kernel_space" );

    kernel_space->get_asid()->init();

    dummy_tcb = (tcb_t *) kmem.alloc(kmem_tcb, MIPS32_PAGE_SIZE);
	
    ASSERT( ((word_t)dummy_tcb & 0xfff) == 0 && "unexpected malloc alignment in init_kernel_space" );
}


/**
 * Initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area) {

    ASSERT( ((word_t)utcb_area.get_base() & 0xfff) == 0 && "unexpected alignment in space_t::init" );
    ASSERT( ((word_t)kip_area.get_base() & 0xfff) == 0 && "unexpected alignment in space_t::init" );
	
    lx.utcb_area = utcb_area;
    lx.kip_area = kip_area;

    this->get_asid()->init();

    add_mapping(kip_area.get_base(), virt_to_phys( (addr_t)get_kip() ), pgent_t::size_4k, false, false, false, true);
}


/**
 * Allocate a UTCB
 *
 * @param tcb	Owner of the utcb
 */
utcb_t *space_t::allocate_utcb( tcb_t *tcb )
{
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    addr_t page;
    pgent_t::pgsize_e pgsize;
    pgent_t * pg;
	
    if( lookup_mapping( (addr_t) utcb, &pg, &pgsize ) ) {
        // Already a valid page mapped at UTCB address
        ASSERT( pgsize == pgent_t::size_4k );
        page = phys_to_virt( pg->address(this, pgent_t::size_4k) );
    }
    else {
        // Allocate a new UTCB page.
        page = kmem.alloc( kmem_utcb, 4096 ); // XXX 4096
        add_mapping( utcb, virt_to_phys(page), pgent_t::size_4k, true, false, false, true );
    }

    return (utcb_t *)addr_offset( page, (word_t)utcb & 0x00000fff ); 
}


/**
 * Release mappings that belong to the kernel (UTCB, KIP)
 *
 * @param vaddr		virtual address in the space
 * @param paddr		physical address the mapping refers to
 * @param log2size	log2(size of mapping)
 */
void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size) {

    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
        kmem.free (kmem_utcb, phys_to_virt( paddr ), 1UL << log2size); 
}


/**
 * Establish a mapping in sigma0's space
 *
 * @param addr	the fault address in sigma0
 *
 * This function should install a mapping that allows sigma0 to make
 * progress. Sigma0's space is available as this.
 */
void space_t::map_sigma0(addr_t addr) {

    add_mapping(addr, addr, pgent_t::size_4k, true, false, false, true);
}


/**
 * Try to copy a mapping from kernel space into the current space
 *
 * @param addr  The address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 *
 * Synchronization must happen at the highest level, allowing sharing.
 */
bool space_t::sync_kernel_space(addr_t addr) {

    return( false );
}


/**
 * Install a dummy TCB
 *
 * @param addr	address where the dummy TCB should be installed
 *
 * The dummy TCB must be read-only and fail all validity tests.
 */
void space_t::map_dummy_tcb (addr_t addr) {

    ASSERT( ((word_t)addr & 0xfff ) == 0 && "unexpected alignment in space_t::map_dummy_tcb" );
    add_mapping( addr, virt_to_phys((addr_t)dummy_tcb), pgent_t::size_4k, false, true, true, true );
}


/**
 * Map memory usable for TCB
 *
 * @param addr  address of the TCB that should be made usable
 *
 * This function is called when a TCB should be made usable the first
 * time. Usually, this happens when a) no page is mapped at the TCB
 * address at all, or b) a read-only page is mapped and now a write
 * access to the TCB occured.
 *
 * @see space_t::map_dummy_tcb
 */
void space_t::allocate_tcb(addr_t addr) {

    ASSERT( (word_t)addr >= KTCB_AREA_START );
    addr_t page = kmem.alloc( kmem_tcb, MIPS32_PAGE_SIZE );
	
    ASSERT( ( (word_t)page & 0xfff ) == 0 && "unexpected alignment in space_t::allocate_tcb" );
    ASSERT( ( (word_t)addr & 0xfff ) == 0 && "unexpected alignment in space_t::allocate_tcb" );

    this->add_mapping( addr, virt_to_phys(page), pgent_t::size_4k, true, true, true, true );
}


/**
 * Translate a user accessible UTCB address to a kernel accessible one
 *
 * @param utcb  user accessible address of UTCB
 * @returns kernel accessible address of UTCB
 *
 * The returned address must be accessible in the current address
 * space. This is required for checking values in the UTCB of a thread
 * in a different address space.
 */
utcb_t * space_t::utcb_to_kernel_space(utcb_t * utcb) {

    pgent_t::pgsize_e pgsize;
    pgent_t * pg;

    if( lookup_mapping ((addr_t) utcb, &pg, &pgsize) ) {
        addr_t kaddr = pg->address( this, pgsize );
        return (utcb_t *)addr_offset( phys_to_virt(kaddr), (word_t)utcb & 0x00000fff );  // XXX
    }

    return( (utcb_t*)0 );
}


void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel, bool global, bool cacheable) {
    
    //printf("SPACE: Add mapping: space = 0x%x, vaddr = 0x%x, paddr = 0x%x, w = %s, k = %s, g = %s, c = %s\n", this, vaddr, paddr,
    //		writable ? "true" : "false", kernel ? "true" : "false", global ? "true" : "false", cacheable ? "true" : "false" );
    
    if( (word_t)vaddr >= KTCB_AREA_START ) {
        ASSERT( global == true );
    }
    
    if( (word_t)this == (word_t)get_kernel_space() ) {
        ASSERT( (word_t)vaddr >= KTCB_AREA_START && "space_t::add_mapping" );
    }
    else {
        ASSERT( (word_t)vaddr < KSEG0_BASE && "space_t::add_mapping" );
    }
    
    pgent_t *pde, *pte;
    
    pde = this->pgent( page_table_index( pgent_t::size_4m, vaddr ) );
    
    if( !pde->is_valid( this, size ) ) {
        pde->make_subtree( this, pgent_t::size_4m, kernel );
    }
    
    pte = pde->subtree( this, pgent_t::size_4m )->next( this, pgent_t::size_4k, page_table_index(pgent_t::size_4k, vaddr) );
    
    pte->set_entry( this, pgent_t::size_4k, paddr, writable ? 7 : 5, 0, kernel );
    
    if( global ) pte->set_global( this, pgent_t::size_4k, true );

}

/*****************************************************************************/
/*                      USER MODE TLB MISS HANDLING                          */
/*****************************************************************************/

/*
 * called on a TLB mod exception. This happens on a write to an address in the
 * TLB that is write protected - e.g. a dummy tcb
 */
extern "C" void tlbmod_handler( addr_t _faddr, mips32_irq_context_t* frame ) {

    space_t* space = get_current_tcb()->get_space();
    
    addr_t faddr = (addr_t)( (word_t)_faddr & 0xfffff000 );
	 
    pgent_t::pgsize_e pgsize;
    pgent_t *pg;
    bool kernel;

    word_t global = 0x0;
    word_t asid;

    if( space == NULL || (word_t)faddr >= KTCB_AREA_START ) { 
        space = get_kernel_space();
        global = 0x1;
        asid = KERNEL_ASID;
    }
    else {
        asid = (word_t)(space->get_asid()->get());
    }
	
    if( space->lookup_mapping( faddr, &pg, &pgsize ) ) {
		
        if( pg->is_writable(space, pgsize) ) {
            get_tlb()->put( (word_t)faddr, asid, pg);
            return;
        }
    }

    if( space->is_user_area(faddr) ) {
        kernel = frame->status & ST_UM ? false : true;
    }
    else {
        kernel = true;
    }
    space->handle_pagefault (faddr, (addr_t)frame->epc, space_t::write, kernel);
}


extern "C" void touch_tcb( void* tcb ) {

    volatile char toodles;
    ASSERT( !"touch_tcb reached" ); // XXX simply remove ( though this is unlikely to get invoked in tcb_t::switch_to )
    toodles = *((char*)tcb);
}


/**
 * Standard TLB miss handler called on TLB missed in user and kernel mode
 */
extern "C" void tlbmiss_handler( addr_t _faddr, mips32_irq_context_t* frame ) {
	
    //register unsigned esp asm("$29");
    //printf("\n\tuser_tlbmiss - faddr=0x%x, frame=0x%x , ip=0x%x, status=0x%x ,", _faddr, frame, frame->epc, frame->status );
    space_t* space = get_current_tcb()->get_space();
    //printf("space = 0x%x, tcb = 0x%x\n", space, get_current_tcb() );
    //printf("\tSaved stack = 0x%x, current stack = 0x%x\n", frame->sp, esp );
	
    addr_t faddr = (addr_t)( (word_t)_faddr & 0xfffff000 );
	 
    pgent_t::pgsize_e pgsize;
    space_t::access_e access;
    pgent_t *pg;
    bool kernel, twice;

    word_t global = 0x0;
    word_t asid;
	
    if( space == NULL || (word_t)faddr >= KTCB_AREA_START ) { 
        space = get_kernel_space();
        global = 0x1;
        asid = KERNEL_ASID;
    }
    else {
        asid = (word_t)(space->get_asid()->get());
    }
	
    access = ( frame->cause & CAUSE_EXCCODE ) == ( 3 << 2 ) ? space_t::write : space_t::read;
    twice = false;

    while (1) {
        if( space->lookup_mapping( faddr, &pg, &pgsize ) ) {
            if( !( ((access == space_t::write) && pg->is_writable(space, pgsize)) ||
                   ((access == space_t::read) && pg->is_readable(space, pgsize))) ) {
                ASSERT( 0 && "access violation in kernel tlbmiss handler" );
            }
            get_tlb()->put( (word_t)faddr, asid, pg);
            return;
        }

        if( twice ) { 
            // printf("======== Segfault ;) ========\n"); 
            return;
        }

        if( space->is_user_area(faddr) ) {
            kernel = frame->status & ST_UM ? false : true;
        }
        else {
            kernel = true;	/* User-space will cause address error */
        }
        space->handle_pagefault (faddr, (addr_t)frame->epc, access, kernel);
        twice = true;
    }
}



