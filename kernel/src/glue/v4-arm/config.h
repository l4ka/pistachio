/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-arm/config.h
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
 * $Id: config.h,v 1.23 2004/12/01 23:57:01 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_ARM__CONFIG_H__
#define __GLUE__V4_ARM__CONFIG_H__

#include INC_API(config.h)
#include INC_ARCH(page.h)

#if defined(ASSEMBLY)
#define UL(x)	x
#else
#define UL(x)	x##ul
#endif

/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_PGSIZE     pgent_t::size_4k
#define KTCB_BITS       12
#define KTCB_SIZE	(UL(1) << KTCB_BITS)
#define KTCB_MASK       (~(KTCB_SIZE - 1))

#define UTCB_BITS       9
#define UTCB_SIZE       (1ul << UTCB_BITS)
#define UTCB_MASK       (~(UTCB_SIZE - 1))

#define UTCB_AREA_PGSIZE pgent_t::size_4k

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

#if CONFIG_BIGENDIAN
  #define KIP_API_FLAGS	{SHUFFLE2(endian:1, word_size:0)} /* 32-bit, big endian */
#else
  #define KIP_API_FLAGS	{ endian:0, word_size:0 } /* 32-bit, little endian */
#endif

/**
 * minimum size of UTCB area and number of UTCBs in this
 */
#define KIP_UTCB_INFO   { SHUFFLE3(multiplier: 1, alignment: UTCB_BITS, size:PAGE_BITS_4K) }

/**
 * size of kernel interface page
 */
#define KIP_KIP_BITS    PAGE_BITS_4K
#define KIP_KIP_SIZE    (1 << KIP_KIP_BITS)
#define KIP_KIP_PGSIZE  pgent_t::size_4k
#define KIP_KIP_AREA	{ size:KIP_KIP_BITS }   // 4KB

/**
 * supported page sizes and access bits
 */
#define KIP_ARCH_PAGEINFO { SHUFFLE2(rwx:2, size_mask:HW_VALID_PGSIZES >> 10) }

/* memory layout
 *
 * The first (14/16ths = 3.54GB) are user address space.  Immediately after this, is the
 * beginning of kernel space (2/16ths = 512MB).
 * There is 256 MB of KTCBs, giving 16 valid bits for thread IDs.
 *
 * Next is 128 MB of space for kernel code and structures (such as the CPD,
 * root page table and bootstack).
 *
 * After this is 64 MB of space for the copy area.
 *
 * Following this is 16 MB of space for storing variables for space_t such as
 * kip_area, utcb_area, thread_count and domain. It is important that the
 * 2 LSBs _MUST_ be 00, when using these for data or security may be compromised.
 * 
 * Following is 47 MB for IO space mappings, then
 * 
 * Finally a 1 MB section is reserved for mapping in the exception vectors.
 */


#define USER_AREA_SECTIONS	((ARM_NUM_SECTIONS * 14) / 16)
#define USER_AREA_START		0ul
#define USER_AREA_SIZE		(USER_AREA_SECTIONS * ARM_SECTION_SIZE)
#define USER_AREA_END		(USER_AREA_START + USER_AREA_SIZE)

#define VALID_THREADNO_BITS	16
#define VALID_THREADNO_MASK	((1ul << VALID_THREADNO_BITS)-1)
#define KTCB_AREA_SECTIONS	(KTCB_AREA_SIZE / ARM_SECTION_SIZE)
#define KTCB_AREA_START		USER_AREA_END
#define KTCB_AREA_SIZE		(1ul << (KTCB_BITS + VALID_THREADNO_BITS))
#define KTCB_AREA_END		(KTCB_AREA_START + KTCB_AREA_SIZE)

#define KERNEL_AREA_SECTIONS	64
#define KERNEL_AREA_START	KTCB_AREA_END
#define KERNEL_AREA_SIZE	(KERNEL_AREA_SECTIONS * ARM_SECTION_SIZE)
#define KERNEL_AREA_END		(KERNEL_AREA_START + KERNEL_AREA_SIZE)

