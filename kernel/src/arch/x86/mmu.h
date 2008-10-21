/*********************************************************************
 *                
 * Copyright (C) 2003-2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/mmu.h
 * Description:   X86 specific MMU Stuff
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __ARCH__X86__MMU_H__
#define __ARCH__X86__MMU_H__

#include INC_ARCH(cpu.h)

class x86_mmu_t
{
public:
    static void flush_tlb(bool global = false);
    static void flush_tlbent(word_t addr);
    static void enable_paging();
    static void disable_paging();
    static void enable_pae_mode();
#if defined(CONFIG_IS_64BIT)
    static bool has_long_mode();
    static void enable_long_mode();
    static bool long_mode_active();
#endif
    static void enable_global_pages();
    static void enable_super_pages();

    static word_t get_active_pagetable(void);
    static void set_active_pagetable(word_t addr);
    static word_t get_pagefault_address(void);
};

/**
 * Flushes the tlb
 *
 * @param global        specifies whether global TLB entries are also flushed
 */
INLINE void x86_mmu_t::flush_tlb(bool global)
{
    word_t dummy1, dummy2;

#if defined(CONFIG_X86_PGE)
    if (!global)
    {
        __asm__ __volatile__(
                "mov    %%cr3, %0   \n\t"
                "mov    %0, %%cr3   \n\t"
                : "=r" (dummy1));
    }
    else
    {
        __asm__ __volatile__(
                "mov    %%cr4, %0       \n"
                "and    %2, %0          \n"
                "mov    %0, %%cr4       \n"
                "mov    %%cr3, %1       \n"
                "mov    %1, %%cr3       \n"
                "or     %3, %0          \n"
                "mov    %0, %%cr4       \n"
                : "=r"(dummy1), "=r"(dummy2)
                : "i" (~X86_CR4_PGE), "i" (X86_CR4_PGE));
    }
#else
    __asm__ __volatile__(
            "mov    %%cr3, %0   \n\t"
            "mov    %0, %%cr3   \n\t"
            : "=r" (dummy1));
#endif
}


/**
  * Flushes the TLB entry for a linear address
  *
  * @param addr  linear address
  */    
INLINE void x86_mmu_t::flush_tlbent(word_t addr)
{
    __asm__ __volatile__ (
            "invlpg (%0)  \n"
            :
            :"r" (addr));
}


/**
 * Enables paged mode for X86
 */
INLINE void x86_mmu_t::enable_paging()
{
    x86_cr0_set(X86_CR0_PG | X86_CR0_WP | X86_CR0_PE);
    __asm__ __volatile__ ("jmp penabled; penabled:");
}


/**
 * Disable paged mode for X86
 */
INLINE void x86_mmu_t::disable_paging()
{
    x86_cr0_mask(X86_CR0_PG);
}


/**
 * Enables physical address extensions
 * Needed for long and compatibility mode 
 */
INLINE void x86_mmu_t::enable_pae_mode()
{
    x86_cr4_set(X86_CR4_PAE);
}    


#if defined(CONFIG_IS_64BIT)
/**
 * Checks if CPU has long mode
 *       
 */
INLINE bool x86_mmu_t::has_long_mode()
{
    if (!(x86_x64_has_cpuid()))
        return false;
    
    u32_t features, lfn, dummy;
    
    x86_cpuid(CPUID_MAX_EXT_FN_NR, &lfn, &dummy, &dummy, &dummy);
    
    if (lfn < CPUID_AMD_FEATURES) 
        return false;
    
    x86_cpuid(CPUID_AMD_FEATURES, &dummy, &dummy, &dummy, &features);
    
    return (features & CPUID_AMD_HAS_LONGMODE);
}


/**
 * Enables long mode
 * Note: This doesn't mean that long mode is actually activated,
 *       physical address extensions have to be enabled, too
 *       
 */
INLINE void x86_mmu_t::enable_long_mode()
{
    word_t efer = x86_rdmsr(X86_MSR_EFER);
    efer |= X86_MSR_EFER_LME;
    x86_wrmsr(X86_MSR_EFER, efer);
}


/**
 * Checks if long mode is active
 *       
 */
INLINE bool x86_mmu_t::long_mode_active()
{
    word_t efer = x86_rdmsr(X86_MSR_EFER);
    return (efer & X86_MSR_EFER_LMA);
}
#endif /* defined(CONFIG_IS_64BIT) */


/**
 * Enables global pages
 */
INLINE void x86_mmu_t::enable_global_pages()
{
      x86_cr4_set(X86_CR4_PGE);
}


/**
 * Enable super pages
 */
INLINE void x86_mmu_t::enable_super_pages(void)
{
    x86_cr4_set(X86_CR4_PSE);
}


/**
 * Get active page map
 * Note: We can safely call this function in 32bit code, as 
 * the instruction are encoded identically   
 */
INLINE word_t x86_mmu_t::get_active_pagetable(void)
{
    word_t pgm;
    
    __asm__ __volatile__ ("mov %%cr3, %0\n" :"=a" (pgm));
    
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
INLINE void x86_mmu_t::set_active_pagetable(word_t root)
{
   __asm__ __volatile__ ("mov %0, %%cr3 \n"
                        :
                        : "r"(root));
}  


/**
 * Get pagefault address
 */
INLINE word_t x86_mmu_t::get_pagefault_address(void)
{
    register word_t tmp;

    __asm__ ("mov   %%cr2, %0   \n"
            :"=r" (tmp));
    
    return tmp;
}

#endif /* !__ARCH__X86__MMU_H__ */
