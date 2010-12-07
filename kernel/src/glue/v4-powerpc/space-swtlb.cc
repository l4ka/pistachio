/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/space-swtlb.cc
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
#include INC_GLUE(syscalls.h)

#include INC_ARCH(swtlb.h)
#include INC_ARCH(phys.h)

EXTERN_KMEM_GROUP(kmem_utcb);
EXTERN_KMEM_GROUP(kmem_tcb);
EXTERN_KMEM_GROUP(kmem_pgtab);
EXTERN_KMEM_GROUP(kmem_space);

//#define TRACE_TLB(x...)	TRACEF(x)
#define TRACE_TLB(x...)

word_t space_t::pinned_mapping;
// used by initialization code...
word_t swtlb_high_water;

#ifdef CONFIG_SMP
// XXX: move into init data section!
static struct {
    ppc_tlb0_t tlb0;
    ppc_tlb1_t tlb1;
    ppc_tlb2_t tlb2;
} init_swtlb[PPC_MAX_TLB_ENTRIES];
#endif

// tlb index for replacement
UNIT("cpulocal") ppc_swtlb_t swtlb;

// ASID management
UNIT("cpulocal") asid_manager_t<space_t, CONFIG_MAX_NUM_ASIDS> asid_manager;

#if 0
void dump_tlb()
{
    for (int i = 0; i < PPC_MAX_TLB_ENTRIES; i++)
    {
	ppc_tlb0_t tlb0;
	ppc_tlb1_t tlb1;
	ppc_tlb2_t tlb2;
	ppc_mmucr_t mmucr;
	tlb0.read(i);
	tlb1.read(i);
	tlb2.read(i);

	printf("%02d: %c [%02x:%d] %08x sz:%08x [%04x:%08x] U:%c%c%c S:%c%c%c  C:[%c%c%c%c%c]\n",
	       i, tlb0.is_valid() ? 'V' : 'I', mmucr.read().get_search_id(),
	       tlb0.trans_space, tlb0.get_vaddr(), tlb0.get_size(),
	       (word_t)(tlb1.get_paddr() >> 32), (word_t)(tlb1.get_paddr()),
	       tlb2.user_execute ? 'X' : '-', tlb2.user_write ? 'W' : '-', 
	       tlb2.user_read ? 'R' : '-', tlb2.super_execute ? 'X' : '-', 
	       tlb2.super_write ? 'W' : '-', tlb2.super_read ? 'R' : '-',
	       tlb2.write_through ? 'W' : '-', tlb2.inhibit ? 'I' : '-',
	       tlb2.mem_coherency ? 'M' : '-', tlb2.guarded ? 'G' : '-',
	       tlb2.endian ? 'E' : '-');
    }
}
#endif

bool space_t::sync_kernel_space(addr_t addr)
{
    /* nothing to sync; we handle the kernel space in the TLB miss
     * handler */
    return false;
}

void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
	int i;
	this->utcb_area = utcb_area;
    this->kip_area = kip_area;

    this->add_mapping( kip_area.get_base(), (paddr_t)virt_to_phys(get_kip()), 
		       pgent_t::size_4k, false, false );

    // XXX: do upon migration!
    for (i = 0; i < CONFIG_SMP_MAX_CPUS; i++)
	get_asid(i)->init();
}

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    int i;
    this->pinned_mapping = PINNED_AREA_START;

    //initialize translation table
    for (i=0; i < TRANSLATION_TABLE_ENTRIES; ++i) {
    	transtable[i].s0addr = 0;
    	transtable[i].physaddr = 0;
    	transtable[i].size = 0;
    }

}

