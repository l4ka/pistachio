/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/space.cc
 * Description:   address space management
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
 * $Id: space.cc,v 1.51 2006/11/18 09:51:24 stoess Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <generic/lib.h>

#include INC_API(tcb.h)
#include INC_API(smp.h)

#include INC_ARCHX(x86,mmu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(pgent.h)
#include <linear_ptab.h>

#include INC_GLUEX(x86,memory.h)
#include INC_GLUE(space.h)
#include INC_API(kernelinterface.h)

#define PGSIZE_KTCB	(pgent_t::size_4k)
#define PGSIZE_UTCB	(pgent_t::size_4k)
#define PGSIZE_KERNEL	(KERNEL_PAGE_SIZE == IA32_SUPERPAGE_SIZE) ? pgent_t::size_4m : pgent_t::size_4k
#define PGSIZE_KIP	(pgent_t::size_4k)
#define PGSIZE_SIGMA    PGSIZE_KERNEL

EXTERN_KMEM_GROUP (kmem_space);
EXTERN_KMEM_GROUP(kmem_iofp);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

/* helpers */
INLINE void align_memregion(mem_region_t & region, word_t size)
{
    region.low = addr_t((word_t)region.low & ~(size - 1));
    region.high = addr_t(((word_t)region.high + size - 1) & ~(size - 1));
}

space_t * kernel_space = NULL;
tcb_t * dummy_tcb = NULL;

static tcb_t * get_dummy_tcb()
{
    if (!dummy_tcb)
    {
	dummy_tcb = (tcb_t*)kmem.alloc(kmem_tcb, IA32_PAGE_SIZE);
	ASSERT(dummy_tcb);
	dummy_tcb = virt_to_phys(dummy_tcb);
    }
    return dummy_tcb;
}

void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel, bool global, bool cacheable)
{
    pgent_t::pgsize_e curr_size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(curr_size, vaddr));

    //TRACEF("space=%p, v=%p, p=%p, size=%d, w=%d, k=%d\n", this, vaddr, paddr, size, writable, kernel);

    if (size < curr_size)
    {
	if (!pgent->is_valid(this, curr_size))
	{
	    pgent->make_subtree(this, curr_size, kernel);
	}
	else
	{
	    /* check if already mapped as a 4MB page */
	    if (!pgent->is_subtree(this, curr_size))
	    {
		/* check that alignement of virtual and physical page fits */
		ASSERT(addr_mask(vaddr, ~IA32_SUPERPAGE_MASK) ==
		       addr_mask(paddr,~IA32_SUPERPAGE_MASK));

		if (((addr_t)pgent->address(this, curr_size)) == 
		    addr_mask(paddr, IA32_SUPERPAGE_MASK))
		    return;
		ASSERT(0);
	    }
	}
	curr_size--;
	//TRACEF("%p, %p\n", pgent->subtree(this, curr_size + 1), pgent);
	pgent = pgent->subtree(this, curr_size + 1)->next(
	    this, curr_size, page_table_index(curr_size, vaddr));
    }
    //TRACEF("%p\n", pgent);
    pgent->set_entry(this, curr_size, paddr, writable ? 7 : 5, 0, kernel);

    // default is cacheable
    if (!cacheable) pgent->set_cacheability (this, curr_size, false);

    // default: kernel->global, user->non-global
    if (kernel != global) pgent->set_global(this, curr_size, global);
}

void space_t::remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
			 word_t len, bool writable, bool kernel, bool global)
{
    TRACE_INIT("remap area %p->%p len=%x, w=%d, k=%d, size=%d\n", 
	       vaddr, paddr, len, writable, kernel, pgsize);

    word_t page_size = (pgsize == pgent_t::size_4k) ? IA32_PAGE_SIZE : 
	IA32_SUPERPAGE_SIZE;

    // length must be page-size aligned
    ASSERT((len & (page_size - 1)) == 0);

    for (word_t offset = 0; offset < len; offset += page_size)
	add_mapping(addr_offset(vaddr, offset), addr_offset(paddr, offset), 
		    pgsize, writable, kernel, global);
    add_mapping(addr_offset(vaddr, len-1), addr_offset(paddr, len-1), 
                pgsize, writable, kernel, global);
}


