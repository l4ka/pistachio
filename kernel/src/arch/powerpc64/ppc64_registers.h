/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/ppc64_registers.h
 * Description:	SPR register encodings, and functions which misc PowerPC64
 * 		registers.
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
 * $Id: ppc64_registers.h,v 1.5 2005/01/18 13:25:36 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__PPC64_REGISTERS_H__
#define __ARCH__POWERPC64__PPC64_REGISTERS_H__

/*  SPR encodings for mfspr
 *  .._R - means user-read only, .._W means supervisor write
 */
#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18	/* 32-bit */
#define SPR_DAR		19	/* 32-bit */
#define SPR_DEC		22	/* 32-bit */
#define SPR_SDR1	25
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_TBL_R	268
#define SPR_TBU_R	269
#define SPR_ASR		280
#if 0
#define SPR_EAR		282
#endif
#define SPR_TBL_W	286	/* 32-bit */
#define SPR_TBU_W	285	/* 32-bit */
#define SPR_PVR		287
#define SPR_DABR	1013

#if CONFIG_CPU_POWERPC64_PPC970
#define	SPR_VRSAVE	256	/* 32-bit */
#define SPR_SCOMMC	276
#define	SPR_SCOMMD	277
#define SPR_HIOR	311
#define SPR_UPMC1	771	/* 32-bit */
#define SPR_UPMC2	772	/* 32-bit */
#define SPR_UPMC3	773	/* 32-bit */
#define SPR_UPMC4	774	/* 32-bit */
#define SPR_UPMC5	775	/* 32-bit */
#define SPR_UPMC6	776	/* 32-bit */
#define SPR_UPMC7	777	/* 32-bit */
#define SPR_UPMC8	778	/* 32-bit */
#define SPR_USIAR	780
#define SPR_USDAR	781
#define SPR_UMMCR0	779
#define SPR_UMMCR1	782
#define SPR_UMMCR2	770
#define SPR_UIMC	783
#define SPR_PMC1	787	/* 32-bit */
#define SPR_PMC2	788	/* 32-bit */
#define SPR_PMC3	789	/* 32-bit */
#define SPR_PMC4	790	/* 32-bit */
#define SPR_PMC5	791	/* 32-bit */
#define SPR_PMC6	792	/* 32-bit */
#define SPR_PMC7	793	/* 32-bit */
#define SPR_PMC8	794	/* 32-bit */
#define SPR_SIAR	796
#define SPR_SDAR	797
#define SPR_MMCR0	795
#define SPR_MMCR1	798
#define SPR_MMCR2	786
#define SPR_IMC		799
#define SPR_TRIG0	976
#define SPR_TRIG1	977
#define SPR_TRIG2	978
#define SPR_HID0	1008
#define SPR_HID1	1009
#define SPR_HID4	1012
#define SPR_HID5	1014
#define SPR_DABRX	1015
#define SPR_PIR		1023	/* 32-bit */
#endif

#ifndef ASSEMBLY

INLINE word_t ppc64_get_sprg( const int which )
{
    word_t val;
    asm volatile( "mfsprg %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc64_set_sprg( const int which, word_t val )
{
    asm volatile( "mtsprg %0, %1" : : "i" (which), "r" (val) );
}

INLINE word_t ppc64_get_spr( const int which )
{
    word_t val;
    asm volatile( "mfspr %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc64_set_spr( const int which, word_t val )
{
    asm volatile( "mtspr %0, %1" : : "i" (which), "r" (val) );
}

INLINE word_t ppc64_get_sdr1( void )
{
    word_t val;
    asm volatile( "mfsdr1 %0" : "=r" (val) );
    return val;
}

INLINE void ppc64_set_sdr1( word_t val )
{
    asm volatile( "mtsdr1 %0" : : "r" (val) );
}

#if 0
INLINE void ppc_set_srr0( word_t val )
	{ asm volatile( "mtsrr0 %0" : : "r" (val) ); }

INLINE void ppc_set_srr1( word_t val )
	{ asm volatile( "mtsrr1 %0" : : "r" (val) ); }

INLINE word_t ppc_get_srr0( void )
{
	word_t val;
	asm volatile( "mfsrr0 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_srr1( void )
{
	word_t val;
	asm volatile( "mfsrr1 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_dar( void )
{
    word_t val;
    asm volatile( "mfspr %0, 19" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dsisr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 18" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dabr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 1013" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_ear( void )
{
    word_t val;
    asm volatile( "mfspr %0, 282" : "=r" (val) );
    return val;
}
#endif

INLINE word_t ppc64_get_pvr( void )
{
    word_t val;
    asm volatile( "mfpvr %0" : "=r" (val) );
    return val;
}

INLINE word_t ppc64_get_dec( void )
{
    word_t val;
    asm volatile( "mfdec %0" : "=r" (val) );
    return val;
}

INLINE void ppc64_set_dec( word_t val )
{
    asm volatile( "mtdec %0" : : "r" (val) );
}

INLINE word_t ppc64_get_tb( void )
{
    word_t val;
    asm volatile( "mftb %0" : "=r" (val) );
    return val;
}

INLINE void ppc64_set_tbl( word_t val )
{
    asm volatile( "mttbl %0" : : "r" (val) );
}

INLINE void ppc64_set_tbu( word_t val )
{
    asm volatile( "mttbu %0" : : "r" (val) );
}


#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC64__PPC64_REGISTERS_H__ */

