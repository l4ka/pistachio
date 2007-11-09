/****************************************************************************
 *
 * Copyright (C) 2002, 2006, Karlsruhe University
 *
 * File path:	glue/v4-amd64/space.cc
 * Description:	AMD64 space_t implementation.
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
 * $Id: space.cc,v 1.23 2006/11/18 10:15:54 stoess Exp $
 *
 ***************************************************************************/

#include <debug.h>			
#include <linear_ptab.h>
#include <generic/memregion.h>
#include <generic/lib.h>

#include INC_API(smp.h)
#include INC_API(space.h)
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(mmu.h)
#include INC_ARCH(trapgate.h)

#include INC_GLUE(memory.h)

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

#define PGSIZE_KERNEL    (KERNEL_PAGE_SIZE == AMD64_2MPAGE_SIZE) ? pgent_t::size_2m : pgent_t::size_4k
EXTERN_KMEM_GROUP (kmem_iofp);
EXTERN_KMEM_GROUP(kmem_misc);
EXTERN_KMEM_GROUP(kmem_space);
EXTERN_KMEM_GROUP(kmem_pgtab);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

space_t * kernel_space = NULL;
tcb_t * dummy_tcb = NULL;
addr_t utcb_page = NULL;


/**********************************************************************
 *
 *                         Helper functions
 *
 **********************************************************************/

INLINE void align_memregion(mem_region_t & region, word_t size)
{
    region.low = addr_t((word_t) region.low & ~(size - 1));
    region.high = addr_t(((word_t) region.high + size - 1) & ~(size - 1));
}


static tcb_t * get_dummy_tcb()
{
    
    if (!dummy_tcb) 
    { 
 	dummy_tcb = (tcb_t *) kmem.alloc(kmem_tcb, AMD64_4KPAGE_SIZE); 
 	ASSERT(dummy_tcb); 
 	dummy_tcb = virt_to_phys(dummy_tcb);
    } 
    return dummy_tcb; 
}


/**********************************************************************
 *
 *                         space_t implementation
 *
 **********************************************************************/

/**
 * initialize a space
 *
 * @param utcb_area	fpage describing location of UTCB area
 * @param kip_area	fpage describing location of KIP
 */
void space_t::init (fpage_t utcb_area, fpage_t kip_area)
{
    ASSERT( (KERNEL_AREA_END - KERNEL_AREA_START) <= AMD64_PML4_SIZE);

    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	*this->pgent(page_table_index(pgent_t::size_512g, (addr_t) KERNEL_AREA_START), cpu) =
	    *kernel_space->pgent(page_table_index(pgent_t::size_512g, (addr_t) KERNEL_AREA_START), cpu);
    set_kip_page_area(kip_area);
    set_utcb_page_area(utcb_area);


#if defined(CONFIG_X86_IO_FLEXPAGES)
    ASSERT(tss.get_io_bitmap_offset() == AMD64_4KPAGE_SIZE);

    /* map tss and default IO bitmap */
    
    /* map 1st page globally */
    remap_area((addr_t) TSS_MAPPING,
	       virt_to_phys(&tss),
	       pgent_t::size_4k, 
	       AMD64_4KPAGE_SIZE,
	       true, true, true);
	       
    /* map 2nd,3rd page non-globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, AMD64_4KPAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), AMD64_4KPAGE_SIZE),
	       pgent_t::size_4k, 
	       2 * AMD64_4KPAGE_SIZE,
	       true, true, false);

    /* map 4th page globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, 3 * AMD64_4KPAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), 3 * AMD64_4KPAGE_SIZE),
	       pgent_t::size_4k, 
	       AMD64_4KPAGE_SIZE,
	       true, true, true);

#warning js: avoid !sigma0_space -> is_sigma0_space(this) assumption   
    if (!sigma0_space)
    {
	set_io_space(new vrt_io_t);
	get_io_space()->populate_sigma0();
    }
	

#endif
    
    
    /* map kip read-only to user */
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    if (misc.compatibility_mode)
	add_mapping(kip_area.get_base(), virt_to_phys((addr_t) x32::get_kip()), pgent_t::size_4k, false, false);
    else
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */
	add_mapping(kip_area.get_base(), virt_to_phys((addr_t) get_kip()), pgent_t::size_4k, false, false);

}


/**
 * Release mappings that belong to the kernel (UTCB, KIP)
 * @param vaddr		virtual address in the space
 * @param paddr		physical address the mapping refers to
 * @param log2size	log2(size of mapping)
 */
