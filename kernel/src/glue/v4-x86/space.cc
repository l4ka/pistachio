/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/space.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <generic/lib.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_API(tcb.h)
#include INC_API(smp.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(mmu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(pgent.h)

#include INC_GLUE(cpu.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(space.h)

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#endif

EXTERN_KMEM_GROUP  (kmem_space);
DECLARE_KMEM_GROUP (kmem_iofp);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

tcb_t * dummy_tcb = NULL;
space_t * kernel_space = NULL;
addr_t utcb_page = NULL;
cpuid_t	current_cpu UNIT("x86.cpulocal");;

/**********************************************************************
 *
 *                    space_t implementation
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
    for (word_t addr = KERNEL_AREA_END; addr >= KERNEL_AREA_START; addr -= X86_TOP_PDIR_SIZE)
	sync_kernel_space((addr_t) addr);
    
    set_kip_page_area(kip_area);
    set_utcb_page_area(utcb_area);

    
    /* map kip read-only to user */
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    if (data.compatibility_mode)
	add_mapping(kip_area.get_base(), virt_to_phys((addr_t) x32::get_kip()), pgent_t::size_4k, false, false, false);
    else
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */
#if defined(CONFIG_X86_IO_FLEXPAGES)
    if (!sigma0_space)
    {
	set_io_space(new vrt_io_t);
	get_io_space()->populate_sigma0();
    }
#endif
    
    add_mapping(kip_area.get_base(), virt_to_phys((addr_t) get_kip()), pgent_t::size_4k, false, false, false);

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
    //enter_kdebug("allocate tcb");

    addr_t page = kmem.alloc(kmem_tcb, X86_PAGE_SIZE);
    ASSERT(page);
    //TRACEF("tcb=%p, page=%p\n", addr, page);

    // map tcb kernel-writable, global 
    get_kernel_space()->add_mapping(addr, virt_to_phys(page), PGSIZE_KTCB, 
			      true, true, true);
    
    flush_tlbent (this, addr, page_shift(PGSIZE_KTCB));

    sync_kernel_space(addr);
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
						     (word_t) utcb & (~X86_PAGE_MASK)));
    else
    {
	// allocate new UTCB page
	addr_t page = kmem.alloc(kmem_utcb, X86_PAGE_SIZE);
	ASSERT(page);
	add_mapping((addr_t)utcb, virt_to_phys(page), PGSIZE_UTCB, true, false, false);
	result = (utcb_t *) addr_offset(page, (word_t) utcb & (~X86_PAGE_MASK));
    }

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    result->set_compatibility_mode(data.compatibility_mode);
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

    return result;
}

space_t * space_t::allocate_space() 
{
    space_t * space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t) + sizeof(top_pdir_t));
    ASSERT(space);
    space->data.cpu_ptab[get_current_cpu()].top_pdir = (top_pdir_t*)addr_offset(addr_t(space), sizeof(space_t));
    // create backlink to space 
    space->data.cpu_ptab[get_current_cpu()].top_pdir->space = space;
    space->data.reference_ptab = get_current_cpu();
    return space;
}    




void space_t::free_space() 
{
#ifdef CONFIG_SMP
    for (cpuid_t cpuid = 0; cpuid < CONFIG_SMP_MAX_CPUS; cpuid++)
	if (data.cpu_ptab[cpuid].top_pdir)
	    free_cpu_top_pdir(cpuid);
    kmem.free(kmem_space, (addr_t)this, sizeof(space_t));
#else
    kmem.free(kmem_space, (addr_t)this, sizeof(space_t) + sizeof(top_pdir_t));
#endif    
}


void space_t::remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
			 word_t len, bool writable, bool kernel, bool global)
{
    //TRACEF("\tremap area %p->%p len=%x, size=%d, %s, %s\n", 
    //   vaddr, paddr, len, pgsize,
    //   (writable ? "w" : "ro"), (kernel  ? "k" : "u"));
    
    word_t page_size = (pgsize == pgent_t::size_4k) ? X86_PAGE_SIZE : 
	X86_SUPERPAGE_SIZE;

    // length must be page-size aligned
    ASSERT((len & (page_size - 1)) == 0);

    for (word_t offset = 0; offset < len; offset += page_size)
	add_mapping(addr_offset(vaddr, offset), addr_offset(paddr, offset), 
		    pgsize, writable, kernel, global);
}