/**
 * reads a word from a given physical address, uses a remap window and
 * maps a 4MB page for the access
 *
 * @param paddr		physical address to read from
 * @return the value at the given address
 */
word_t space_t::readmem_phys (addr_t paddr)
{
    int cpu = get_current_cpu();

    // get the _real_ space, use CR3 for that
    space_t * space = ptab_to_space(x86_mmu_t::get_active_pagetable(), cpu);

#if defined(CONFIG_IA32_PSE)
    // map physical 4MB page into remap window
    if (!space->x[cpu].readmem_area.is_valid() ||
	( space->x[cpu].readmem_area.get_address(ia32_pgent_t::size_4m) != 
	  addr_mask(paddr, IA32_SUPERPAGE_MASK) ))
    {
	space->x[cpu].readmem_area.set_entry
	    (addr_mask(paddr, IA32_SUPERPAGE_MASK),
	     ia32_pgent_t::size_4m, 
	     IA32_PAGE_KERNEL | IA32_PAGE_VALID);
	// kill potentially stale TLB entry in remap-window
	x86_mmu_t::flush_tlbent (MEMREAD_AREA_START);
#if 0
	printf("readmem_phys: space=%p, mapped %p @ %p\n", 
	       space, space->x[cpu].readmem_area.get_raw(), 
	       &space->x[cpu].readmem_area);
#endif
    }
    return *(word_t*)addr_offset(addr_mask(paddr,~IA32_SUPERPAGE_MASK), 
				 MEMREAD_AREA_START);
#else /* !CONFIG_IA32_PSE */

    pgent_t* pgent = space->pgent( page_table_index(pgent_t::size_max, (addr_t)MEMREAD_AREA_START));

    if (!pgent->is_valid(space, pgent_t::size_max))
	pgent->make_subtree(space, pgent_t::size_max, true);

    pgent = pgent->subtree(space, pgent_t::size_max)->next(
	space, pgent_t::size_4k, page_table_index(pgent_t::size_4k, paddr));

//    pgent->set_entry(space, pgent_t::size_4k, paddr, true, false, false, true);
    pgent->set_entry(space, pgent_t::size_4k, paddr, 1, 0, true);

    // kill potentially stale TLB entry in remap-window
    x86_mmu_t::flush_tlbent(
	(word_t)addr_offset(addr_mask(paddr, page_mask (pgent_t::size_4m)),
	    MEMREAD_AREA_START));

    return *(word_t*)addr_offset(addr_mask(paddr,~IA32_SUPERPAGE_MASK), 
				 MEMREAD_AREA_START);
#endif /* !CONFIG_IA32_PSE */
}


/**
 * ACPI memory handling
 */
addr_t acpi_remap(addr_t addr)
{

    //TRACE_INIT("ACPI remap: %p\n", addr); 
    /* 
     * For now, make sure ACPI mappings are 4M; readmem_phys will map 4M-pages
     * at MEMREAD_AREA_START by directly writing the pagedir entry -- in which
     * case the pagetable created for 4K acpi mappings would be stale.
     */
    ASSERT(ACPI_PGENTSZ == pgent_t::size_4m);
    addr_t vaddr = (addr_t) MEMREAD_AREA_START;
    addr_t paddr = addr;
    
    get_kernel_space()->add_mapping(
        addr_mask (vaddr, ~page_mask (ACPI_PGENTSZ)),
	addr_mask (paddr, ~page_mask (ACPI_PGENTSZ)),
	ACPI_PGENTSZ, true, true, false);

    vaddr = addr_offset(vaddr, page_size(ACPI_PGENTSZ));
    paddr = addr_offset(paddr, page_size(ACPI_PGENTSZ));
    
    get_kernel_space()->add_mapping( 
	addr_mask (vaddr, ~page_mask (ACPI_PGENTSZ)),
	addr_mask (paddr, ~page_mask (ACPI_PGENTSZ)),
	ACPI_PGENTSZ, true, true, false);
    
    x86_mmu_t::flush_tlb();
    
        
    return addr_offset(addr_mask(addr, page_mask (ACPI_PGENTSZ)), 
			MEMREAD_AREA_START);
    
}

void acpi_unmap(addr_t addr)
{
    /* empty right now */
}