void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area().is_addr_in_fpage(vaddr))
	kmem.free(kmem_utcb, phys_to_virt(paddr), 1UL << log2size);
}

/**
 * establish a mapping in sigma0's space
 * @param addr	the fault address in sigma0
 *
 * This function should install a mapping that allows sigma0 to make
 * progress. Sigma0's space is available as this.
 */
void space_t::map_sigma0(addr_t addr)
{
    //TRACEF("%p\n", addr);
    add_mapping(addr, addr, pgent_t::size_2m, true, false);   
}

/**
 * Install a dummy TCB
 * @param addr	address where the dummy TCB should be installed
 *
 * The dummy TCB must be read-only and fail all validity tests.
 */
void space_t::map_dummy_tcb (addr_t addr)
{
    //TRACEF("%p\n", addr);
   
    kernel_space->add_mapping(addr, (addr_t) get_dummy_tcb(), pgent_t::size_4k, false, false);

}

/* 
 * Allocate a new UTCB
 * @param addr	address where the dummy TCB should be installed
 *
 * precondition: this is a valid utcb location for the space 
 */

utcb_t * space_t::allocate_utcb(tcb_t * tcb)
{

    ASSERT(tcb);
    
    addr_t utcb = (addr_t)tcb->get_utcb_location();
    
    /* walk ptab, to see if a page is already mapped */
    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(size, utcb));
    
    while (size > pgent_t::size_4k && pgent->is_valid(this, size))
    {
	ASSERT(pgent->is_subtree(this, size));

	pgent = pgent->subtree(this, size);
	size--;
	pgent = pgent->next(this, size, page_table_index(size, utcb));
    }

    utcb_t * result;

    /* if pgent is valid a page is mapped, otherwise allocate a new one */
    if (pgent->is_valid(this, size))
	result = (utcb_t *) phys_to_virt(addr_offset(pgent->address(this, size),
						     (word_t) utcb & (~AMD64_4KPAGE_MASK)));
    else
    {
	// allocate new UTCB page
	addr_t page = kmem.alloc(kmem_utcb, AMD64_4KPAGE_SIZE);
	ASSERT(page);
	add_mapping((addr_t)utcb, virt_to_phys(page), pgent_t::size_4k, true, false);
	result = (utcb_t *) addr_offset(page, (word_t) utcb & (~AMD64_4KPAGE_MASK));
    }

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    result->set_compatibility_mode(misc.compatibility_mode);
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

    return result;
}
/**
 * Map memory usable for TCB
 * @param addr address of the TCB that should be made usable
 *
 * This function is called when a TCB should be made usable the first
 * time. Usually, this happens when a) no page is mapped at the TCB
 * address at all, or b) a read-only page is mapped and now a write
 * access to the TCB occured.
 *
 * @see space_t::map_dummy_tcb
 */
void space_t::allocate_tcb(addr_t addr)
{
    addr_t page = kmem.alloc(kmem_tcb, AMD64_4KPAGE_SIZE);
    ASSERT(page);
    
    x86_mmu_t::flush_tlbent((word_t) addr);
    kernel_space->add_mapping(addr, virt_to_phys(page), pgent_t::size_4k, true, true, true);
    sync_kernel_space(addr);    
}

/**
 * Translate a user accessible UTCB address to a kernel accessible one
 * @param utcb	user accessible address of UTCB
 * @returns kernel accessible address of UTCB
 *
 * The returned address must be accessible in the current address
 * space. This is required for checking values in the UTCB of a thread
 * in a different address space.
 */
utcb_t * space_t::utcb_to_kernel_space(utcb_t * utcb)
{
    UNIMPLEMENTED();
    return (utcb_t *) NULL;
}

