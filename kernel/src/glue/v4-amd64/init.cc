/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/init.cc
 * Description:   System initialization
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
 * $Id: init.cc,v 1.27 2007/02/21 07:06:14 stoess Exp $ 
 *                
 ********************************************************************/
#include <init.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>


#include INC_API(smp.h)

#include INC_API(kernelinterface.h)
#include INC_API(types.h)
#include INC_API(processor.h)
#include INC_API(schedule.h)

#include INC_ARCH(cpuid.h)
#include INC_ARCH(descreg.h)
#include INC_ARCHX(x86,amdhwcr.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH(tss.h)
#include INC_ARCHX(x86,apic.h)
#include INC_ARCH(config.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUEX(x86,timer.h)
#include INC_GLUE(idt.h)
#include INC_GLUEX(x86,memory.h)

#include INC_PLAT(rtc.h)
#include INC_PLAT(perfmon.h)


#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
#include INC_GLUE(ia32/kernelinterface.h)
#include INC_GLUE(ia32/init.h)
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

amd64_cpu_features_t boot_cpu_ft UNIT("amd64.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 1);
x86_tss_t tss UNIT("amd64.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 2);
bool tracebuffer_initialized UNIT("amd64.cpulocal");


struct gdt_struct {
    amd64_segdesc_t segdsc[GDT_SIZE - 2];	/* 6 entries a  8 byte */
    amd64_tssdesc_t tssdsc;			/* 1 entries a 16 byte */
} gdt UNIT("amd64.cpulocal");

u8_t amd64_cache_line_size;


// from glue/v4-x86/
void clear_bss (void);


/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/

#if defined(CONFIG_SMP)
amd64_segdesc_t	smp_boot_gdt[3];
void setup_smp_boot_gdt (void)
{
    /* segment descriptors in long mode and legacy mode are almost identical.
     *  However, in long mode, most of the fields are ignored, thus we can set
     *  up those segments although the APs are not yet in long mode when they
     *  are used.
     */
#   define gdt_idx(x) ((x) >> 3)
  smp_boot_gdt[gdt_idx(X86_KCS)].set_seg((u64_t)0, amd64_segdesc_t::code, 0, amd64_segdesc_t::m_comp);
  smp_boot_gdt[gdt_idx(X86_KDS)].set_seg((u64_t)0, amd64_segdesc_t::data, 0, amd64_segdesc_t::m_comp);  
#   undef gdt_idx
}
#endif



/**
 * Check CPU features 
 * 
 */

void SECTION(SEC_INIT) check_cpu_features()
{

#if 0
    boot_cpu_ft.dump_features();
#endif
    amd64_cache_line_size = boot_cpu_ft.get_l1_cache().d.dcache.l_size;
}

/**
 * Initialize boot memory 
 * 
 */

void SECTION(SEC_INIT) init_bootmem (void)
{
    
    extern u8_t _start_bootmem[];
    extern u8_t _end_bootmem[];
    
    
    for (u8_t * p = _start_bootmem; p < _end_bootmem; p++){
	*p = 0;
    }

    kmem.init(start_bootmem, end_bootmem); 
}



void add_more_kmem (void)
{

    /* 
     * Scan memory descriptors for a block of reserved physical memory
     * that is within the range of (to be) contiguously mapped
     * physical memory.  If there's one, it has been set up by the
     * boot loader for us. 
     */
    
    bool found = false;

    for (word_t i = 0;
         i < get_kip()->memory_info.get_num_descriptors();
         i++)
    {
        memdesc_t* md = get_kip()->memory_info.get_memdesc(i);

        if (!md->is_virtual() &&
            (md->type() == memdesc_t::reserved) &&
            (word_t) md->high() <= KERNEL_AREA_END)
        {
	    TRACE_INIT("Using region %x size %x for kernel memory\n", md->low(), md->size());
            /* Map region kernel writable  */
            get_kernel_space()->remap_area(
                phys_to_virt(md->low()), md->low(),
                (KERNEL_PAGE_SIZE == AMD64_2MPAGE_SIZE)
                 ? pgent_t::size_2m
                 : pgent_t::size_4k,
                (md->size() + (KERNEL_PAGE_SIZE-1)) & ~(KERNEL_PAGE_SIZE-1),
                true, true, true);

            /* Add it to allocator */
            kmem.add(phys_to_virt(md->low()),
                     md->size());
	    found = true;
        }
    }
    
    if (!found)
	TRACE_INIT("Did not find any region usable for kernel memory\n");

}

void SECTION(SEC_INIT) init_meminfo (void)
{

    extern word_t _memory_descriptors_size[];

    if (get_kip()->memory_info.get_num_descriptors() == (word_t) &_memory_descriptors_size){
	TRACE_INIT("\tBootloader did not patch memory info...\n");
	get_kip()->memory_info.n = 0;
    }
    
    /* 
     * reserve ourselves
     */
    
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_text_phys, end_text_phys);

    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_bootmem_phys, end_bootmem_phys);

    get_kip()->memory_info.insert(memdesc_t::reserved, false,
				  start_syscalls_phys, end_syscalls_phys);

    /* 
     * add user area
     */

    get_kip()->memory_info.insert(memdesc_t::conventional, true,
				  (void *) USER_AREA_START, (void *) USER_AREA_END);
    

    /* 
     * dump reserved memory regions
     */
    
    //TRACE_INIT("Memory descriptors (p=%p/n=%d):\n",
    //       get_kip()->memory_info.get_memdesc(0),
    //     get_kip()->memory_info.get_num_descriptors());
    //for (word_t i=0; i < get_kip()->memory_info.get_num_descriptors(); i++){
	//memdesc_t *m = get_kip()->memory_info.get_memdesc(i);
	//TRACE_INIT("\t#%d, low=%x, high=%x, type=%x\n", i, 
	//   m->low(),  m->high(),  m->type());
    //}

}