/**********************************************************************
 *
 *                        SMP handling
 *
 **********************************************************************/

#ifdef CONFIG_SMP
static word_t cpu_remote_flush UNIT("cpulocal");
static word_t cpu_remote_flush_global UNIT("cpulocal");
active_cpu_space_t active_cpu_space;

#if defined(CONFIG_IA32_SMALL_SPACES_GLOBAL)
#define __FLUSH_GLOBAL__	entry->param[0]
#else
#define __FLUSH_GLOBAL__	false
#endif

static void do_xcpu_flush_tlb(cpu_mb_entry_t * entry)
{
    spin(60, get_current_cpu());
    x86_mmu_t::flush_tlb (__FLUSH_GLOBAL__);
}

INLINE void tag_flush_remote (space_t * curspace)
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
    {
	if (cpu == get_current_cpu())
	    continue;
	if (active_cpu_space.get(cpu) == curspace)
	    cpu_remote_flush |= (1 << cpu);

#if defined(CONFIG_IA32_SMALL_SPACES)
	// For small spaces we must also do TLB shootdown if current
	// space is small, or current active space on remote CPU is a
	// small space.
	if ((active_cpu_space.get(cpu) &&
	     active_cpu_space.get(cpu)->is_small ()) ||
	    curspace->is_small ())
	{
	    cpu_remote_flush |= (1 << cpu);
#if defined(CONFIG_IA32_SMALL_SPACES_GLOBAL)
	    // If we are dealing with small spaces having global bit
	    // set, we must do a complete TLB flush.
	    if (curspace->is_small ())
		cpu_remote_flush_global |= (1 << cpu);
#endif
	}
#endif
    }
}

void space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace || IS_SPACE_SMALL (this))
	x86_mmu_t::flush_tlb (IS_SPACE_GLOBAL (this));
    tag_flush_remote (this);
}

void space_t::flush_tlbent (space_t * curspace, addr_t addr, word_t log2size)
{
    if (this == curspace || IS_SPACE_SMALL (this))
	x86_mmu_t::flush_tlbent ((u32_t) addr);
    tag_flush_remote (this);
}

void space_t::end_update ()
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	if (cpu_remote_flush & (1 << cpu))
	    sync_xcpu_request(cpu, do_xcpu_flush_tlb, NULL,
			      cpu_remote_flush_global & (1 << cpu));
    cpu_remote_flush = 0;
    cpu_remote_flush_global = 0;
}
#endif



/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != IA32_SUPERPAGE_SIZE) && \
    (KERNEL_PAGE_SIZE != IA32_PAGE_SIZE)
# error invalid kernel page size - please adapt
#endif

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    /* we map both reserved areas into the kernel area */
    mem_region_t reg = get_kip()->reserved_mem0;
    ASSERT(!reg.is_empty());
    
    align_memregion(reg, KERNEL_PAGE_SIZE);
    remap_area(phys_to_virt(reg.low), reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true, true);

    if (!get_kip()->reserved_mem1.is_empty())
    {
	reg = get_kip()->reserved_mem1;
	align_memregion(reg, IA32_PAGEDIR_SIZE);
	remap_area(phys_to_virt(reg.low), reg.low, PGSIZE_KERNEL, 
		   reg.get_size(), true, true, true);
    }

    /* map init memory */
    reg.set(start_init, end_init);
    align_memregion(reg, KERNEL_PAGE_SIZE);
    remap_area(reg.low, reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true, true);

    /* map low 4MB pages for initialization */
    reg.set((addr_t)0, (addr_t)0x00400000);
    align_memregion(reg, IA32_SUPERPAGE_SIZE);
    remap_area(reg.low, reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true, false);

    /* map video mem to kernel */
    add_mapping(phys_to_virt((addr_t)VIDEO_MAPPING), (addr_t)VIDEO_MAPPING,
		pgent_t::size_4k, true, true, true);

    /* MYUTCB mapping
     * allocate a full page for all myutcb pointers.
     * access must be performed via gs:0, when setting up the gdt
     * each processor gets a full cache line to avoid bouncing 
     * page is user-writable and global
     */
    EXTERN_KMEM_GROUP(kmem_misc);
    add_mapping((addr_t)MYUTCB_MAPPING,	
		virt_to_phys(kmem.alloc(kmem_misc, IA32_PAGE_SIZE)), 
		pgent_t::size_4k, true,	false, true);