/* JS: TODO nx bit */
void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel, bool global, bool cacheable)
{
    pgent_t::pgsize_e curr_size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(curr_size, vaddr));

    //TRACEF("space=%p, v=%p, p=%08p, s=%d %s %s %s %s\n", this, vaddr, paddr, (long) size, 
    //   (writable ? "w" : "ro"), (kernel  ? "k" : "u"), (global  ? "g" : "ng"), (cacheable  ? "c" : "nc")); 
	 
    /*
     * Sanity checking on page size (must be 4k or 2m)
     */
    if (!is_page_size_valid(size))
    {
	printf("Mapping invalid pagesize (%dKB)\n", page_size(size) >> 10);
	enter_kdebug("invalid page size");
	return;
    }

    /*
     * Walk down hierarchy
     */
    
    while (size < curr_size)
    {
	
	/* Check if already mapped as larger mapping */
	if (pgent->is_valid(this, curr_size))
	{
	    
	    if (!pgent->is_subtree(this, curr_size))
	    {
		/* check that alignement of virtual and physical page fits */
		ASSERT(addr_mask(vaddr, ~AMD64_2MPAGE_MASK) ==
		       addr_mask(paddr, ~AMD64_2MPAGE_MASK));

		if (((addr_t) pgent->address(this, curr_size)) == 
		    addr_mask(paddr, AMD64_2MPAGE_MASK))
		{
		    TRACEF("2MB mapping @ %p space %p already exists.\n",  vaddr, this);
		    return;
		}
	    }

	}
	else
	{
	    pgent->make_subtree(this, curr_size, kernel);
	    //TRACEF("make_subtree %p -> %p sz %d\n", 
	    //   &pgent->raw, pgent->raw, curr_size);
	}
	curr_size--;
	pgent = pgent->subtree(this, curr_size + 1)->next(this, curr_size, page_table_index(curr_size, vaddr));

    }
    pgent->set_entry(this, size, paddr, writable ? 7 : 5, 0, kernel);
    //    TRACEF("set_entry %p -> %p, curr_size %d\n", &pgent->raw, pgent->raw, curr_size);

    // default is cacheable
    if (!cacheable) pgent->set_cacheability (this, curr_size, false);
    
    // default: kernel->global, user->non-global
    if (kernel != global) pgent->set_global(this, curr_size, global);

}

void space_t::remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
			 word_t len, bool writable, bool kernel, bool global)
{
    //TRACEF("\tremap area %p->%p len=%x, size=%d, %s, %s\n", 
    //       vaddr, paddr, len, pgsize,
    // (writable ? "w" : "ro"), (kernel  ? "k" : "u"));

    word_t page_size = (pgsize == pgent_t::size_4k) ? AMD64_4KPAGE_SIZE : 
	AMD64_2MPAGE_SIZE;

    // length must be page-size aligned
    ASSERT((len & (page_size - 1)) == 0);

    for (word_t offset = 0; offset < len; offset += page_size)
	add_mapping(addr_offset(vaddr, offset), addr_offset(paddr, offset), 
		    pgsize, writable, kernel, global);
}

/**
 * ACPI memory handling
 */
addr_t acpi_remap(addr_t addr)
{

    //TRACE_INIT("ACPI remap: %p -> %x\n", addr, (word_t) addr + REMAP_32BIT_START); 
    ASSERT((word_t) addr < (1ULL << 32));
    return (addr_t) ((word_t) addr + REMAP_32BIT_START); 
}

void acpi_unmap(addr_t addr)
{
    /* empty  */
}


#if defined(CONFIG_X86_COMPATIBILITY_MODE)

word_t space_t::space_control(word_t ctrl)
{
    // Ignore parameter if 'c' bit is not set.
    if ((ctrl & (((word_t) 1) << 63)) == 0)
	return 0;

    if (!misc.compatibility_mode)
    {
	misc.compatibility_mode = true;

	/* Add 32-bit UTCB mapping, since the gs segment descriptor
	   is truncated in 32-bit mode.
	   Copied from init_kernel_mappings. */
	remap_area((addr_t) UTCB_MAPPING_32,
		   virt_to_phys(utcb_page),
		   pgent_t::size_4k, AMD64_4KPAGE_SIZE, true, false, false);

	/* Replace 64-bit KIP mapping with 32-bit KIP.
	   Copied from init. */
	if (is_initialized())
	    add_mapping(get_kip_page_area().get_base(), virt_to_phys((addr_t) x32::get_kip()), pgent_t::size_4k, false, false);
    }

    // Set 'e' bit.
    return (((word_t) 1) << 63);
}

#else /* !defined(CONFIG_X86_COMPATIBILITY_MODE) */

word_t space_t::space_control(word_t ctrl)
{
    return 0;
}

#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */


/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != AMD64_2MPAGE_SIZE) && \
    (KERNEL_PAGE_SIZE != AMD64_4KPAGE_SIZE)
#error invalid kernel page size - please adapt
#endif

