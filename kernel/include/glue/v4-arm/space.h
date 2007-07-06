/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-arm/space.h
 * Description:	ARM specific space implementation.
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
 * $Id: space.h,v 1.21 2006/10/20 19:04:15 reichelt Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_ARM__SPACE_H
#define __GLUE__V4_ARM__SPACE_H

#include INC_API(fpage.h)			/* fpage_t	*/
#include INC_API(thread.h)			/* threadid_t	*/
#include INC_ARCH(fass.h)
#include INC_ARCH(pgent.h)
#include INC_CPU(cache.h)
#include INC_GLUE(config.h)

/* forward declarations - space_t depends on tcb_t and utcb_t */
class tcb_t;
class utcb_t;

/**
 * The address space representation
 */
class space_t {
public:
    /* TCB management */
    void allocate_tcb(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t * allocate_utcb(tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void *ptr);

    /* Address ranges */
    bool is_user_area(addr_t);
    bool is_user_area(fpage_t);
    bool is_tcb_area(addr_t addr);
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t);
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
    word_t space_control (word_t ctrl) { 
#ifdef CONFIG_ENABLE_FASS
        set_pid((ctrl & 0xe) << 25); /* Only 7 LSBs significant for PID */ 
#endif
        return 0; 
    }

    /* space update hooks */
    static void begin_update (void) {}
    static void end_update (void) {}

    bool is_initialized();

    void map_fpage(fpage_t snd_fp, word_t base, space_t * t_space, 
            fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();

    enum access_e {
	read = 0,
	write = 1,
	readwrite = -1,
	execute = 2
    };

    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    void map_sigma0(addr_t addr);
    bool sync_kernel_space(addr_t addr);
    utcb_t * utcb_to_kernel_space(utcb_t * utcb);

    /* Methods needed by linear page table walker. */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg,
			 pgent_t::pgsize_e * size);
    bool readmem (addr_t vaddr, word_t * contents);
    word_t readmem_phys (addr_t paddr);

    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    void init_kernel_mappings(void);

    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
            bool writeable, bool kernel, bool uncached);

    inline void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size,
            bool writeable, bool kernel)
    {
	add_mapping(vaddr, paddr, size, writeable, kernel, false);
    }

    void remove_mapping(addr_t vaddr, pgent_t::pgsize_e size,
	    bool kernel);

    /* TLB releated methods used by linear page table walker. */
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    inline bool does_tlbflush_pay(word_t log2size)
    { return true; }    

#ifdef CONFIG_ENABLE_FASS
    arm_domain_t get_domain(void);
    void set_domain(arm_domain_t new_domain);
    arm_pid_t get_pid(void); 
    void set_pid(arm_pid_t pid);
#endif

    union pt_u {
	/* Pagetable is 2x because MDB linknodes after the page table */
	struct {
	    pgent_t pdir  [1 << ARM_SECTION_BITS];
	    /* FIXME - should MDB linknodes be cached? */
	    pgent_t lnodes[1 << ARM_SECTION_BITS];
	};
	struct {
	    pgent_t user_area[USER_AREA_SECTIONS];
	    pgent_t ktcb_area[KTCB_AREA_SECTIONS];
	    pgent_t kernel_area[KERNEL_AREA_SECTIONS];
	    pgent_t uncache_area[UNCACHE_AREA_SECTIONS];
	    pgent_t copy_area[COPY_AREA_SECTIONS];

	    /* Should be cache aligned here */

	    /* (ht) This is tricky - potentially a security problem.
	     * All of these variables _must_ have their lower two bits
	     * set to 00, since that will give a fault. 
	     */
	    arm_domain_t domain;
	    word_t pid;
	    tcb_t *threads;    // tcb & will be word aligned at least
	    fpage_t utcb_area; // assume only modified in space_t::init
	    fpage_t kip_area;  // assume only modified in space_t::init
	    word_t thread_count;
	    pgent_t var_area[VAR_AREA_SECTIONS - 6];

	    pgent_t io_area[IO_AREA_SECTIONS];
	    pgent_t misc_area[MISC_AREA_SECTIONS];
	    pgent_t high_int_vector;
        } pd_split;
    } pt;
};

