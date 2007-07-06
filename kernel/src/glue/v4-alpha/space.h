/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/space.h
 * Created:       24/07/2002 23:56:20 by Simon Winwood (sjw)
 * Description:   Address space data structure 
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
 * $Id: space.h,v 1.28 2006/11/14 18:44:56 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__SPACE_H__
#define __ARCH__ALPHA__SPACE_H__

#include <debug.h>
#include <asid.h>

#include INC_API(types.h)

#include INC_API(fpage.h)			/* fpage_t	*/
#include INC_API(thread.h)			/* threadid_t	*/
#include INC_ARCH(pgent.h)
#include INC_ARCH(palcalls.h)
#include INC_GLUE(config.h)

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

/* forward declarations - space_t depends on tcb_t and utcb_t */
class tcb_t;
class utcb_t;

/**
 * The address space representation.
 *
 * In a 43 bit system we stick the space data structure in the 256 entries that are unused 
 * due to kseg.
 * In a 48 bit system, we stick the space data structure after the 32 entries in the 
 * L0 page table. 
 */
class space_t {
public:
    enum access_e {
	read = 0, write = 1, execute = -1, readwrite = 1
    };

    space_t *allocate(void);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();
    bool sync_kernel_space(addr_t addr);
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    bool is_initialized();

    /* Mapping */
    /* linear_ptab_walker.cc */
    void map_sigma0(addr_t addr); 
    
    /* linear_ptab_walker.cc */
    void map_fpage(fpage_t snd_fp, word_t base, space_t * t_space, fpage_t rcv_fp, bool grant);  

    /* linear_ptab_walker.cc */
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);

    /* tcb management */
    void allocate_tcb(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t * allocate_utcb(tcb_t * tcb);
    utcb_t * utcb_to_kernel_space(utcb_t * utcb);
    utcb_t * map_utcb(utcb_t * utcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void *ptr);

    /* Address ranges */
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t);
    bool is_user_area(addr_t);
    bool is_user_area(fpage_t);
    bool is_tcb_area(addr_t addr);
    bool is_arch_mappable(addr_t addr, size_t size) { return true; }

    /* Copy area related methods */
    bool is_copy_area (addr_t addr);
    word_t get_copy_limit (addr_t addr, word_t limit);

    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    fpage_t get_utcb_page_area();

    /* Reference counting */
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);

    /* space control */
    word_t space_control (word_t ctrl) { return 0; }

    /*
     *  Alpha Specifics
     */
    
    /* Linear ptab stuff */
    /* Provides: */
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg,
			 pgent_t::pgsize_e * size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
        { return *(word_t *) phys_to_virt((word_t *) paddr); }

    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    /* Requires: */
    pgent_t *pgent(word_t num, word_t cpu = 0);
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);

    /* sjw (28/07/2002): Follow PPC ... doesn't really pay to flush the TLB ... */
    bool does_tlbflush_pay (word_t log2size)
	{ return false; }

    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, bool writable, bool kernel);

    /* Alpha Specifics */
    word_t translate(addr_t va);

    asid_t *get_asid(void) 
	{ return &asid; }

    pgent_t *get_ptbr(void) {
	return (pgent_t *) ((word_t) this & ALPHA_PAGE_MASK);	
    }
    
    static space_t *ptbr_to_space(addr_t ptbr);
    void init_kernel_mappings(void);

    static void begin_update(void) {}
    static void end_update(void) {}


 private:
    /* Generic stuff */
    fpage_t kip_area;
    fpage_t utcb_area;
    word_t thread_count;

    /* Alpha specifics */
    asid_t asid;
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
    return addr >= (addr_t) USER_AREA_START && addr < (addr_t) USER_AREA_END;
}
        
INLINE bool space_t::is_tcb_area (addr_t addr)
{
    return addr >= (addr_t) KTCB_AREA_START && addr < (addr_t) KTCB_AREA_END;
}

INLINE bool space_t::is_copy_area (addr_t addr)
{
    UNIMPLEMENTED();
    return false;
}

INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit)
{
    UNIMPLEMENTED ();
    return limit;
}

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
    /* sjw (26/07/2002): This won't allocate a TCB ... */
    return (tcb_t*)((KTCB_AREA_START) + (tid.get_threadno() * KTCB_SIZE));
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
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    /* sjw (30/07/2002): SMP unsafe */
    thread_count--;
    return (thread_count == 0);
}

/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb)
{
    /* sjw (30/07/2002): SMP unsafe */
    thread_count++;
}

/**
 * ptbr_to_space - Given a page table, returns the associated space.
 * @ptbr: The page table in question
 *
 *
 **/
INLINE space_t *space_t::ptbr_to_space(addr_t ptbr)
{
    return (space_t *) ((word_t) ptbr + PTBR_SPACE_OFFSET);
}


INLINE pgent_t *space_t::pgent (word_t num, word_t cpu)
{
    return get_ptbr()->next(this, pgent_t::size_max, num);
}


/* From the Alpha ARM */
#define VPT_LSHIFT  (64 - ((ALPHA_PAGE_BITS * (ALPHA_PT_LEVELS + 1)) - (ALPHA_PT_LEVELS * 3)))
#define VPT_RSHIFT  (64 - ((ALPHA_PAGE_BITS * (ALPHA_PT_LEVELS + 1)) - (ALPHA_PT_LEVELS * 3)) + ALPHA_PAGE_BITS - 3)

/**
 * Translate a valid VA to the corresponding VA
 * @param va    The virtual address to translate.
 *
 * This method translates a virtual address to the corresponding
 * physical address.  Note that this uses the linear page table, so it
 * is possible to take a TLB miss or page fault here.
 *
 * This only works on the current AS, or on shared mappings (like the
 * KTCB array).  Do NOT use this method for VAs in address spaces
 * other than current and kernel_space.
 *
 * This method should be reasonably fast, especially if the VA has
 * been recently touched.
 **/
INLINE word_t space_t::translate(addr_t va)
{
    word_t idx = ((word_t) va & ALPHA_PAGE_MASK) << VPT_LSHIFT;
    idx >>= VPT_RSHIFT;
    
    word_t *pte = (word_t *) ((VLPT_AREA_START | idx) & ~0x3); 

    /* sjw (06/08/2002): We should really use the pgent_t here, but this is much quicker ;( */
    return (((*pte >> 32) << ALPHA_PAGE_BITS) | ((word_t) va & ALPHA_OFFSET_MASK)); 
}

#undef VPT_LSHIFT
#undef VPT_RSHIFT


/* PAL doesn't support flushing based on ASNs, so if we want to flush an address space
 * we need to flush the entire TLB.  Another idea would be to recycle the ASN */
INLINE void space_t::flush_tlb(space_t * curspace)
{
    /* Only flush kernel mappings if requested */
    if(curspace == get_kernel_space())
	PAL::tbia();
    else
	PAL::tbiap();
}
 
INLINE void space_t::flush_tlbent(space_t * curspace, addr_t vaddr, word_t log2size)
{
    /* sjw (30/07/2002): Braindead */
    flush_tlb(curspace);
}

void init_kernel_space(void);

#endif /* __ARCH__ALPHA__SPACE_H__ */
