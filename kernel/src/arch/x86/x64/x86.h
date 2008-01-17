/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/x86.h
 * Description:   X86-64 CPU Specific constants
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
 * $Id: amd64.h,v 1.6 2006/09/28 08:03:20 stoess Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X64__X86_H__
#define __ARCH__X86__X64__X86_H__

#include INC_ARCH(x86.h)


/**********************************************************************
 *    MMU
 **********************************************************************/

/* Sign extend 63..48 */
#define X86_X64_SIGN_EXTEND_BITS	48
#define X86_X64_SIGN_EXTEND_SIZE	(X86_64BIT_ONE << X86_X64_SIGN_EXTEND_BITS)
#define X86_X64_SIGN_EXTEND_MASK	(~(X86_X64_SIGN_EXTEND_SIZE - 1))
#define X86_X64_SIGN_EXTENSION		(~(X86_X64_SIGN_EXTEND_SIZE - 1))

/* Page map 47.. 39 */
#define X86_X64_PML4_BITS		39
#define X86_X64_PML4_SIZE		(X86_64BIT_ONE << X86_X64_PML4_BITS)
#define X86_X64_PML4_MASK		((~(X86_X64_PML4_SIZE - 1)) ^ (~(X86_X64_SIGN_EXTEND_SIZE - 1)))
#define X86_X64_PML4_IDX(x)		((x & X86_X64_PML4_MASK) >> X86_X64_PML4_BITS)

/* Page directory pointer 38..30 */
#define X86_X64_PDP_BITS		30
#define X86_X64_PDP_SIZE		(X86_64BIT_ONE << X86_X64_PDP_BITS)
#define X86_X64_PDP_MASK		((~(X86_X64_PDP_SIZE - 1))  ^ (~(X86_X64_PML4_SIZE - 1)))
#define X86_X64_PDP_IDX(x)		((x & X86_X64_PDP_MASK) >> X86_X64_PDP_BITS)

/* Page directory 29..21 */
#define X86_X64_PDIR_BITS		21
#define X86_X64_PDIR_SIZE		(X86_64BIT_ONE << X86_X64_PDIR_BITS)
#define X86_X64_PDIR_MASK		((~(X86_X64_PDIR_SIZE - 1))  ^ (~(X86_X64_PDP_SIZE - 1)))
#define X86_X64_PDIR_IDX(x)		((x & X86_X64_PDIR_MASK) >> X86_X64_PDIR_BITS)

/* Pagetable 20..12  */
#define X86_X64_PTAB_BITS		12
#define X86_X64_PTAB_SIZE		(X86_64BIT_ONE << X86_X64_PTAB_BITS)
#define X86_X64_PTAB_MASK		((~(X86_X64_PTAB_SIZE - 1))  ^ (~(X86_X64_PTAB_SIZE - 1)))
#define X86_X64_PTAB_IDX(x)		((x & X86_X64_PTAB_MASK) >> X86_X64_PTAB_BITS)


#define X86_PAGE_CPULOCAL		(1<<9)
#define X86_PAGE_NX			(1<<63)

/**
 * 
 * Bits to zero out invalid parts of pagetable entries 
 */

/* Normal pagetable entry 11..0  */
#define X86_X64_PTE_BITS			12
#define X86_X64_PTE_SIZE			(X86_64BIT_ONE << X86_X64_PTE_BITS) 
#define X86_X64_PTE_MASK			(~(X86_X64_PTE_SIZE - 1))
#define X86_X64_PTE_FLAGS_MASK		(0x0e3f)

/* 2 MByte (Super-) Pages 20..0  */
#define X86_SUPERPAGE_BITS		21
#define X86_SUPERPAGE_SIZE		(__UL(1) << X86_SUPERPAGE_BITS) 
#define X86_SUPERPAGE_MASK		(~(X86_SUPERPAGE_SIZE - 1))
#define X86_SUPERPAGE_FLAGS_MASK	(0x1fff)

#define X86_PAGE_FLAGS_MASK		(0x0fff)

#define X86_TOP_PDIR_BITS		X86_X64_PML4_BITS
#define X86_TOP_PDIR_SIZE		X86_X64_PML4_SIZE
#define X86_TOP_PDIR_IDX(x)		X86_X64_PML4_IDX(x)

#define X86_PAGEFAULT_BITS		(X86_PFAULT_RW | X86_PFAULT_ID)

/**********************************************************************
 * Model specific register locations.
 **********************************************************************/

#define X86_X64_MSR_MCG_CAP			0x0179  /* Machine Check Global Capabilities */
#define X86_X64_MSR_MCG_STATUS			0x0179  /* Machine Check Global Status  */
#define X86_X64_MSR_MCG_CTL			0x0179  /* Machine Check Global Control  */

#define X86_X64_MSR_MC0_MISC			0x0403  /* Machine Check Error Information */
#define X86_X64_MSR_MC1_MISC			0x0407  /* Machine Check Error Information */
#define X86_X64_MSR_MC2_MISC			0x040b  /* Machine Check Error Information */
#define X86_X64_MSR_MC3_MISC			0x040f  /* Machine Check Error Information */

#define X86_X64_MSR_STAR			0xC0000081      /* SYSCALL/RET CS,
							 * SYSCALL EIP (legacy) */
#define X86_X64_MSR_LSTAR			0xC0000082      /* SYSCALL RIP (long) */
#define X86_X64_MSR_CSTAR			0xC0000083      /* SYSCALL RIP (comp) */
#define X86_X64_MSR_SFMASK			0xC0000084      /* SYSCALL flag mask */


#define X86_X64_MSR_FS				0xC0000100      /* FS Register */
#define X86_X64_MSR_GS				0xC0000101      /* GS Register */
#define X86_X64_MSR_KRNL_GS			0xC0000102      /* Kernel GS Swap  */


/**********************************************************************
 *   Cache line configurations
 **********************************************************************/

#define X86_X64_CACHE_LINE_SIZE          64


#endif /* !__ARCH__X86__X64__X86_H__ */
