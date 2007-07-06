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
 * $Id: page.h,v 1.3 2003/09/24 19:06:37 skoglund Exp $
 *
 ***************************************************************************/
#ifndef __PIGGYBACKER__INCLUDE__POWERPC__PAGE_H__
#define __PIGGYBACKER__INCLUDE__POWERPC__PAGE_H__

#define L1_CACHE_LINE_SIZE	32
#define PAGE_BITS	12
#define PAGE_SIZE	(1 << PAGE_BITS)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define BAT_BL_128K     0x000
#define BAT_BL_256K     0x001
#define BAT_BL_512K     0x003
#define BAT_BL_1M       0x007
#define BAT_BL_2M       0x00f
#define BAT_BL_4M       0x01f
#define BAT_BL_8M       0x03f
#define BAT_BL_16M      0x07f
#define BAT_BL_32M      0x0ff
#define BAT_BL_64M      0x1ff
#define BAT_BL_128M     0x3ff
#define BAT_BL_256M     0x7ff

#define BAT_PP_NO_ACCESS        0x0
#define BAT_PP_READ_ONLY        0x1
#define BAT_PP_READ_WRITE       0x2

class ppc_bat_t {
public:
    union {
	struct {
	    L4_Word32_t bepi : 15;
	    L4_Word32_t reserved1 : 4;
	    L4_Word32_t bl : 11;
	    L4_Word32_t vs : 1;
	    L4_Word32_t vp : 1;
	    L4_Word32_t brpn : 15;
	    L4_Word32_t reserved2 : 10;
	    L4_Word32_t w : 1;
	    L4_Word32_t i : 1;
	    L4_Word32_t m : 1;
	    L4_Word32_t g : 1;
	    L4_Word32_t reserved3 : 1;
	    L4_Word32_t pp : 2;
	} x;
	struct {
	    L4_Word32_t upper;
	    L4_Word32_t lower;
	} raw;
    };
};

#define DEF_SET_BAT(name, reg)						\
L4_INLINE void name (L4_Word32_t val)					\
{									\
            asm volatile("mtspr " reg ", %0" : : "r" (val) );		\
}

DEF_SET_BAT(ppc_set_ibat0u, "528")
DEF_SET_BAT(ppc_set_ibat0l, "529")
DEF_SET_BAT(ppc_set_ibat1u, "530")
DEF_SET_BAT(ppc_set_ibat1l, "531")
DEF_SET_BAT(ppc_set_ibat2u, "532")
DEF_SET_BAT(ppc_set_ibat2l, "533")
DEF_SET_BAT(ppc_set_ibat3u, "534")
DEF_SET_BAT(ppc_set_ibat3l, "535")

DEF_SET_BAT(ppc_set_dbat0u, "536")
DEF_SET_BAT(ppc_set_dbat0l, "537")
DEF_SET_BAT(ppc_set_dbat1u, "538")
DEF_SET_BAT(ppc_set_dbat1l, "539")
DEF_SET_BAT(ppc_set_dbat2u, "540")
DEF_SET_BAT(ppc_set_dbat2l, "541")
DEF_SET_BAT(ppc_set_dbat3u, "542")
DEF_SET_BAT(ppc_set_dbat3l, "543")
#undef DEF_SET_BAT

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
	    /*  We just started a new cache line, so flush the
	     *  prior cache line.
	     */
	    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
    }
    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
    cache_complete_code_sync();
}


#endif	/* __PIGGYBACKER__INCLUDE__POWERPC__PAGE_H__ */