/**
 * 
 * initialize kernel mappings
 * 
 * 
 */

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    
    mem_region_t reg;

    /* remap kernel */
    reg.set(start_text_phys, end_text_phys);
    align_memregion(reg, KERNEL_PAGE_SIZE);

    remap_area(phys_to_virt(reg.low), reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true, true);

    /* map init memory */
    reg.set(start_init, end_init);
    align_memregion(reg, AMD64_2MPAGE_SIZE);
    remap_area(reg.low, reg.low, PGSIZE_KERNEL, reg.get_size(), true, true, true);

    /* map low 1MB  for initialization */
    reg.set((addr_t)0, (addr_t)0x00100000);
    align_memregion(reg, AMD64_2MPAGE_SIZE);
    remap_area(reg.low, reg.low, pgent_t::size_2m, reg.get_size(), true, true);

    /* map video mem (use 2MB mapping, don't waste a pagetable) */
    remap_area(phys_to_virt((addr_t) VIDEO_MAPPING), (addr_t) VIDEO_MAPPING,
 		pgent_t::size_2m, AMD64_2MPAGE_SIZE, true, true, true);


    /* map syscalls read-only/executable to user  */
    // TODO: nx bit ?
    ASSERT(((word_t) end_syscalls - (word_t) start_syscalls) <= KERNEL_PAGE_SIZE);
    remap_area(start_syscalls, virt_to_phys(start_syscalls), 
	       pgent_t::size_2m, AMD64_2MPAGE_SIZE, false, false, true);

    /*  
     * MYUTCB mapping
     * allocate a full page for all myutcb pointers.
     * access must be performed via gs:0, when setting up the gdt
     * each processor gets a full cache line to avoid bouncing 
     * 
     */

    remap_area((addr_t) UTCB_MAPPING,	
	       virt_to_phys(utcb_page), 
	       pgent_t::size_4k, AMD64_4KPAGE_SIZE, true, false, true);
  

    /*  
     * Remap 4GB physical memory (e.g. for readmem) 
     */

    for (word_t p = REMAP_32BIT_START; p < REMAP_32BIT_END; p += AMD64_2MPAGE_SIZE)
	remap_area((addr_t) p, (addr_t) (p - REMAP_32BIT_START), pgent_t::size_2m, AMD64_2MPAGE_SIZE, true, true, true);
   

