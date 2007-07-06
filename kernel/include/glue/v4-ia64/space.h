/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/space.h
 * Description:   IA-64 specific space implementation.
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
 * $Id: space.h,v 1.40 2006/11/14 18:44:56 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__SPACE_H__
#define __GLUE__V4_IA64__SPACE_H__

#if !defined(ASSEMBLY)
#include INC_GLUE(config.h)
#include INC_GLUE(utcb.h)
#include INC_API(thread.h)
#include INC_API(fpage.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(tlb.h)
#endif

#define KTCB_ALLOC_SIZE_SHIFT	(14)
#define KTCB_ALLOC_SIZE		(pgent_t::size_16k)
#define UTCB_ALLOC_SIZE		(pgent_t::size_4k)

#if !defined(ASSEMBLY)
#include <debug.h>		// Need UNIMPLEMENTED ()

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>


class tcb_t;
class pgent_t;

class space_t
{
    union {
	struct {
	    u64_t user[(1 << 10) - 3];
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    word_t thread_count;
	};
	u64_t pagedir[(1 << 10)];
    };

public:
    enum access_e {
	execute 	= 1,
	write		= 2,
	read		= 4,
	readwrite	= 6,
    };

    bool is_read (access_e a)
	{ return a & 4; }

    bool is_write (access_e a)
	{ return a & 2; }

    bool is_execute (access_e a)
	{ return a & 1; }


    //void map_tcb(tcb_t * tcb);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void init_cpu_mappings (cpuid_t cpu);
    void free();
    bool sync_kernel_space (addr_t addr) { return false; }
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    bool is_initialized();

    void map_sigma0(addr_t addr);
    void map_fpage(fpage_t snd_fp, word_t base, 
	space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);

    /* TCB management */
    void allocate_tcb(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t * allocate_utcb (tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    /* Address ranges */
    inline bool is_user_area (addr_t addr);
    bool is_user_area (fpage_t fpage);
    bool is_tcb_area (addr_t addr);
    bool is_mappable (addr_t addr);
    bool is_mappable (fpage_t addr);
    bool is_arch_mappable (addr_t addr, size_t size) { return true; }

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
    
    /* space update hooks */
    static void begin_update (void) {}
    static void end_update (void) {}

    /* generic page table walker */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg,
			 pgent_t::pgsize_e * size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
	{ return *(word_t*)phys_to_virt(paddr); }

    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay (word_t log2size)
	{ return log2size >= 64; }

    word_t get_region_id (void)
	{ return ((word_t) this >> 12) & ((1 << 18) - 1); }

    void add_mapping (addr_t vaddr, addr_t paddr,
		      pgent_t::pgsize_e size, bool kernel,
		      translation_t::memattrib_e memattrib,
		      translation_t::access_rights_e access_rights,
		      translation_t::type_e type,
		      word_t key);

    bool space_t::insert_translation (addr_t vaddr, space_t::access_e access);
};


INLINE fpage_t space_t::get_kip_page_area (void)
{
    return kip_area;
}

INLINE fpage_t space_t::get_utcb_page_area (void)
{
    return utcb_area;
}

INLINE bool space_t::is_user_area (addr_t addr)
{
    return (addr >= (addr_t) USER_AREA_START && 
	    addr < (addr_t) USER_AREA_END);
}

INLINE bool space_t::is_tcb_area (addr_t addr)
{
    return (addr >= (addr_t) KTCB_AREA_START &&
	    addr < (addr_t) KTCB_AREA_END);
}


INLINE bool space_t::is_copy_area (addr_t addr)
{
    return IA64_RR_NUM (addr) == 4;
}

INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit)
{
    word_t end = (word_t) virt_to_phys (addr) + limit;
    if (end >= USER_AREA_END)
	return (USER_AREA_END - (word_t) addr);
    return limit;
}


/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb (tcb_t * tcb)
{
    // Avoid touching bit 0 (page present)
    thread_count += 2;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb (tcb_t * tcb)
{
    ASSERT (thread_count != 0);
    thread_count -= 2;
    return (thread_count == 0);
}

INLINE tcb_t * space_t::get_tcb (threadid_t tid)
{
    return (tcb_t *) (KTCB_AREA_START + (tid.get_threadno () * KTCB_SIZE));
}

/**
 * space_t::get_tcb: translates a pointer into a valid tcb pointer
 */
INLINE tcb_t * space_t::get_tcb (void * ptr)
{
   return (tcb_t *) ((word_t) ptr & KTCB_MASK);
}



INLINE pgent_t * space_t::pgent (word_t num, word_t cpu)
{
    return ((pgent_t *) this)->next (this, pgent_t::size_max, num);
}

INLINE void space_t::flush_tlb (space_t * curspace)
{
    purge_tc (0, ia64_num_vaddr_bits, get_region_id ());
}

INLINE void space_t::flush_tlbent (space_t * curspace, addr_t vaddr,
				   word_t log2size)
{
    purge_tc (vaddr, log2size, get_region_id ());
}


/**********************************************************************
 *
 *                 global function declarations
 *
 **********************************************************************/

INLINE space_t * get_kernel_space (void)
{
    extern space_t * kernel_space;
    return kernel_space;
};

void init_kernel_space (void);


#endif /* !ASSEMBLY */
#endif /* !__GLUE__V4_IA64__SPACE_H__ */
