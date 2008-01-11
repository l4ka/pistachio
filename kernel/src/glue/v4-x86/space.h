/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/space.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__SPACE_H__
#define __GLUE__V4_X86__SPACE_H__

#include INC_GLUE_SA(space.h)

extern cpuid_t current_cpu;

class space_t : public x86_space_t
{
public:
    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();
    bool sync_kernel_space(addr_t addr);
    static void switch_to_kernel_space(cpuid_t cpu);
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
    utcb_t *  allocate_utcb(tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    /* address ranges */
    bool is_user_area(addr_t addr);
    bool is_user_area(fpage_t fpage);
    bool is_tcb_area(addr_t addr);
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);
    bool is_arch_mappable(addr_t addr, size_t size);

    /* Copy area related methods */
    bool is_copy_area (addr_t addr);
    word_t get_copy_limit (addr_t addr, word_t limit);
    void delete_copy_area (word_t n, cpuid_t cpu);
    void populate_copy_area (word_t n, tcb_t *tcb, space_t *partner, cpuid_t cpu);
 
    /* reference counting */
    void add_tcb(tcb_t * tcb, cpuid_t cpu = current_cpu);
    bool remove_tcb(tcb_t * tcb, cpuid_t cpu = current_cpu);
    void move_tcb(tcb_t * tcb, cpuid_t src_cpu, cpuid_t dst_cpu);

    /* space control */
    word_t space_control (word_t ctrl);

    /* x86 specific functions */
    static void init_kernel_space();
    static space_t * allocate_space();
    void free_space();
    static space_t * top_pdir_to_space(word_t ptab);

    void init_kernel_mappings();
    void init_cpu_mappings(cpuid_t cpu);
    
    /* SMP specific handling */
    
    top_pdir_t * get_top_pdir(cpuid_t cpu = current_cpu);
    x86_pgent_t * get_top_pdir_phys(cpuid_t cpu = current_cpu);
    x86_pgent_t * get_reference_top_pdir();
    void alloc_cpu_top_pdir(cpuid_t cpu = current_cpu);
    void free_cpu_top_pdir(cpuid_t cpu = current_cpu);
    bool has_cpu_top_pdir(cpuid_t cpu = current_cpu);
    
    /* user space access */
    u8_t get_from_user(addr_t);

    /* update hooks */
    static void begin_update() { }
    static void end_update();

    /* generic page table walker */
    pgent_t * pgent (word_t num);
    pgent_t * pgent (word_t num, word_t cpu);
    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
		     bool writable, bool kernel, bool global, bool cacheable = true);
    void remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize, 
		    word_t len, bool writable, bool kernel, bool global);
    bool lookup_mapping( addr_t vaddr, pgent_t ** r_pg, pgent_t::pgsize_e *r_size, cpuid_t cpu);
    bool lookup_mapping( addr_t vaddr, pgent_t ** r_pg, pgent_t::pgsize_e *r_size)
	{ return lookup_mapping(vaddr, r_pg, r_size, data.reference_ptab); }
    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);
    
    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    static bool does_tlbflush_pay (word_t log2size)
	{ return log2size >= 28; }
    
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr);


    /* kip and utcb handling */
    fpage_t get_kip_page_area()
	{ return data.kip_area; }
    void set_kip_page_area(fpage_t f)
	{ data.kip_area = f; }
    fpage_t get_utcb_page_area()
	{ return data.utcb_area; }
    void set_utcb_page_area(fpage_t f)
	{ data.utcb_area = f; }


#if defined(CONFIG_X86_IO_FLEXPAGES)
    addr_t install_io_bitmap(bool create);
    void free_io_bitmap(void);
    bool sync_io_bitmap();
    addr_t get_io_bitmap(cpuid_t cpu = current_cpu);
    #   define HAVE_ARCH_FREE_SPACE
    void arch_free (void);
    void set_io_space(io_space_t *n)
	{  data.io_space = n; n->set_space(this); }
    io_space_t *get_io_space(void) 
	{ return data.io_space; }
#endif

    
    friend class pgent_t;

};


/**********************************************************************
 *
 *                      inline functions
 *
 **********************************************************************/

INLINE bool space_t::is_user_area (addr_t addr)
{
    return (((word_t) sign_extend(addr)) >= USER_AREA_START &&
	    ((word_t) sign_extend(addr)) < USER_AREA_END);
}
        
INLINE bool space_t::is_tcb_area (addr_t addr)
{
    return (((word_t) sign_extend(addr)) >= KTCB_AREA_START &&
	    ((word_t) sign_extend(addr)) < KTCB_AREA_END);
}

INLINE bool space_t::is_copy_area (addr_t addr)
{
    return (((word_t)sign_extend(addr)) >= COPY_AREA_START &&
	    ((word_t)sign_extend(addr)) < COPY_AREA_END);
}

INLINE bool space_t::is_arch_mappable (addr_t addr, size_t size)
{
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    return (!this->data.compatibility_mode
	    || ((word_t) sign_extend(addr_offset(addr, size))) <= UTCB_MAPPING_32
	    || (word_t) sign_extend(addr) >= UTCB_MAPPING_32 + X86_PAGE_SIZE);
#else
    return true;
#endif
}



