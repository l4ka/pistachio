/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:	  arch/mips64/cache.h
 * Description:   Functions which manipulate the MIPS cache
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
 * $Id: cache.h,v 1.10 2004/12/02 00:01:28 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__ERPCN01__CACHE_H__
#define __PLATFORM__ERPCN01__CACHE_H__

#include INC_ARCH(cache.h)
#include INC_ARCH(mipsregs.h)
#include INC_ARCH(addrspace.h)
#include "linear_ptab.h"

#define CONFIG_MIPS64_DCACHE_SIZE	(32*1024)
#define CONFIG_MIPS64_ICACHE_SIZE	(32*1024)
#define CONFIG_MIPS64_CACHE_LINE_SIZE	32
#define CONFIG_MIPS64_CACHE_WAYS	2

#define CACHE_WAYS		CONFIG_MIPS64_CACHE_WAYS
//#define CACHE_LINE_SIZE		CONFIG_MIPS64_CACHE_LINE_SIZE	in config.h
#define DCACHE_SIZE		CONFIG_MIPS64_DCACHE_SIZE
#define ICACHE_SIZE		CONFIG_MIPS64_ICACHE_SIZE

/*
 * Cache Operations
 */
#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Invalidate_SI     0x02
#define Index_Writeback_Inv_SD  0x03
#define Index_Load_Tag_I        0x04
#define Index_Load_Tag_D        0x05
#define Index_Load_Tag_SI       0x06
#define Index_Load_Tag_SD       0x07
#define Index_Store_Tag_I       0x08
#define Index_Store_Tag_D       0x09
#define Index_Store_Tag_SI      0x0A
#define Index_Store_Tag_SD      0x0B
#define Create_Dirty_Excl_D     0x0d
#define Create_Dirty_Excl_SD    0x0f
#define Hit_Invalidate_I        0x10
#define Hit_Invalidate_D        0x11
#define Hit_Invalidate_SI       0x12
#define Hit_Invalidate_SD       0x13
#define Fill                    0x14
#define Hit_Writeback_Inv_D     0x15
                                        /* 0x16 is unused */
#define Hit_Writeback_Inv_SD    0x17
#define Hit_Writeback_I         0x18
#define Hit_Writeback_D         0x19
                                        /* 0x1a is unused */
#define Hit_Writeback_SD        0x1b
                                        /* 0x1c is unused */
                                        /* 0x1e is unused */
#define Hit_Set_Virtual_SI      0x1e
#define Hit_Set_Virtual_SD      0x1f

#define cache32_unroll32(base,op)                               \
	__asm__ __volatile__("                                  \
		.set noreorder;                                 \
		cache %1, 0x000(%0); cache %1, 0x020(%0);       \
		cache %1, 0x040(%0); cache %1, 0x060(%0);       \
		cache %1, 0x080(%0); cache %1, 0x0a0(%0);       \
		cache %1, 0x0c0(%0); cache %1, 0x0e0(%0);       \
		cache %1, 0x100(%0); cache %1, 0x120(%0);       \
		cache %1, 0x140(%0); cache %1, 0x160(%0);       \
		cache %1, 0x180(%0); cache %1, 0x1a0(%0);       \
		cache %1, 0x1c0(%0); cache %1, 0x1e0(%0);       \
		cache %1, 0x200(%0); cache %1, 0x220(%0);       \
		cache %1, 0x240(%0); cache %1, 0x260(%0);       \
		cache %1, 0x280(%0); cache %1, 0x2a0(%0);       \
		cache %1, 0x2c0(%0); cache %1, 0x2e0(%0);       \
		cache %1, 0x300(%0); cache %1, 0x320(%0);       \
		cache %1, 0x340(%0); cache %1, 0x360(%0);       \
		cache %1, 0x380(%0); cache %1, 0x3a0(%0);       \
		cache %1, 0x3c0(%0); cache %1, 0x3e0(%0);       \
		.set reorder"                                   \
		:                                               \
		: "r" (base),                                   \
		  "i" (op));

static inline void blast_dcache32(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + DCACHE_SIZE);

    while(start < end) {
	cache32_unroll32(start,Index_Writeback_Inv_D);
        start += 0x400;
    }
}