#if defined(CONFIG_IA32_SMALL_SPACES) && defined(CONFIG_IA32_SYSENTER)
    /* User-level trampoline for ipc_sysexit, readonly but global. */
    extern word_t _start_utramp_p[];
    add_mapping ((addr_t) UTRAMP_MAPPING,
		 (addr_t) &_start_utramp_p,
		 pgent_t::size_4k, false, false, true);
#endif
    
#if defined(CONFIG_IO_FLEXPAGES)
    /* map tss and default IO bitmap */

    TRACEF("tss.get_io_bitmap_offset() = %d\n", tss.get_io_bitmap_offset());
    ASSERT(tss.get_io_bitmap_offset() == IA32_PAGE_SIZE);
    /* map 1st page globally */
    remap_area((addr_t) TSS_MAPPING,
	       virt_to_phys(&tss),
	       pgent_t::size_4k, 
	       IA32_PAGE_SIZE,
	       true, true, true);
	       
    /* map 2nd,3rd page non-globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, IA32_PAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), IA32_PAGE_SIZE),
	       pgent_t::size_4k, 
	       2 * IA32_PAGE_SIZE,
	       true, true, false);

    /* map 4th page globally */
    remap_area(addr_offset((addr_t) TSS_MAPPING, 3 * IA32_PAGE_SIZE),
	       addr_offset(virt_to_phys(&tss), 3 * IA32_PAGE_SIZE),
	       pgent_t::size_4k, 
	       IA32_PAGE_SIZE,
	       true, true, true);

#endif

    
}



void SECTION (".init") space_t::init_cpu_mappings(cpuid_t cpu)
{
#if defined(CONFIG_SMP)
    /* CPU 0 gets the always initialized page table */
    if (cpu == 0) return;

    TRACE_INIT("init cpu mappings for cpu %d\n", cpu);

    mem_region_t reg = { start_cpu_local, end_cpu_local };
    align_memregion(reg, IA32_PAGEDIR_SIZE);

    TRACE_INIT("remapping CPU local memory %p - %p (%p - %p)\n", 
	       start_cpu_local, end_cpu_local, reg.low, reg.high);

    pgent_t::pgsize_e size = pgent_t::size_max;

    for (addr_t addr = reg.low; addr < reg.high; 
	 addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
    {
	pgent_t * dst_pgent = this->pgent(page_table_index(size, addr), cpu);
	pgent_t * src_pgent = this->pgent(page_table_index(size, addr), 0);
	dst_pgent->make_cpu_subtree(this, size, true);

	ASSERT(src_pgent->is_subtree(this, size));

	src_pgent = src_pgent->subtree(this, size);
	dst_pgent = dst_pgent->subtree(this, size);
	
	size--;

	// now copy the page table
	for (addr_t pgaddr = addr; 
	     pgaddr < addr_offset(addr, IA32_PAGEDIR_SIZE);
	     pgaddr = addr_offset(pgaddr, IA32_PAGE_SIZE))
	{
	    //TRACE_INIT("copying ptab @ %p (%p, %p)\n", pgaddr, src_pgent, dst_pgent);
	    if ((pgaddr < start_cpu_local || pgaddr > end_cpu_local)
#if defined(CONFIG_IO_FLEXPAGES)
		&& (pgaddr < (addr_t) TSS_MAPPING || pgaddr > (addr_t) (TSS_MAPPING + 4 * IA32_PAGE_SIZE))
#endif
		)
		*dst_pgent = *src_pgent; // global data
	    else
	    {
		addr_t page = kmem.alloc(kmem_pgtab, IA32_PAGE_SIZE);
		TRACE_INIT("allocated cpu local page %p -> %p\n", pgaddr, page);
		dst_pgent->set_entry(this, size, virt_to_phys(page), 7, 0, true);
		memcpy(page, pgaddr, IA32_PAGE_SIZE);
	    }
	    src_pgent = src_pgent->next(this, size, 1);
	    dst_pgent = dst_pgent->next(this, size, 1);
	}
	size++;
    }
    TRACE_INIT("switching to CPU local pagetable %p\n", get_pagetable(cpu));
    x86_mmu_t::set_active_pagetable((u32_t)get_pagetable(cpu));
    x86_mmu_t::flush_tlb(true);
    TRACE_INIT("cpu pagetable activated (%x)\n", 
	       x86_mmu_t::get_active_pagetable());
#endif
}

void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space); // on UP kernel space is only initialized once

    kernel_space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(kernel_space);

    kernel_space->init_kernel_mappings();
    x86_mmu_t::set_active_pagetable((u32_t)kernel_space->get_pagetable(0));
}

