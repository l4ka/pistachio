/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/space.h
 * Description:   Address space for MIPS32
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
 * $Id: space.h,v 1.3 2006/11/14 18:44:56 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__SPACE_H__
#define __GLUE__V4_MIPS32__SPACE_H__

#include INC_API(fpage.h)			/* fpage_t	*/
#include INC_API(thread.h)			/* threadid_t	*/
#include INC_API(config.h)

#include INC_GLUE(config.h)
#include INC_GLUE(hwspace.h)

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

private:

    union {
        pgent_t pdir[1024];			// page directory covering 4GB
        struct {
            word_t padding_0[512];	// kuseg pagetable
				
            fpage_t utcb_area;		// this part (kseg0) will never be mapped
            fpage_t kip_area;		//   -> use mem to store some other stuff 
            word_t thread_count; 
            asid_t asid;

            word_t padding_1[508];	// nothing and kseg2/kseg3 pagetable
        } lx;
    };
		
public:

    pgent_t* get_pdir() {
        return( this->pdir );
    }

    utcb_t * allocate_utcb (tcb_t * tcb);

    /**
     * Test whether an address is in a mappable page, that is, no
     * kernel pages, not the KIP and not within the UTCB area.
     * @param addr address to test
     * Returns true if address is in a mappable page.
     */
    bool is_mappable(addr_t addr);

    /**
     * test whether an fpage is mappable
     */
    /* Address ranges */
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

    /* space control */
    word_t space_control (word_t ctrl) { 
        return 0; 
    }

    bool is_initialized();

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void *ptr);

    void map_fpage(fpage_t snd_fp, word_t base, space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);
    bool allocate_utcb(tcb_t * tcb, utcb_t * &kern_utcb, utcb_t * &user_utcb);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);
    void allocate_tcb(addr_t addr);
    void free();

    enum access_e {
        read, write, readwrite, execute,
    };
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    void map_sigma0(addr_t addr);
    bool sync_kernel_space(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t * utcb_to_kernel_space(utcb_t * utcb);

    /* Methods needed by linear page table walker. */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg, pgent_t::pgsize_e * size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr) { 
        return *(word_t*)phys_to_virt((word_t*)paddr); 
    }
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    /* TLB releated methods used by linear page table walker. */
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay (word_t log2size);

    /* Update hooks to allow for efficient XCPU TLB updates and * invalidations, invoked by the linear ptab walker */
    static void begin_update() {}
    static void end_update() {}

    /* generic page table walker */
    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, bool writable, bool kernel, bool global, bool cacheable = true);
		
    /* virtual asid management */
    asid_t* get_asid() {
        return( &(lx.asid) );
    }

};

INLINE space_t* get_kernel_space() {
    extern space_t* kernel_space;
    return kernel_space;
}


/**
 * get the KIP area of an address space
 * @returns the KIP area of the address space as an fpage
 */
INLINE fpage_t space_t::get_kip_page_area (void) {
    return( lx.kip_area );
}

/**
 * get the UTCB area of an address space
 * @returns the utcb area of the address space as an fpage
 */
INLINE fpage_t space_t::get_utcb_page_area (void) {
    return( lx.utcb_area );
}

INLINE bool space_t::is_user_area (addr_t addr) {
    return( (word_t)addr >= 0 && (word_t)addr < KSEG0_BASE );
}
        
INLINE bool space_t::is_tcb_area (addr_t addr) {
    return( (u32_t)addr >= (u32_t)KTCB_AREA_START );
}


/**
 * Check whether address resides within copy area.
 *
 * @param addr			address to check against
 *
 * @return true if address is within copy area; false otherwise
 */
INLINE bool space_t::is_copy_area (addr_t addr) {
    return( (word_t)addr >= COPY_AREA_START && (word_t)addr < COPY_AREA_END );
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
INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit) {
	
    word_t end = (word_t)addr + limit;

    if( is_user_area(addr) ) {
        if( end >= COPY_AREA_END )
            return( COPY_AREA_END - (word_t)addr );
    }
    else {
        ASSERT( !"space_t::get_copy_limit - address not in user space" );
    }

    return limit;

}

/**
 * translates a global thread ID into a valid tcb pointer
 * @param tid thread ID
 * @returns pointer to the TCB of thread tid
 */
INLINE tcb_t * space_t::get_tcb( threadid_t tid ) {
    return( (tcb_t*)( (word_t)KTCB_AREA_START + KTCB_SIZE * tid.get_threadno() ) );
}

/**
 * translates a pointer within a tcb into a valid tcb pointer
 * @param ptr pointer to somewhere in the TCB
 * @returns pointer to the TCB
 */
INLINE tcb_t * space_t::get_tcb (void * ptr) {
    return( (tcb_t*)((word_t)ptr & KTCB_MASK) );
}


/**
 * adds a thread to the space
 * @param tcb pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb) {
    lx.thread_count++;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb) {
    ASSERT( lx.thread_count > 0 );
    lx.thread_count--;
    return( lx.thread_count == 0 );
}


/* TLB releated methods used by linear page table walker. */
INLINE void space_t::flush_tlb (space_t * curspace) {
    ASSERT( !"space_t::flush_tlb(space_t*) is not implemented" );
    //get_tlb()->flush( (word_t)(curspace->get_asid()->get()) );
}

INLINE void space_t::flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size) {
    ASSERT( curspace->get_asid()->is_valid() );
    get_tlb()->flush( (word_t)(curspace->get_asid()->get()) ); // XXX only flush area
}

INLINE bool space_t::does_tlbflush_pay (word_t log2size) {
    return( false ); 
}

/* Methods needed by linear page table walker. */
INLINE pgent_t * space_t::pgent (word_t num, word_t cpu ) {
    return( ((pgent_t *)pdir)->next( this, pgent_t::size_4m, num ) );
}


void init_kernel_space();

#endif /* !__GLUE__V4_MIPS32__SPACE_H__ */