INLINE space_t::top_pdir_t *space_t::get_top_pdir(cpuid_t cpu)
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    return data.cpu_ptab[cpu].top_pdir;
}


INLINE x86_pgent_t * space_t::get_top_pdir_phys(cpuid_t cpu)
{
    return virt_to_phys(&get_top_pdir(cpu)->pgent[0].pgent);
}


INLINE bool space_t::has_cpu_top_pdir(cpuid_t cpu)
{
    return data.cpu_ptab[cpu].top_pdir != NULL;
}

/**
 * converts the current page table into a space_t
 */
INLINE space_t *space_t::top_pdir_to_space(word_t ptab)
{
    return phys_to_virt((top_pdir_t*)ptab)->space;
}


INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit)
{
    word_t end = (word_t) addr + limit;

    if (is_user_area (addr))
    {
#if defined(CONFIG_X86_SMALL_SPACES)
	if (is_small () && end > smallspace_size ())
	{
	    // IPC copy exeecds small space boundaries.  Try to
	    // promote space into a large one.
	    make_large ();
	}
#endif

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


INLINE u8_t space_t::get_from_user(addr_t addr)
{
#if defined(CONFIG_X86_SMALL_SPACES)
    if (EXPECT_FALSE (is_small ()))
	return *(u8_t *) (smallspace_offset () + (word_t) addr);
#endif
    return *(u8_t *) (addr);
}

/**
 * add a thread to the space
 * @param tcb	pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb, cpuid_t cpu)
{
    data.thread_count++;
#ifdef CONFIG_SMP
    data.cpu_ptab[cpu].thread_count++;
#endif

}

/**
 * remove a thread from a space
 * @param tcb	thread control block
 * @return	true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb, cpuid_t cpu)
{
    ASSERT (data.thread_count !=  0);
    data.thread_count--;
#ifdef CONFIG_SMP
    ASSERT (data.cpu_ptab[cpu].thread_count !=  0);
    data.cpu_ptab[cpu].thread_count--;
#endif
    return (data.thread_count == 0);
}


/**
 * translates a threadid into a tcb pointer
 * @param tid thread id of the thread
 * @return pointer to thread control block
 */
INLINE tcb_t * space_t::get_tcb(threadid_t tid)
{ 
    return (tcb_t*)((KTCB_AREA_START) + ((tid.get_threadno() & VALID_THREADNO_MASK) * KTCB_SIZE)); 
}

INLINE tcb_t * space_t::get_tcb(void * ptr)
{
   return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}



INLINE pgent_t * space_t::pgent (word_t num, word_t cpu)
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    if (!data.cpu_ptab[cpu].top_pdir)
	return NULL;
    return &data.cpu_ptab[cpu].top_pdir->pgent[num];
}

INLINE pgent_t * space_t::pgent (word_t num)
{
    return pgent (num, data.reference_ptab);
}


#if defined(CONFIG_X86_SMALL_SPACES)
#define IS_SPACE_SMALL(s)	(s)->is_small ()
#else
#define IS_SPACE_SMALL(s)	false
#endif

#if defined(CONFIG_X86_SMALL_SPACES_GLOBAL)
#define IS_SPACE_GLOBAL(s)	(s)->is_small ()
#else
#define IS_SPACE_GLOBAL(s)	false
#endif


#ifndef CONFIG_SMP
/**
 * Flush complete TLB
 */
INLINE void space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace || IS_SPACE_SMALL (this))
	x86_mmu_t::flush_tlb (IS_SPACE_GLOBAL (this));
}

/**
 * Flush a specific TLB entry
 * @param addr	virtual address of TLB entry
 */
INLINE void space_t::flush_tlbent (space_t * curspace, addr_t addr,
				   word_t log2size)
{
    if (this == curspace || IS_SPACE_SMALL (this))
	x86_mmu_t::flush_tlbent ((word_t) addr);
}

/**
 * Update functions are empty in non-SMP case
 */
inline void space_t::end_update() { }

#else

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
#endif



/**
 * establish a mapping in sigma0's space
 * @param addr	the fault address in sigma0
 *
 * This function should install a mapping that allows sigma0 to make
 * progress. Sigma0's space is available as this.
 */

INLINE void space_t::map_sigma0(addr_t addr)
{
    add_mapping(addr, addr, PGSIZE_SIGMA, true, false, false);
}

/**********************************************************************
 *
 *                 global function declarations
 *
 **********************************************************************/

INLINE space_t* get_kernel_space()
{
    extern space_t * kernel_space;
    return kernel_space;
}


INLINE void reload_user_segregs (void)
{
    asm volatile (
	"	movl %0, %%es	\n"
#if !defined(CONFIG_TRACEBUFFER)
	"	movl %0, %%fs	\n"
#endif
	"	movl %1, %%gs	\n"
	:
	: "r" (X86_UDS), "r" (X86_UTCBS));
}


#if defined(CONFIG_X86_IO_FLEXPAGES)
void init_io_space();
#endif

/**********************************************************************
 *
 *                         Helper functions
 *
 **********************************************************************/

INLINE void align_memregion(mem_region_t & region, word_t size)
{
    region.low = addr_t((word_t)region.low & ~(size - 1));
    region.high = addr_t(((word_t)region.high + size - 1) & ~(size - 1));
}




#endif /* !__GLUE__V4_X86__SPACE_H__ */