/**********************************************************************
 *
 *  processor local initialization, performed by all CPUs
 *
 **********************************************************************/


#if defined(CONFIG_TRACEBUFFER)
tracebuffer_t * tracebuffer;
EXTERN_KMEM_GROUP (kmem_misc);

void SECTION(SEC_INIT) setup_tracebuffer (void)
{
    tracebuffer = (tracebuffer_t *) kmem.alloc (kmem_misc, TRACEBUFFER_SIZE);
    ASSERT (TRACEBUFFER_SIZE == MB (2));
    get_kernel_space ()->add_mapping (tracebuffer,
				     virt_to_phys (tracebuffer),
				     pgent_t::size_2m,
				     true, false, true);
    tracebuffer->initialize ();
}
#endif /* defined(CONFIG_TRACEBUFFER) */

/**
 * Setup global descriptor table 
 * 
 */

void SECTION(SEC_INIT) setup_gdt(x86_tss_t &tss, cpuid_t cpuid)
{

    /* Initialize GDT */
    gdt.segdsc[GDT_IDX(AMD64_INVS)].set_seg((u64_t) 0, amd64_segdesc_t::inv, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KCS)].set_seg((u64_t) 0, amd64_segdesc_t::code, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KDS)].set_seg((u64_t) 0, amd64_segdesc_t::data, 0, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_UCS)].set_seg((u64_t) 0, amd64_segdesc_t::code, 3, amd64_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_UDS)].set_seg((u64_t) 0, amd64_segdesc_t::data, 3, amd64_segdesc_t::m_long);
    
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    gdt.segdsc[GDT_IDX(AMD64_UCS32)].set_seg((u64_t) 0, amd64_segdesc_t::code, 3, amd64_segdesc_t::m_comp);
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

    /* TODO: Assertion correct ? */
    ASSERT(unsigned(cpuid * AMD64_CACHE_LINE_SIZE) < AMD64_2MPAGE_SIZE);
    
    /* Set TSS */
#if defined(CONFIG_IO_FLEXPAGES)
    gdt.tssdsc.set_seg((u64_t) TSS_MAPPING, sizeof(amd64_tss_t) - 1);
#else 
    gdt.tssdsc.set_seg((u64_t) &tss, sizeof(amd64_tss_t) - 1);
