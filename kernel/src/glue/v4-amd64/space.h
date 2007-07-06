/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/space.h
 * Description:   AMD64 space_t implementation
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
 * $Id: space.h,v 1.20 2006/11/14 18:44:56 skoglund Exp $
 * 
 ********************************************************************/
#ifndef __GLUE__V4_AMD64__SPACE_H__
#define __GLUE__V4_AMD64__SPACE_H__

#include INC_API(types.h)	
#include INC_API(fpage.h)	
#include INC_API(thread.h)	
#include INC_ARCH(ptab.h)
#include INC_ARCH(pgent.h)
#include INC_ARCH(amd64.h)
#include INC_GLUE(config.h)

#if defined(CONFIG_IO_FLEXPAGES)
#include INC_PLAT(io_space.h)
#endif

// Even if new MDB is not used we need the mdb_t::ctrl_t
#include <mdb.h>

/* forward declarations - space_t depends on tcb_t and utcb_t */
class tcb_t;
class utcb_t;

typedef struct {
    amd64_pgent_t copy_area[COPY_AREA_COUNT][COPY_AREA_SIZE >> AMD64_PDP_BITS];
    amd64_pgent_t reserved[512 - 6 -  COPY_AREA_COUNT * ((COPY_AREA_SIZE >> AMD64_PDP_BITS))];
    amd64_pgent_t ktcb;
    amd64_pgent_t remap32[4];
    amd64_pgent_t kernel_area;
} amd64_kernel_pdp_t __attribute__((aligned(AMD64_PTAB_BYTES)));

    
/**
 * The address space representation
 */
class space_t {
public:
    enum access_e {
	read		= 0,
	write		= 2,
	readwrite	= -1,
	execute		= 16
    };

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
    bool is_arch_mappable(addr_t addr, size_t size);

    /* Copy area related methods */
    bool is_copy_area (addr_t addr);
    word_t get_copy_limit (addr_t addr, word_t limit);
    void delete_copy_area (word_t n, cpuid_t cpu);
    void populate_copy_area (word_t n,pgent_t * src_pdp, cpuid_t  cpu);


    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    void set_kip_page_area(fpage_t utcb);
    fpage_t get_utcb_page_area();
    void set_utcb_page_area(fpage_t utcb);

    bool is_initialized();

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void *ptr);

    void map_fpage(fpage_t snd_fp, word_t base, space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);
    fpage_t mapctrl (fpage_t fpage, mdb_t::ctrl_t ctrl,
		     word_t attribute, bool unmap_all);
    utcb_t * allocate_utcb(tcb_t * tcb);
    void init(fpage_t utcb_area, fpage_t kip_area);
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);
    void allocate_tcb(addr_t addr);
    void free();

    /* space control */
    word_t space_control (word_t ctrl);

    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    void map_sigma0(addr_t addr);
    bool sync_kernel_space(addr_t addr);
    utcb_t * map_utcb(utcb_t * utcb);
    void map_dummy_tcb(addr_t addr);
    utcb_t * utcb_to_kernel_space(utcb_t * utcb);

    /* Methods needed by linear page table walker. */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** pg,
			 pgent_t::pgsize_e * size);
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr)
	{ 
	    ASSERT( (word_t) paddr < (1ULL << 32));
	    return * (word_t *) ( (word_t) paddr + REMAP_32BIT_START); 
	}

    /* TLB releated methods used by linear page table walker. */
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay (word_t log2size) { return false; };

#if defined(CONFIG_IO_FLEXPAGES)
    /* io-flexpages specific functions*/
    void install_io_bitmap(addr_t new_bitmap);
    void free_io_bitmap(void);
    addr_t get_io_bitmap();
    void set_io_space(io_space_t *n);
    io_space_t *get_io_space(void); 
    #   define HAVE_ARCH_FREE_SPACE
    void arch_free (void);
#endif

#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    /* Compatibility Mode specific functions */
    bool is_compatibility_mode();
#endif

    /* amd64 specific functions */
    void init_kernel_mappings();
    void init_cpu_mappings(cpuid_t cpu);
    amd64_pgent_t *get_pml4(cpuid_t cpu=0);
    word_t get_from_user(addr_t); 
    
    /* generic page table walker */
    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, bool writable, bool kernel, bool global = false, bool cacheable = true);
    void remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize, word_t len, bool writable, bool kernel, bool global = false);
    void free_pagetables(fpage_t fpage);

    /* update hooks */
    static void begin_update() { }
    static void end_update(); 