/**********************************************************************
 *
 *                    Small address spaces
 *
 **********************************************************************/

#if defined(CONFIG_IA32_SMALL_SPACES)

word_t space_t::space_control (word_t ctrl)
{
    // Ignore parameter if 's' bit is not set.
    if ((ctrl & (1 << 31)) == 0)
	return 0;

    word_t old_control = smallid ()->get_raw ();

    smallspace_id_t id;
    id.set_raw (ctrl);

    if (make_small (id))
	// Set 'e' bit if small space operation was successful.
	old_control |= (1 << 31);

    return old_control;
}

void space_t::arch_free (void)
{
    make_large ();
    dequeue_polluted ();
}
#else /* !CONFIG_IA32_SMALL_SPACES */

word_t space_t::space_control (word_t ctrl)
{
    return 0;
}

#endif



/**********************************************************************
 *
 *                    IO-FlexPages
 *
 **********************************************************************/

#if defined(CONFIG_IO_FLEXPAGES)
/* 
 * void install_iopbm()
 * 
 * installs an intialised 8k Region as IOPBM 
 *
 */
void space_t::install_io_bitmap(addr_t new_bitmap)
{
    space_t *kspace = get_kernel_space();
    addr_t io_bitmap_mapping = (addr_t) (TSS_MAPPING + tss.get_io_bitmap_offset());
    
    word_t pdir_idx = page_table_index(pgent_t::size_4m, io_bitmap_mapping);
    word_t ptab_idx = page_table_index(pgent_t::size_4k, io_bitmap_mapping);
    pgent_t *subtree = kspace->pgent(pdir_idx)->subtree(kspace, pgent_t::size_4m);
	
    /* 
     * Allocate a second level pagetable.
     * Do not use ->make_subtree, as we have to 
     * set it up _before_ installing it
     */
    pgent_t *new_subtree = (pgent_t *) kmem.alloc (kmem_iofp, IA32_PAGE_SIZE);
    
    if (new_subtree == NULL)
    {
	panic("install_iopbm(): out of memory\n");
    }
    
    /*	Copy all entries from Kernel Page Table	 */
    pgent_t *src_pgent = subtree;
    pgent_t *dst_pgent = new_subtree;
    
    for (u32_t i=0; i<1024; i++)
    {
	
	*dst_pgent = *src_pgent;

	dst_pgent = dst_pgent->next(this, pgent_t::size_4k, 1); 
	src_pgent = src_pgent->next(this, pgent_t::size_4k, 1); 
    } 

    /*	Set the two special entries  */
    new_subtree->next(this, pgent_t::size_4k, ptab_idx)->
	set_entry(this, pgent_t::size_4k,
		  virt_to_phys(new_bitmap), 4, 0, false);

    new_subtree->next(this, pgent_t::size_4k, ptab_idx + 1)->
	set_entry(this, pgent_t::size_4k,
		  virt_to_phys(addr_offset(new_bitmap, 4096)), 4, 0, false);

#if defined(CONFIG_IA32_PGE)
    /* 
     * Unset global bit of IOPBM entries if PGE enabled 
     */
    new_subtree->next(this, pgent_t::size_4k, ptab_idx)->
	set_global(this, pgent_t::size_4k, false);
    new_subtree->next(this, pgent_t::size_4k, ptab_idx + 1)->
	set_global(this, pgent_t::size_4k, false);
#endif
    
    
    /* Install new Pagetable  */
    pgent(pdir_idx)->
	set_entry(this, pgent_t::size_4k,
		  virt_to_phys( (addr_t) new_subtree), 7, 0, false); 

    flush_tlbent(get_current_space(), io_bitmap_mapping, page_shift (pgent_t::size_4k)); 
    flush_tlbent(get_current_space(), addr_offset(io_bitmap_mapping, 4096), page_shift (pgent_t::size_4k));

}

