/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/space.h
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
 * $Id: space.h,v 1.15 2006/11/14 18:44:56 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC64__SPACE_H__
#define __GLUE__V4_POWERPC64__SPACE_H__

#include <debug.h>

#include INC_ARCH(vsid_asid.h)
#include INC_ARCH(pghash.h)

#include INC_API(fpage.h)
#include INC_API(thread.h)

#include INC_GLUE(pgent.h)
#include INC_GLUE(hwspace.h)

#if CONFIG_POWERPC64_STAB
#include INC_ARCH(stab.h)

#define HAVE_ARCH_FREE_SPACE
#endif

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
    void free();
#if defined(HAVE_ARCH_FREE_SPACE)
    void arch_free();
#endif
    bool sync_kernel_space (addr_t addr) { return false; }	// glue
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel); // api
    bool is_initialized();

    /* sigma0 handling */
    void map_sigma0(addr_t addr);				// glue
    void map_fpage(fpage_t snd_fp, word_t base, space_t * t_space,
		    fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all );
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);
    
    /* tcb management */
    void allocate_tcb(addr_t addr);				// glue
    void map_dummy_tcb(addr_t addr);				// glue
    utcb_t * allocate_utcb(tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    /* address ranges */
    inline bool is_user_area(addr_t addr);
    bool is_user_area(fpage_t fpage);
    bool is_tcb_area(addr_t addr);
    static inline bool is_kernel_area(addr_t addr);
    bool is_cpu_area(addr_t addr);

    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);

    bool is_arch_mappable(addr_t addr, size_t size) { return true; }

    /* Copy area related methods */
    inline bool is_copy_area (addr_t addr);
    word_t get_copy_limit (addr_t addr, word_t limit);

    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    fpage_t get_utcb_page_area();

    /* reference counting */
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);

    /* space control */
    word_t space_control (word_t ctrl) { return 0; }

    /* tlb */
    void flush_tlb( space_t *curspace );
    void flush_tlbent( space_t *curspace, addr_t addr, word_t log2size );
    static bool does_tlbflush_pay( word_t log2size )
	{ return log2size != POWERPC64_PAGE_BITS; }

    /* update hooks */
    static void begin_update() {}
    static void end_update() {}

public:
    /* powerpc64 specific functions */
    void init_kernel_mappings();

    bool handle_hash_miss( addr_t vaddr );
    bool handle_protection_fault( addr_t vaddr, bool dsi );
    bool handle_segment_miss( addr_t vaddr );

    inline pgent_t * get_pdir() { return this->pdir; }
    inline word_t get_vsid_asid() { return x.vsid_asid.get( this ); }
    word_t get_vsid( addr_t vaddr );
    inline static space_t *lookup_space( word_t vsid )
    {
	return get_vsid_asid_cache()->lookup( vsid );
    }

    word_t get_from_user( addr_t );

    /* Methods needed by linear page table walker. */
    pgent_t *pgent( word_t num, word_t cpu=0 );
    bool lookup_mapping( addr_t vaddr, pgent_t ** r_pg,
			 pgent_t::pgsize_e *r_size, cpuid_t cpu=0);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
	{ return *phys_to_virt((word_t*)paddr); }
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

#if CONFIG_POWERPC64_STAB
    ppc64_stab_t *get_seg_table() { return &x.segment_table; }
#endif
#if CONFIG_POWERPC64_SLB
    word_t  get_segment(int i) { return x.segments[i]; }
    void set_segment(int i, word_t val) { x.segments[i] = val; }
#endif
private:
    // TODO: when we create a new mapping that disables the cache,
    // we must flush the cache for that page to avoid cache paradoxes.

    void space_t::add_mapping( addr_t vaddr, addr_t paddr,
		    bool writable, bool executable,
		    bool kernel, pgent_t::pgsize_e size );

    void add_4k_mapping( addr_t vaddr, addr_t paddr, 
	    bool writable, bool kernel );

    void add_4k_mapping_noexecute( addr_t vaddr, addr_t paddr, 
	    bool writable, bool kernel );

    /* Power4 Large Mappings - 16MB */
    void add_large_mapping( addr_t vaddr, addr_t paddr, 
	    bool writable, bool kernel );

    /* In order to provide an almost full 64-bit user address space.
     * space_t is 1024 bytes (minimum kalloc size) */
    union {
    	pgent_t pdir[64];	/* 6-bits of top level page table */
	struct {
	    pgent_t user[64];
	} map;
    };
    union {
	word_t general[64];	/* 64 general pupose locations */
	struct {
	    /* If you add variables to this area, subtract corresponding 
	     * space from the resv[] region.
	     */
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    word_t  thread_count;
	    vsid_asid_t vsid_asid;  /* 9-bit ASID - shifted to fit VSID */

#if CONFIG_POWERPC64_SLB
	    /* XXX - we don't use this yet, we just do random replacement */
	    u64_t   slb_bitmap;	    /* Segement lookaside buffer usage bitmap */
	    word_t  segments[48];
	    word_t  resv[64-48-5];
#elif CONFIG_POWERPC64_STAB

	    ppc64_stab_t segment_table;
	    word_t  resv[64-5];
#endif
	} x;
    };
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
    return (get_pdir())->next( this, pgent_t::size_max, num );
}