void SECTION(".init.memory") space_t::init_cpu_mappings(cpuid_t cpu)
{
    extern char _begin_cpu_local[], _end_cpu_local[];
    extern char _cpu_phys[];
    ppc_tlb0_t tlb0;
    ppc_tlb1_t tlb1;
    ppc_tlb2_t tlb2;
    addr_t page;

    // determine size
    int log2size = 10;
    for (; (1 << log2size) < (_end_cpu_local - _begin_cpu_local) || 
	     !ppc_tlb0_t::is_valid_pagesize(log2size); log2size++);
    
    if (cpu == 0)
	page = phys_to_virt(_cpu_phys);
    else
    {
	page = kmem.alloc(kmem_pgtab, 1 << log2size);
	memcpy(page, phys_to_virt(_cpu_phys), 1 << log2size);
    }

    TRACE_INIT("\tMapping %p/%p -> %p, log2sz=%d, TLB entry: %d (CPU %d)\n", 
	       page, virt_to_phys(page), CPU_AREA_START, log2size, 
	       swtlb_high_water, cpu);

    tlb0.init_vaddr_size(CPU_AREA_START, log2size);
    tlb1.init_paddr((paddr_t)virt_to_phys(page));
    //tlb2.init_cpu_local();
    tlb2.init_shared_smp();
    tlb2.set_kernel_perms(true, true, false);

    ppc_mmucr_t::write_search_id(0);
    tlb0.write(swtlb_high_water);
    tlb1.write(swtlb_high_water);
    tlb2.write(swtlb_high_water);

    /* 
     * CPU local mappings exist now 
     */
    TRACE_INIT("\tASID manager init %x -> %x (CPU %d)\n", 1, CONFIG_MAX_NUM_ASIDS-1, cpu);
    asid_manager.init(1, CONFIG_MAX_NUM_ASIDS - 1);
    ASSERT(this == get_kernel_space());
    this->cpu[cpu].asid.init_kernel(0);
    swtlb.init(swtlb_high_water - 1);

#ifdef CONFIG_SMP
    /* safe all kernel mappings except CPU local */
    if (cpu == 0)
    {
	for (unsigned idx = swtlb_high_water + 1; idx < PPC_MAX_TLB_ENTRIES; idx++)
	{
	    init_swtlb[idx].tlb0.read(idx);
	    init_swtlb[idx].tlb1.read(idx);
	    init_swtlb[idx].tlb2.read(idx);
	    TRACEF("\tTLB%d: %lx, %lx, %lx\n", idx, init_swtlb[idx].tlb0.raw,
		   init_swtlb[idx].tlb1.raw, init_swtlb[idx].tlb2.raw);
	}
    }

    current_cpu = cpu;

#endif
}

NOINLINE bool space_t::handle_tlb_miss( addr_t lookup_vaddr, addr_t install_vaddr, 
					bool user, bool global )
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;

    /* check kernel fault for device mappings */
    TRACE_TLB("handle_tlb_miss %p, %p, %s\n", 
	      lookup_vaddr, install_vaddr, user ? "user" : "kernel");

    if (! lookup_mapping (lookup_vaddr, &pg, &pgsize))
        return false;

    size_t  size  = page_shift (pgsize);
    word_t  vaddr = (word_t) install_vaddr;
    paddr_t paddr = pg->address (this, pgsize) | (vaddr & ((1ul << size) - 1));

    while (!ppc_tlb0_t::is_valid_pagesize (size))
        size--;

    vaddr &= ~((1ul << size) - 1);
    paddr &= ~((1ull << size) - 1);

    TRACE_TLB("mapping found (%p): %p -> %lx (%x)\n",
	      pg, lookup_vaddr, static_cast<word_t>(paddr), size);
    TRACE_TLB("[%c%c%c], cache=%x, erpn=%x\n", 
	      pg->map.read ? 'R' : ' ', pg->map.write ? 'W' : ' ',
	      pg->map.execute ? 'X' : ' ', pg->map.caching, pg->map.erpn);

    ppc_tlb0_t tlb0 (vaddr, size);
    ppc_tlb1_t tlb1 (paddr);
    ppc_tlb2_t tlb2;

    switch (pg->map.caching)
    {
    case pgent_t::cache_standard:  tlb2.init_shared_smp(); break;
    case pgent_t::cache_inhibited: tlb2.init_device(); break;
    case pgent_t::cache_guarded: tlb2.init_guarded(); break;
    default: UNIMPLEMENTED();
    }
    if (user)
	tlb2.set_user_perms(pg->map.read, pg->map.write, pg->map.execute);
    tlb2.set_kernel_perms(pg->map.read, pg->map.write, 0);

    ppc_mmucr_t::write_search_id(global ? 0 : ppc_get_pid());
    word_t tlb_index = swtlb.allocate();

    TRACE_TLB("inserting TLB entry %d: %08x, %08x, %08x\n",
	      tlb_index, tlb0.raw, tlb1.raw, tlb2.raw);

    tlb0.write(tlb_index);
    tlb1.write(tlb_index);
    tlb2.write(tlb_index);

    return true;
}

