/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, University of New South Wales
 *
 * File path:    glue/v4-sparc64/ultrasparc/space.h
 * Description:  space_t implmentation for UltraSPARC CPUs.
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
 * $Id: space.h,v 1.8 2006/11/14 18:44:57 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__ULTRASPARC__SPACE_H__
#define __GLUE__V4_SPARC64__ULTRASPARC__SPACE_H__

#include <asid.h>
#include <debug.h>           /* for UNIMPLEMENTED() */

#include INC_API(types.h)
#include INC_API(fpage.h)    /* fpage_t    */
#include INC_API(thread.h)   /* threadid_t */
#include INC_API(queueing.h)
#include INC_GLUE_API_CPU(pgent.h)
#include INC_CPU(tsb.h)

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

#define PGSIZE_KTCB	(pgent_t::size_8k)
#define PGSIZE_KIP	(pgent_t::size_8k)
#define PGSIZE_UTCB	(pgent_t::size_8k)

/*******************
* inline functions *
*******************/

INLINE space_t * get_kernel_space()
{
    extern space_t kernel_space;

    return &kernel_space;
}

/* forward declarations - space_t depends on tcb_t and utcb_t */
class tcb_t;
class utcb_t;

/**
 * The address space representation
 */
class space_t {
public:
    enum access_e {
	read,
	write,
	readwrite,
	execute

    }; // access_e

    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();
    bool sync_kernel_space(addr_t addr);
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    bool is_initialized();

    /* mapping */
    void map_sigma0(addr_t addr);
    void map_fpage(fpage_t snd_fp, word_t base, 
		   space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);

    /* tcb management */
    void allocate_tcb(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t * allocate_utcb(tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    ringlist_t<space_t> get_spaces_list() {
	return spaces_list;
    }
    tcb_t * get_thread_list() {
	return thread_list;
    }
    void enqueue_spaces();
    void dequeue_spaces();

    /* address ranges */
    bool is_user_area(addr_t addr);
    bool is_user_area(fpage_t fpage);
    bool is_tcb_area(addr_t addr);
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);
    bool is_arch_mappable(addr_t addr, size_t size) { return true; }

    /* Copy area related methods */
    bool is_copy_area(addr_t addr);
    word_t get_copy_limit(addr_t addr, word_t limit);

    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    fpage_t get_utcb_page_area();

    /* reference counting */
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);

    /* space control */
    word_t space_control(word_t ctrl);

public: /* SPARC v9 specific functions. */

    /* update hooks */
    static void begin_update(void);
    static void end_update(void);

    /* linear page table walker methods */
    pgent_t * pgent(word_t num, word_t cpu = 0);
    void release_kernel_mapping(addr_t vaddr, addr_t paddr, word_t log2size);
    bool lookup_mapping(addr_t vaddr, pgent_t ** pg, pgent_t::pgsize_e * size);
    bool readmem(addr_t vaddr, word_t * contents);
    static word_t readmem_phys(addr_t paddr);

    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
		     tlb_t::cache_attrib_e cache, bool writable, bool executable,
		     bool kernel);

public: /* UltraSPARC specific functions. */

    bool handle_tsb_miss(addr_t vaddr, tlb_t::tlb_e tsb);
    word_t get_context() { return asid.get(); }
    pgent_t* get_pdir() { return (pgent_t*)(pgdir << SPARC64_PAGE_BITS); }
    static space_t* lookup_space(hw_asid_t hw_asid);

    /* linear page table walker TLB maintainence methods. */
    void flush_tlb(space_t * curspace);
    void flush_tlbent(space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay(word_t log2size);

    /* Kernel space. */
    static void SECTION(".init.memory") init_kernel_space(void);

private: /* SPARC v9 specific fields. */

    fpage_t kip_area;
    fpage_t utcb_area;
    word_t  thread_count;
    ringlist_t<space_t>	spaces_list;
    tcb_t * thread_list;

private: /* UltraSPARC specific fields. */

    struct {
	BITFIELD2(u64_t,
		  pgdir   : 51, /* */
		  context : 13  /* */

		 ) // BITFIELD2()

    }; // struct
    asid_t asid;

}__attribute__((packed)); // space_t