#if defined(CONFIG_X86_IO_FLEXPAGES)
    ASSERT(tss.get_io_bitmap_offset() == AMD64_4KPAGE_SIZE);

    /* map 1st page globally */
    remap_area((addr_t) TSS_MAPPING,
	       virt_to_phys(&tss),
	       pgent_t::size_4k, 
	       AMD64_4KPAGE_SIZE,
	       true, true, true);

	       
    /* map 2nd,3rd page non-globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, AMD64_4KPAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), AMD64_4KPAGE_SIZE),
	       pgent_t::size_4k, 
	       2 * AMD64_4KPAGE_SIZE,
	       true, true, false);

    /* map 4th page globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, 3 * AMD64_4KPAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), 3 * AMD64_4KPAGE_SIZE),
	       pgent_t::size_4k, 
	       AMD64_4KPAGE_SIZE,
	       true, true, true);
   
#endif

}

void space_t::init_cpu_mappings(cpuid_t cpu)
{
 
#if defined(CONFIG_SMP)
    /* cpu 0 operates on already initialized pagetables */
    if ( cpu == 0 ) return; 
    
    TRACE_INIT("Initializing cpu mappings for cpu %d\n", cpu);

    mem_region_t reg = { start_cpu_local, end_cpu_local };

    ASSERT(reg.get_size() < 0x40000000); // region must be less than 1g

    TRACE_INIT("start_cpu_local = %x, end_cpu_local = %x\n", start_cpu_local, end_cpu_local);

    /* assert that region is 2m aligned */
    ASSERT( !( (word_t)start_cpu_local & (AMD64_PDIR_SIZE - 1) ) );
    ASSERT( !( (word_t)end_cpu_local & (AMD64_PDIR_SIZE - 1) )  );
    
    pgent_t * src_pgent = this->pgent(511, 0);
    pgent_t * dst_pgent = this->pgent(511, cpu);

    ASSERT(src_pgent->is_subtree(this, pgent_t::size_max));
	       
    /* src_pgent->address(...) returns the address pml4_src_pgent points to. 
     * By adding cpu AMD64_PTAB_BYTES, we get the address of the PDP of the
     * current cpu
     */
    addr_t addr = (addr_t)( (word_t)src_pgent->address(this, pgent_t::size_4k) + cpu * AMD64_PTAB_BYTES );
    dst_pgent->assign_cpu_subtree(this, pgent_t::size_max, phys_to_virt(addr), true);

    src_pgent = src_pgent->subtree(this, pgent_t::size_max); // descend to src PDP
    dst_pgent = dst_pgent->subtree(this, pgent_t::size_max); // descend to dst PDP

    src_pgent = src_pgent->next(this, pgent_t::size_1g, 511);
    dst_pgent = dst_pgent->next(this, pgent_t::size_1g, 511);

    // get the address of current CPU's PDIR
    addr = (addr_t)( (word_t)src_pgent->address(this, pgent_t::size_4k) + cpu * AMD64_PTAB_BYTES );
    dst_pgent->assign_cpu_subtree(this, pgent_t::size_1g, phys_to_virt(addr), true);

    src_pgent = src_pgent->subtree(this, pgent_t::size_1g); // descend to src PDIR
    dst_pgent = dst_pgent->subtree(this, pgent_t::size_1g); // descend to dst PDIR

    pgent_t * dst_pdir_start = dst_pgent;
   
    /* set entries for CPU-local memory inside the current CPU's PDIR */
    for ( addr_t addr = reg.low; addr < reg.high;
	  addr = addr_offset(addr, AMD64_PDIR_SIZE) ) // walk region in 2MB steps
    {
	word_t pdir_idx = page_table_index(pgent_t::size_2m, addr);
	dst_pgent = dst_pgent->next(this, pgent_t::size_2m, pdir_idx);

	//TRACEF("allocating cpu-local page\n");
	addr_t page = kmem.alloc(kmem_pgtab, AMD64_PDIR_SIZE);
	//printf("allocated 2m page for cpu-local data at %x\n", page);
	dst_pgent->set_cpu_entry(this, pgent_t::size_2m, virt_to_phys(page), 7, true);
	memcpy(page, addr, AMD64_PDIR_SIZE);

	dst_pgent = dst_pdir_start; // reset pgent to point to the beginning of current CPU's pdir
	
    }

    TRACE_INIT("Switching to CPU local pagetable %p\n", get_pagetable(cpu));
    x86_mmu_t::set_active_pagetable((word_t)get_pagetable(cpu));
    x86_mmu_t::flush_tlb(true);
    TRACE_INIT("CPU pagetable activated (%x)\n",
	       x86_mmu_t::get_active_pagetable());
    


#endif
}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space);
    kernel_space = (space_t *) kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(kernel_space);
    //TRACEF("space=%p, size = %d \n", (u64_t *) kernel_space, sizeof(space_t));

    utcb_page = kmem.alloc(kmem_misc, AMD64_4KPAGE_SIZE);
    ASSERT(utcb_page);

    kernel_space->init_kernel_mappings();
    x86_mmu_t::set_active_pagetable((u64_t) kernel_space->get_pagetable(0));
    __asm__ __volatile__("addq %0, %%rsp" :: "i" (KERNEL_OFFSET));
}

/**********************************************************************
 *
 *                    IO-FlexPages
 *
 **********************************************************************/

#if defined(CONFIG_X86_IO_FLEXPAGES)
/* 
 * void install_io_bitmap()
 * 
 * installs an allocated 8k region as IO bitmap 
 *
 */
void space_t::install_io_bitmap(addr_t new_bitmap)
{

    add_mapping(addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()), 
		virt_to_phys(new_bitmap), 
		pgent_t::size_4k, true, true, false);
    
    add_mapping(addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset() + AMD64_4KPAGE_SIZE), 
		virt_to_phys(addr_offset(new_bitmap, AMD64_4KPAGE_SIZE)), 
		pgent_t::size_4k, true, true, false);
    
    flush_tlbent(get_current_space(), 
		 addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()), 
		 page_shift (pgent_t::size_4k));

    flush_tlbent(get_current_space(), 
		 addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset() + AMD64_4KPAGE_SIZE), 
		 page_shift (pgent_t::size_4k));

    //TRACEF("new_bitmap %p -> %p\n", new_bitmap, addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()));
}

/* 
 * void free_io_bitmap()
 * 
 * releases the IO bitmap of an address space and sets the tss bitmap back to the
 * default bitmat
 *
 */
