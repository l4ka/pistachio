/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:   glue/v4-sparc64/config.h
 * Description:   
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
 * $Id: config.h,v 1.6 2004/02/12 01:35:22 philipd Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__CONFIG_H__
#define __GLUE__V4_SPARC64__CONFIG_H__

#include INC_CPU(mmu.h)
#include INC_GLUE_API_ARCH(utcb.h)

#warning awiggins (26-07-03): 4KB for the KTCB might be excessive.
/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_BITS       12 // 4KB
#define KTCB_SIZE	(1 << KTCB_BITS)
#define KTCB_MASK	(~(KTCB_SIZE - 1))

/*************
* KIP Fields *
*************/

/**
   attributes for system call functions
   @param x is the name of the system call lacking the leading sys_ .
   This makes it possible to place every system call in its own section
   if required. Default is empty.
 */
#define KIP_SYSCALL(x)  ((word_t) (x) - (word_t) &kip)

/**
 * endianess and word size
 */
#define KIP_API_FLAGS	{SHUFFLE2(endian:1, word_size:1)} // 64-bit, big endian

#warning awiggins (26-07-03): Need to check a single page is enough for KIP.
/**
 * size of kernel interface page
 */
#define KIP_AREA_BITS   SPARC64_PAGE_BITS
#define KIP_AREA_SIZE   (1L << KIP_AREA_BITS)
#define KIP_KIP_AREA	{size:KIP_AREA_BITS}

/**
 * minimum size of UTCB area and number of UTCBs in this
 */
/* 8 byte aligned, 1KB size, 8KB area size */
#define KIP_UTCB_INFO {SHUFFLE3(multiplier:1, alignment:UTCB_BITS, \
                                size:SPARC64_PAGE_BITS)}

/**
 * supported page sizes and access bits
 */
#define KIP_ARCH_PAGEINFO {SHUFFLE2(rwx:HW_ACCESS_BITS, \
                                    size_mask:(HW_VALID_PGSIZES >> 10))}

/*******************************
* Virtual Address Space Layout *
*******************************/

#ifdef SPARC64_HAS_ASI_NUCLEUS
/** If the processor supports a NUCLEUS address-space we can map the kernel
 *  into its own private address space giving the user space the full 64-bit
 *  address space to use. 
 */

/* User area (Full address space) */

#if (SPARC64_VIRTUAL_ADDRESS_BITS < 64)

#define USER_AREA_LOWER_START (0x0L)
#define USER_AREA_LOWER_LIMIT ((1L << (SPARC64_VIRTUAL_ADDRESS_BITS - 1)) - 1)
#define USER_AREA_UPPER_START (0L - (1L  << (SPARC64_VIRTUAL_ADDRESS_BITS - 1)))
#define USER_AREA_UPPER_LIMIT (-1L)

#else /* SPARC64_VIRTUAL_ADDRESS_BITS == 64 */

#define USER_AREA_START (0x0L)
#define USER_AREA_LIMIT (-1L)

#endif /* (SPARC64_VIRTUAL_ADDRESS_BITS < 64) */

/* Copy area (Not required) */

/**
 *  We split the virtual address space between KTCB_AREA and the rest of the
 *  kernel, with the kernel mapped in the upper half and the KTCB area starting
 *  in the middle of the lower half.
 */

/* Kernel thread control blocks */
#define KTCB_AREA_START (1ULL << (SPARC64_VIRTUAL_ADDRESS_BITS - 2))
#define KTCB_AREA_SIZE  (1ULL << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_END   (KTCB_AREA_START + KTCB_AREA_SIZE)

#if (SPARC64_VIRTUAL_ADDRESS_BITS < (L4_GLOBAL_THREADNO_BITS + KTCB_BITS + 2))

#define VALID_THREADNO_BITS (SPARC64_VIRTUAL_ADDRESS_BITS - (KTCB_BITS + 2))

#else

#define VALID_THREADNO_BITS L4_GLOBAL_THREADNO_BITS

#endif /* (SPARC64_VIRTUAL_ADDRESS_BITS < ...) */

#define THREADNO_MASK ((1L << VALID_THREADNO_BITS) - 1)

/* Kernel area. KERNEL_AREA_START *must* be 32KB aligned */

#define KERNEL_AREA_START (KERNEL_AREA_LIMIT - KERNEL_AREA_SIZE + 1)
#define KERNEL_AREA_BITS  (SPARC64_VIRTUAL_ADDRESS_BITS - 1) /* Half Virt AS */
#define KERNEL_AREA_SIZE  (1 << KERNEL_AREA_BITS)
#define KERNEL_AREA_LIMIT (-1) /* Top of the virtual address space */

/* CPU local area stick it at the top of the kernel address space. */
#define KERNEL_CPULOCAL_START (KERNEL_AREA_LIMIT + 1 - KERNEL_CPULOCAL_SIZE)
#define KERNEL_CPULOCAL_SIZE  CPULOCALPAGE_SIZE
#define KERNEL_CPULOCAL_LIMIT KERNEL_AREA_LIMIT

/* Kernel doesn't use trap instructions, so we can use the smaller trap table */
#define TTABLE_SHORT 1

/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START (0x0UL - (ROOT_MAX_THREADS << UTCB_BITS))

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START (ROOT_UTCB_START - KIP_AREA_SIZE)

#else

#error awiggins (26-07-03): Need to add support for CPUs without ASI_NUCLEUS

#endif /* SPARC64_HAS_ASI_NUCLEUS */

#define TIMER_TICK_LENGTH	(2000) /* usec */

#endif /* !__GLUE__V4_SPARC64__CONFIG_H__ */