private:
    union {
	/* 1 per CPU */
	amd64_pgent_t pml4[512 * CONFIG_SMP_MAX_CPUS];
	struct {
	    amd64_pgent_t user[AMD64_PML4_IDX(USER_AREA_END)];
#if defined(CONFIG_IO_FLEXPAGES)
	    amd64_pgent_t tss_pdp;
#endif
	    amd64_pgent_t kernel_pdp;
         } x [CONFIG_SMP_MAX_CPUS] __attribute__((aligned(AMD64_PTAB_BYTES)));
    };

    struct {
	fpage_t kip_area;
	fpage_t utcb_area;
	word_t thread_count;
#if defined(CONFIG_IO_FLEXPAGES)
	io_space_t *io_space;
#endif
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
	bool compatibility_mode;
#endif
    }  misc;

};

#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
/**
 * get Compatibility Mode setting
 * @returns true if Compatibility Mode is activated
 */
INLINE bool space_t::is_compatibility_mode()
{
    return misc.compatibility_mode;
}
#endif

INLINE amd64_pgent_t * space_t::get_pml4(cpuid_t cpu)
{
#if !defined(CONFIG_SMP)
    ASSERT(cpu == 0);
#else
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
#endif
    return virt_to_phys(&pml4[cpu * 512]);
}

/**
 * get the KIP area of an address space
 * @returns the KIP area of the address space as an fpage
 */
INLINE fpage_t space_t::get_kip_page_area (void)
{
    //TRACEF("kip_area = %p\n", misc.kip_area);
    return misc.kip_area;
}

/**
 * set the KIP area of an address space
 */
INLINE void space_t::set_kip_page_area (fpage_t kip_area)
{
    this->misc.kip_area = kip_area;
}

/**
 * get the UTCB area of an address space
 * @returns the utcb area of the address space as an fpage
 */

INLINE fpage_t space_t::get_utcb_page_area (void)
{
    //TRACEF("utcb_area = %p\n", misc.utcb_area);
    return misc.utcb_area;
}


/**
 * set the UTCB area of an address space
 * @returns the utcb area of the address space as an fpage
 */

INLINE void space_t::set_utcb_page_area (fpage_t utcb_area)
{
    this->misc.utcb_area = utcb_area;
}


INLINE bool space_t::is_user_area (addr_t addr)
{
    return (((word_t) addr | AMD64_SIGN_EXTENSION) >= USER_AREA_START &&
	    ((word_t) addr | AMD64_SIGN_EXTENSION) < USER_AREA_END);
}
        
INLINE bool space_t::is_tcb_area (addr_t addr)
{
    return (((word_t) addr | AMD64_SIGN_EXTENSION) >= KTCB_AREA_START &&
	    ((word_t) addr | AMD64_SIGN_EXTENSION) < KTCB_AREA_END);
}