/* 
 * void release_iopbm()
 * 
 * releases the IOPBM of a task and sets the pointers back to the  default-IOPBM
 *
 */
void space_t::free_io_bitmap()
{

    /*
     * Do not release the default IOPBM
     */
    if (get_io_bitmap() == tss.get_io_bitmap() || get_io_bitmap() == NULL)
	return;

    /*
     * Access the task's IOPBM by the virtual address kmem.alloc() returned
     * iopbm_virt points to the current task's IOPBM 
     */ 
    space_t *kspace = get_kernel_space();
    addr_t io_bitmap = get_io_bitmap();
    addr_t io_bitmap_mapping = (addr_t) (TSS_MAPPING + tss.get_io_bitmap_offset());
    word_t pdir_idx = page_table_index(pgent_t::size_4m, io_bitmap_mapping);

    pgent_t *old_subtree = kspace->pgent(pdir_idx)->subtree(kspace, pgent_t::size_4m);
    
    /*
     * Pagetable to be released 
     */ 
    pgent_t *subtree = pgent(pdir_idx)->subtree(this, pgent_t::size_4m);
    
    /*	Insert the default PGT	*/
    pgent(pdir_idx)-> \
	set_entry(this, pgent_t::size_4m, virt_to_phys( (addr_t) old_subtree),
		  6, 0, true); 

    /*	Release the Pagetable  */
    kmem.free(kmem_iofp, (addr_t) subtree, IA32_PAGE_SIZE);
	
    /* Release the IOPBM */
    kmem.free(kmem_iofp, io_bitmap, IOPERMBITMAP_SIZE);

	
    /* Flush the corresponding TLB entries */
    flush_tlbent(get_current_space(), io_bitmap_mapping, page_shift (pgent_t::size_4k)); 
    flush_tlbent(get_current_space(), addr_offset(io_bitmap_mapping, 4096), page_shift (pgent_t::size_4k));
    
    //TRACEF("done %p/%p\n", subtree, io_bitmap);

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

#endif /* CONFIG_IO_FLEXPAGES */



/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/

bool space_t::sync_kernel_space(addr_t addr)
{
    if (this == kernel_space) return false;

    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * dst_pgent = this->pgent(page_table_index(size, addr));
    pgent_t * src_pgent = kernel_space->pgent(page_table_index(size, addr));

    /* (already valid) || (kernel space invalid) */
    if (dst_pgent->is_valid(this, size) || 
	(!src_pgent->is_valid(kernel_space, size)))
    {
#if 0
	TRACE("sync ksp @ %p (src=%p (%d), dst=%p (%d))\n", 
	      addr, kernel_space, src_pgent->is_valid(kernel_space, size),
	      this, dst_pgent->is_valid(this, size));
#endif
	return false;
    }

#if !defined(CONFIG_SMP)
    *dst_pgent = *src_pgent;
#else
    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	*this->pgent(page_table_index(size, addr), cpu) = *kernel_space->pgent(page_table_index(size, addr), cpu);
#endif
    return true;
}

/**
 * space_t::init initializes the space_t
 *
 * maps the kernel area and initializes shadow ptabs etc.
 */
void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
#warning VU: space_t has to be restructured having an init and activate function
    for (addr_t addr = (addr_t)KERNEL_AREA_START;
	 addr < (addr_t)KERNEL_AREA_END;
	 addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
	sync_kernel_space(addr);

    set_utcb_area (utcb_area);
    set_kip_area (kip_area);
    
    // map kip read-only to user
    add_mapping(kip_area.get_base(), virt_to_phys((addr_t)get_kip()), PGSIZE_KIP, false, false, false);
    
#if defined(CONFIG_IO_FLEXPAGES)
#warning js: avoid !sigma0_space -> is_sigma0_space(this) assumption   
    if (!sigma0_space)
    {
	set_io_space(new vrt_io_t);
	get_io_space()->populate_sigma0();
    }
	
#endif
}

void space_t::allocate_tcb(addr_t addr)
{
    //enter_kdebug("allocate tcb");

    addr_t page = kmem.alloc(kmem_tcb, IA32_PAGE_SIZE);
    ASSERT(page);
    //TRACEF("tcb=%p, page=%p\n", addr, page);

    // map tcb kernel-writable, global 
    x86_mmu_t::flush_tlbent ((word_t)addr);
    kernel_space->add_mapping(addr, virt_to_phys(page), PGSIZE_KTCB, 
			      true, true, true);
    sync_kernel_space(addr);
}

void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}


