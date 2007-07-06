/*****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *  
 * File path:	arch/powerpc/bat.h
 * Description:	Block Address Translation constants and register manipulation.
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
 * $Id: bat.h,v 1.9 2003/09/24 19:04:30 skoglund Exp $
 *
 ****************************************************************************/

#ifndef __ARCH__POWERPC__BAT_H__
#define __ARCH__POWERPC__BAT_H__

#if !defined(ASSEMBLY)

#include <types.h>

/****************************************************************************/

#define DEF_GET_BAT(name, reg)						\
INLINE u32_t name (void)						\
{									\
	u32_t val;							\
	asm volatile("mfspr %0, " reg : "=r" (val) );			\
	return val;							\
}

DEF_GET_BAT(ppc_get_ibat0u, "528")
DEF_GET_BAT(ppc_get_ibat0l, "529")
DEF_GET_BAT(ppc_get_ibat1u, "530")
DEF_GET_BAT(ppc_get_ibat1l, "531")
DEF_GET_BAT(ppc_get_ibat2u, "532")
DEF_GET_BAT(ppc_get_ibat2l, "533")
DEF_GET_BAT(ppc_get_ibat3u, "534")
DEF_GET_BAT(ppc_get_ibat3l, "535")

DEF_GET_BAT(ppc_get_dbat0u, "536")
DEF_GET_BAT(ppc_get_dbat0l, "537")
DEF_GET_BAT(ppc_get_dbat1u, "538")
DEF_GET_BAT(ppc_get_dbat1l, "539")
DEF_GET_BAT(ppc_get_dbat2u, "540")
DEF_GET_BAT(ppc_get_dbat2l, "541")
DEF_GET_BAT(ppc_get_dbat3u, "542")
DEF_GET_BAT(ppc_get_dbat3l, "543")

#undef DEF_GET_BAT

/****************************************************************************/

#define DEF_SET_BAT(name, reg)						\
INLINE void name (u32_t val)						\
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

/****************************************************************************/

#endif	/* !ASSEMBLY */

#define BAT_SMALL_PAGE_BITS	17
#define BAT_SMALL_PAGE_SIZE	(1 << BAT_SMALL_PAGE_BITS)
#define BAT_SMALL_PAGE_MASK	(~(BAT_SMALL_PAGE_SIZE - 1))

#define BAT_128K_PAGE_BITS	BAT_SMALL_PAGE_BITS
#define BAT_128K_PAGE_SIZE	BAT_SMALL_PAGE_SIZE
#define BAT_128K_PAGE_MASK	BAT_SMALL_PAGE_MASK

#define BAT_256K_PAGE_BITS	18
#define BAT_256K_PAGE_SIZE	(1 << BAT_256K_PAGE_BITS)
#define BAT_256K_PAGE_MASK	(~(BAT_256K_PAGE_SIZE - 1))

#define BAT_512K_PAGE_BITS	19
#define BAT_512K_PAGE_SIZE	(1 << BAT_512K_PAGE_BITS)
#define BAT_512K_PAGE_MASK	(~(BAT_512K_PAGE_SIZE - 1))

#define BAT_32M_PAGE_BITS	25
#define BAT_32M_PAGE_SIZE	(1 << BAT_32M_PAGE_BITS)
#define BAT_32M_PAGE_MASK	(~(BAT_32M_PAGE_SIZE - 1))

#define BAT_256M_PAGE_BITS	28
#define BAT_256M_PAGE_SIZE	(1 << BAT_HUGE_PAGE_BITS)
#define BAT_256M_PAGE_MASK	(~(BAT_HUGE_PAGE_SIZE - 1))

#define BAT_HUGE_PAGE_BITS	BAT_256M_PAGE_BITS
#define BAT_HUGE_PAGE_SIZE	BAT_256M_PAGE_SIZE
#define BAT_HUGE_PAGE_MASK	BAT_256M_PAGE_MASK

#define BAT_VP		0	/* user mode valid bit		*/
#define BAT_VS		1	/* supervisor mode valid bit	*/
#define BAT_BL		2	/* block length mask		*/
#define BAT_BEPI	17	/* block effective page index	*/
#define BAT_PP		0	/* memory protection bits	*/
#define BAT_G		3	/* gaurded			*/
#define BAT_M		4	/* memory coherence		*/
#define BAT_I		5	/* caching-inhibited		*/
#define BAT_W		6	/* write-through		*/
#define BAT_BRPN	17	/* physical block number	*/

/*  NOTE: W and G bits must not be set for ibats.
 */

#define BAT_BLOCK_MASK	0xfffe0000

#define BAT_BL_128K	0x000
#define BAT_BL_256K	0x001
#define BAT_BL_512K	0x003
#define BAT_BL_1M	0x007
#define BAT_BL_2M	0x00f
#define BAT_BL_4M	0x01f
#define BAT_BL_8M	0x03f
#define BAT_BL_16M	0x07f
#define BAT_BL_32M	0x0ff
#define BAT_BL_64M	0x1ff
#define BAT_BL_128M	0x3ff
#define BAT_BL_256M	0x7ff

#define BAT_PP_NO_ACCESS	0x0
#define BAT_PP_READ_ONLY	0x1
#define BAT_PP_READ_WRITE	0x2

#if !defined(ASSEMBLY)
class ppc_bat_t {
public:
    union {
	struct {
	    word_t bepi : 15;
	    word_t reserved1 : 4;
	    word_t bl : 11;
	    word_t vs : 1;
	    word_t vp : 1;
	    word_t brpn : 15;
	    word_t reserved2 : 10;
	    word_t w : 1;
	    word_t i : 1;
	    word_t m : 1;
	    word_t g : 1;
	    word_t reserved3 : 1;
	    word_t pp : 2;
	} x;
	struct {
	    u32_t upper;
	    u32_t lower;
	} raw;
    };
};
#endif	/* !ASSEMBLY */

#endif	/* __ARCH__POWERPC__BAT_H__ */

