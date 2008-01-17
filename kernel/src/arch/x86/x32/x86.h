/*********************************************************************
 *  *                
 * Copyright (C) 2003-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x32/x86.h
 * Description:   IA32 specific constants
 *		  do _not_ put any functions into this file
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
 * $Id: ia32.h,v 1.10 2006/11/16 20:01:58 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X32__X86_X32_H__
#define __ARCH__X86__X32__X86_X32_H__

#include INC_ARCH(x86.h)

/**********************************************************************
 *    MMU
 **********************************************************************/

#define X86_X32_PDIR_BITS		22
#define X86_X32_PDIR_SIZE          (__UL(1) << X86_X32_PDIR_BITS)
#define X86_X32_PDIR_MASK          (~(X86_X32_PDIR_SIZE - 1))
#define X86_X32_PDIR_IDX(x)	((x & X86_X32_PDIR_MASK) >> X86_X32_PDIR_BITS)

/* Pagetable 20..12  */
#define X86_X32_PTAB_BITS          12
#define X86_X32_PTAB_SIZE          (__UL(1) << X86_X32_PTAB_BITS)
#define X86_X32_PTAB_MASK          ((~(X86_X32_PTAB_SIZE - 1))  ^ (~(X86_X32_PTAB_SIZE - 1)))
#define X86_X32_PTAB_IDX(x)        ((x & X86_X32_PTAB_MASK) >> X86_X32_PTAB_BITS)


#define X86_X32_PTAB_FLAGS_MASK	(0x0007)

#define X86_SUPERPAGE_BITS	22
#define X86_SUPERPAGE_SIZE	(__UL(1) << X86_SUPERPAGE_BITS)
#define X86_SUPERPAGE_MASK	(~(X86_SUPERPAGE_SIZE - 1))
#define X86_SUPERPAGE_FLAGS_MASK (0x11ff)

#define X86_PAGE_FLAGS_MASK	 (0x01ff)

#define X86_TOP_PDIR_BITS	X86_X32_PDIR_BITS
#define X86_TOP_PDIR_SIZE	X86_X32_PDIR_SIZE
#define X86_TOP_PDIR_IDX(x)	X86_X32_PDIR_IDX(x)

#define X86_PAGEFAULT_BITS	(X86_PAGE_WRITABLE)

/**********************************************************************
 *    CPU features (CPUID)  
 **********************************************************************/
#define X86_X32_FEAT_FPU	(1 << 0)
#define X86_X32_FEAT_VME	(1 << 1)
#define X86_X32_FEAT_DE	(1 << 2)
#define X86_X32_FEAT_PSE	(1 << 3)
#define X86_X32_FEAT_TSC	(1 << 4)
#define X86_X32_FEAT_MSR	(1 << 5)
#define X86_X32_FEAT_PAE	(1 << 6)
#define X86_X32_FEAT_MCE	(1 << 7)
#define X86_X32_FEAT_CXS	(1 << 8)
#define X86_X32_FEAT_APIC	(1 << 9)
#define X86_X32_FEAT_SEP	(1 << 11)
#define X86_X32_FEAT_MTRR	(1 << 12)
#define X86_X32_FEAT_PGE	(1 << 13)
#define X86_X32_FEAT_MCA	(1 << 14)
#define X86_X32_FEAT_CMOV	(1 << 15)
#define X86_X32_FEAT_FGPAT	(1 << 16)
#define X86_X32_FEAT_PSE36	(1 << 17)
#define X86_X32_FEAT_PSN	(1 << 18)
#define X86_X32_FEAT_CLFLH	(1 << 19)
#define X86_X32_FEAT_DS	(1 << 21)
#define X86_X32_FEAT_ACPI	(1 << 22)
#define X86_X32_FEAT_MMX	(1 << 23)
#define X86_X32_FEAT_FXSR	(1 << 24)
#define X86_X32_FEAT_XMM	(1 << 25)


/**********************************************************************
 *   Cache line configurations
 **********************************************************************/

/* 486 */
#if defined(CONFIG_CPU_X86_I486)
# define X86_X32_CACHE_LINE_SIZE_L1        16
# define X86_X32_CACHE_LINE_SIZE_L2        16

/* P1, PII, PIII */
#elif defined(CONFIG_CPU_X86_I586) || defined(CONFIG_CPU_X86_I686) ||\
      defined(CONFIG_CPU_X86_C3)   || defined(CONFIG_CPU_X86_SIMICS)
# define X86_X32_CACHE_LINE_SIZE_L1	32
# define X86_X32_CACHE_LINE_SIZE_L2	32

/* P4, Xeon */
#elif defined(CONFIG_CPU_X86_P4)
# define X86_X32_CACHE_LINE_SIZE_L1	32
# define X86_X32_CACHE_LINE_SIZE_L2	64

/* AMD K8 (Opteron) */
#elif defined(CONFIG_CPU_X86_K8)
# define X86_X32_CACHE_LINE_SIZE_L1	64
# define X86_X32_CACHE_LINE_SIZE_L2	64


#else
# error unknown architecture - specify cache line size
#endif



#endif /* !__ARCH__X86__X32__X86_X32_H__ */