#endif    

    /* Load descriptor registers */
    amd64_descreg_t::setdescreg(amd64_descreg_t::gdtr, (u64_t) &gdt, sizeof(gdt));
    amd64_descreg_t::setdescreg(amd64_descreg_t::tr, AMD64_TSS);


    /*
     * As reloading fs/gs clobbers the upper 32bit of the segment descriptor 
     * registers, we have to set them twice:
     * - before loading the segment selectors (otherwise #GP because of invalid segment)
     * - after reloading the segment selectors  (otherwise upper 32 bits = 0)
     * registers 
     */ 
	
    gdt.segdsc[GDT_IDX(X86_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * AMD64_CACHE_LINE_SIZE),
				           amd64_segdesc_t::data,
				           3,
				           amd64_segdesc_t::m_long,
				           amd64_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(X86_TBS)].set_seg((u64_t) tracebuffer,
					 amd64_segdesc_t::data,
					 3,
					 amd64_segdesc_t::m_long,
					 amd64_segdesc_t::msr_fs);
#endif



    /* Load segment registers */
    asm("mov  %0, %%ds		\n\t"		// load data  segment (DS)
	"mov  %0, %%es		\n\t"		// load extra segment (ES)
	"mov  %0, %%ss		\n\t"		// load stack segment (SS)
	"mov  %1, %%gs		\n\t"		// load UTCB segment  (GS)
#ifdef CONFIG_TRACEBUFFER       
	"mov  %2, %%fs		\n\t"		// tracebuffer segment (FS)
#else
	"mov  %0, %%fs		\n\t"	        // no tracebuffer
#endif  
 	"pushq  %3	      	\n\t"		// new CS
 	"pushq $1f		\n\t"		// new IP		
 	"lretq			\n\t"
 	"1:			\n\t"	
	: /* No Output */ : "r" (0), "r" (X86_UTCBS), "r" (X86_TBS), "r" ((u64_t) X86_KCS)
	);
    
    
    gdt.segdsc[GDT_IDX(X86_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * AMD64_CACHE_LINE_SIZE),
					   amd64_segdesc_t::data,
					   3,
					   amd64_segdesc_t::m_long,
				           amd64_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(X86_TBS)].set_seg((u64_t)tracebuffer,
					 amd64_segdesc_t::data,
					 3,
					 amd64_segdesc_t::m_long,
					 amd64_segdesc_t::msr_fs);
#endif

}

/**
 * setup_msrs: initializes all model specific registers for CPU
 */
void setup_msrs (void)
{
    
    /* sysret (63..48) / syscall (47..32)  CS/SS MSR */
    x86_wrmsr(AMD64_STAR_MSR, ((AMD64_SYSRETCS << 48) | (AMD64_SYSCALLCS << 32)));
    
    /* long mode syscalls MSR */
    x86_wrmsr(AMD64_LSTAR_MSR, (u64_t)(syscall_entry));

    /* compatibility mode syscalls MSR */
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
#if defined(CONFIG_CPU_AMD64_EM64T)
    x86_wrmsr(X86_SYSENTER_CS_MSR, AMD64_SYSCALLCS);
    x86_wrmsr(X86_SYSENTER_EIP_MSR, (u64_t)(sysenter_entry_32));
#if defined(CONFIG_IO_FLEXPAGES)
    x86_wrmsr(X86_SYSENTER_ESP_MSR, (u64_t)(TSS_MAPPING) + 4);
#else
    x86_wrmsr(X86_SYSENTER_ESP_MSR, (u64_t)(&tss) + 4);
#endif
#else /* !defined(CONFIG_CPU_AMD64_EM64T) */
    x86_wrmsr(AMD64_CSTAR_MSR, (u64_t)(syscall_entry_32));
#endif /* !defined(CONFIG_CPU_AMD64_EM64T) */
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */

    /* long mode syscall RFLAGS MASK  */
    x86_wrmsr(AMD64_SFMASK_MSR, (u64_t)(AMD64_SYSCALL_FLAGMASK));

    /* enable syscall/sysret in EFER */
    word_t efer = x86_rdmsr(AMD64_EFER_MSR);
    efer |= AMD64_EFER_SCE;
    x86_wrmsr(AMD64_EFER_MSR, efer);
    
}