void space_t::free_io_bitmap()
{
    /*
     * Do not release the default IOPBM
     */
    if (get_io_bitmap() == tss.get_io_bitmap())
	return;

    addr_t io_bitmap = get_io_bitmap();

    add_mapping(addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()), 
		virt_to_phys(tss.get_io_bitmap()),
		pgent_t::size_4k, true, true, false);
    add_mapping(addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset() + AMD64_4KPAGE_SIZE), 
		virt_to_phys(addr_offset(tss.get_io_bitmap(), AMD64_4KPAGE_SIZE)),
		pgent_t::size_4k, true, true, false);


    /* Release the IOPBM */
    kmem.free(kmem_iofp, io_bitmap, IOPERMBITMAP_SIZE);

	
    /* Flush the corresponding TLB entries */
    flush_tlbent(get_current_space(), 
		 addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()), 
		 page_shift (pgent_t::size_4k));

    flush_tlbent(get_current_space(), 
		 addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset() + AMD64_4KPAGE_SIZE), 
		 page_shift (pgent_t::size_4k));
}


void space_t::arch_free (void)
{
    //TRACEF("unmap %x in %x (%x)\n", fpage_t::complete_arch (), this, this->get_io_space());
    if (get_io_space())
    {
	// Unmap IO-space
	mdb_t::ctrl_t ctrl (0);
	ctrl.unmap = ctrl.mapctrl_self = true;
	get_io_space()->mapctrl (fpage_t::complete_arch (), ctrl, 0, 0);
	free_io_bitmap();
    }
}

#endif /* CONFIG_X86_IO_FLEXPAGES */



/**********************************************************************
 *
 *                    global functions
 *
 **********************************************************************/

/**
 * exc_pagefault: trap gate for x86-64 pagefault handler
 */
X86_EXCWITH_ERRORCODE(exc_pagefault, 0)
{

    u64_t pf = x86_mmu_t::get_pagefault_address();

    //TRACE("pagefault @ %p, ip=%p, sp=%p, a=%x, k=%s\n",
    // pf, frame->rip, frame->rsp, 
    //(space_t::access_e)(frame->error & (AMD64_PF_RW | AMD64_PF_ID)),
    //(frame->error & (AMD64_PF_US)) ? "false": "true");
    
    space_t * space = get_current_space();
    
    /* 
     * if the idle thread accesses the tcb area -
     * we will get a pagefault with an invalid space
     * so we use CR3 to figure out the space
     */
    if (EXPECT_FALSE(space == NULL))
    {
        space = pml4_to_space(x86_mmu_t::get_active_pagetable(),
                              get_current_cpu());
    }
    
    space->handle_pagefault(
        (addr_t)pf,
        (addr_t)frame->rip,
        (space_t::access_e)(frame->error & (AMD64_PF_RW | AMD64_PF_ID)),
        (frame->error & (AMD64_PF_US)) ? false : true);
}



 /**********************************************************************
 *
 *                        SMP handling
 *
 **********************************************************************/

#ifdef CONFIG_SMP
active_cpu_space_t active_cpu_space;
static word_t cpu_remote_flush UNIT("x86.cpulocal");
static word_t cpu_remote_flush_global UNIT("x86.cpulocal");

#define __FLUSH_GLOBAL__ false

static void do_xcpu_flush_tlb( cpu_mb_entry_t * entry )
{
    spin(60, get_current_cpu());
    x86_mmu_t::flush_tlb(__FLUSH_GLOBAL__);
}

INLINE void tag_flush_remote( space_t * curspace )
{
    for ( int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
    {
	if ( cpu == get_current_cpu() )
	    continue;
	if ( active_cpu_space.get(cpu) == curspace )
	    cpu_remote_flush |= ((word_t)1 << cpu);
    }
}

void space_t::flush_tlb( space_t * curspace )
{
    if ( this == curspace )
	x86_mmu_t::flush_tlb(__FLUSH_GLOBAL__); // global entries are not flushed
    tag_flush_remote(this);
}

void space_t::flush_tlbent( space_t * curspace, addr_t addr, word_t log2size )
{
    if ( this == curspace )
	x86_mmu_t::flush_tlbent((u64_t)addr);
    tag_flush_remote(this);
}

void space_t::end_update()
{
    for ( int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++ )
	if ( cpu_remote_flush & (1 << cpu) )
	    sync_xcpu_request(cpu, do_xcpu_flush_tlb, NULL,
			      cpu_remote_flush_global & (1 << cpu));
    cpu_remote_flush = 0;
    cpu_remote_flush_global = 0;
}

#endif