INLINE bool space_t::is_arch_mappable (addr_t addr, size_t size)
{
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    return (!this->misc.compatibility_mode
	    || ((word_t) addr_offset(addr, size)) <= UTCB_MAPPING_32
	    || (word_t) addr >= UTCB_MAPPING_32 + AMD64_4KPAGE_SIZE);
#else
    return true;
#endif
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
    return (((word_t)addr | AMD64_SIGN_EXTENSION) >= COPY_AREA_START &&
	    ((word_t)addr | AMD64_SIGN_EXTENSION) < COPY_AREA_END);
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
    word_t end = (word_t) virt_to_phys (addr) + limit;

    if (is_user_area (addr))
    {
	// Address in user area.  Do not go beyond user-area boundary.
	if (end >= USER_AREA_END)
	    return (USER_AREA_END - (word_t) addr);   return limit;
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
 
INLINE void space_t::delete_copy_area (word_t n, cpuid_t cpu)
{

    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    amd64_kernel_pdp_t *kpdp = (amd64_kernel_pdp_t *) phys_to_virt(x[cpu].kernel_pdp.get_address());
    amd64_pgent_t *pgent = kpdp->copy_area[n];
    
    //TRACEF("n = %d, cpu = %d\n", n, cpu);
    for (word_t i = 0; i < (COPY_AREA_SIZE >> AMD64_PDP_BITS); i++)
	 pgent[i].clear();
}

INLINE void space_t::populate_copy_area (word_t n, pgent_t * src_pdp, cpuid_t cpu)
{


    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    amd64_kernel_pdp_t *kpdp = (amd64_kernel_pdp_t *) phys_to_virt(x[cpu].kernel_pdp.get_address());
    amd64_pgent_t *dst = kpdp->copy_area[n];
    amd64_pgent_t *src = &src_pdp->pgent;

    //TRACEF("cpu = %d, n = %d, src = %x, dst = %x\n", cpu, n, src, dst);
    for (word_t i = 0; i < (COPY_AREA_SIZE >> AMD64_PDP_BITS); i++, src++)
	dst[i].copy(*src);
    

}

/**
 * translates a global thread ID into a valid tcb pointer
 * @param tid thread ID
 * @returns pointer to the TCB of thread tid
 */
INLINE tcb_t * space_t::get_tcb( threadid_t tid )
{
    return (tcb_t*)((KTCB_AREA_START) + ((tid.get_threadno() & VALID_THREADNO_MASK) * KTCB_SIZE)); 
    
}



INLINE word_t space_t::get_from_user(addr_t addr)
{
    return *(word_t *)(addr);
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
INLINE void space_t::add_tcb(tcb_t * tcb)
{
    misc.thread_count++;
}

/**
 * removes a thread from a space
 * @param tcb_t thread control block
 * @return true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    misc.thread_count--;
    return (misc.thread_count == 0);
}

/**
 * Get PML4 entry
 */

INLINE pgent_t * space_t::pgent (word_t num, word_t cpu)
{
    return ((pgent_t *) phys_to_virt (get_pml4(cpu)))->
	next (this, pgent_t::size_512g, num);
}

#if !defined(CONFIG_SMP)
/**
 * Flush complete TLB
 */
INLINE void space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace)
	amd64_mmu_t::flush_tlb ();
}

/**
 * Flush a specific TLB entry
 * @param addr  virtual address of TLB entry
 */
INLINE void space_t::flush_tlbent (space_t * curspace, addr_t addr,
				   word_t log2size)
{
    if (this == curspace)
	amd64_mmu_t::flush_tlbent ((u64_t) addr);
}

/**
 * Update functions are empty in non-SMP case
 */
inline void space_t::end_update() {}

#else /* !CONFIG_SMP */

class active_cpu_space_t
{
public:
    void set( cpuid_t cpu, space_t * s )
    {
	if ( s )
	active_space[cpu].space = s;
    }

    space_t *get( cpuid_t cpu )
    {
	return active_space[cpu].space;
    }

private:
    struct 
    {
	space_t * space;
	char __pad[ CACHE_LINE_SIZE - sizeof(space_t*) ];
    } active_space[CONFIG_SMP_MAX_CPUS];
};
extern active_cpu_space_t active_cpu_space;

#endif /* CONFIG_SMP */


/**
 * Try to copy a mapping from kernel space into the current space
 * @param addr the address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 * Synchronization must happen at the highest level, allowing sharing.
 */
INLINE bool space_t::sync_kernel_space(addr_t addr)
{ 
    /* 
     * don't need to sync since TCB pointers are included in the single
     * PML4 kernel area entry
     */
    return false;
}


#if defined(CONFIG_IO_FLEXPAGES)
/* 
 * Sets the VRT object representing the IO-space
 */
INLINE void space_t::set_io_space(io_space_t *n)
{  
    misc.io_space = n; 
    n->set_space(this); 
}

/* 
 * Returns the VRT object representing the IO-space
 */

INLINE io_space_t *space_t::get_io_space(void) 
{ 
    return misc.io_space; 
}

#endif


/**********************************************************************
 *
 *                 global function declarations
 *
 **********************************************************************/

INLINE void reload_user_segregs (void)
{

    /* Load segment registers */
    asm("mov  %0, %%es		\n\t"		// load extra segment (ES)
	"mov  %1, %%gs		\n\t"		// load UTCB segment  (GS)
#ifdef CONFIG_TRACEBUFFER       
	"mov  %2, %%fs		\n\t"		// tracebuffer segment (FS)
#else
	"mov  %0, %%fs		\n\t"	        // no tracebuffer
#endif  
	: /* No Output */ : "r" (0), "r" (AMD64_UTCBS), "r" (AMD64_TBS)
	);
    
}

/*
 * returns the initial kernel space
 */

INLINE space_t* get_kernel_space()
{
    extern space_t * kernel_space;
    return kernel_space;
}


/**
 * converts the current page table into a space_t
 */
INLINE space_t * pml4_to_space(u64_t pml4, cpuid_t cpu = 0)
{
#if defined(CONFIG_SMP)
    return phys_to_virt((space_t*)(pml4 - (cpu * AMD64_PTAB_BYTES))); 
#else
    return phys_to_virt((space_t*)(pml4));
#endif
}

/**
 * init kernel space
 */
void SECTION(".init.memory") init_kernel_space();

/**
 * init I/O space
 */

#if defined(CONFIG_IO_FLEXPAGES)
void init_io_space();
#endif



#endif /* !__GLUE__V4_AMD64__SPACE_H__ */