#define UNCACHE_AREA_SECTIONS	64
#define UNCACHE_AREA_START	KERNEL_AREA_END
#define UNCACHE_AREA_SIZE	(UNCACHE_AREA_SECTIONS * ARM_SECTION_SIZE)
#define UNCACHE_AREA_END	(UNCACHE_AREA_START + UNCACHE_AREA_SIZE)

#define COPY_AREA_SECTIONS	 64
#define COPY_AREA_BLOCK_SECTIONS 8
#define COPY_AREA_START		 UNCACHE_AREA_END
#define COPY_AREA_BLOCK_SIZE	 (COPY_AREA_BLOCK_SECTIONS * ARM_SECTION_SIZE)
#define COPY_AREA_SIZE		 (COPY_AREA_SECTIONS * ARM_SECTION_SIZE)
#define COPY_AREA_COUNT		 (COPY_AREA_SIZE / COPY_AREA_BLOCK_SIZE)
#define COPY_AREA_END		 (COPY_AREA_START + COPY_AREA_SIZE)

#define VAR_AREA_SECTIONS	16
#define VAR_AREA_START		COPY_AREA_END
#define VAR_AREA_SIZE		(VAR_AREA_SECTIONS * ARM_SECTION_SIZE)
#define VAR_AREA_END		(VAR_AREA_START + VAR_AREA_SIZE)

#define IO_AREA_SECTIONS	32
#define IO_AREA_START		VAR_AREA_END
#define IO_AREA_SIZE		(IO_AREA_SECTIONS * ARM_SECTION_SIZE)
#define IO_AREA_END		(IO_AREA_START + IO_AREA_SIZE)

#define MISC_AREA_SECTIONS	15
#define MISC_AREA_START		IO_AREA_END
#define MISC_AREA_SIZE		(MISC_AREA_SECTIONS * ARM_SECTION_SIZE)
#define MISC_AREA_END		(MISC_AREA_START + MISC_AREA_SIZE)

/* User UTCB reference page at 0xff000000 */

#define USER_UTCB_REF_PAGE	MISC_AREA_START

#define EXCPT_AREA_START	MISC_AREA_END
#define EXCPT_AREA_SIZE		ARM_SECTION_SIZE
#define EXCPT_AREA_END		(EXCPT_AREA_START + EXCPT_AREA_SIZE)

/* 1MB IO Areas in the Virtual Address space. Define more if needed */
#define IO_AREA0_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*0))
#define IO_AREA1_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*1))
#define IO_AREA2_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*2))
#define IO_AREA3_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*3))
#define IO_AREA4_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*4))
#define IO_AREA5_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*5))
#define IO_AREA6_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*6))
#define IO_AREA7_VADDR		(IO_AREA_START + (ARM_SECTION_SIZE*7))
#define PHYSMAPPING_VADDR	(IO_AREA_START + (ARM_SECTION_SIZE*10))

/* Note on io area usage:
 *
 * SA1100 uses:
 *  #define CONSOLE_VADDR		IO_AREA0_VADDR
 *  #define ZERO_BANK_VADDR		IO_AREA1_VADDR
 *  #define SA1100_OS_TIMER_BASE	IO_AREA2_VADDR
 *
 * XSCALE uses:
 *  #define IODEVICE_VADDR		IO_AREA0_VADDR
 *
 * OMAP1510 uses:
 *  #define IODEVICE_VADDR		IO_AREA0_VADDR
 *
 * CSB337 uses:
 *  #define SYS_VADDR			IO_AREA0_VADDR
*/

/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START		(USER_AREA_END - ARM_SECTION_SIZE)

/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		(USER_AREA_END - KIP_KIP_SIZE)

/**
 * Address of start of arm_high_vector - exception handling code
 */
#define ARM_HIGH_VECTOR_VADDR	(EXCPT_AREA_START | 0x000f0000 )

/**
 * Address of the syscall area (for prefect aborts) is in
 * the last page of virtual memory. Do not map anything
 * with user access here.
 */
#define ARM_SYSCALL_VECTOR	(0xffffff00)

#include INC_PLAT(timer.h)

#endif /* __GLUE__V4_ARM__CONFIG_H__ */