/* JS: TODO nx bit */
void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel, bool global, bool cacheable)
{
    pgent_t::pgsize_e curr_size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(curr_size, vaddr));

    //TRACEF("space=%p, v=%p, p=%08p, s=%d %s %s %s %s\n", this, vaddr, paddr, (long) size, 
    // (writable ? "w" : "ro"), (kernel  ? "k" : "u"), (global  ? "g" : "ng"), (cacheable  ? "c" : "nc")); 
	 
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
		ASSERT(addr_mask(vaddr, ~X86_SUPERPAGE_MASK) ==
		       addr_mask(paddr, ~X86_SUPERPAGE_MASK));


		if (((addr_t) pgent->address(this, curr_size)) == 
		    addr_mask(paddr, X86_SUPERPAGE_MASK))
		{
		    //TRACEF("superpage mapping @ %p space %p already exists.\n",  vaddr, this);
		    return;
		}
		TRACEF("already exsisting but inconsistend mapping %p space %p .\n",  
		       vaddr, this);
		ASSERT(false);
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


static tcb_t * get_dummy_tcb()
{
    if (!dummy_tcb)
    {
	dummy_tcb = (tcb_t*)kmem.alloc(kmem_tcb, page_size(PGSIZE_KTCB));
	ASSERT(dummy_tcb);
	dummy_tcb = virt_to_phys(dummy_tcb);
    }
    return dummy_tcb;
}

/**
 * Install a dummy TCB
 * @param addr	address where the dummy TCB should be installed
 *
 * The dummy TCB must be read-only and fail all validity tests.
 */
void space_t::map_dummy_tcb(addr_t addr)
{
    //TRACEF("%p\n", addr);
    //enter_kdebug("map dummy");
    get_kernel_space()->add_mapping(addr, (addr_t)get_dummy_tcb(), PGSIZE_KTCB, 
				    false, true, false); 
    
    flush_tlbent (this, addr, page_shift(PGSIZE_KTCB));
    sync_kernel_space(addr);
}


void space_t::switch_to_kernel_space(cpuid_t cpu)
{
    x86_mmu_t::set_active_pagetable((word_t)get_kernel_space()->get_top_pdir_phys(cpu));
}

/**
 * Try to copy a mapping from kernel space into the current space
 * @param addr the address for which the mapping should be copied
 * @return true if something was copied, false otherwise.
 * Synchronization must happen at the highest level, allowing sharing.
 */

bool space_t::sync_kernel_space(addr_t addr)
{
    if (this == get_kernel_space()) return false;
    cpuid_t cpu = get_current_cpu();
    
    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * dst_pgent = this->pgent(page_table_index(size, addr), cpu);
    pgent_t * src_pgent = get_kernel_space()->pgent(page_table_index(size, addr), cpu);

    /* (already valid) || (kernel space invalid) */
    if (dst_pgent->is_valid(this, size) || 
	(!src_pgent->is_valid(get_kernel_space(), size)))
    {
#if 0
	if (cpu == 1)
	    TRACE("sync kspace dst is valid @ %p (src=%p (%d), dst=%p (%d))\n", 
		  addr, kernel_space, src_pgent->is_valid(kernel_space, size),
		  this, dst_pgent->is_valid(this, size));
 #endif
	return false;
    }

#if 0
	TRACE("sync ksp @ %p (src=%p (%d), dst=%p (%d))\n", 
	      addr, kernel_space, src_pgent->is_valid(kernel_space, size),
	      this, dst_pgent->is_valid(this, size));
#endif

#if !defined(CONFIG_SMP)
    *dst_pgent = *src_pgent;
#else
    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	if (this->data.cpu_ptab[cpu].top_pdir) {
	    *this->pgent(page_table_index(size, addr), cpu) = 
		*get_kernel_space()->pgent(page_table_index(size, addr), cpu);
	}
#endif
    return true;
}


EXTERN_TRACEPOINT(IPC_STRING_COPY);

