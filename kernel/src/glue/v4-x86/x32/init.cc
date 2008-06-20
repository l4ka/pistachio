/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                 
* File path:     glue/v4-x86/x32/init.cc
 * Description:   ia32-specific initialization
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
 * $Id: init.cc,v 1.60 2007/03/20 14:56:23 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>
#include <init.h>

/* cpu specific types */
#include INC_ARCH(cpu.h)

/* pagetable and mmu management */
#include INC_ARCH(mmu.h)
#include INC_ARCH_SA(ptab.h)

/* idt, tss, gdt etc. */
#include INC_ARCH(segdesc.h)
#include INC_ARCH_SA(tss.h)

/* floating point unit */
#include INC_ARCH(fpu.h)

/* K8 flush filter support  */
#if defined(CONFIG_CPU_X86_K8)
#include INC_ARCH(amdhwcr.h)
#endif 

#include INC_GLUE(config.h)
#include INC_GLUE(idt.h)
#include INC_GLUE(space.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(syscalls.h)

#include INC_API(smp.h)
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(processor.h)

#include INC_PLAT(rtc.h)
#include INC_PLAT(perfmon.h)

static word_t init_pdir[1024] __attribute__((aligned(4096))) SECTION(".init.data");

#if defined (CONFIG_X86_PSE)

#define MAX_KERNEL_MAPPINGS	64
#define PAGE_ATTRIB_INIT	(X86_PAGE_VALID | X86_PAGE_WRITABLE | X86_PAGE_SUPER)

#else /* !CONFIG_X86_PSE */

#define MAX_KERNEL_MAPPINGS	8
#define PAGE_ATTRIB_INIT	(X86_PAGE_VALID | X86_PAGE_WRITABLE)
// 2nd-level page tables for the initial page table
static word_t init_ptable[MAX_KERNEL_MAPPINGS][1024] __attribute__((aligned(4096))) SECTION(".init.data");

#endif /* !CONFIG_X86_PSE */



/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/
#if defined(CONFIG_SMP)
x86_segdesc_t	smp_boot_gdt[3];
void setup_smp_boot_gdt (void)
{
#   define gdt_idx(x) ((x) >> 3)
    smp_boot_gdt[gdt_idx(X86_KCS)].set_seg(0, ~0UL, 0, x86_segdesc_t::code);
    smp_boot_gdt[gdt_idx(X86_KDS)].set_seg(0, ~0UL, 0, x86_segdesc_t::data);
#   undef gdt_idx
}
#endif



/**********************************************************************
 *
 *  processor local initialization, performed by all IA32 CPUs
 *
 **********************************************************************/

/* processor local data */
x86_segdesc_t	gdt[GDT_SIZE] UNIT("x86.cpulocal");
x86_x32_tss_t	tss UNIT("x86.cpulocal.tss");



void SECTION(SEC_INIT) setup_gdt(x86_tss_t &tss, cpuid_t cpuid)
{
#   define gdt_idx(x) ((x) >> 3)

    gdt[gdt_idx(X86_KCS)].set_seg(0, ~0UL, 0, x86_segdesc_t::code);
    gdt[gdt_idx(X86_KDS)].set_seg(0, ~0UL, 0, x86_segdesc_t::data);
    gdt[gdt_idx(X86_UCS)].set_seg(0, ~0UL, 3, x86_segdesc_t::code);
    gdt[gdt_idx(X86_UDS)].set_seg(0, ~0UL, 3, x86_segdesc_t::data);

    /* MyUTCB pointer, 
     * we use a separate page for all processors allocated in space_t 
     * and have one UTCB entry per cache line in the SMP case */
    ASSERT(unsigned(cpuid * CACHE_LINE_SIZE) < X86_PAGE_SIZE);
    gdt[gdt_idx(X86_UTCBS)].set_seg((u32_t)UTCB_MAPPING + 
				    (cpuid * CACHE_LINE_SIZE),
				    sizeof(threadid_t) - 1, 
				    3, x86_segdesc_t::data);

    /* the TSS
     * The last byte in x86_x32_tss_t is a stopper for the IO permission bitmap.
     * That's why we set the limit in the GDT to one byte less than the actual
     * size of the structure. (IA32-RefMan, Part 1, Chapter Input/Output) */
    //TRACEF("gdt @ %d = %x (size %x)\n", gdt_idx(X86_X32_TSS), &tss, sizeof(x86_x32_tss_t)-1);
    gdt[gdt_idx(X86_TSS)].set_sys((u32_t) &tss, sizeof(x86_x32_tss_t)-1, 
				   0, x86_segdesc_t::tss);
    
#ifdef CONFIG_TRACEBUFFER
    if (get_tracebuffer())
        gdt[gdt_idx(X86_TBS)].set_seg((u32_t)get_tracebuffer(), TRACEBUFFER_SIZE-1, 3,
				       x86_segdesc_t::data);
    else gdt[gdt_idx(X86_TBS)].set_seg(0, ~0UL, 3, x86_segdesc_t::data);
#endif

	
    /* create a temporary GDT descriptor to load the GDTR from */

    x86_descreg_t gdtr((word_t) &gdt, sizeof(gdt));
    gdtr.setdescreg(x86_descreg_t::gdtr);
    x86_descreg_t tr(X86_TSS);
    tr.setselreg(x86_descreg_t::tr);


    asm("ljmp	%0,$1f		\n"	/* refetch code segment	descr.	*/
	"1:			\n"	/*   by jumping across segments	*/
	:
	: "i" (X86_KCS)
	);
    

    /* set the segment registers from the freshly installed GDT
       and load the Task Register with the TSS via the GDT */
    asm("mov  %0, %%ds		\n"	/* reload data segment		*/
	"mov  %0, %%es		\n"	/* need valid %es for movs/stos	*/
	"mov  %1, %%ss		\n"	/* reload stack segment		*/
	"mov  %2, %%gs		\n"	/* load UTCB segment		*/
#ifdef CONFIG_TRACEBUFFER
        "mov  %3, %%fs          \n"     /* tracebuffer segment          */
#else
	"mov  %0, %%fs		\n"	/* default without tracebuffer	*/
#endif
	"xor %%eax, %%eax	\n"	/*				*/
	"lldt %%ax		\n"	/* clear LDTR			*/
	:
	:
#if defined(CONFIG_X86_SMALL_SPACES)
	"r"(X86_KDS),
#else
	"r"(X86_UDS),
#endif
	"r"(X86_KDS), "r"(X86_UTCBS), "r"(X86_TBS)
	: "eax"
	);
}



