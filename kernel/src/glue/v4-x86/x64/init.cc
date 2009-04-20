/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/init.cc
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

#include INC_ARCH(cpu.h)
#include INC_ARCH(amdhwcr.h)
#include INC_ARCH(fpu.h)
#include INC_ARCH(apic.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH_SA(tss.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(idt.h)
#include INC_GLUE(memory.h)

#include INC_PLAT(rtc.h)
#include INC_PLAT(perfmon.h)


#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#include INC_GLUE_SA(x32comp/init.h)
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */


x86_x64_cpu_features_t boot_cpu_ft UNIT("x86.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 1);
x86_tss_t tss UNIT("x86.cpulocal") CTORPRIO(CTORPRIO_GLOBAL, 2);
bool tracebuffer_initialized UNIT("x86.cpulocal");


struct gdt_struct {
    x86_segdesc_t segdsc[GDT_SIZE - 2];	/* 6 entries a  8 byte */
    x86_tssdesc_t tssdsc;		/* 1 entries a 16 byte */
} gdt UNIT("x86.cpulocal");

u8_t x86_x64_cache_line_size;


// from glue/v4-x86/
void clear_bss (void);


/**********************************************************************
 *
 * SMP specific code and data
 *
 **********************************************************************/

#if defined(CONFIG_SMP)
x86_segdesc_t	smp_boot_gdt[3];
void setup_smp_boot_gdt (void)
{
    /* segment descriptors in long mode and legacy mode are almost identical.
     *  However, in long mode, most of the fields are ignored, thus we can set
     *  up those segments although the APs are not yet in long mode when they
     *  are used.
     */
#   define gdt_idx(x) ((x) >> 3)
  smp_boot_gdt[gdt_idx(X86_KCS)].set_seg((u64_t)0, x86_segdesc_t::code, 0, x86_segdesc_t::m_comp);
  smp_boot_gdt[gdt_idx(X86_KDS)].set_seg((u64_t)0, x86_segdesc_t::data, 0, x86_segdesc_t::m_comp);  
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
    x86_x64_cache_line_size = boot_cpu_ft.get_l1_cache().d.dcache.l_size;
}

void SECTION(SEC_INIT) init_meminfo (void)
{

    extern word_t _memory_descriptors_size[];

    if (get_kip()->memory_info.get_num_descriptors() == (word_t) &_memory_descriptors_size)
    {
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


/**
 * Setup global descriptor table 
 * 
 */

void SECTION(SEC_INIT) setup_gdt(x86_tss_t &tss, cpuid_t cpuid)
{

    /* Initialize GDT */
    gdt.segdsc[GDT_IDX(X86_X64_INVS)].set_seg((u64_t) 0, x86_segdesc_t::inv, 0, x86_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KCS)].set_seg((u64_t) 0, x86_segdesc_t::code, 0, x86_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_KDS)].set_seg((u64_t) 0, x86_segdesc_t::data, 0, x86_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_UCS)].set_seg((u64_t) 0, x86_segdesc_t::code, 3, x86_segdesc_t::m_long);
    gdt.segdsc[GDT_IDX(X86_UDS)].set_seg((u64_t) 0, x86_segdesc_t::data, 3, x86_segdesc_t::m_long);
    
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    gdt.segdsc[GDT_IDX(X86_UCS32)].set_seg((u64_t) 0, x86_segdesc_t::code, 3, x86_segdesc_t::m_comp);
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

    /* TODO: Assertion correct ? */
    ASSERT(unsigned(cpuid * X86_X64_CACHE_LINE_SIZE) < X86_SUPERPAGE_SIZE);
    
    /* Set TSS */
    gdt.tssdsc.set_seg((u64_t) &tss, sizeof(x86_x64_tss_t) - 1);

    /* Load descriptor registers */
    x86_descreg_t gdtr((word_t) &gdt, sizeof(gdt));
    gdtr.setdescreg(x86_descreg_t::gdtr);
    x86_descreg_t tr(X86_TSS);
    tr.setselreg(x86_descreg_t::tr);


    /*
     * As reloading fs/gs clobbers the upper 32bit of the segment descriptor 
     * registers, we have to set them twice:
     * - before loading the segment selectors (otherwise #GP because of invalid segment)
     * - after reloading the segment selectors  (otherwise upper 32 bits = 0)
     * registers 
     */ 
	
    gdt.segdsc[GDT_IDX(X86_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * X86_X64_CACHE_LINE_SIZE),
				           x86_segdesc_t::data,
				           3,
				           x86_segdesc_t::m_long,
				           x86_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(X86_TBS)].set_seg((u64_t) get_tracebuffer(),
					 x86_segdesc_t::data,
					 3,
					 x86_segdesc_t::m_long,
					 x86_segdesc_t::msr_fs);
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
	"xor  %%rax, %%rax	\n"		//
	"lldt %%ax		\n"		// clear LDTR
	"pushq  %3	      	\n\t"		// new CS
 	"pushq $1f		\n\t"		// new IP		
 	"lretq			\n\t"
 	"1:			\n\t"	
	: /* No Output */ 
	: "r" (0), "r" (X86_UTCBS), "r" (X86_TBS), "r" ((u64_t) X86_KCS)
	: "rax", "memory"
	);
    
    
    gdt.segdsc[GDT_IDX(X86_UTCBS)].set_seg(UTCB_MAPPING + (cpuid * X86_X64_CACHE_LINE_SIZE),
					   x86_segdesc_t::data,
					   3,
					   x86_segdesc_t::m_long,
				           x86_segdesc_t::msr_gs);
    
#if defined(CONFIG_TRACEBUFFER)
    gdt.segdsc[GDT_IDX(X86_TBS)].set_seg( (u64_t) get_tracebuffer(),
					 x86_segdesc_t::data,
					 3,
					 x86_segdesc_t::m_long,
					 x86_segdesc_t::msr_fs);
#endif

}

/**
 * setup_msrs: initializes all model specific registers for CPU
 */
void setup_msrs (void)
{
#if defined(CONFIG_X86_FXSR)
    x86_fpu_t::enable_osfxsr();
#endif

    /* sysret (63..48) / syscall (47..32)  CS/SS MSR */
    x86_wrmsr(X86_X64_MSR_STAR, ((X86_SYSRETCS << 48) | (X86_SYSCALLCS << 32)));
    
    /* long mode syscalls MSR */
    x86_wrmsr(X86_X64_MSR_LSTAR, (u64_t)(syscall_entry));

    /* compatibility mode syscalls MSR */
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#if defined(CONFIG_X86_EM64T)
    x86_wrmsr(X86_MSR_SYSENTER_CS, X86_SYSCALLCS);
    x86_wrmsr(X86_MSR_SYSENTER_EIP, (u64_t)(sysenter_entry_32));
    x86_wrmsr(X86_MSR_SYSENTER_ESP, (u64_t)(&tss) + 4);
    
#else /* !defined(CONFIG_X86_EM64T) */
    x86_wrmsr(X86_X64_MSR_CSTAR, (u64_t)(syscall_entry_32));
#endif /* !defined(CONFIG_X86_EM64T) */
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

    /* long mode syscall RFLAGS MASK  */
    x86_wrmsr(X86_X64_MSR_SFMASK, (u64_t)(X86_X64_SYSCALL_FLAGMASK));

    /* enable syscall/sysret in EFER */
    word_t efer = x86_rdmsr(X86_MSR_EFER);
    efer |= X86_MSR_EFER_SCE;
    x86_wrmsr(X86_MSR_EFER, efer);
    
}

