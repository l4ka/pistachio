/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/config.h
 * Created:       24/07/2002 23:54:34 by Simon Winwood (sjw)
 * Description:   Arch configuration 
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
 * $Id: config.h,v 1.14 2003/09/24 19:04:34 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_ALPHA__CONFIG_H__
#define __GLUE__V4_ALPHA__CONFIG_H__

#include INC_API(config.h)
#include INC_ARCH(page.h)

/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_SIZE	4096
#define KTCB_BITS       12
#define KTCB_MASK       (~((1 << KTCB_BITS) - 1))

#define UTCB_SIZE       1024
#define UTCB_BITS       10
#define UTCB_MASK       (~((1 << UTCB_BITS) - 1))

#define CACHE_LINE_SIZE  (ALPHA_CACHE_LINE_SIZE)
#define KERNEL_PAGE_SIZE (ALPHA_PAGE_SIZE)

#ifndef __ASSEMBLY__

#define KIP_SYSCALL(x)		((word_t) (x) - (word_t) &kip)

/**
 * endianess and word size
 */
#define KIP_API_FLAGS	{endian:0, word_size:1} /* 64-bit, little endian */

/**
 * minimum size of UTCB area and number of UTCBs in this
 */
/* sjw (11/04/2003): Deprecated? */
#define KIP_UTCB_AREA	{size:ALPHA_PAGE_BITS, no:(1 << (ALPHA_PAGE_BITS - UTCB_BITS))}   /* 8 threads, 8KB */

/* 8 byte aligned, 1KB size, 8KB area size */
#define KIP_UTCB_INFO           {multiplier:1, alignment:UTCB_BITS, size:ALPHA_PAGE_BITS}

/**
 * size of kernel interface page
 */
#define KIP_KIP_AREA	{size:ALPHA_PAGE_BITS}   /* 8KB */

/**
 * supported page sizes and access bits
 */
#define KIP_ARCH_PAGEINFO {rwx:6, size_mask:\
                                     HW_VALID_PGSIZES  >> 10}
/**
 * Base address of the root task's UTCB area
 */

#endif /* __ASSEMBLY__ */


/* The kernel will execute out of kseg as it reduces TLB overhead and
 * bootstrapping easier (can delay setting up page tables etc.).  The kernel will
 * use seg1 for its virtual area (for the VPT, KTCBs, etc.)
 *
 * We need somewhere to put the console and its data structures (HWRPB being an important one).
 * The console needs <256M (from what I can tell from the Alpha HWRM --- it puts the console at 256M and
 * system software at 512M).
 *
 * Finally, we need some VM for KTCBs (although we could put them in kseg?) and the string IPC copy windows.
 *
 * I will deal with copy windows when they are implemented.
 * 
 */

/* Per space */
#define USER_AREA_START         (0)
#define USER_AREA_SIZE          AS_SEG0_SIZE
#define USER_AREA_END           AS_SEG0_END

/* VPT goes at the start of seg1 */
#define VLPT_AREA_START         AS_SEG1_START
/* This is 2**(33) for 43 bit AS, 2**(38) for 48 bit AS. */
#define VLPT_AREA_BITS          (CONFIG_ALPHA_ADDRESS_BITS - ALPHA_PAGE_BITS + 3)
#define VLPT_AREA_SIZE          (1UL << VLPT_AREA_BITS)
#define VLPT_AREA_END           (VLPT_AREA_START + VLPT_AREA_SIZE)

/* Shared */

/* Align this up to the next toplevel PTE */
#define CONSOLE_AREA_START      (VLPT_AREA_START + (1UL << TOPLEVEL_PT_BITS))
#define CONSOLE_AREA_SIZE       (1UL << TOPLEVEL_PT_BITS)
#define CONSOLE_AREA_END        (CONSOLE_AREA_START + CONSOLE_AREA_SIZE)
#define HWRPB_VADDR             CONSOLE_AREA_START

/* Somewhat arbitrary ... just give them 1 top level PT */
/* #define VALID_THREADNO_BITS     (TOPLEVEL_PT_BITS - KTCB_BITS) */
#define VALID_THREADNO_BITS     16
#define KTCB_AREA_SIZE		(1UL << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_START		(CONSOLE_AREA_END)
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)


#define ROOT_UTCB_START		(USER_AREA_END - (ROOT_MAX_THREADS << UTCB_BITS))

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		(ROOT_UTCB_START - ALPHA_PAGE_SIZE)

/* sjw (06/08/2002): This is technically in the HWRPB ... it says that it is 1kHz, 
 * but the interrupts look to come every 40ms?  Anyway, 1 1kHz clock gives a length of
 * 976ms 
 */
#define TIMER_TICK_LENGTH       (976)

#endif /* __GLUE__V4_ALPHA__CONFIG_H__ */
