/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/space.h
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

#ifndef __GLUE__V4_POWERPC__SPACE_H__
#define __GLUE__V4_POWERPC__SPACE_H__

#include <debug.h>
#include <asid.h>

#include INC_API(types.h)
#include INC_API(fpage.h)
#include INC_API(thread.h)

#if defined(CONFIG_PPC_MMU_SEGMENTS)
#include INC_ARCH(pgent-pghash.h)
#else
#include INC_ARCH(pgent-swtlb.h)
#include INC_ARCH(swtlb.h)
#define HAVE_ARCH_FREE_SPACE
#include INC_ARCH(softhvm.h)
#endif
#include INC_GLUE(hwspace.h)
#include INC_ARCH(atomic.h)

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

class utcb_t;
class tcb_t;

class space_t
{
public:
    enum access_e {
	read		= 0,
	write		= 1,
	readwrite	= -1,
	execute		= 2
    };

    void init(fpage_t utcb_area, fpage_t kip_area);		// glue
    void free();						// api
    void arch_free();
    bool sync_kernel_space(addr_t addr);			// glue
    static void switch_to_kernel_space(cpuid_t cpu) {}
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel); // api
    bool is_initialized();

    /* sigma0 handling */
    void map_sigma0(addr_t addr);				// glue
    void map_fpage(fpage_t snd_fp, word_t base,
	space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all );
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);
    
    /* tcb management */
    void allocate_tcb(addr_t addr);				// glue
    void map_dummy_tcb(addr_t addr);				// glue
    utcb_t * allocate_utcb(tcb_t * tcb);

    /* address ranges */
    static bool is_user_area(addr_t addr);
    static bool is_user_area(fpage_t fpage);
    static bool is_kernel_area(addr_t addr);
    static bool is_tcb_area(addr_t addr);
    static  bool is_copy_area (addr_t addr);

    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);
    static const bool is_arch_mappable(addr_t addr, size_t size) { return true; }
    static const addr_t sign_extend(addr_t addr) { return addr; }

    bool is_kernel_paged_area(addr_t addr);

    /* Copy area related methods */
    word_t get_copy_limit (addr_t addr, word_t limit);

    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    fpage_t get_utcb_page_area();

    /* reference counting */
    void add_tcb(tcb_t * tcb, cpuid_t cpu);
    bool remove_tcb(tcb_t * tcb, cpuid_t cpu);
    void move_tcb(tcb_t * tcb, cpuid_t src_cpu, cpuid_t dst_cpu);

    /* allocation functions */
    static space_t * allocate_space();
    static void free_space(space_t *space);

    /* space control */
    word_t space_control (word_t ctrl);

    /* tlb */
    void flush_tlb( space_t *curspace, addr_t start = (addr_t)0, addr_t end = (addr_t)~0U );
    void flush_tlbent( space_t *curspace, addr_t addr, word_t log2size );
    static bool does_tlbflush_pay( word_t log2size )
	{ return log2size != POWERPC_PAGE_BITS; }
#ifdef CONFIG_X_PPC_SOFTHVM
    void flush_tlb_hvm( space_t *curspace, word_t start = 0, word_t end = ~0U );
#endif

    /* update hooks */
    static void begin_update() {}
    static void end_update() {}

    /* sigma0 translation hooks */
    static paddr_t sigma0_translate(addr_t addr, pgent_t::pgsize_e size);
    static word_t sigma0_attributes(pgent_t *pg, addr_t addr, pgent_t::pgsize_e size);

public:
    /* powerpc specific functions */
    static void init_kernel_space();

    void init_kernel_mappings();
    void init_cpu_mappings(cpuid_t cpu);
    addr_t map_device( paddr_t paddr, word_t size, bool kernel, word_t attrib );

#ifdef CONFIG_PPC_MMU_SEGMENTS
    bool handle_hash_miss( addr_t vaddr );
    ppc_segment_t get_segment_id();
#endif

#ifdef CONFIG_PPC_MMU_TLB
    addr_t map_device_pinned( paddr_t paddr, word_t size, bool kernel, word_t attrib );
    bool handle_tlb_miss( addr_t lookup_vaddr, addr_t install_vaddr, bool user, bool global = false );
    bool handle_hvm_tlb_miss( ppc_softhvm_t*, ppc_softhvm_t::tlb_t*, word_t gvaddr, paddr_t &gpaddr );
    asid_t *get_asid(); // asid of current cpu
    asid_t *get_asid(cpuid_t cpu);
    void allocate_asid();
    void allocate_hw_asid(word_t hw_asid) {}
    void release_hw_asid(word_t hw_asid)
	{ flush_tlb(NULL); }
    static asid_manager_t<space_t,CONFIG_MAX_NUM_ASIDS> *get_asid_manager();
