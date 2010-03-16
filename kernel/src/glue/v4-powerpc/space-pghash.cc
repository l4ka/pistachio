/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/space-pghash.cc
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

bool space_t::handle_hash_miss( addr_t vaddr )
{
    TRACEPOINT(hash_miss_cnt);

    pgent_t *pgent = this->page_lookup( vaddr );
    if( !pgent || !pgent->is_valid(this, pgent_t::size_4k) )
	return false;

    TRACEPOINT(hash_insert_cnt);

    get_pghash()->insert_4k_mapping( this, vaddr, pgent );
    return true;
}

#include INC_PLAT(ofppc.h)

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
	add_mapping( page, virt_to_phys(page), pgent_t::size_4k, false, true, true );
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
	add_mapping( page, virt_to_phys(page), pgent_t::size_4k, false, true, true );
    }
}

void SECTION(".init.memory") space_t::init_cpu_mappings(cpuid_t cpu)
{
     ppc_bat_t bat;
     bat.raw.upper = bat.raw.lower = 0;
     bat.x.bepi = KERNEL_CPU_OFFSET >> BAT_BEPI;
     bat.x.bl = BAT_BL_128K;
     bat.x.vs = 1;
     bat.x.brpn = cpu_phys_area(cpu) >> BAT_BRPN;
     bat.x.m = 0;	/* We don't need memory coherency. */
     bat.x.pp = BAT_PP_READ_WRITE;
     ppc_set_cpu_dbat( l, bat.raw.lower );
     ppc_set_cpu_dbat( u, bat.raw.upper );
     isync();
}

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
    this->add_mapping( kip_area.get_base(), virt_to_phys(get_kip()), 
		       pgent_t::size_4k, false, false );
}

void space_t::flush_tlb( space_t *curspace )
{
    // TODO: flush the tlb for a given address space.
    ppc_invalidate_tlb();
}

void space_t::flush_tlbent( space_t *curspace, addr_t addr, 
			    word_t log2size )
{
    ppc_invalidate_tlbe( addr );
}

DECLARE_TRACEPOINT(PPC_EXCEPT_ISI);
DECLARE_TRACEPOINT(PPC_EXCEPT_DSI);

EXCDEF( isi_handler )
{
    TRACEPOINT(PPC_EXCEPT_ISI, "except_isi_cnt");

    // Let the debugger have a first shot at inspecting the instr fault.
    try_to_debug( frame, EXCEPT_ID(ISI) );

    space_t *space = get_current_tcb()->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    if( EXCEPT_IS_ISI_MISS(srr1) ) 
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)srr0)) ) 
	    except_return();

    space->handle_pagefault( (addr_t)srr0, (addr_t)srr0, 
	    space_t::execute, ppc_is_kernel_mode(srr1) );

    except_return();
}

EXCDEF( dsi_handler )
{
    TRACEPOINT(PPC_EXCEPT_DSI, "except_dsi_cnt");
    
    word_t dar = ppc_get_spr(SPR_DAR);
    word_t dsisr = ppc_get_spr(SPR_DSISR);

    // Let the debugger have a first shot at inspecting the data fault.
    try_to_debug( frame, EXCEPT_ID(DSI), dar, dsisr );

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    // Do we have a page hash miss?
    if( EXCEPT_IS_DSI_MISS(dsisr) )
    {

	// Is the page hash miss in the copy area?
	if( EXPECT_FALSE(space->is_copy_area((addr_t)dar)) )
	{
	    // Resolve the fault using the partner's address space!
	    tcb_t *partner = tcb_t::get_tcb( tcb->get_partner() );
	    if( partner )
	    {
		addr_t real_fault = tcb->copy_area_real_address( (addr_t)dar );
		if( partner->get_space()->handle_hash_miss(real_fault) )
	    	    except_return();
	    }
	}

	// Normal page hash miss.
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)dar)) )
	    except_return();
    }

    space->handle_pagefault( (addr_t)dar, (addr_t)srr0, 
	    EXCEPT_IS_DSI_WRITE(dsisr) ?  space_t::write : space_t::read,
	    ppc_is_kernel_mode(srr1) );

    except_return();
}
