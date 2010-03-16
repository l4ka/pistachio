/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/config.h
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __GLUE__V4_POWERPC__CONFIG_H__
#define __GLUE__V4_POWERPC__CONFIG_H__

#include INC_GLUE(offsets.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(page.h)
#include INC_ARCH(bat.h)

#define CACHE_LINE_SIZE		(POWERPC_CACHE_LINE_SIZE)
#define KERNEL_PAGE_SIZE	(POWERPC_PAGE_SIZE)

#define KTCB_BITS		(11)
#define KTCB_SIZE		(1 << KTCB_BITS)
#define KTCB_MASK		(~(KTCB_SIZE-1))
#define TOTAL_TCB_SIZE		(KTCB_SIZE)

#define VALID_THREADNO_BITS	(28 - KTCB_BITS)	// 256 MB for all KTCBs
#define TOTAL_KTCBS		(__UL(1) << VALID_THREADNO_BITS)
#define VALID_THREADNO_MASK	(TOTAL_KTCBS - 1)


/****************************************************************************
 *
 * Division of the kernel's 1 gig address space.
 *
 ****************************************************************************/

#define KERNEL_AREA_START	(KERNEL_OFFSET)
#define KERNEL_AREA_LOG2SIZE	28 /* 256M */
#define KERNEL_AREA_SIZE	(1 << KERNEL_AREA_LOG2SIZE)
#define KERNEL_AREA_END		(KERNEL_AREA_START + KERNEL_AREA_SIZE)

#define DEVICE_AREA_START	0xD0000000
#define DEVICE_AREA_SIZE	0x02000000
#define DEVICE_AREA_BAT_SIZE	BAT_256K_PAGE_SIZE
#define DEVICE_AREA_END		(DEVICE_AREA_START + DEVICE_AREA_SIZE)

#define PINNED_AREA_START	DEVICE_AREA_END
#define PINNED_AREA_SIZE	0x01000000
#define PINNED_AREA_END		(PINNED_AREA_START + PINNED_AREA_SIZE)

#define CONSOLE_AREA_START	PINNED_AREA_END
#define CONSOLE_AREA_SIZE	0x01000000
#define CONSOLE_AREA_END	(CONSOLE_AREA_START + CONSOLE_AREA_SIZE)

#if (KERNEL_AREA_END > DEVICE_AREA_START)
# error "The kernel area overlaps the device area."
#endif

#define CPU_AREA_START		(KERNEL_CPU_OFFSET)
#define CPU_AREA_SIZE		(BAT_128K_PAGE_SIZE)
#define CPU_AREA_END		(CPU_AREA_START + CPU_AREA_SIZE)

#ifdef CONFIG_PPC_MMU_SEGMENTS
/* Keep 32MB aligned (max size of page hash) */
#define PGHASH_AREA_START	(CONSOLE_AREA_END)
#define PGHASH_AREA_SIZE	(BAT_32M_PAGE_SIZE)
#define PGHASH_AREA_END		(PGHASH_AREA_START + PGHASH_AREA_SIZE)

#if ((PGHASH_AREA_START & BAT_32M_PAGE_MASK) != PGHASH_AREA_START)
# error "The page hash area is not aligned to 32MB."
#endif

#if (PGHASH_AREA_END > CPU_AREA_START)
# error "The page hash area overlaps the cpu data area."
#endif

#define KTCB_AREA_START		0xE0000000
#define KTCB_AREA_SIZE		(1 << (VALID_THREADNO_BITS + KTCB_BITS))
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

#if (CPU_AREA_END > KTCB_AREA_START)
# error "The cpu area overlaps the KTCB area."
#endif

#elif defined(CONFIG_PPC_MMU_TLB)

#define CONFIG_MAX_NUM_ASIDS	64
/* #define CONFIG_PREEMPT_ASIDS */
#endif

#define COPY_AREA_START		0xF0000000
#if defined(CONFIG_PPC_MMU_TLB)
#define COPY_AREA_SIZE		0x00800000
#else
#define COPY_AREA_SIZE		0x10000000
#endif
#define COPY_AREA_END		(COPY_AREA_START + COPY_AREA_SIZE - 1)
#define COPY_AREA_SEGMENT	(COPY_AREA_START >> 28)

#if (KTCB_AREA_END > COPY_AREA_START)
# error "The ktcb area overlaps the copy area."
#endif

/****************************************************************************
 *
 * Division of the user's 3 gig address space.
 *
 ****************************************************************************/

#define USER_AREA_START		0x00000000
#define USER_AREA_SIZE		0xC0000000
#define USER_AREA_END		(USER_AREA_START + USER_AREA_SIZE)

#define ROOT_UTCB_START		0xBF000000
#define ROOT_KIP_START		0xBFF00000

/****************************************************************************
 *
 * KIP configuration.
 *
 ****************************************************************************/

/* big endian, 32-bit */
#define KIP_API_FLAGS		{SHUFFLE2(endian:1, word_size:0)}
/* 512 bytes aligned, 512 bytes size, 4k area size */
#define KIP_UTCB_INFO		{SHUFFLE3(multiplier:1, alignment:9, size:12)}
/* 4KB */
#define KIP_KIP_AREA		{ 12 }
#define KIP_MIN_MEMDESCS	(16)

#ifdef CONFIG_PPC_MMU_SEGMENT
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:6, size_mask:(1 << POWERPC_PAGE_BITS) >> 10)}
#else
#define HW_VALID_PGSIZES	((1 << 12) | (1 << 22))
#define KIP_ARCH_PAGEINFO	{SHUFFLE2(rwx:7, size_mask: HW_VALID_PGSIZES >> 10)}
#endif

/****************************************************************************
 *
 * Other.
 *
 ****************************************************************************/

/* The MPC750 decrements the dec register at 1/4 the bus speed. */
#define TIMER_TICK_LENGTH	(1953)

/* The special pupose register for the cpu spill. */
#define SPRG_CPU		0
/* The special purpose register for the current TCB. */
#define SPRG_CURRENT_TCB	1
/* Temporary special purpose registers. */
#define SPRG_TMP0		2
#define SPRG_TMP1		3

/* Cache information */
#define PPC_DEVICE_CACHE_COHERENT

#endif /* !__GLUE__V4_POWERPC__CONFIG_H__ */
