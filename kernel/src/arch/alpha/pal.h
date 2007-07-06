/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/pal.h
 * Created:       23/07/2002 18:08:44 by Simon Winwood (sjw)
 * Description:   PAL definitions (for asm and C) 
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
 * $Id: pal.h,v 1.4 2003/09/24 19:04:25 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__PAL_H__
#define __ARCH__ALPHA__PAL_H__

/* Interrupt types */
#define PAL_INT_IPI      0
#define PAL_INT_CLOCK    1
#define PAL_INT_ERR      2
#define PAL_INT_MCHK     2
#define PAL_INT_DEV      3
#define PAL_INT_PERF     4

/* entIF reasons */
#define PAL_IF_BPT       0
#define PAL_IF_BUGCHK    1
#define PAL_IF_GENTRAP   2
#define PAL_IF_FEN       3
#define PAL_IF_OPDEC     4

/* PAL calls, unprivileged */
#define PAL_bpt          128  /* Breakpoint trap: Kernel and user */
#define PAL_bugchk       129  /* Bugcheck trap: Kernel and user */
#define PAL_callsys      131  /* System call: User */
/* #define PAL_clrfen = ?         Clear floating-point enable: User */
#define PAL_gentrap      170  /* Generate trap: Kernel and user */
#define PAL_imb          134  /* I-stream memory barrier: Kernel and user */
#define PAL_rdunique     158  /* Read unique: Kernel and user */
/* #define  PAL_urti = ?           Return from user mode trap: User mode */
#define PAL_wrunique     159  /* Write Unique: Kernel and user */    


/* PAL calls, privileged */
#define PAL_cflush       1  /* Cache flush */ 
#define PAL_cserve       9  /* Console Service */
#define PAL_draina       2  /* Drain aborts */
#define PAL_halt         0  /* Halt the processor */
/* PAL_jtopal = 46 */
/* PAL_nphalt = 190 */
#define PAL_rdmces       16  /* Read machine check error summary register */
#define PAL_rdps         54  /* Read processor status */       
#define PAL_rdusp        58  /* Read user stack pointer */
#define PAL_rdval        50  /* Read system value */         
#define PAL_retsys       61  /* Return from syscall */
#define PAL_rti          63  /* Return from trap, fault, or interrupt */
#define PAL_swpctx       48  /* Swap process context */
#define PAL_swpipl       53  /* Swap IPL */
#define PAL_tbi          51  /* TB invalidate */
#define PAL_whami        60  /* Who am I */
#define PAL_wrent        52  /* Write system entry address */
#define PAL_wrfen        43  /* Write floating point enable */
#define PAL_wripir       13  /* Write interprocessor interrupt request */
#define PAL_wrkgp        55  /* Write kernel global pointer */
#define PAL_wrmces       17  /* Write machine check error summary register */
#define PAL_wrperfmon    57  /* Performance monitoring function */
#define PAL_wrusp        56  /* Write user stack pointer */
#define PAL_wrval        49  /* Write system value */
#define PAL_wrvptptr     45     /* Write virtual page table pointer */
/* PAL_wtint = ?          Wait for interrupt */

/* MMCSR values */
#define PAL_MMCSR_INVALID 0
#define PAL_MMCSR_ACCESS  1
#define PAL_MMCSR_FOR     2
#define PAL_MMCSR_FOE     3
#define PAL_MMCSR_FOW     4

#define PAL_PS_USER       (1 << 3)

#endif /* __ARCH__ALPHA__PAL_H__ */
