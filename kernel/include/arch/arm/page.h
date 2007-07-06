/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/page.h
 * Description:   ARM specific MM
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
 * $Id: page.h,v 1.7 2004/09/02 22:43:01 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__PAGE_H__
#define __ARCH__ARM__PAGE_H__

/*
 * Supported Page Sizes: 
 * - 1k ("tiny" pages) ** do we really want to support these? (shans) **
 * - 4k
 * - 64k
 * - 1M ("sections")
 */
#define ARM_SECTION_BITS	12
#define ARM_SECTION_SIZE	(1ul << (32 - ARM_SECTION_BITS))
#define ARM_NUM_SECTIONS	(1ul << ARM_SECTION_BITS)

#if defined(CONFIG_ARM_TINY_PAGES)
 #define ARM_PAGE_SIZE		PAGE_SIZE_1K
 #define ARM_PAGE_BITS		PAGE_BITS_1K
 #define TINY_PAGE		(1<<10) |
 #define HW_PGSHIFTS		{ 10, 12, 16, 20, 32 }
 #define MDB_NUM_PGSIZES	(4)
#else
 #define ARM_PAGE_SIZE		PAGE_SIZE_4K
 #define ARM_PAGE_BITS		PAGE_BITS_4K
 #define TINY_PAGE
 #define HW_PGSHIFTS		{ 12, 16, 20, 32 }
 #define MDB_NUM_PGSIZES	(3)
#endif

#define MDB_PGSHIFTS		HW_PGSHIFTS

#define	HW_VALID_PGSIZES (	\
		TINY_PAGE	\
		(1<<12) |	\
		(1<<16) |	\
		(1<<20))

#define PAGE_SIZE_1K		(1ul << 10)
#define PAGE_SIZE_4K		(1ul << 12)
#define PAGE_SIZE_64K		(1ul << 16)
#define PAGE_SIZE_1M		(1ul << 20)

#define PAGE_BITS_1K		(10)
#define PAGE_BITS_4K		(12)
#define PAGE_BITS_64K		(16)
#define PAGE_BITS_1M		(20)

#define PAGE_MASK_1K		(~(PAGE_SIZE_1K - 1))
#define PAGE_MASK_4K		(~(PAGE_SIZE_4K - 1))
#define PAGE_MASK_64K		(~(PAGE_SIZE_64K -1))
#define PAGE_MASK_1M		(~(PAGE_SIZE_1M - 1))

#endif /* __ARCH__ARM__PAGE_H__ */
