/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/space.h
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
 * $Id: space.h,v 1.75 2006/11/14 18:44:56 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__SPACE_H__
#define __GLUE__V4_POWERPC__SPACE_H__

#include <debug.h>

#include INC_ARCH(page.h)
#include INC_ARCH(pgtab.h)
#include INC_ARCH(pghash.h)

#include INC_API(fpage.h)
#include INC_API(thread.h)

#include INC_GLUE(pgent.h)
#include INC_GLUE(hwspace.h)

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
    bool sync_kernel_space(addr_t addr);			// glue
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

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    /* address ranges */
    bool is_user_area(addr_t addr);
    bool is_user_area(fpage_t fpage);
    bool is_tcb_area(addr_t addr);
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);
    bool is_arch_mappable(addr_t addr, size_t size) { return true; }

    /* Copy area related methods */
    bool is_copy_area (addr_t addr);
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
	{ return log2size != POWERPC_PAGE_BITS; }

    /* update hooks */
    static void begin_update() {}
    static void end_update() {}

public:
    /* powerpc specific functions */
    void init_kernel_mappings();
    addr_t map_device( addr_t paddr, word_t size, bool cacheable );

    bool handle_hash_miss( addr_t vaddr );

    pgent_t *get_pdir();
    ppc_segment_t get_segment_id();
    word_t get_vsid( addr_t vaddr );

    pgent_t * page_lookup( addr_t vaddr );
    word_t get_from_user( addr_t );
    pgent_t *pgent( word_t num, word_t cpu=0 );
    bool lookup_mapping( addr_t vaddr, pgent_t ** r_pg,
			 pgent_t::pgsize_e *r_size, cpuid_t cpu = 0);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
	{ return *phys_to_virt((word_t*)paddr); }
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    static space_t *vsid_to_space( word_t vsid )
    {
	return (space_t *)( (vsid & 0xfffffff0) << (POWERPC_PAGE_BITS - 4) );
    }

private:
    // TODO: when we create a new mapping that disables the cache,
    // we must flush the cache for that page to avoid cache paradoxes.
#if 0
    enum mapping_e {
	tcb = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED |
		PPC_PAGE_DIRTY | PPC_PAGE_SMP_SAFE),

	dummytcb = (ppc_pgent_t::read_only | PPC_PAGE_ACCESSED),

	utcb = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED |
		PPC_PAGE_DIRTY | PPC_PAGE_SMP_SAFE),

	kip = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED | 
		PPC_PAGE_DIRTY | PPC_PAGE_SMP_SAFE),

	kmap = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED |
		PPC_PAGE_DIRTY | PPC_PAGE_SMP_SAFE),

	ptab = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED |
		PPC_PAGE_DIRTY | PPC_PAGE_SMP_SAFE),

	sigma0 = (ppc_pgent_t::read_write | PPC_PAGE_SMP_SAFE),

	device = (ppc_pgent_t::read_write | PPC_PAGE_ACCESSED | 
		PPC_PAGE_DIRTY | PPC_PAGE_GUARDED | PPC_PAGE_CACHE_INHIBIT),

	syscall = (ppc_pgent_t::read_only | PPC_PAGE_ACCESSED |
		PPC_PAGE_DIRTY)
    };
#endif

    void add_4k_mapping( addr_t vaddr, addr_t paddr, 
	    bool writable, bool kernel, bool cacheable=true );
//    ppc_pgent_t * add_4k_mapping( ppc_pgent_t *pgent, addr_t vaddr, addr_t paddr, mapping_e type );
    void flush_4k_mapping( addr_t vaddr, pgent_t *pgent );

    union {
    	pgent_t pdir[1024];
	struct {
	    pgent_t user[ USER_AREA_END >> PPC_PAGEDIR_BITS ];
	    pgent_t kernel[ KERNEL_AREA_SIZE >> PPC_PAGEDIR_BITS ];
	    pgent_t device[ DEVICE_AREA_SIZE >> PPC_PAGEDIR_BITS ];

	    /* If you add variables to this area, subtract corresponding 
	     * space from the pghash[] region.  We put them in the pghash
	     * region since we will never have page dir entries that
	     * map the pghash area (it is covered by a BAT register).
	     */
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    word_t thread_count;
	    pgent_t pghash[ (PGHASH_AREA_SIZE >> PPC_PAGEDIR_BITS) - 3 ];

	    pgent_t cpu[ (KTCB_AREA_START - CPU_AREA_START)>>PPC_PAGEDIR_BITS ];
	    pgent_t tcb[ KTCB_AREA_SIZE >> PPC_PAGEDIR_BITS ];
	} x;
    };

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
	    ((tid.get_threadno() & KTCB_THREADNO_MASK) * KTCB_SIZE));
}

INLINE tcb_t * space_t::get_tcb(void * ptr)
{
   return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}

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

/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb, cpuid_t cpu)
{
    /* Make sure the valid bit does not get set */
    x.thread_count += 16;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb, cpuid_t cpu)
{
    ASSERT(x.thread_count != 0);
    /* Make sure the valid bit does not get set */
    x.thread_count -= 16;
    return (x.thread_count == 0);
}

INLINE void space_t::flush_tlb( space_t *curspace )
{
    // TODO: flush the tlb for a given address space.
    ppc_invalidate_tlb();
}

INLINE void space_t::flush_tlbent( space_t *curspace, addr_t addr, 
	word_t log2size )
{
    ppc_invalidate_tlbe( addr );
}

#endif /* __GLUE__V4_POWERPC__SPACE_H__ */
