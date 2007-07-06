/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/powerpc/page.h
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
 * $Id: page.h,v 1.3 2006/02/23 21:02:29 ud3 Exp $
 *
 ***************************************************************************/
#ifndef __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__
#define __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__


#define L1_CACHE_LINE_SIZE	32
#define PAGE_BITS	12
#define PAGE_SIZE	(1ul << PAGE_BITS)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#warning POWERPC version
L4_INLINE void cache_partial_code_sync( L4_Word_t address )
{
    asm volatile( "dcbst 0,%0 ; sync ; icbi 0,%0" : : "r" (address) );
}

L4_INLINE void cache_complete_code_sync( void )
{
    asm volatile( "isync" );
}

L4_INLINE void memcpy_cache_flush( L4_Word_t *dst, const L4_Word_t *src, L4_Word_t size )
    /*  dst and src must be aligned by sizeof(L4_Word_t).  size must be
     *  a multiple of sizeof(L4_Word_t).
     */
{
    L4_Word_t cnt;
    int line_words = L1_CACHE_LINE_SIZE / sizeof(L4_Word_t);

    for( cnt = 0; cnt < size/sizeof(L4_Word_t); cnt++ ) {
	dst[cnt] = src[cnt];
	if( cnt && ((cnt % line_words) == 0) )
	{
	    /*  We just started a new cache line, so flush the
	     *  prior cache line.
	     */
	    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
	}
    }
    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
    cache_complete_code_sync();
}


#endif	/* __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__ */