/* XXX these can be optimised */
INLINE bool space_t::is_user_area(addr_t addr)
{
    return (addr >= (addr_t)USER_AREA_START &&
	    addr < (addr_t)USER_AREA_END);
}

INLINE bool space_t::is_tcb_area(addr_t addr)
{
    return (addr >= (addr_t)KTCB_AREA_START &&
	    addr < (addr_t)KTCB_AREA_END);
}

INLINE bool space_t::is_copy_area (addr_t addr)
{
    return (addr > (addr_t)COPY_AREA_START &&
	    addr < (addr_t)COPY_AREA_END);
}

INLINE bool space_t::is_kernel_area(addr_t addr)
{
    return (addr >= (addr_t)KERNEL_AREA_START &&
	    addr < (addr_t)KERNEL_AREA_END);
}

INLINE bool space_t::is_cpu_area(addr_t addr)
{
    return (addr >= (addr_t)CPU_AREA_START &&
	    addr < (addr_t)CPU_AREA_END);
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
    return this->x.kip_area;
}

INLINE fpage_t space_t::get_utcb_page_area()
{
    return this->x.utcb_area;
}

INLINE word_t space_t::get_from_user(addr_t addr)
{
    return *(word_t *)(addr);
}

INLINE tcb_t * space_t::get_tcb( threadid_t tid )
{
    return (tcb_t *)((KTCB_AREA_START) + 
		    ((tid.get_threadno() & (VALID_THREADNO_MASK)) * KTCB_SIZE));
}

INLINE tcb_t * space_t::get_tcb(void * ptr)
{
   return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}

INLINE word_t space_t::get_vsid( addr_t addr )
{
    word_t vsid;

    /* Kernel VSID = 0 */
    if( (word_t)addr >= USER_AREA_END)
	vsid = 0;
    else
	vsid = this->x.vsid_asid.get( this ) >> 12;

    /* Add VSID and ASID */
    return vsid | (((word_t)addr >> POWERPC64_SEGMENT_BITS) & ((1ul << CONFIG_POWERPC64_ESID_BITS)-1));
}

/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb, cpuid_t cpu)
{
    x.thread_count ++;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb, cpuid_t cpu)
{
    ASSERT(x.thread_count != 0);
    x.thread_count --;
    return (x.thread_count == 0);
}

INLINE void space_t::flush_tlb( space_t *curspace )
{
    // TODO: flush the tlb for a given address space.
    ppc64_invalidate_tlb();
}

INLINE void space_t::flush_tlbent( space_t *curspace, addr_t addr, 
	word_t log2size )
{
    ppc64_invalidate_tlbe( addr, (log2size == POWERPC64_PAGE_BITS) ? 0 : 1 );
}

INLINE void space_t::add_4k_mapping( addr_t vaddr, addr_t paddr, 
	bool writable, bool kernel )
{
    add_mapping( vaddr, paddr, writable, true, kernel, pgent_t::size_4k );
}

INLINE void space_t::add_4k_mapping_noexecute( addr_t vaddr, addr_t paddr, 
	bool writable, bool kernel )
{
    add_mapping( vaddr, paddr, writable, false, kernel, pgent_t::size_4k );
}

INLINE void space_t::add_large_mapping( addr_t vaddr, addr_t paddr, 
	bool writable, bool kernel )
{
#ifdef CONFIG_POWERPC64_LARGE_PAGES
    add_mapping( vaddr, paddr, writable, true, kernel, pgent_t::size_16m );
#else
    ASSERT(!"No large page support");
#endif
}

#if defined(HAVE_ARCH_FREE_SPACE)

INLINE void space_t::arch_free()
{
#if CONFIG_POWERPC64_STAB
    this->x.segment_table.free();
#endif
}

#endif

#endif /* __GLUE__V4_POWERPC64__SPACE_H__ */