void space_t::populate_copy_area (word_t n, tcb_t *tcb, space_t *partner, cpuid_t cpu)
{
    ASSERT(tcb);
    ASSERT(partner);
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS && data.cpu_ptab[cpu].top_pdir);
    
    if (tcb->resources.copy_area_pdir_idx(n,0) != ~0UL)
    {
	pgent_t::pgsize_e pgsize = pgent_t::size_max;
		    
	pgent_t *src_pgent = partner->pgent(tcb->resources.copy_area_pdir_idx(n,0));
	pgent_t *dst_pgent = pgent(page_table_index(pgsize, (addr_t) (COPY_AREA_START + n * COPY_AREA_SIZE)));
	
	    
	for (word_t i=1; i < COPY_AREA_PDIRS; i++)
	{
	    pgsize--;
	    src_pgent = src_pgent->subtree(partner, pgsize)->
		next(partner, pgsize, tcb->resources.copy_area_pdir_idx(n,i));
			
	    dst_pgent = dst_pgent->subtree(partner, pgsize)->
		next(this, pgsize, page_table_index(pgsize, 
						    (addr_t) (COPY_AREA_START + n * COPY_AREA_SIZE)));
	}
	    
	for (word_t i = 0; i < (COPY_AREA_SIZE >> page_shift(pgsize)); i++)
	{
	    TRACEPOINT(IPC_STRING_COPY, 
		       "populate copy area %wx @ %wx -> %wx @ %wx idx=%d i=%d shift %d\n", 
		       src_pgent->pgent.get_raw(), src_pgent, 
		       dst_pgent->pgent.get_raw(), dst_pgent,
		       tcb->resources.copy_area_pdir_idx(n,0), i, page_shift(pgsize));
	    dst_pgent++->set_entry (this, pgsize, *src_pgent++);
	}
    }
}

void space_t::delete_copy_area (word_t n, cpuid_t cpu)
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS && data.cpu_ptab[cpu].top_pdir);
    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    
    
    pgent_t *copy_pgent = 
	pgent(page_table_index(pgsize, (addr_t) (COPY_AREA_START + n * COPY_AREA_SIZE)),
	      data.reference_ptab);
    
    for (word_t i=1; i < COPY_AREA_PDIRS; i++)
	copy_pgent = copy_pgent->subtree(this, pgsize)->
	    next(this, pgsize, page_table_index(pgsize, (addr_t) (COPY_AREA_START + n * COPY_AREA_SIZE)));

    for (word_t i = 0; i < (COPY_AREA_SIZE >> page_shift(pgsize)); i++)
	copy_pgent->pgent.clear();
}


#if defined(CONFIG_X86_IO_FLEXPAGES)
/*
 * get_io_bitmap
 *
 * returns the physical address of the IO Bitmap
 * synchronizes entries on SMP if required  
 */

addr_t space_t::get_io_bitmap(cpuid_t cpu)
{
    pgent_t *pgent;
    pgent_t::pgsize_e pgsize;

    if (this->lookup_mapping(tss.get_io_bitmap(), &pgent, &pgsize, cpu))
	return phys_to_virt(pgent->address(this, pgsize));
    
    TRACEF("BUG: get_io_bitmap_phys returns NULL\n");
    enter_kdebug("IO-Fpage BUG?");
    return NULL;
}


/* 
 * void install_io_bitmap()
 * 
 * installs an allocated 8k region as IO bitmap 
 *
 */
