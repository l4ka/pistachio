/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,   University of New South Wales
 *                
 * File path:     glue/v4-mips64/space.h
 * Description:   MIPS64 specific space implementation.
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
 * $Id: space.h,v 1.22 2006/11/14 18:44:56 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS64__SPACE_H__
#define __GLUE__V4_MIPS64__SPACE_H__

#include <debug.h> /* for UNIMPLMENTED() */

#include INC_API(types.h)

#include INC_API(fpage.h)			/* fpage_t	*/
#include INC_API(thread.h)			/* threadid_t	*/
#include INC_GLUE(config.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(tlb.h)
#include <asid.h>

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

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

    bool is_initialized();

    void map_fpage(fpage_t snd_fpage, word_t snd_base, space_t * dst_space, fpage_t rcv_fpage, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();

    enum access_e {
	read, write, readwrite, execute,
    };
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    void map_sigma0(addr_t addr);
    bool sync_kernel_space(addr_t addr);
    utcb_t * map_utcb(utcb_t * utcb);
    utcb_t * utcb_to_kernel_space(utcb_t * utcb);

    /* Methods needed by linear page table walker. */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg,
			 pgent_t::pgsize_e * size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
	{ return *(word_t*)phys_to_virt((word_t*)paddr); }
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    void init_kernel_mappings(void);
    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, bool writable, bool kernel);

    /* TLB releated methods used by linear page table walker. */
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay (word_t log2size)
	{ return false; }

    /* MIPS64 specific */
    asid_t *get_asid(void) 
	{ return &asid; }

private:

    union {
	struct {
	    u64_t maps[(1 << 10) - 4];
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    word_t thread_count;
	    /* MIPS sepecific */
	    asid_t asid;
	};
	u64_t pagedir[(1 << 10)];
    };
};


/**
 * get the KIP area of an address space
 * @returns the KIP area of the address space as an fpage
 */
INLINE fpage_t space_t::get_kip_page_area (void)
{
    return kip_area;
}

/**
 * get the UTCB area of an address space
 * @returns the utcb area of the address space as an fpage
 */
INLINE fpage_t space_t::get_utcb_page_area (void)
{
    return utcb_area;
}

INLINE bool space_t::is_user_area (addr_t addr)
{
    return (addr >= (addr_t) USER_AREA_START && 
	    addr <= (addr_t) USER_AREA_END);
}
        
INLINE bool space_t::is_tcb_area (addr_t addr)
{
    /* carl page table hack */
    addr = (addr_t)((word_t)addr & (~(1UL << (hw_pgshifts[pgent_t::size_max+1]-1))));
    return (addr >= (addr_t) KTCB_AREA_START &&
	    addr <= (addr_t) KTCB_AREA_END);
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
    return (addr >= (addr_t)COPY_AREA_START &&
	    addr < (addr_t)COPY_AREA_END);
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
    word_t end = (word_t)addr + limit;

    if (is_user_area (addr))
    {
	// Address in user area.  Do not go beyond user-area boundary.
	if (end >= USER_AREA_END)
	    return (USER_AREA_END - (word_t) addr);
    }
    else
    {
	// Address in copy-area.  Make sure that we do not go beyond
	// the boundary of current copy area.
	ASSERT (is_copy_area (addr));
	if (addr_align (addr, COPY_AREA_SIZE) !=
	    addr_align ((addr_t) end, COPY_AREA_SIZE))
	{
	    return (word_t) addr_align_up (addr, COPY_AREA_SIZE) -     
		(word_t) addr;
	}
    }

    return limit;
}

INLINE space_t* get_kernel_space()
{
    extern space_t * kernel_space;
    return kernel_space;
}

INLINE space_t * get_saved_pagetable(void)
{
    word_t  saved_page_table;
    asm (
	"dmfc0	%0, "STR(CP0_CONTEXT)"\n\t"
	"dsra	%0, %0, 32\n\t"
	: "=r" (saved_page_table)
    );

    return (space_t *) saved_page_table;
};


/**
 * translates a global thread ID into a valid tcb pointer
 * @param tid thread ID
 * @returns pointer to the TCB of thread tid
 */
INLINE tcb_t * space_t::get_tcb( threadid_t tid )
{
    return (tcb_t *) (KTCB_AREA_START +
		    ((tid.get_threadno() & (VALID_THREADNO_MASK)) * KTCB_SIZE));
}

/**
 * translates a pointer within a tcb into a valid tcb pointer
 * @param ptr pointer to somewhere in the TCB
 * @returns pointer to the TCB
 */
INLINE tcb_t * space_t::get_tcb (void * ptr)
{
    return (tcb_t *)((word_t)(ptr) & KTCB_MASK);
}


/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb)
{
    thread_count++;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    thread_count--;
    return (thread_count == 0);
}

INLINE pgent_t *space_t::pgent (word_t num, word_t cpu)
{
    return ((pgent_t *)this)->next(this, pgent_t::size_max, num);
}

INLINE void space_t::flush_tlb(space_t * curspace)
{
    flush_tc (this->get_asid()->get());
}

INLINE void space_t::flush_tlbent(space_t * curspace, addr_t vaddr, word_t log2size)
{
    purge_tc (vaddr, log2size, this->get_asid()->get());
}

void init_kernel_space(void);

#endif /* !__GLUE__V4_MIPS64__SPACE_H__ */
