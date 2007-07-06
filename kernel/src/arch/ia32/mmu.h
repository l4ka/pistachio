/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     arch/ia32/mmu.h
 * Description: Methods for managing the IA-32 MMU
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
 * $Id: mmu.h,v 1.8 2003/09/24 19:04:27 skoglund Exp $
 *
 ********************************************************************/
#ifndef __ARCH_IA32_MMU_H__
#define __ARCH_IA32_MMU_H__

#include INC_ARCH(cpu.h)

class ia32_mmu_t 
{
public:
    static void flush_tlb(bool global = false);
    static void flush_tlbent(u32_t addr);
    static void enable_super_pages();
    static void enable_global_pages();
    static void enable_paged_mode();
    static u32_t get_pagefault_address(void);
    static u32_t get_active_pagetable(void);
    static void set_active_pagetable(u32_t pdir);
};    

/**
 * Flushes the tlb
 * 
 * @param global	specifies whether global TLB entries are also flushed
 */
INLINE void ia32_mmu_t::flush_tlb(bool global)
{
    u32_t dummy, dummy2;
    if (!global)
	__asm__ __volatile__("movl %%cr3, %0\n"
			     "movl %0, %%cr3\n"
			     : "=r" (dummy));
    else
	__asm__ __volatile__("movl	%%cr4, %0	\n"
			     "andl	%2, %0		\n"
			     "movl	%0, %%cr4	\n"
			     "movl	%%cr3, %1	\n"
			     "movl	%1, %%cr3	\n"
			     "orl	%3, %0		\n"
			     "movl	%0, %%cr4	\n"
			     : "=r"(dummy), "=r"(dummy2)
			     : "i" (~IA32_CR4_PGE), "i" (IA32_CR4_PGE));
}

/**
 * Flushes the TLB entry for a linear address
 * 
 * @param addr	linear address
 */
INLINE void ia32_mmu_t::flush_tlbent(u32_t addr)
{
    __asm__ __volatile__ ("invlpg (%0)	\n"
			  :
			  :"r" (addr));
}


/**
 * Enables extended page size (4M) support for IA32
 */
INLINE void ia32_mmu_t::enable_super_pages()
{
    ia32_cr4_set(IA32_CR4_PSE);
}

/**
 * Enables global page support for IA32
 */
INLINE void ia32_mmu_t::enable_global_pages()
{
    ia32_cr4_set(IA32_CR4_PGE);
}

/**
 * Enables paged mode for IA32
 */
INLINE void ia32_mmu_t::enable_paged_mode()
{
    asm("mov %0, %%cr0\n"
	"nop;nop;nop\n"
	:
	: "r"(IA32_CR0_PG | IA32_CR0_WP | IA32_CR0_PE)
	);
}

/**
 * @returns the linear address of the last pagefault
 */
INLINE u32_t ia32_mmu_t::get_pagefault_address(void)
{
    register u32_t tmp;

    asm ("movl	%%cr2, %0\n"
	 :"=r" (tmp));

    return tmp;
}

/**
 * Get the active page table
 * 
 * @returns the physical base address of the currently active page directory
 */
INLINE u32_t ia32_mmu_t::get_active_pagetable(void)
{
    u32_t tmp;

    asm volatile ("movl %%cr3, %0	\n"
		  :"=a" (tmp));

    return tmp;
}

/**
 * Sets the active page table
 * 
 * @param pdir	page directory base address (physical)
 */
INLINE void ia32_mmu_t::set_active_pagetable(u32_t pdir)
{
    asm volatile ("mov %0, %%cr3\n"
		  :
		  : "r"(pdir));
}

#endif /* __ARCH_IA32_MMU_H__ */