#ifdef CONFIG_ENABLE_FASS
    INLINE arm_domain_t space_t::get_domain(void)
    {
        return this->pt.pd_split.domain >> 2;
    }

    INLINE void space_t::set_domain(arm_domain_t new_domain)
    {
        this->pt.pd_split.domain = new_domain << 2;
    }

    INLINE arm_pid_t space_t::get_pid(void)
    {
        return this->pt.pd_split.pid;
    }

    INLINE void space_t::set_pid(arm_pid_t new_pid)
    {
        this->pt.pd_split.pid = new_pid;
    }
#endif

INLINE pgent_t *space_t::pgent(word_t num, word_t cpu) 
{
    return (pgent_t*)virt_to_page_table(&(pt.pdir[num]));
}

/**
 * get the KIP area of an address space
 * @returns the KIP area of the address space as an fpage
 */
INLINE fpage_t space_t::get_kip_page_area (void)
{
    return pt.pd_split.kip_area; 
}

/**
 * get the UTCB area of an address space
 * @returns the utcb area of the address space as an fpage
 */
INLINE fpage_t space_t::get_utcb_page_area (void)
{
    return pt.pd_split.utcb_area;
}

INLINE bool space_t::is_user_area (addr_t addr)
{
    return ((word_t)addr < USER_AREA_END);
}

/* KTCB Area */
INLINE bool space_t::is_tcb_area (addr_t addr)
{
    return ((word_t)addr >= KTCB_AREA_START && (word_t)addr < KTCB_AREA_END);
}


/**
 * Check whether address resides within copy area.
 *
 * @param addr			address to check against
 *
 * @return true if address is within copy area; false otherwise
 */
INLINE bool space_t::is_copy_area (addr_t addr)
{
    return ((word_t)addr >= COPY_AREA_START && (word_t)addr < COPY_AREA_END);
}

/**
 * Get the limit of an IPC copy operation (e.g., copy from operation
 * is not allowed to go beyond the boundaries of the user area).
 *
 * @param addr			address to copy from/to
 * @param limit			intended copy size
 *
 * @return limit clipped to the allowed copy size
 */
INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit)
{
    word_t end = (word_t) addr + limit;

    if (is_user_area (addr)) {
        // Address in user area.  Do not go beyond user-area boundary.
        if (end >= USER_AREA_END)
            return (USER_AREA_END - (word_t) addr);
    } else {
        // Address in copy-area.  Make sure that we do not go beyond
        // the boundary of current copy area.
        ASSERT (is_copy_area (addr));
        if (addr_align (addr, COPY_AREA_BLOCK_SIZE) !=
                addr_align ((addr_t) end, COPY_AREA_BLOCK_SIZE)) {
            return (word_t) addr_align_up (addr, COPY_AREA_BLOCK_SIZE) -
                    (word_t) addr;
        }
    }

    return limit;
}

/* May as well return kernel space since mappings should be identical for
 * the kernel area as in the each of the user's address spaces. 
 */

INLINE space_t* get_kernel_space()
{
    extern space_t * kernel_space;
    return kernel_space;
}


/**
 * translates a global thread ID into a valid tcb pointer
 * @param tid thread ID
 * @returns pointer to the TCB of thread tid
 */
INLINE tcb_t * space_t::get_tcb( threadid_t tid )
{
    return (tcb_t*)((KTCB_AREA_START) +
		    ((tid.get_threadno() & (VALID_THREADNO_MASK)) * KTCB_SIZE));
}

/**
 * translates a pointer within a tcb into a valid tcb pointer
 * @param ptr pointer to somewhere in the TCB
 * @returns pointer to the TCB
 */
INLINE tcb_t * space_t::get_tcb (void * ptr)
{
    return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}


/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */

#define THR_COUNT_SHIFT 2
 
INLINE void space_t::add_tcb(tcb_t * tcb)
{
    pt.pd_split.thread_count+= (1<<THR_COUNT_SHIFT);
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    pt.pd_split.thread_count-= (1<<THR_COUNT_SHIFT);
    return (pt.pd_split.thread_count == 0);
}

void init_kernel_space();

#endif