/* precondition: this is a valid utcb location for the space */
utcb_t * space_t::allocate_utcb(tcb_t * tcb)
{
    ASSERT(tcb);
    addr_t utcb = (addr_t)tcb->get_utcb_location();

    /* walk ptab, to see if a page is already mapped */
    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(size, utcb));
    while (size > PGSIZE_UTCB && pgent->is_valid(this, size)) 
    {
	ASSERT(pgent->is_subtree(this, size));

	pgent = pgent->subtree(this, size);
	size--;
	pgent = pgent->next(this, size, page_table_index(size, utcb));
    }

    /* if pgent is valid a page is mapped, otherwise allocate a new one */
    if (pgent->is_valid(this, size))
	return (utcb_t*)phys_to_virt(addr_offset(pgent->address(this, size), 
						 (word_t)utcb & (~IA32_PAGE_MASK)));
    else 
    {
	// allocate new UTCB page, non-global
	addr_t page = kmem.alloc(kmem_utcb, IA32_PAGE_SIZE);
	ASSERT(page);
	add_mapping((addr_t)utcb, virt_to_phys(page), PGSIZE_UTCB, 
		    true, false, false);
	return (utcb_t*)addr_offset(page, (word_t)utcb & (~IA32_PAGE_MASK));
    }
}


void space_t::map_dummy_tcb(addr_t addr)
{
    //TRACEF("%p\n", addr);
    //enter_kdebug("map dummy");
    kernel_space->add_mapping(addr, (addr_t)get_dummy_tcb(), PGSIZE_KTCB, 
		false, true, false);
    sync_kernel_space(addr);
}

void space_t::map_sigma0(addr_t addr)
{
    //TRACEF("%p\n", addr);
    add_mapping(addr, addr, PGSIZE_SIGMA, true, false, false);
}


/**********************************************************************
 *
 *                    global functions
 *
 **********************************************************************/

/**
 * exc_pagefault: trap gate for ia32 pagefault handler
 */
X86_EXCWITH_ERRORCODE(exc_pagefault, 0)
{
    u32_t pf = x86_mmu_t::get_pagefault_address();
    //TRACEF("pagefault @ %p, ip=%p, sp=%p\n", pf, frame->eip, frame->esp);

    space_t * space = get_current_space();

#if defined(CONFIG_IA32_SMALL_SPACES)
    if (space->is_smallspace_area ((addr_t) pf))
    {
	if (space->sync_smallspace ((addr_t) pf))
	    return;

	pf -= space->smallspace_offset ();
    }

#if defined(CONFIG_IA32_SYSENTER)
    extern char sysexit_tramp_uaccess;
    extern char reenter_sysexit_uaccess;

    // In some cases kernel code might raise user-level pagefaults
    // (e.g., when reading the user-stack contents in the
    // reenter_sysexit trampoline).  Detect this so that regular
    // page-fault IPCs are generated.
    if (frame->eip == (word_t) &sysexit_tramp_uaccess ||
	frame->eip == (word_t) &reenter_sysexit_uaccess)
    {
	frame->error |= 4;
    }

#endif
#endif

    /* if the idle thread accesses the tcb area - 
     * we will get a pagefault with an invalid space
     * so we use CR3 to figure out the space
     */
    if (EXPECT_FALSE( space == NULL ))
	space = ptab_to_space(x86_mmu_t::get_active_pagetable(), 
			      get_current_cpu());

    space->handle_pagefault(
	(addr_t)pf, 
	(addr_t)frame->eip,
	(space_t::access_e)(frame->error & IA32_PAGE_WRITABLE),
	(frame->error & 4) ? false : true);
}