addr_t space_t::install_io_bitmap(bool create)
{
    addr_t new_bitmap = NULL;
    cpuid_t cpu = get_current_cpu();
    
    if (create)
    {
	new_bitmap = (word_t*) kmem.alloc(kmem_iofp, IOPERMBITMAP_SIZE);
	if (!new_bitmap) 
	    return NULL;
    }
    else
    {
	ASSERT(cpu != data.reference_ptab);
	/* Get bitmap from reference page table */
	new_bitmap = get_io_bitmap(data.reference_ptab);
    }
    ASSERT(new_bitmap);

    
    /* 
     * Allocate second level pagetables.
     * Do not use ->make_subtree, as we have to 
     * set it up _before_ installing it
     */
    addr_t io_bitmap_mapping = tss.get_io_bitmap();
    word_t top_idx = page_table_index(pgent_t::size_max, io_bitmap_mapping);
    pgent_t *src_pgent = pgent(top_idx, cpu);
    pgent_t *dst_pgent = create ? pgent(top_idx, data.reference_ptab) :  pgent(top_idx, cpu);
    pgent_t::pgsize_e size = pgent_t::size_max;

    while (size > PGSIZE_KERNEL)
    {
	pgent_t *new_subtree = (pgent_t *) kmem.alloc (kmem_iofp, X86_PAGE_SIZE);
	
	if (new_subtree == NULL)
	{
	    kmem.free(kmem_iofp, new_bitmap, IOPERMBITMAP_SIZE);
	    return NULL;
	}

		
	/* Copy all entries from original page table */
	src_pgent = src_pgent->subtree(this, size);
	ASSERT(src_pgent);
	
	memcpy(new_subtree, src_pgent, X86_PTAB_BYTES);
	
	ASSERT(dst_pgent);
	dst_pgent->set_entry(this, pgent_t::size_4k, virt_to_phys( (addr_t) new_subtree), 7, 0, false); 
	dst_pgent = dst_pgent->subtree(this, size);
	       
	size--;
	
	//TRACEF("New subtree %x\n", new_subtree);
	src_pgent = src_pgent->next(this, size, page_table_index(size, io_bitmap_mapping));
	dst_pgent = dst_pgent->next(this, size, page_table_index(size, io_bitmap_mapping));


    }
	
    /*	Set the two special entries  */
    ASSERT(size == pgent_t::size_4k);
    dst_pgent->set_entry(this, size, virt_to_phys(new_bitmap), 4, 0, false);	
#if defined(CONFIG_X86_PGE)
    dst_pgent->set_global(this, pgent_t::size_4k, false);
#endif
    
    dst_pgent = dst_pgent->next(this, size, 1); 
    dst_pgent->set_entry(this, size, virt_to_phys(addr_offset(new_bitmap, X86_PAGE_SIZE)), 4, 0, false);
#if defined(CONFIG_X86_PGE)
    dst_pgent->set_global(this, pgent_t::size_4k, false);
#endif

    flush_tlbent(get_current_space(), 
		 io_bitmap_mapping, 
		 page_shift (pgent_t::size_4k)); 
    flush_tlbent(get_current_space(), 
		 addr_offset(io_bitmap_mapping, X86_PAGE_SIZE), 
		 page_shift (pgent_t::size_4k));

    ASSERT(get_io_bitmap(data.reference_ptab) == new_bitmap);

    return new_bitmap;
}


/* 
 * void free_io_bitmap()
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
    

    space_t *kspace = get_kernel_space();
    addr_t io_bitmap = get_io_bitmap();
    addr_t io_bitmap_mapping = tss.get_io_bitmap();
    
    word_t top_idx = page_table_index(pgent_t::size_max, io_bitmap_mapping);
    pgent_t::pgsize_e size = pgent_t::size_max;
    
    pgent_t *orig_pgent = kspace->pgent(top_idx)->subtree(kspace, size);
    pgent_t *new_pgent = pgent(top_idx)->subtree(this, size);

    /* 
     * Restore original page entry
     */
    /*	Insert the default PGT	*/
    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	if (this->data.cpu_ptab[cpu].top_pdir) 
	    pgent(top_idx,cpu)->set_entry(this, size, virt_to_phys( (addr_t) orig_pgent), 6, 0, true); 
    
    /* 
     * Release private lower level pagetables.
     */

    while (size-- > PGSIZE_KERNEL)
    {
	addr_t subtree = (addr_t) new_pgent;
	new_pgent = new_pgent->next(this, size+1, page_table_index(size, io_bitmap_mapping));
	new_pgent = new_pgent->subtree(this, size);    
	
	/*  Release the Pagetable  */
	//TRACEF("Free subtree %x\n", subtree);
	kmem.free(kmem_iofp, subtree, X86_PTAB_BYTES);
	
    }
   
    /* Release the IOPBM */
    kmem.free(kmem_iofp, io_bitmap, IOPERMBITMAP_SIZE);

	
    /* Flush the corresponding TLB entries */
    flush_tlbent(get_current_space(), io_bitmap_mapping, page_shift (pgent_t::size_4k)); 
    flush_tlbent(get_current_space(), addr_offset(io_bitmap_mapping, 4096), page_shift (pgent_t::size_4k));
    
    //TRACEF("done %p/%p\n", subtree, io_bitmap);

}