#endif

    pgent_t *get_pdir();
    word_t get_vsid( addr_t vaddr );

    pgent_t * page_lookup( addr_t vaddr );
    word_t get_from_user( addr_t );
    pgent_t *pgent( word_t num, word_t cpu=0 );
    bool lookup_mapping( addr_t vaddr, pgent_t ** r_pg,
			 pgent_t::pgsize_e *r_size, cpuid_t cpu = 0);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (paddr_t paddr)
	{ return *phys_to_virt((word_t*)paddr); }
    void release_kernel_mapping (addr_t vaddr, paddr_t paddr, word_t log2size);

    static space_t *vsid_to_space( word_t vsid )
    {
	return (space_t *)( (vsid & 0xfffffff0) << (POWERPC_PAGE_BITS - 4) );
    }

    void add_mapping( addr_t vaddr, paddr_t paddr, pgent_t::pgsize_e size, 
		      bool writable, bool kernel, 
		      word_t attrib = pgent_t::cache_standard );
    void flush_mapping( addr_t vaddr, pgent_t::pgsize_e size, pgent_t *pgent );

private:
    pgent_t pdir[1024];
    union {
	word_t raw[1024];
	struct {
	    word_t linknode[USER_AREA_SIZE >> PPC_PAGEDIR_BITS];
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    atomic_t thread_count;
#ifdef CONFIG_X_PPC_SOFTHVM
	    bool hvm_mode;
#endif
	    struct {
		atomic_t thread_count;
#ifdef CONFIG_PPC_MMU_TLB
		asid_t asid;
		pgent_t cpulocal;
#endif
	    } cpu[CONFIG_SMP_MAX_CPUS];
	};
    };

    static word_t pinned_mapping;
    friend class kdb_t;
};

/**********************************************************************
 *
 *                 global declarations
 *
 ***********************************************************************/

extern void init_kernel_space();

INLINE space_t *get_kernel_space()
{
    extern space_t *kernel_space;
    return kernel_space;
}

/**********************************************************************
 *
 *                 inline functions
 *
 ***********************************************************************/

INLINE pgent_t * space_t::pgent( word_t num, word_t cpu )
{
    return (get_pdir())->next( this, pgent_t::size_4m, num );
}

INLINE pgent_t * space_t::get_pdir()
{
    return this->pdir;
}

INLINE bool space_t::is_kernel_paged_area(addr_t addr)
{
    return (addr > (addr_t)DEVICE_AREA_START &&
	    addr < (addr_t)DEVICE_AREA_END);
}

INLINE word_t space_t::get_copy_limit (addr_t addr, word_t len)
{
    word_t end = (word_t)addr + len;

    if( is_user_area(addr) )
    {
	if( end >= USER_AREA_END )
	    return (USER_AREA_END - (word_t)addr);
    }
    else
    {
	ASSERT( is_copy_area(addr) );
	word_t max = COPY_AREA_SIZE - ((word_t)addr - COPY_AREA_START);
	if( len > max )
	    return max;
    }

    return len;
}

INLINE fpage_t space_t::get_kip_page_area()
{
    return this->kip_area;
}

INLINE fpage_t space_t::get_utcb_page_area()
{
    return this->utcb_area;
}

INLINE word_t space_t::get_from_user(addr_t addr)
{
    return *(word_t *)(addr);
}

#ifdef CONFIG_PPC_MMU_SEGMENTS
INLINE word_t space_t::get_vsid( addr_t addr )
{
    // TODO: get_vsid() needs optimisation
    ppc_segment_t seg;
    if( (word_t)addr > KERNEL_OFFSET )
	seg = get_kernel_space()->get_segment_id();
    else
	seg = this->get_segment_id();
    return seg.raw | ((word_t)addr >> 28);
}

INLINE ppc_segment_t space_t::get_segment_id()
{
    ppc_segment_t seg;
    seg.raw = ((word_t)this >> POWERPC_PAGE_BITS) << 4;
    return seg;
}
#elif defined(CONFIG_PPC_MMU_TLB)
INLINE asid_manager_t<space_t, CONFIG_MAX_NUM_ASIDS> *get_asid_manager()
{
    extern asid_manager_t<space_t, CONFIG_MAX_NUM_ASIDS> asid_manager;
    return &asid_manager;
}

INLINE asid_t *space_t::get_asid(cpuid_t cpu)
{
#if defined(CONFIG_X_PPC_SOFTHVM)
    return hvm_mode ? NULL : &this->cpu[cpu].asid;
#else
    return &this->cpu[cpu].asid;
#endif
}
#endif

/**
 * add a thread to the space
 * @param tcb	pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb, cpuid_t cpu)
{
    thread_count++;
#ifdef CONFIG_SMP
    this->cpu[cpu].thread_count++;
#endif
}

/**
 * remove a thread from a space
 * @param tcb	thread control block
 * @return	true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb, cpuid_t cpu)
{
    ASSERT (thread_count !=  0);
#ifdef CONFIG_SMP
    this->cpu[cpu].thread_count--;
#endif
    thread_count--;
    return thread_count == 0;
}

INLINE void space_t::move_tcb(tcb_t * tcb, cpuid_t src_cpu, cpuid_t dst_cpu)
{
    cpu[dst_cpu].thread_count++;
    cpu[src_cpu].thread_count--;
}

#if defined(CONFIG_PPC_MMU_TLB)
void setup_kernel_mappings( void );
void install_exception_handlers( cpuid_t cpu );
#endif

#endif /* __GLUE__V4_POWERPC__SPACE_H__ */