addr_t space_t::map_device_pinned(paddr_t paddr, word_t size, bool kernel, word_t attrib)
{
    word_t log2sz;
    word_t vaddr = pinned_mapping;

    for (log2sz = 1; log2sz < 32; log2sz++)
	if ((1U << log2sz) >= size && ppc_tlb0_t::is_valid_pagesize(log2sz))
	    break;

    if (log2sz >= 32)
	return NULL;

    size = 1 << log2sz;

    paddr_t paddr_align = paddr & ~((paddr_t)size - 1);
    if (vaddr & (size - 1) != 0)
	vaddr = (vaddr + size) & ~(size - 1);
    
    ppc_tlb0_t tlb0(vaddr, log2sz);
    ppc_tlb1_t tlb1(paddr_align);
    ppc_tlb2_t tlb2;
    tlb2.init_device();
    tlb2.set_kernel_perms(true, true, false);
    if (!kernel)
        tlb2.set_user_perms(true, true, false);

    word_t tlb_index = swtlb.allocate_pinned();
    tlb0.write(tlb_index);
    tlb1.write(tlb_index);
    tlb2.write(tlb_index);

    TRACE_TLB("mapping pinned device: %x.%08x, sz=%x, %x.%08x, [%08x, %08x, %08x]\n",
	      (word_t)(paddr >> 32), (word_t)paddr, size, (word_t)(paddr_align >> 32),
	      (word_t)paddr_align, tlb0.raw, tlb1.raw, tlb2.raw);

    pinned_mapping = vaddr + size;

    return addr_offset((addr_t)vaddr, paddr - paddr_align);
}

asid_t *space_t::get_asid()
{
    return get_asid(get_current_cpu());
}

void space_t::allocate_asid()
{
    asid_manager.allocate_asid(this);
}

void space_t::flush_tlb( space_t *curspace, addr_t start, addr_t end )
{
    asid_t *asid = get_asid();
    if (!asid->is_valid())
	return;

    TRACEF("flush_tlb %p, [%p-%p]\n", this, start, end);

    word_t hw_asid = asid->get();
    for (word_t idx = 0; idx < swtlb.high_water; idx++)
    {
	ppc_tlb0_t tlb0;
	ppc_mmucr_t mmucr;
	tlb0.read(idx);
	mmucr.read();

	if (!tlb0.is_valid())
	    continue;

	if (mmucr.get_search_id() == hw_asid &&
	    (addr_t)tlb0.get_vaddr() >= start && 
	    addr_offset((addr_t)tlb0.get_vaddr(), tlb0.get_size() - 1) <= end )
	{
	    ppc_tlb0_t::invalid().write(idx);
	    swtlb.set_free(idx);
	}
    }
}

void space_t::flush_tlbent( space_t *curspace, addr_t addr, word_t log2size )
{
    asid_t *asid = get_asid();
    if (!asid->is_valid())
	return;

    word_t idx;
    ppc_mmucr_t::write_search_id(asid->get());
    isync();

    if (ppc_tlbsx((word_t)addr, idx))
    {
	TRACEF("invalidating TLB entry %d\n", idx);
	ppc_tlb0_t::invalid().write(idx);
	swtlb.set_free(idx);
    }
}

void space_t::arch_free()
{
    flush_tlb(this, (addr_t)USER_AREA_START, (addr_t)USER_AREA_END);
}

#define RELOC(s0addr, physaddr, size) \
    case s0addr ... s0addr + size - 1: paddr = physaddr + reinterpret_cast<paddr_t>(addr_offset(addr, -s0addr)); break;

paddr_t space_t::sigma0_translate(addr_t addr, pgent_t::pgsize_e size)
{
	word_t i;
	paddr_t paddr = (paddr_t)addr;
    for (i = 0; i < TRANSLATION_TABLE_ENTRIES; ++i) {
    	if ((transtable[i].size > 0) && ((word_t)addr >= transtable[i].s0addr) && ((word_t)addr <= transtable[i].s0addr + transtable[i].size - 1)) {
    		paddr = transtable[i].physaddr + (word_t)addr_offset(addr, -transtable[i].s0addr);
    		transtable[i].size = 0;
    		break;
    	}
    }
    return paddr;
}

word_t space_t::sigma0_attributes(pgent_t *pg, paddr_t addr, pgent_t::pgsize_e size)
{
    /* device memory is guarded */
    //if (sigma0_translate(addr, size) >= 0x100000000ULL)
	if (addr >= 0x100000000ULL)
	return pgent_t::cache_inhibited;
    else
	return pgent_t::cache_standard;
}

/**********************************************************************
 *		    Paging initialization; unpaged mode!
 **********************************************************************/