/**
 * setup_msrs: initializes all model specific registers for CPU
 */
void setup_msrs (void)
{
#ifdef CONFIG_X86_SYSENTER
    /* here we also setup the model specific registers for the syscalls */
    x86_wrmsr(X86_MSR_SYSENTER_CS, (u32_t)(X86_KCS));
    x86_wrmsr(X86_MSR_SYSENTER_EIP, (u32_t)(exc_user_sysipc));
    x86_wrmsr(X86_MSR_SYSENTER_ESP, (u32_t)(&tss) + 4);
#endif

#if defined(CONFIG_X86_FXSR)
    x86_fpu_t::enable_osfxsr();
#endif
    x86_fpu_t::disable();
    
#if defined(CONFIG_X86_PAT)
    // Initialize all PAT values to their default after-reset state,
    // except PAT4 which is set to write-combining (instead of
    // write-back), and PAT7 which is set to write-protected (instead
    // of uncacheable).  This setup allows for safe "fallback" to
    // regular page-based cache settings in the case the PAT bit in
    // the page table is ignored, and it allows for regular behaviour
    // for the PCD and PWT bits if the PAT bit is not set.

    u64_t pats =
	((u64_t) X86_MSR_PAT_WB << (8*0)) | ((u64_t) X86_MSR_PAT_WT << (8*1)) |
	((u64_t) X86_MSR_PAT_UM << (8*2)) | ((u64_t) X86_MSR_PAT_UC << (8*3)) |
	((u64_t) X86_MSR_PAT_WC << (8*4)) | ((u64_t) X86_MSR_PAT_WT << (8*5)) |
	((u64_t) X86_MSR_PAT_UM << (8*6)) | ((u64_t) X86_MSR_PAT_WP << (8*7));

    x86_wrmsr (X86_MSR_CR_PAT, pats);
#endif
}

/**
 * checks the IA32 features (CPUID) to make sure the processor
 * has all necessary features */
void SECTION(".init.cpu") check_cpu_features()
{
    u32_t req_features = X86_X32_FEAT_FPU;
#ifdef CONFIG_X86_PSE
    req_features |= X86_X32_FEAT_PSE;
#endif
#ifdef CONFIG_X86_PGE
    req_features |= X86_X32_FEAT_PGE;
#endif
#ifdef CONFIG_X86_FXSR
    req_features |= X86_X32_FEAT_FXSR;
#endif
#ifdef CONFIG_X86_SYSENTER
    req_features |= X86_X32_FEAT_SEP;
#endif
#ifdef CONFIG_IOAPIC
    req_features |= X86_X32_FEAT_APIC;
#endif
    u32_t avail_features = x86_x32_get_cpu_features();

    if ((req_features & avail_features) != req_features)
    {
	printf("CPU does not support all features (%x) -- halting\n", req_features);
#if defined(CONFIG_VERBOSE_INIT)
	const char* x86_x32_features[] = {
	    "fpu",  "vme",    "de",   "pse",   "tsc",  "msr", "pae",  "mce",
	    "cx8",  "apic",   "?",    "sep",   "mtrr", "pge", "mca",  "cmov",
	    "pat",  "pse-36", "psn",  "cflsh", "?",    "ds",  "acpi", "mmx",
	    "fxsr", "sse",    "sse2", "ss",    "ht",   "tm",  "ia64", "pbe" };
	for (int i = 0; i < 32; i++)
	    if ((req_features & 1 << i) && (!(avail_features & 1 << i)))
		printf("%s ", x86_x32_features[i]);
	printf("missing\n");
#endif
	spin_forever();
    }
}