#include INC_GLUE_API_CPU(pgent_inline.h)

/**
 * enqueue a spaces into the spaces list
 * the present list primarily exists for debugging reasons
 */
#ifdef CONFIG_DEBUG
extern space_t * global_spaces_list;
extern spinlock_t spaces_list_lock;
#endif

INLINE void space_t::enqueue_spaces()
{
#ifdef CONFIG_DEBUG
    spaces_list_lock.lock();
    ENQUEUE_LIST_TAIL(global_spaces_list, this, spaces_list);
    spaces_list_lock.unlock();
#endif
}

INLINE void space_t::dequeue_spaces()
{
#ifdef CONFIG_DEBUG
    spaces_list_lock.lock();
    DEQUEUE_LIST(global_spaces_list, this, spaces_list);
    spaces_list_lock.unlock();
#endif
}

/* space_t: UltraSPARC specific functions. */

INLINE bool
space_t::handle_tsb_miss(addr_t vaddr, tlb_t::tlb_e tlb)
{
    pgent_t *pg;
    pgent_t::pgsize_e size;
    tsb_t::tsb_e tsb;

    if(!lookup_mapping(vaddr, &pg, &size)) {
	return false;
    }

    if(tlb == tlb_t::d_tlb || tlb == tlb_t::all_tlb) {
	tsb = (size == pgent_t::size_8k) ? tsb_t::d8k_tsb : tsb_t::d64k_tsb;
	pg->insert(this, size, vaddr, tsb);
    }

    if(tlb == tlb_t::i_tlb || tlb == tlb_t::all_tlb) {
	tsb = (size == pgent_t::size_8k) ? tsb_t::i8k_tsb : tsb_t::i64k_tsb;
	pg->insert(this, size, vaddr, tsb);
    }

    return true;
}

INLINE space_t *
space_t::lookup_space(hw_asid_t hw_asid)
{
    asid_cache_t* asid_cache = get_asid_cache();
    asid_t* asid = asid_cache->lookup(hw_asid);
    if(asid == (asid_t*)-1) { return NULL; }
    return (space_t*)((word_t)asid - offsetof(space_t, asid));
}

INLINE void
space_t::flush_tlb(space_t * curspace)
{
    if(this == get_kernel_space()) {
	/**
	 *  The kernel context must not be flushed otherwise the pinned mappings
	 *  for the Trap Table, etc will be flushed and the system will end up
	 *  in a bad state. If we ever need to implment this the handler will
	 *  need to manually sift through the TLB entries one at a time and
	 *  flush the unlocked ones only.
	 */
	UNIMPLEMENTED();

    } else { // User space.
	//if(this->context.is_valid()) { // An invalid context has no TLB entries.
	//  mmu_t::unmap(this->context, (this == curspace) ? 
	//		   context_t::primary : context_t::secondary,
	//		   tlb_t::all_tlb);
	//}
    }
}

INLINE void
space_t::flush_tlbent(space_t * curspace, addr_t vaddr, word_t log2size)
{
    if(this == get_kernel_space()) {
	//mmu_t::unmap(this->context, context_t::nucleus, vaddr, tlb_t::all_tlb);

    } else { // User space.

	//if(this->context.is_valid()) { // An invalid context has no TLB entries.
	//  mmu_t::unmap(this->context,(this == curspace) ?
	//		   context_t::primary : context_t::secondary,
	//		   vaddr, tlb_t::all_tlb);
	//}
    }
}

INLINE bool
space_t::does_tlbflush_pay(word_t log2size)
{
    UNIMPLEMENTED();

    return false;
}


#endif /* !__GLUE__V4_SPARC64__SPACE_H__ */
