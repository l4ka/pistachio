/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/amd64.h
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
#ifndef __ARCH__AMD64__AMD64_H__
#define __ARCH__AMD64__AMD64_H__

/* RFLAGS register bits */
#define AMD64_RFL_CF    (1 <<  0)       /* carry flag                   */
#define AMD64_RFL_PF    (1 <<  2)       /* parity flag                  */
#define AMD64_RFL_AF    (1 <<  4)       /* auxiliary carry flag         */
#define AMD64_RFL_ZF    (1 <<  6)       /* zero flag                    */
#define AMD64_RFL_SF    (1 <<  7)       /* sign flag                    */
#define AMD64_RFL_TF    (1 <<  8)       /* trap flag                    */
#define AMD64_RFL_IF    (1 <<  9)       /* interrupt enable flag        */
#define AMD64_RFL_DF    (1 << 10)       /* direction flag               */
#define AMD64_RFL_OF    (1 << 11)       /* overflow flag                */
#define AMD64_RFL_NT    (1 << 14)       /* nested task flag             */
#define AMD64_RFL_RF    (1 << 16)       /* resume flag                  */
#define AMD64_RFL_VM    (1 << 17)       /* virtual 8086 mode            */
#define AMD64_RFL_AC    (1 << 18)       /* alignement check             */
#define AMD64_RFL_VIF   (1 << 19)       /* virtual interrupt flag       */
#define AMD64_RFL_VIP   (1 << 20)       /* virtual interrupt pending    */
#define AMD64_RFL_ID    (1 << 21)       /* CPUID flag                   */
#define AMD64_RFL_IOPL(x)       ((x & 3) << 12) /* the IO privilege level field */

/* exception error code bits */
#define AMD64_PF_RW	(1 << 1)	/* Pagefault on read/write	*/
#define AMD64_PF_US	(1 << 2)	/* Pagefault in user/kernel	*/
#define AMD64_PF_ID	(1 << 4)	/* Pagefault on insn./data	*/

/* control register bits */
#define AMD64_CR0_PE    (1 <<  0)       /* enable protected mode        */
#define AMD64_CR0_MP    (1 <<  2)       /* disable emulation            */
#define AMD64_CR0_EM    (1 <<  2)       /* enable emulation (by #UD)    */
#define AMD64_CR0_TS    (1 <<  3)       /* task switched                */
#define AMD64_CR0_WP    (1 << 16)       /* force write protection on user
                                           read only pages for kernel   */
#define AMD64_CR0_AM    (1 << 18)       /* alignment mask               */
#define AMD64_CR0_NW    (1 << 29)       /* not write through            */
#define AMD64_CR0_CD    (1 << 30)       /* cache disabled               */
#define AMD64_CR0_PG    (1 << 31)       /* enable paging                */

#define AMD64_CR3_PWT   (1 <<  3)       /* page-level writes transparent*/
#define AMD64_CR3_PCD   (1 <<  4)       /* page-level cache disable     */

#define AMD64_CR4_VME   (1 <<  0)       /* virtual 8086 mode extension  */
#define AMD64_CR4_PVI   (1 <<  1)       /* enable protected mode
                                           virtual interrupts           */
#define AMD64_CR4_TSD   (1 <<  2)       /* time stamp disable           */
#define AMD64_CR4_DE    (1 <<  3)       /* debug extensions             */
#define AMD64_CR4_PSE   (1 <<  4)       /* page size extension (4MB)    */
#define AMD64_CR4_PAE   (1 <<  5)       /* physical address extension   */
#define AMD64_CR4_MCE   (1 <<  6)       /* machine check extensions     */
#define AMD64_CR4_PGE   (1 <<  7)       /* enable global pages          */
#define AMD64_CR4_PCE   (1 <<  8)       /* allow user to use rdpmc      */
#define AMD64_CR4_OSFXSR (1 <<  9)      /* enable fxsave/fxrstor + sse  */
#define AMD64_CR4_OSXMMEXCPT (1 << 10)  /* support for unmsk. SIMD exc. */

/* extended feature register (EFER) bits */
#define AMD64_EFER_SCE  (1 <<  0)       /* system call extensions       */
#define AMD64_EFER_LME  (1 <<  8)       /* long mode enabled            */
#define AMD64_EFER_LMA  (1 << 10)       /* long mode active             */
#define AMD64_EFER_NXE  (1 << 11)       /* nx bit enable                */

/*
 * Model specific register locations.
 */
 
#define AMD64_TSC_MSR                   0x0010  /* Time Stamp Counter */
 
#define AMD64_SYSENTER_CS_MSR           0x0174  /* Sysenter Code Segment */
#define AMD64_SYSENTER_ESP_MSR          0x0175  /* Sysenter Stack Pointer */
#define AMD64_SYSENTER_EIP_MSR          0x0176  /* Sysenter Instruction Pointer */
 
#define AMD64_MCG_CAP_MSR               0x0179  /* Machine Check Global Capabilities */
#define AMD64_MCG_STATUS_MSR            0x0179  /* Machine Check Global Status  */
#define AMD64_MCG_CTL_MSR               0x0179  /* Machine Check Global Control  */

#define AMD64_DEBUGCTL_MSR              0x01d9  /* Debug-Control  */

#define AMD64_MC0_MISC_MSR              0x0403  /* Machine Check Error Information */
#define AMD64_MC1_MISC_MSR              0x0407  /* Machine Check Error Information */
#define AMD64_MC2_MISC_MSR              0x040b  /* Machine Check Error Information */
#define AMD64_MC3_MISC_MSR              0x040f  /* Machine Check Error Information */


#define AMD64_EFER_MSR                  0xC0000080      /* Extended Features */
#define AMD64_STAR_MSR                  0xC0000081      /* SYSCALL/RET CS,
							 * SYSCALL EIP (legacy) */
#define AMD64_LSTAR_MSR                 0xC0000082      /* SYSCALL RIP (long) */
#define AMD64_CSTAR_MSR                 0xC0000083      /* SYSCALL RIP (comp) */
#define AMD64_SFMASK_MSR                0xC0000084      /* SYSCALL flag mask */

#define AMD64_FS_MSR                    0xC0000100      /* FS Register */
#define AMD64_GS_MSR                    0xC0000101      /* GS Register */
#define AMD64_KRNL_GS_MSR               0xC0000102      /* Kernel GS Swap  */

#define AMD64_HWCR_MSR		        0xC0010015      /* HW configuration MSR */

#define KERNEL_VERSION_VER              KERNEL_VERSION_CPU_AMD64

#define AMD64_PERFCTR0                  0x0c1
#define AMD64_PERFCTR1                  0x0c2
#define AMD64_EVENTSEL0                 0x186
#define AMD64_EVENTSEL1                 0x187
#define AMD64_LASTBRANCHFROMIP          0x1db
#define AMD64_LASTBRANCHTOIP            0x1dc
#define AMD64_LASTINTFROMIP             0x1dd
#define AMD64_LASTINTTOIP               0x1de
#define AMD64_MTRRBASE(x)               (0x200 + 2*(x) + 0)
#define AMD64_MTRRMASK(x)               (0x200 + 2*(x) + 1)

/* if cpuid does not provide it, we use this value */
#define AMD64_CACHE_LINE_SIZE          64


#endif /* !__ARCH__AMD64__AMD64_H__ */