extern "C" SECTION(".einit") void init_paging( int cpu )
{
    u32_t curr_entry;
    word_t index;

    /* switch to global space */
    ppc_set_pid(0);

    /* set search id to global */
    ppc_mmucr_t::write_search_id(0);

    ppc_tlbsx((u32_t)&init_paging, curr_entry); /* can't fail */

    // Clear out all mappings except for the one we run on
    for (index = 0; index < PPC_MAX_TLB_ENTRIES; index++)
    {
	if (index == curr_entry)
	    continue;
	ppc_tlb0_t::invalid().write(index);
    }

    ppc_tlb0_t tlb0;
    ppc_tlb1_t tlb1;
    ppc_tlb2_t tlb2;
    ppc_mmucr_t mmucr;
    word_t log2size;

    // initialize MMUCR
    mmucr.raw = 0;
#ifdef CONFIG_SUBPLAT_440_BGP
    mmucr.u2_store_without_allocate = 1;
#endif
    mmucr.write();

    for (log2size = KERNEL_AREA_LOG2SIZE; 
         !ppc_tlb0_t::is_valid_pagesize(log2size);
         log2size--);

    index = PPC_MAX_TLB_ENTRIES - 1;

    tlb0.init_vaddr_size(KERNEL_OFFSET, log2size);
    tlb1.init_paddr(0ULL);
    tlb2.init_shared_smp();
    tlb2.set_kernel_perms(true, true, true);
    tlb2.set_user_perms(false, false, true);

    // map the kernel area
    for (; tlb0.get_vaddr() < KERNEL_AREA_END; 
	 tlb0 += (1 << log2size), tlb1 += (1 << log2size), index--)
    {
	if (index == curr_entry)
	    index--;
	tlb0.write(index);
	tlb1.write(index);
	tlb2.write(index);
    }
    isync();

    /*
     * Paging is on now and we can access kernel data
     */
    if (cpu == 0)
	swtlb_high_water = index;
    else
    {
#ifdef CONFIG_SMP
	for (unsigned idx = swtlb_high_water + 1; idx < PPC_MAX_TLB_ENTRIES; idx++)
	{
	    init_swtlb[idx].tlb0.write(idx);
	    init_swtlb[idx].tlb1.write(idx);
	    init_swtlb[idx].tlb2.write(idx);
	}
	isync();
#endif
    }
}

#if defined(CONFIG_TRACEBUFFER)
FEATURESTRING ("tracebuffer");
tracebuffer_t * tracebuffer;
EXTERN_KMEM_GROUP (kmem_misc);
void setup_tracebuffer (void)
{
    tracebuffer = (tracebuffer_t *) kmem.alloc (kmem_misc, TRACEBUFFER_SIZE);
    if (!tracebuffer)
        return;
    
    addr_t vaddr = get_kernel_space()->map_device_pinned(virt_to_phys((paddr_t)tracebuffer), 
                                                         TRACEBUFFER_SIZE, false, pgent_t::cache_standard );
    get_kip()->memory_info.insert(memdesc_t::reserved, true, vaddr,
                                  addr_offset(vaddr, TRACEBUFFER_SIZE -1));

    tracebuffer->initialize ();
}
#endif /* CONFIG_TRACEBUFFER */

addr_t setup_console_mapping(paddr_t paddr, int log2size)
{
    static word_t console_area = CONSOLE_AREA_START;

    word_t size = 1 << log2size;
    word_t vaddr = console_area;
    paddr_t paddr_align = paddr & ~((paddr_t)size - 1);

    if (vaddr & (size - 1) != 0)
	vaddr = (vaddr + size) & ~(size - 1);

    ppc_tlb0_t tlb0(vaddr, log2size);
    ppc_tlb1_t tlb1(paddr_align);
    ppc_tlb2_t tlb2;
    tlb2.init_device();
    tlb2.set_kernel_perms(true, true, false);

    ppc_mmucr_t::write_search_id(0);
    tlb0.write(swtlb_high_water);
    tlb1.write(swtlb_high_water);
    tlb2.write(swtlb_high_water);
    isync();

    swtlb_high_water--;
    console_area = vaddr + size;

    return addr_offset((addr_t)vaddr, paddr - paddr_align);
}

SECTION(".init") void setup_kernel_mappings( void )
{
    /* flush boot mapping */
    u32_t entry;
    ppc_tlb0_t tlb0;
    ppc_tlbsx((u32_t)&init_paging, entry);
    tlb0.read(entry);

    TRACE_INIT("Flush boot mapping %x, vaddr=%x, size=%x (%x)\n", 
               entry, tlb0.get_vaddr(), tlb0.get_size(), tlb0.raw);
    ppc_tlb0_t::invalid().write(entry);
}

/**********************************************************************
 * Exception handlers
 **********************************************************************/

