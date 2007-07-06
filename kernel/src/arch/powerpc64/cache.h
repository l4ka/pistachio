/*****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *  
 * File path:	arch/powerpc64/cache.h
 * Description:	Functions which manipulate the PowerPC64 cache, and which
 * 		synchronize the data and instruction caches.
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
 * $Id: cache.h,v 1.5 2005/01/18 13:22:22 cvansch Exp $
 *
 ****************************************************************************/

#ifndef __ARCH__POWERPC64__CACHE_H__
#define __ARCH__POWERPC64__CACHE_H__

/*  take a guess at the cache line size  */

#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3 || CONFIG_CPU_POWERPC64_PPC970)
#define POWERPC64_CACHE_LINE_SIZE	128
#else
#error Cache size
#endif

#if !defined(ASSEMBLY)

#include <types.h>

#if 0
INLINE void cache_code_sync( word_t address )
{
	asm volatile("dcbst 0,%0 ; sync ; icbi 0,%0 ; isync" : : "r" (address));
}
#endif

INLINE void cache_partial_code_sync( word_t address )
{
	asm volatile( "dcbst 0,%0 ; sync ; icbi 0,%0" : : "r" (address) );
}

INLINE void cache_complete_code_sync( void )
{
	asm volatile( "sync" );
}

INLINE void ppc64_cache_zero_block( word_t address )
	{ asm volatile( "dcbz 0,%0" : : "r" (address) ); }

#if 0
INLINE void ppc_cache_alloc_block( word_t address )
	{ asm volatile( "dcba 0,%0" : : "r" (address) ); }

INLINE void ppc_cache_flush_block( word_t address )
	{ asm volatile( "dcbf 0,%0" : : "r" (address) ); }

INLINE void ppc_cache_invalidate_block( word_t address )
	{ asm volatile( "dcbi 0,%0" : : "r" (address) ); }

INLINE void ppc_cache_store_block( word_t address )
	{ asm volatile( "dcbst 0,%0" : : "r" (address) ); }

INLINE void ppc_cache_prefetch( word_t address )
	{ asm volatile( "dcbt 0,%0" : : "r" (address) ); }

INLINE void ppc_cache_prefetch_for_store( word_t address )
	{ asm volatile( "dcbtst 0,%0" : : "r" (address) ); }

#endif

INLINE void sync( void )
{
    asm volatile ("sync");
}

INLINE void isync( void )
{
    asm volatile ("isync");
}

INLINE void eieio( void )
{
    asm volatile ("eieio");
}

#endif	/* ASSEMBLER */

#endif	/* __ARCH__POWERPC__CACHE_H__ */
