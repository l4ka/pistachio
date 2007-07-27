/*********************************************************************
 *                
 * Copyright (C) 2003-2005, 2007,  Karlsruhe University
 *                
 * File path:     arch/amd64/mmu.h
 * Description:   X86-64 specific MMU Stuff
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
 * $Id: mmu.h,v 1.4 2005/04/21 15:57:00 stoess Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__MMU_H__
#define __ARCH__AMD64__MMU_H__

#include INC_ARCH(cpu.h)
#include INC_ARCH(cpuid.h)

class amd64_mmu_t{
public:
    static void flush_tlb(bool global = false);
    static void flush_tlbent(word_t addr);
    static void enable_paging();
    static void disable_paging();
    static void enable_pae_mode();
    static bool has_long_mode();
    static void enable_long_mode();
    static bool long_mode_active();
    static void enable_global_pages();
    static word_t get_active_pml4(void);
    static void set_active_pml4(word_t addr);
    static word_t get_pagefault_address(void);
};

/**
 * Flushes the tlb
 *
 * @param global        specifies whether global TLB entries are also flushed
 */
INLINE void amd64_mmu_t::flush_tlb(bool global){
    
    word_t dummy1, dummy2;
    
    if (!global)
	__asm__ __volatile__("movq %%cr3, %0\n\t"
			     "movq %0, %%cr3\n\t"
			     : "=r" (dummy1));
    else
	__asm__ __volatile__("movq      %%cr4, %0       \n"
			     "andq      %2, %0          \n"
			     "movq      %0, %%cr4       \n"
			     "movq      %%cr3, %1       \n"
			     "movq      %1, %%cr3       \n"
			     "orq       %3, %0          \n"
			     "movq      %0, %%cr4       \n"
			     : "=r"(dummy1), "=r"(dummy2)
			     : "i" (~X86_CR4_PGE), "i" (X86_CR4_PGE));
}
/**
  * Flushes the TLB entry for a linear address
  *
  * @param addr  linear address
  */	
INLINE void amd64_mmu_t::flush_tlbent(word_t addr){
    __asm__ __volatile__ ("invlpg (%0)  \n"
			  :
			  :"r" (addr));
}

/**
 * Enables paged mode for X86_64
 */
INLINE void amd64_mmu_t::enable_paging(){
    amd64_cr0_set(X86_CR0_PG | X86_CR0_WP | X86_CR0_PE);
    asm("jmp penabled; penabled:");
}

/**
 * Disable paged mode for X86_64
 */
INLINE void amd64_mmu_t::disable_paging(){
    amd64_cr0_mask(X86_CR0_PG);
}

/**
 * Enables physical address extensions
 * Needed for long and compatibility mode 
 */
INLINE void amd64_mmu_t::enable_pae_mode(){
    amd64_cr4_set(X86_CR4_PAE);
}    

/**
 * Checks if CPU has long mode
 *       
 */

INLINE bool amd64_mmu_t::has_long_mode(){
    
    if (!(amd64_cpu_features_t::has_cpuid()))
	return false;
    
    u32_t features, lfn, dummy;
    
    amd64_cpu_features_t::cpuid(CPUID_MAX_EXT_FN_NR, &lfn, &dummy, &dummy, &dummy);
    
    if (lfn < CPUID_AMD_FEATURES) 
	return false;
    
    amd64_cpu_features_t::cpuid(CPUID_AMD_FEATURES, &dummy, &dummy, &dummy, &features);
    
    return (features & CPUID_AMD_HAS_LONGMODE);
    
}

    

/**
 * Enables long mode
 * Note: This doesn't mean that long mode is actually activated,
 *       'cause physical address extensions have to be enabled, too
 *       
 */
INLINE void amd64_mmu_t::enable_long_mode(){

    word_t efer = amd64_rdmsr(AMD64_EFER_MSR);
    efer |= AMD64_EFER_LME;
    amd64_wrmsr(AMD64_EFER_MSR, efer);
    
}

/**
 * Checks if long mode is active
 *       
 */
INLINE bool amd64_mmu_t::long_mode_active(){

    word_t efer = amd64_rdmsr(AMD64_EFER_MSR);
    return (efer & AMD64_EFER_LMA);
    
}

/**
 * Enables global pages
 */
INLINE void amd64_mmu_t::enable_global_pages(){
      amd64_cr4_set(X86_CR4_PGE);
}

/**
 * Get active page map
 * Note: We can safely call this function in 32bit code, as 
 * the instruction are encoded identically   
 */
INLINE word_t amd64_mmu_t::get_active_pml4(void){
    
    word_t pgm;
    
    asm volatile ("mov %%cr3, %0\n" :"=a" (pgm));
    
    return pgm;
    
}

/**
 * Set active page map
 * Note: We can safely call this function from 32bit code, as 
 * the instructions are encoded identically. But then of course 
 * the page map has to be placed at a location lower than 4 GB
 * as sizeof(word_t) = 32! 
 * 
 */

INLINE void amd64_mmu_t::set_active_pml4(word_t pml4){
   asm volatile ("mov %0, %%cr3\n"
		 :
		 : "r"(pml4));
}  

/**
 * Get pagefault address
 */

INLINE word_t amd64_mmu_t::get_pagefault_address(void) {

    register word_t tmp;
    asm ("movq  %%cr2, %0\n"
	 :"=r" (tmp));
    
    return tmp;
}
#endif /* !__ARCH__AMD64__MMU_H__ */