DECLARE_TRACEPOINT(PPC_DTLB_MISS);
DECLARE_TRACEPOINT(PPC_ITLB_MISS);
DECLARE_TRACEPOINT(PPC_EXCEPT_ISI);
DECLARE_TRACEPOINT(PPC_EXCEPT_DSI);

EXCDEF( isi_handler )
{
    TRACEPOINT(PPC_EXCEPT_ISI,
	       "ISI MISS: IP: %08x", srr0);
    
    space_t *space = get_current_space();
    ASSERT(space);

    space->handle_pagefault( (addr_t)srr0, (addr_t)srr0, 
	    space_t::execute, ppc_is_kernel_mode(srr1) );

    return_except();
}

EXCDEF( dsi_handler )
{
    word_t dear = ppc_get_spr(SPR_DEAR);
    ppc_esr_t esr;
    esr.read();

    TRACEPOINT(PPC_EXCEPT_DSI,
	       "DSI MISS: IP: %08x, ESR: %08x, DEAR: %p", 
	       srr0, esr.raw, dear);

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    ASSERT(space);

    space->handle_pagefault( (addr_t)dear, (addr_t)srr0, 
	    esr.x.store ?  space_t::write : space_t::read,
	    ppc_is_kernel_mode(srr1) );

    return_except();
}

EXCDEF( dtlb_miss_handler )
{
    addr_t dear = (addr_t)ppc_get_spr(SPR_DEAR);
    bool kernel_mode = ppc_is_kernel_mode(srr1);
    ppc_esr_t esr; esr.read();
    bool user = false;
    
    
    TRACEPOINT(PPC_DTLB_MISS,
	       "DTLB MISS: IP: %08x, ESR: %08x, DEAR: %p", 
	       srr0, esr.raw, dear);

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    if (!space) 
    {
        ASSERT(space->is_kernel_paged_area(dear));
        space = get_kernel_space();
    }
   
    if (space->is_user_area(dear))
    {
	if ( EXPECT_TRUE(space->handle_tlb_miss(dear, dear, true)) )
	    return_except();
	user = true;
    }
    else if (EXPECT_FALSE(kernel_mode))
    {
	if (space->is_kernel_paged_area(dear))
	{
	    if (!get_kernel_space()->handle_tlb_miss(dear, dear, user, true))
		panic("kernel accessed unmapped device @ %08x (IP=%08x)", dear, srr0);
	    return_except();
	}
	else if (space->is_copy_area(dear))
	{
	    // Resolve the fault using the partner's address space!
	    tcb_t *partner = tcb_t::get_tcb( tcb->get_partner() );
	    if( partner )
	    {
		addr_t real_fault = tcb->copy_area_real_address( (addr_t)dear );
		TRACE_TLB("copy area DTLB miss: %p -> %p\n", dear, real_fault);
		if (!partner->get_space()->handle_tlb_miss(real_fault, dear, user))
		{
		    space->handle_pagefault(dear, (addr_t)srr0, space_t::write, 
					    ppc_is_kernel_mode(srr1));
		    partner->get_space()->handle_tlb_miss(real_fault, dear, user);
		}
		return_except();
	    }
	}
	else 
	{
	    printf("kernel page fault @ %08x?", dear);
	    enter_kdebug("unhandled kernel TLB miss");
	}
    }

    space->handle_pagefault( dear, (addr_t)srr0,
			     esr.x.store ?  space_t::write : space_t::read,
			     ppc_is_kernel_mode(srr1) );

    // pro-actively try to load the TLB; if miss wasn't fulfilled we
    // are in trouble anyhow...
    space->handle_tlb_miss(dear, dear, user);

    return_except();
}

EXCDEF( itlb_miss_handler )
{
    ppc_esr_t esr; 
    esr.read();

    TRACEPOINT(PPC_ITLB_MISS,
	       "ITLB MISS: IP: %08x, LR: %08x, ESR: %08x",
	       srr0, frame->lr, esr.raw);

    space_t *space = get_current_tcb()->get_space();

    ASSERT(space);
    ASSERT(!ppc_is_kernel_mode(srr1));

    // first try to refill TLB
    if ( EXPECT_FALSE(!space->handle_tlb_miss((addr_t)srr0, (addr_t)srr0, true)) )
    {
	space->handle_pagefault( (addr_t)srr0, (addr_t)srr0,
				 space_t::execute, ppc_is_kernel_mode(srr1) );

	// pro-actively try to load the TLB; if miss wasn't fulfilled we
	// are in trouble anyhow...
	space->handle_tlb_miss((addr_t)srr0, (addr_t)srr0, true);
    }

    return_except();
}