static inline void blast_icache32(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + ICACHE_SIZE);

    while(start < end) {
	cache32_unroll32(start,Index_Invalidate_I);
	start += 0x400;
    }
}

static inline void init_dcache32(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + DCACHE_SIZE);

    asm (
	"mtc0	$0, "STR(CP0_TAGLO)"\n\t"
	"mtc0	$0, "STR(CP0_TAGHI)"\n\t"
    );

    while(start < end) {
	asm (
	    "cache  %1, 0(%0)"
	    : : "r" (start), "i" (Index_Store_Tag_D)
	);
	start += CACHE_LINE_SIZE;
    }
}

static inline void init_icache32(void)
{
    unsigned long start = KSEG0;
    unsigned long end = (start + ICACHE_SIZE);

    asm (
	"mtc0	$0, "STR(CP0_TAGLO)"\n\t"
	"mtc0	$0, "STR(CP0_TAGHI)"\n\t"
    );

    while(start < end) {
	asm (
	    "cache  %1, 0(%0)"
	    : : "r" (start), "i" (Index_Store_Tag_I)
	);
	start += CACHE_LINE_SIZE;
    }
}


INLINE void cache_t::init_cpu(void)
{
    word_t temp;

    __asm__ __volatile__ (
	"la	%0, 1f\n\t"
	"or	%0, 0xffffffffa0000000\n\t"
	"jr	%0\n\t"
	"1:\n\t"
	"mfc0	%0, "STR(CP0_CONFIG)"\n\t"
	: "=r" (temp)
    );
    temp &= (~CONFIG_CACHE_MASK);
#if CONFIG_UNCACHED
    temp |= CONFIG_NOCACHE;
#else
    temp |= CONFIG_CACHABLE_NONCOHERENT;
#endif

    __asm__ __volatile__ (
	"mtc0	%0, "STR(CP0_CONFIG)"\n\t"
	: : "r" (temp)
    );

    /* Important that these inline! */
    init_dcache32();
    init_icache32();

    __asm__ __volatile__ (
	"la	%0, 2f\n\t"
	"jr	%0\n\t"
	"2:\n\t"
	: : "r" (temp)
    );
}

INLINE void cache_t::flush_cache_all(void)
{
    blast_dcache32(); blast_icache32();
}

INLINE void cache_t::flush_cache_l1(void)
{
    blast_dcache32(); blast_icache32();
}

INLINE void cache_t::flush_cache_range(unsigned long start, unsigned long end)
{
    start &= (~CACHE_LINE_SIZE);
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    if (end > (start + DCACHE_SIZE))
	end = start + DCACHE_SIZE;

    while (start < end) {
	asm (
	    "cache  %1, 0(%0)"
	    : : "r" (start), "i" (Index_Store_Tag_D)
	);
	start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::flush_icache_range(unsigned long start, unsigned long end)
{
    start &= (~CACHE_LINE_SIZE);
    end = (end + (CACHE_LINE_SIZE-1)) & (~CACHE_LINE_SIZE);

    if (end > (start + ICACHE_SIZE))
	end = start + ICACHE_SIZE;

    while (start < end) {
	asm (
	    "cache  %1, 0(%0)"
	    : : "r" (start), "i" (Index_Store_Tag_I)
	);
	start += CACHE_LINE_SIZE;
    }
}

INLINE void cache_t::flush_cache_page(unsigned long page, pgent_t::pgsize_e pgsize)
{
    unsigned long start = page & (~page_mask(pgsize));
    unsigned long end = start + page_size(pgsize);

    if (end > (start + DCACHE_SIZE))
	end = start + DCACHE_SIZE;

    while (start < end) {
	cache32_unroll32(start, Index_Writeback_Inv_D);
	start += 0x400;
    }
}

INLINE void cache_t::flush_icache_page(unsigned long page, pgent_t::pgsize_e pgsize)
{
    unsigned long start = page & (~page_mask(pgsize));
    unsigned long end = start + page_size(pgsize);

    if (end > (start + ICACHE_SIZE))
	end = start + ICACHE_SIZE;

    while (start < end) {
	cache32_unroll32(start, Index_Invalidate_I);
	start += 0x400;
    }
}

#endif
