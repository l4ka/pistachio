/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/config.h
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
 * $Id: config.h,v 1.10 2005/01/18 13:27:37 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_POWERPC64__CONFIG_H__
#define __GLUE__V4_POWERPC64__CONFIG_H__

#include INC_GLUE(offsets.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(page.h)
#include INC_PLAT(config.h)

/**
 * Size of a kernel TCB in bytes
 */

#define KTCB_BITS       (POWERPC64_PAGE_BITS)
#define KTCB_SIZE	(__UL(1)<<KTCB_BITS)
#define KTCB_MASK       (~((1 << KTCB_BITS) - 1))

#define UTCB_BITS       (10)
#define UTCB_SIZE       (1ul<<UTCB_BITS)
#define UTCB_MASK       (~((1 << UTCB_BITS) - 1))


/**
 * endianess and word size
 */
#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3)
#define KIP_API_FLAGS	{SHUFFLE2(endian:1, word_size:1)}	// 64-bit, big endian
#elif (CONFIG_PLAT_OFG5)
#define KIP_API_FLAGS	{SHUFFLE2(endian:1, word_size:1)}	// 64-bit, big endian
#else
#error "FIXME"
#endif

/**
 * Mapping database - space bits
 */
#define MDB_SPACE_BITS	(BITS_WORD - 10)

/*
 * minimum size of UTCB area and number of UTCBs in this
 * 8 byte aligned, 1KB size, 4KB area size
 */
#define KIP_UTCB_INFO	{SHUFFLE3(multiplier:1, alignment:UTCB_BITS, size:POWERPC64_PAGE_BITS)}

/*
 * attributes for system call functions
 * @param x is the name of the system call lacking the leading sys_ .
 * This makes it possible to place every system call in its own section
 * if required. Default is empty.
 */
#define KIP_SYSCALL(x)          ((word_t) (x) - (word_t) &kip)

#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3)
#define ARCH_SYSCALL0		KIP_SYSCALL (user_rtas_call)

#if !defined(ASSEMBLY) && defined(__cplusplus)
extern "C" void SECTION (".user.rtas_call") user_rtas_call (void);
#endif
#endif

/****************************************************************************
 *
 * Division of the kernel's 4 peta byte address space.
 *
 ****************************************************************************/

/* PowerPC64 Memory Map
 *
 * 0xFFFFFFFFFFFFFFFF
 *     [TCB AREA]	- 4k pages
 * 0xFFFF000000000000
 *     [KERNEL AREA]    - 16M pages
 * 0xFFFE000000000000
 *     [DEVICE AREA]	- 4k  pages?
 * 0xFFFD000000000000
 *     [CPU AREA]	- 4k pages
 * 0xFFFC000000000000
 *     [PGHASH AREA]    - 16M pages
 * 0xFFFB000000000000
 *     [COPY AREA]	- 4k pages
 * 0xFFFA000000000000
 *     [RESERVED]
 * 0xFFF0000000000000
 *     [USER AREA]	- 4k pages (mixed in the future? - NB - must be one size per segment area (256MB))
 * 0x0000000000000000
 */

/* Kernel Area */
#define KERNEL_AREA_START	(KERNEL_OFFSET)
#define KERNEL_AREA_SIZE	(0x0001000000000000ul)
#define KERNEL_AREA_END		(KERNEL_AREA_START + KERNEL_AREA_SIZE)

#define CPU_AREA_START		(KERNEL_CPU_OFFSET)
#define CPU_AREA_SIZE		(256ul*1024*1024)	    /* Arbitrary */
#define CPU_AREA_END		(CPU_AREA_START + CPU_AREA_SIZE)

#define DEVICE_AREA_START	(0xFFFD000000000000ul)
#define DEVICE_AREA_SIZE	(0x0001000000000000ul)
#define DEVICE_AREA_END		(DEVICE_AREA_START + DEVICE_AREA_SIZE)

#define PGHASH_AREA_START	(0xFFFB000000000000ul)
#define PGHASH_AREA_SIZE	(0x0001000000000000ul)
#define PGHASH_AREA_END		(PGHASH_AREA_START + PGHASH_AREA_SIZE)

#define COPY_AREA_START		(0xFFFA000000000000ul)
#define COPY_AREA_SIZE		(0x0001000000000000ul)
#define COPY_AREA_END		(COPY_AREA_START + COPY_AREA_SIZE)

#define VALID_THREADNO_BITS	(32)
#define VALID_THREADNO_MASK	((1ul << VALID_THREADNO_BITS)-1)

#define KTCB_AREA_BITS		(48)
#define KTCB_AREA_START		(0xFFFF000000000000ul)
#define KTCB_AREA_SIZE		(1ul << (VALID_THREADNO_BITS + KTCB_BITS))
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

#if (VALID_THREADNO_BITS + KTCB_BITS > KTCB_AREA_BITS)
# error "KTCB area will not support VALID_THREADNO_BITS size."
#endif


/****************************************************************************
 *
 * Division of the user's 64-bit address space.
 *
 ****************************************************************************/

/* User area affected by space_t structure only - need compressed page tables */
#define USER_AREA_START		(0x0000000000000000ul)

#if POWERPC64_USER_BITS == 64
#define USER_AREA_END		(0xFFF0000000000000ul)
#else
#define USER_AREA_END		(1ul << POWERPC64_USER_BITS)
#endif

#define USER_AREA_SIZE		(USER_AREA_END - USER_AREA_START)

/**
 * size of kernel interface page
 */
#define KIP_KIP_AREA	{size:POWERPC64_PAGE_BITS}

/**
 * supported page sizes and access bits
 */
#if CONFIG_PLAT_OFPOWER4 || CONFIG_CPU_POWERPC64_PPC970
  #define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:7, size_mask:(1 << POWERPC64_PAGE_BITS) >> 10)}
#else
  #define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:6, size_mask:(1 << POWERPC64_PAGE_BITS) >> 10)}
#endif

#define KIP_MIN_MEMDESCS	(16)

/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START		(0x0010ul<<32)

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		(0x0011ul<<32)

/****************************************************************************
 *
 * Other.
 *
 ****************************************************************************/

#define TIMER_TICK_LENGTH	(2000) /* usec */

/* Supervisor general purpose registers usage */
#define	    SPRG_TCB	0	/* Current TCB */
#define	    SPRG_LOCAL	1	/* CPU Local Spill Area */
#define	    SPRG_TEMP0	2	/* Temporary */
#define	    SPRG_TEMP1	3	/* Temporary */

#endif /* !__GLUE__V4_POWERPC64__CONFIG_H__ */
