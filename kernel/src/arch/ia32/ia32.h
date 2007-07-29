/*********************************************************************
 *                
 * Copyright (C) 2003-2007,  Karlsruhe University
 *                
 * File path:     arch/ia32/ia32.h
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
#ifndef __ARCH__IA32__IA32_H__
#define __ARCH__IA32__IA32_H__

#include INC_ARCHX(x86,x86.h)

/**********************************************************************
 *    MMU
 **********************************************************************/

#define IA32_PAGEDIR_BITS	22
#define IA32_PAGEDIR_SIZE	(__UL(1) << IA32_PAGEDIR_BITS)
#define IA32_PAGEDIR_MASK	(~(IA32_PAGEDIR_SIZE - 1))

#define IA32_SUPERPAGE_BITS	22
#define IA32_SUPERPAGE_SIZE	(__UL(1) << IA32_SUPERPAGE_BITS)
#define IA32_SUPERPAGE_MASK	(~(IA32_SUPERPAGE_SIZE - 1))

#define IA32_PAGE_BITS		12
#define IA32_PAGE_SIZE		(__UL(1) << IA32_PAGE_BITS)
#define IA32_PAGE_MASK		(~(IA32_PAGE_SIZE - 1))

#define IA32_PTAB_BYTES		4096

#define IA32_PAGE_VALID		(1<<0)
#define IA32_PAGE_WRITABLE	(1<<1)
#define IA32_PAGE_USER		(1<<2)
#define IA32_PAGE_KERNEL	(0<<2)
#define IA32_PAGE_WRITE_THROUGH	(1<<3)
#define IA32_PAGE_CACHE_DISABLE	(1<<4)
#define IA32_PAGE_ACCESSED	(1<<5)
#define IA32_PAGE_DIRTY		(1<<6)
#define IA32_PAGE_SUPER		(1<<7)
#define IA32_PAGE_GLOBAL	(1<<8)
#define IA32_PAGE_FLAGS_MASK	(0x01ff)
#define IA32_SPAGE_FLAGS_MASK	(0x11ff)
#define IA32_PTAB_FLAGS_MASK	(0x0007)



/**********************************************************************
 *    CPU features (CPUID)  
 **********************************************************************/

#define IA32_FEAT_FPU	(1 << 0)
#define IA32_FEAT_VME	(1 << 1)
#define IA32_FEAT_DE	(1 << 2)
#define IA32_FEAT_PSE	(1 << 3)
#define IA32_FEAT_TSC	(1 << 4)
#define IA32_FEAT_MSR	(1 << 5)
#define IA32_FEAT_PAE	(1 << 6)
#define IA32_FEAT_MCE	(1 << 7)
#define IA32_FEAT_CXS	(1 << 8)
#define IA32_FEAT_APIC	(1 << 9)
#define IA32_FEAT_SEP	(1 << 11)
#define IA32_FEAT_MTRR	(1 << 12)
#define IA32_FEAT_PGE	(1 << 13)
#define IA32_FEAT_MCA	(1 << 14)
#define IA32_FEAT_CMOV	(1 << 15)
#define IA32_FEAT_FGPAT	(1 << 16)
#define IA32_FEAT_PSE36	(1 << 17)
#define IA32_FEAT_PSN	(1 << 18)
#define IA32_FEAT_CLFLSH	(1 << 19)
#define IA32_FEAT_DS	(1 << 21)
#define IA32_FEAT_ACPI	(1 << 22)
#define IA32_FEAT_MMX	(1 << 23)
#define IA32_FEAT_FXSR	(1 << 24)
#define IA32_FEAT_XMM	(1 << 25)



/**********************************************************************
 * Model specific register locations.
 **********************************************************************/

#define IA32_DEBUGCTL			0x1d9

#if defined(CONFIG_CPU_IA32_I686)
# define IA32_LASTBRANCHFROMIP		0x1db
# define IA32_LASTBRANCHTOIP		0x1dc
# define IA32_LASTINTFROMIP		0x1dd
# define IA32_LASTINTTOIP		0x1de
# define IA32_MTRRBASE(x)		(0x200 + 2*(x) + 0)
# define IA32_MTRRMASK(x)		(0x200 + 2*(x) + 1)
#endif /* CONFIG_CPU_IA32_I686 */

#if defined(CONFIG_CPU_IA32_P4)
# define IA32_MISC_ENABLE		0x1a0
# define IA32_COUNTER_BASE		0x300
# define IA32_CCCR_BASE			0x360
# define IA32_TC_PRECISE_EVENT		0x3f0
# define IA32_PEBS_ENABLE		0x3f1
# define IA32_PEBS_MATRIX_VERT		0x3f2
# define IA32_DS_AREA			0x600
# define IA32_LER_FROM_LIP		0x1d7
# define IA32_LER_TO_LIP		0x1d8
# define IA32_LASTBRANCH_TOS		0x1da
# define IA32_LASTBRANCH_0		0x1db
# define IA32_LASTBRANCH_1		0x1dc
# define IA32_LASTBRANCH_2		0x1dd
# define IA32_LASTBRANCH_3		0x1de

/* Processor features in the MISC_ENABLE MSR. */
# define IA32_ENABLE_FAST_STRINGS	(1 << 0)
# define IA32_ENABLE_X87_FPU		(1 << 2)
# define IA32_ENABLE_THERMAL_MONITOR	(1 << 3)
# define IA32_ENABLE_SPLIT_LOCK_DISABLE	(1 << 4)
# define IA32_ENABLE_PERFMON		(1 << 7)
# define IA32_ENABLE_BRANCH_TRACE	(1 << 11)
# define IA32_ENABLE_PEBS		(1 << 12)

/* Preceise Event-Based Sampling (PEBS) support. */
# define IA32_PEBS_REPLAY_TAG_MASK	((1 << 12)-1)
# define IA32_PEBS_UOP_TAG		(1 << 24)
# define IA32_PEBS_ENABLE_PEBS		(1 << 25)
#endif /* CONFIG_CPU_IA32_P4 */

/* Page Attribute Table (PAT) */
#if defined(CONFIG_IA32_PAT)
# define IA32_CR_PAT_MSR		0x277
# define IA32_PAT_UC			0x00
# define IA32_PAT_WC			0x01
# define IA32_PAT_WT			0x04
# define IA32_PAT_WP			0x05
# define IA32_PAT_WB			0x06
# define IA32_PAT_UM			0x07
#endif

/**********************************************************************
 *   Cache line configurations
 **********************************************************************/

/* 486 */
#if defined(CONFIG_CPU_IA32_I486)
# define IA32_CACHE_LINE_SIZE_L1        16
# define IA32_CACHE_LINE_SIZE_L2        16

/* P1, PII, PIII */
#elif defined(CONFIG_CPU_IA32_I586) || defined(CONFIG_CPU_IA32_I686) || defined(CONFIG_CPU_IA32_C3)
# define IA32_CACHE_LINE_SIZE_L1	32
# define IA32_CACHE_LINE_SIZE_L2	32

/* P4, Xeon */
#elif defined(CONFIG_CPU_IA32_P4)
# define IA32_CACHE_LINE_SIZE_L1	32
# define IA32_CACHE_LINE_SIZE_L2	64

/* AMD K8 (Opteron) */
#elif defined(CONFIG_CPU_IA32_K8)
# define IA32_CACHE_LINE_SIZE_L1	64
# define IA32_CACHE_LINE_SIZE_L2	64

#else
# error unknown architecture - specify cache line size
#endif



#endif /* !__ARCH__IA32__IA32_H__ */