/***********************************************************************
 *
 *          system specific initialization and global data 
 *
 **********************************************************************/


/**
 * setup_idt: initializes the interrupt descriptor table
 */


static void __attribute__((unused)) dump_pdir() 
{
    for (int i = 0 ; i < 1024; i++)
	if (init_pdir[i])
	    printf("%d/%x @ %x: %x\n", i, i << 22, 
		   &init_pdir[i], init_pdir[i]);
}


/**
 * init_meminfo: registers memory section with KIP
 */
void SECTION(SEC_INIT) init_meminfo(void)
{
    /* register virtual memory */
    get_kip()->memory_info.insert(memdesc_t::conventional, 
				  true, // virtual memory
				  (void*)USER_AREA_START,
				  (void*)USER_AREA_END);

    // Register kernel code/data as reserved
    get_kip()->memory_info.insert(memdesc_t::reserved,
                                  false,        // not virtual
                                  start_text_phys, end_bootmem_phys);

    // Were we booted via RMGR?
    if (!get_kip()->reserved_mem1.is_empty())
    {
        // Register physical
        get_kip()->memory_info.insert(memdesc_t::conventional,
                                      false,
                                      get_kip()->main_mem.low,
                                      get_kip()->main_mem.high);
        // Register KMEM area as reserved
        if (!get_kip()->reserved_mem1.is_empty())
            get_kip()->memory_info.insert(memdesc_t::reserved,
                                          false,
                                          get_kip()->reserved_mem1.low,
                                          get_kip()->reserved_mem1.high);
        if (!get_kip()->dedicated_mem0.is_empty())
            get_kip()->memory_info.insert(memdesc_t::dedicated,
                                          false,
                                          get_kip()->dedicated_mem0.low,
                                          get_kip()->dedicated_mem0.high);
    }
}


/**
 * init_arch: architecture specific initialization
 *
 * Initializes the kernel debugger, boot memory, and the IDT.
 */
static void SECTION(SEC_INIT) __attribute__((unused)) init_arch() 
{
}



/**
 * init_paging:  initializes the startup pagetable
 *               
 * The startup-pagetable contains two regions, a physical 1:1 mapping
 * at virtual address 0 and upwards and a virtual mapping at the 
 * virtual kernel address range.  These mappings are initialized as 
 * 4MB mappings which reduces the needed memory to the page directory only.  
 * After initializing the kernel memory allocator a new kernel pagetable 
 * is set up which can be populated using kmem_alloc etc. 
 */
extern "C" void SECTION(SEC_INIT) init_paging()
{
    /* zero pagetable */
    for (int i = 0; i < 1024; i++)
	init_pdir[i] = 0;

    /* Setup the initial mappings.  The first MAX_KERNEL_MAPPINGS*4MB
       are mapped 1:1.  The same region is also visible at
       KERNEL_OFFSET */
#warning "use virt_to_phys for page mappings"
#if defined(CONFIG_X86_PSE)
    for (int i = 0; i < MAX_KERNEL_MAPPINGS; i++)
	init_pdir[i] = 
	    init_pdir[(KERNEL_OFFSET >> X86_X32_PDIR_BITS) + i] = 
	    (i << X86_X32_PDIR_BITS) | PAGE_ATTRIB_INIT;
#else
    for (int i = 0; i < MAX_KERNEL_MAPPINGS; i++)
    {
        // Fill 2nd-level page table
	for (int j = 0; j<1024; j++) 
	    init_ptable[i][j] = ((i << X86_X32_PDIR_BITS) |
                                 (j << X86_PAGE_BITS) |
                                 PAGE_ATTRIB_INIT);
        // Install page table in page directory
	init_pdir[i] = 
	    init_pdir[(KERNEL_OFFSET >> X86_X32_PDIR_BITS) + i] = 
	    (word_t)(init_ptable[i]) | PAGE_ATTRIB_INIT;
    }
#endif /* CONFIG_X86_PSE */

    /* now activate the startup pagetable */
#if defined(CONFIG_X86_PSE)
    x86_mmu_t::enable_super_pages();
#endif
#ifdef CONFIG_X86_PGE
    x86_mmu_t::enable_global_pages();
#endif
    x86_mmu_t::set_active_pagetable((u32_t)init_pdir);
    x86_mmu_t::enable_paging();
}