/* 
 * void sync_io_bitmap()
 * 
 * synchronize  IOPBM across processors
 *
 */

bool space_t::sync_io_bitmap()
{
#if defined(CONFIG_SMP)
    /* May be that we've already created a bitmap on a different cpu */
    
    if (current_cpu == data.reference_ptab)
	return false;
    
    if (get_io_bitmap() != get_io_bitmap(data.reference_ptab))
    {
	//TRACEF("Sync IO bitmap entry from cpu %d\n", data.reference_ptab);
	install_io_bitmap(false);
	ASSERT(get_io_bitmap() == get_io_bitmap(data.reference_ptab));
	return true;
    }
#endif
    return false;
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

#endif

/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    
    /* we map both reserved areas into the kernel area */
    mem_region_t reg = get_kip()->reserved_mem0;
    align_memregion(reg, KERNEL_PAGE_SIZE);

    remap_area(phys_to_virt(reg.low), reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true, true);
    
    if (!get_kip()->reserved_mem1.is_empty())
    {
	reg = get_kip()->reserved_mem1;
	align_memregion(reg, KERNEL_PAGE_SIZE);
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
    align_memregion(reg, X86_SUPERPAGE_SIZE);
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
    utcb_page = kmem.alloc(kmem_misc, X86_PAGE_SIZE);
    ASSERT(utcb_page);
    add_mapping((addr_t)UTCB_MAPPING,  virt_to_phys(utcb_page), 
	       pgent_t::size_4k, true,	false, true);
    

    
#if defined(CONFIG_X86_SMALL_SPACES) && defined(CONFIG_X86_SYSENTER)
    /* User-level trampoline for ipc_sysexit, readonly but global. */
    extern word_t _start_utramp_p[];
    add_mapping ((addr_t) UTRAMP_MAPPING,
		 (addr_t) &_start_utramp_p,
		 pgent_t::size_4k, false, false, true);
#endif
    

#if defined(CONFIG_SUBARCH_X64)
    /* 
     * map syscalls read-only/executable to user 
     */
    ASSERT(((word_t) end_syscalls - (word_t) start_syscalls) <= KERNEL_PAGE_SIZE);
    remap_area(start_syscalls, virt_to_phys(start_syscalls),  
	       PGSIZE_KERNEL, KERNEL_PAGE_SIZE, false, false, true);
    /*  
     * Remap 4GB physical memory (e.g. for readmem) 
     */

    remap_area((addr_t) REMAP_32BIT_START, 0, pgent_t::size_2m, REMAP_32BIT_SIZE, true, true, true);
#endif


#if defined(CONFIG_SMP) || defined(CONFIG_X86_IO_FLEXPAGES)
    for (addr_t addr = start_cpu_local; addr < end_cpu_local; 
	 addr = addr_offset(addr, KERNEL_PAGE_SIZE))
    {
	pgent_t::pgsize_e size = pgent_t::size_max;
	pgent_t * pgent = this->pgent(page_table_index(size, start_cpu_local), 0);
	
	while (size != PGSIZE_KERNEL)
	{
	    pgent->set_cpulocal(this, size, true);
	    pgent = pgent->subtree(this, size--);
	    ASSERT(pgent);
	    pgent = pgent->next(this, size, page_table_index(size, addr));
	}
	
	TRACE_INIT("\tClearing global bit for cpulocal data v=%x p=%x (CPU 0) size %d\n", 
		   addr, pgent->raw, size);

#if defined(CONFIG_X86_PGE)
	/* HT boxes share TLB entries, thus need to make entries non-global */
	pgent->set_global(this, size, false);
#endif
	
    }
#endif
    
    return;
}



void SECTION (".init") space_t::init_cpu_mappings(cpuid_t cpu)
{
    if (cpu == 0) return;

#if defined(CONFIG_SMP)
    pgent_t::pgsize_e size;

    /* CPU 0 gets the always initialized page table */
    TRACE_INIT("\tInitialize cpu local mappings (CPU %d)\n", cpu);

    mem_region_t reg = { start_cpu_local, end_cpu_local };
    align_memregion(reg, KERNEL_PAGE_SIZE);
    size = pgent_t::size_max;

    /* allocate kernel top pdir */
    alloc_cpu_top_pdir(cpu);

  
    /*  use CPU0 as reference top dir and fix up cpulocal data afterwards */
    //TRACE_INIT("\tcopying shared TOPD entries from %p -> %p (size %d)\n", 
    // data.cpu_ptab[0].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
    // data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
    // X86_TOP_PDIR_IDX(KERNEL_AREA_SIZE) * sizeof(pgent_t));

    ASSERT(data.cpu_ptab[cpu].top_pdir && data.cpu_ptab[0].top_pdir);
    memcpy(data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
	   data.cpu_ptab[0].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
	   X86_TOP_PDIR_IDX(KERNEL_AREA_SIZE) * sizeof(pgent_t));


    TRACE_INIT("\tRemapping CPU local memory %p - %p (CPU %d)\n", 
	       start_cpu_local, end_cpu_local, cpu);

    pgent_t *src_pgent = this->pgent(page_table_index(size, reg.low), 0);
    pgent_t *dst_pgent = this->pgent(page_table_index(size, reg.low), cpu);

    while (size > PGSIZE_KERNEL)
    {
	src_pgent = src_pgent->subtree(this, size);
	dst_pgent->make_cpu_subtree(this, size, true);
	dst_pgent = dst_pgent->subtree(this, size);

	ASSERT(src_pgent && dst_pgent);
	
	/*  use CPU0 as reference and fix up cpulocal data afterwards */
	//TRACE_INIT("\tcopying shared %s entries from %p -> %p (size %d)\n", 
	//   (size == pgent_t::size_4k ? "PTAB" : 
	//    (size == pgent_t::size_superpage ? "PDIR " : "PDP")),
	//   src_pgent, dst_pgent, X86_PTAB_BYTES);
	
	memcpy(dst_pgent, src_pgent, X86_PTAB_BYTES);
	
	/* proceed to next lower level */
	size--;
	src_pgent = src_pgent->next(this, size, page_table_index(size, reg.low));
	dst_pgent = dst_pgent->next(this, size, page_table_index(size, reg.low));
	dst_pgent->set_cpulocal(this, size, true);

    }
    
    /* set entries for CPU-local memory inside the current CPU's PDIR */
    for ( addr_t addr = reg.low; addr < reg.high;
	  addr = addr_offset(addr, KERNEL_PAGE_SIZE) )
    {
	addr_t ptab = kmem.alloc(kmem_pgtab, KERNEL_PAGE_SIZE);
	
	ASSERT(ptab);
	TRACE_INIT("\tallocated %s cpu-local data at %x -> phys %x\n", 
		   (size == pgent_t::size_superpage ? "PDIR" : "PTAB"),
		   addr, virt_to_phys(ptab));
	
	memcpy(ptab, addr, KERNEL_PAGE_SIZE);
	dst_pgent->set_entry(this, size, virt_to_phys(ptab), 7, 8, true);
#if defined(CONFIG_X86_PGE)
	/* HT boxes share TLB entries, thus need to make entries non-global */
	//TRACE_INIT("\tClearing global bit for cpulocal data %x %x, %p (CPU 0)\n", 
	//   addr, dst_pgent->raw, dst_pgent);
	dst_pgent->set_global(this, size, false);
#endif
	dst_pgent->set_cpulocal(this, size, true);
	dst_pgent = dst_pgent->next(this, size, 1);
	
    }
    
    TRACE_INIT("\tSwitching to CPU local pagetable %p (CPU %d)\n", 
	       get_top_pdir_phys(cpu), cpu);
    x86_mmu_t::set_active_pagetable((word_t)get_top_pdir_phys(cpu));
    x86_mmu_t::flush_tlb(true);
    current_cpu = cpu;
    TRACE_INIT("\tCPU local pagetable activated %x (CPU %d)\n", 
	       x86_mmu_t::get_active_pagetable(), cpu);
#endif

}

/**
 * initialize THE kernel space
 * @see get_kernel_space()
 */
void SECTION(".init.memory") space_t::init_kernel_space()
{
 
    ASSERT(!kernel_space);
    
    kernel_space = (space_t *) kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(kernel_space);

    kernel_space->data.cpu_ptab[0].top_pdir = 
	(space_t::top_pdir_t*)kmem.alloc(kmem_space, sizeof(space_t::top_pdir_t));
    
    kernel_space->data.cpu_ptab[0].top_pdir->space = kernel_space;
    kernel_space->init_kernel_mappings();
    
    TRACE_INIT("\tSwitching to CPU local pagetable %p (CPU %d)\n", 
	       kernel_space->get_top_pdir_phys(0), 0);
    x86_mmu_t::set_active_pagetable((word_t) kernel_space->get_top_pdir_phys(0));


    TRACE_INIT("CPU local pagetable activated %x (CPU %d)\n", 
	       x86_mmu_t::get_active_pagetable(), 0);
	
}


/**********************************************************************
 *
 *                    global functions
 *
 **********************************************************************/

space_t * allocate_space()
{
    return space_t::allocate_space();
}

void free_space(space_t * space)
{
    ASSERT(space);
    space->free_space();
}

/**
 * exc_pagefault: trap gate for ia32 pagefault handler
 */
X86_EXCWITH_ERRORCODE(exc_pagefault, 0)
{
    word_t pf = x86_mmu_t::get_pagefault_address();
    //TRACEF("pagefault @ %p, ip=%p, sp=%p\n", pf, 
    //   frame->regs[x86_exceptionframe_t::ipreg], 
    //   frame->regs[x86_exceptionframe_t::spreg]);

    space_t * space = get_current_space();

#if defined(CONFIG_X86_SMALL_SPACES)
    if (space->is_smallspace_area ((addr_t) pf))
    {
	if (space->sync_smallspace ((addr_t) pf))
	    return;

	pf -= space->smallspace_offset ();
    }

#if defined(CONFIG_X86_SYSENTER)
    extern char sysexit_tramp_uaccess;
    extern char reenter_sysexit_uaccess;

    // In some cases kernel code might raise user-level pagefaults
    // (e.g., when reading the user-stack contents in the
    // reenter_sysexit trampoline).  Detect this so that regular
    // page-fault IPCs are generated.
    if (frame->regs[x86_exceptionframe_t::ipreg] == (word_t) &sysexit_tramp_uaccess ||
	frame->regs[x86_exceptionframe_t::ipreg] == (word_t) &reenter_sysexit_uaccess)
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
	space = space_t::top_pdir_to_space(x86_mmu_t::get_active_pagetable());
    
    ASSERT(space);
    
    space->handle_pagefault(
	(addr_t)pf, 
	(addr_t)frame->regs[x86_exceptionframe_t::ipreg],
	(space_t::access_e)(frame->error & X86_PAGEFAULT_BITS),
	(frame->error & 4) ? false : true);

    
}

 /**********************************************************************
 *
 *                        SMP handling
 *
 **********************************************************************/

#if defined(CONFIG_SMP)
static word_t cpu_remote_flush UNIT("cpulocal");
static word_t cpu_remote_flush_global UNIT("cpulocal");
active_cpu_space_t active_cpu_space;

#if defined(CONFIG_X86_SMALL_SPACES_GLOBAL)
#define __FLUSH_GLOBAL__	entry->param[0]
#else
#define __FLUSH_GLOBAL__	false
#endif



static void do_xcpu_flush_tlb(cpu_mb_entry_t * entry)
{
    spin(60, get_current_cpu());
    //TRACEF("remote flush %x %d\n", get_current_space(), get_current_cpu());
    x86_mmu_t::flush_tlb (__FLUSH_GLOBAL__);
}

static void flush_tlb_remote()
{
    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
	if (cpu_remote_flush & (1 << cpu))
	    sync_xcpu_request(cpu, do_xcpu_flush_tlb, NULL,
			      cpu_remote_flush_global & (1 << cpu));
    cpu_remote_flush = 0;
    cpu_remote_flush_global = 0;
}

INLINE void tag_flush_remote (space_t * curspace, bool force=false)
{
    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
    {
	if (cpu == get_current_cpu())
	    continue;
	
	if (active_cpu_space.get(cpu) == curspace || force)
	{
	    //TRACEF("tag remote flush %x %d\n", curspace, cpu);
	    cpu_remote_flush |= (1 << cpu);
	}

#if defined(CONFIG_X86_SMALL_SPACES)
	// For small spaces we must also do TLB shootdown if current
	// space is small, or current active space on remote CPU is a
	// small space.
	if ((active_cpu_space.get(cpu) &&
	     active_cpu_space.get(cpu)->is_small ()) ||
	    curspace->is_small ())
	{
	    cpu_remote_flush |= (1 << cpu);
#if defined(CONFIG_X86_SMALL_SPACES_GLOBAL)
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
    /* js: for kernel addresses, we force an immediate remote flush */
    bool force = !is_user_area(addr);
    
    if (this == curspace || IS_SPACE_SMALL (this))
	x86_mmu_t::flush_tlbent ((word_t) addr);
    
    tag_flush_remote (this, force);
    if (force)
	flush_tlb_remote();

}


void space_t::end_update ()
{
    flush_tlb_remote();
}


// migration fixup
void space_t::move_tcb(tcb_t * tcb, cpuid_t src_cpu, cpuid_t dst_cpu)
{
    tcb->resources.smp_xcpu_pagetable (tcb, dst_cpu);
    data.cpu_ptab[dst_cpu].thread_count++;
    data.cpu_ptab[src_cpu].thread_count--;
  
}

void space_t::alloc_cpu_top_pdir(cpuid_t cpu) 
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    ASSERT(!data.cpu_ptab[cpu].top_pdir);

    //TRACEF("%x pdir %x cpu %d\n", this, data.cpu_ptab[cpu].top_pdir, cpu);

    /* Allocate PML4 */
    data.cpu_ptab[cpu].top_pdir = (top_pdir_t*)kmem.alloc(kmem_space, sizeof(top_pdir_t));
    ASSERT(data.cpu_ptab[cpu].top_pdir && data.cpu_ptab[data.reference_ptab].top_pdir);
    
    /*  use CPU0 as reference pdir and fix up cpulocal data afterwards */
    //TRACEF("copying user pdir entries from %p -> %p (size %d)\n", 	
    //   data.cpu_ptab[data.reference_ptab].top_pdir->pgent + X86_TOP_PDIR_IDX(USER_AREA_START),
    //   data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(USER_AREA_START),
    //   X86_TOP_PDIR_IDX(USER_AREA_SIZE) * sizeof(pgent_t));

    /* Copy user entries from reference ptab */
    memcpy( data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(USER_AREA_START),
	    data.cpu_ptab[data.reference_ptab].top_pdir->pgent + X86_TOP_PDIR_IDX(USER_AREA_START),
	    X86_TOP_PDIR_IDX(USER_AREA_SIZE) * sizeof(pgent_t));
   
    //TRACEF("copying kernel pdir entries from %p -> %p (size %d)\n", 
    //   get_kernel_space()->data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
    //   data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
    //   X86_TOP_PDIR_IDX(KERNEL_AREA_SIZE) * sizeof(pgent_t));

    /* Copy kernel entries from kernel space ptab */
    memcpy(data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
	   get_kernel_space()->data.cpu_ptab[cpu].top_pdir->pgent + X86_TOP_PDIR_IDX(KERNEL_AREA_START),
	   X86_TOP_PDIR_IDX(KERNEL_AREA_SIZE) * sizeof(pgent_t));
    
    data.cpu_ptab[cpu].top_pdir->space = this;
}

void space_t::free_cpu_top_pdir(cpuid_t cpu) 
{
    //TRACEF("%x pdir %x cpu %d\n", this, data.cpu_ptab[cpu].top_pdir, cpu);

    ASSERT(data.cpu_ptab[cpu].top_pdir);
    ASSERT(data.cpu_ptab[cpu].thread_count == 0);
    top_pdir_t* pdir = data.cpu_ptab[cpu].top_pdir;
    data.cpu_ptab[cpu].top_pdir = NULL; // mem ordering, for X86 no barrier needed
    kmem.free(kmem_space, (addr_t)pdir, sizeof(top_pdir_t));
}



#endif /* defined(CONFIG_SMP) */

