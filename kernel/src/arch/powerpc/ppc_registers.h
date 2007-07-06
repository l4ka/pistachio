/****************************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/ppc_registers.h
 * Description:	SPR register encodings, and functions which misc PowerPC 
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
 * $Id: ppc_registers.h,v 1.7 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__PPC_REGISTERS_H__
#define __ARCH__POWERPC__PPC_REGISTERS_H__

/*  SPR encodings for mfspr
 */
#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18
#define SPR_DAR		19
#define SPR_DEC		22
#define SPR_SDR1	25
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_ASR		280
#define SPR_EAR		282
#define SPR_PVR		287
#define SPR_IBAT0U	528
#define SPR_IBAT0L	529
#define SPR_IBAT1U	530
#define SPR_IBAT1L	531
#define SPR_IBAT2U	532
#define SPR_IBAT2L	533
#define SPR_IBAT3U	534
#define SPR_IBAT3L	535
#define SPR_DBAT0U	536
#define SPR_DBAT0L	537
#define SPR_DBAT1U	538
#define SPR_DBAT1L	539
#define SPR_DBAT2U	540
#define SPR_DBAT2L	541
#define SPR_DBAT3U	542
#define SPR_DBAT3L	543
#define SPR_DABR	1013

#ifndef ASSEMBLY

INLINE word_t ppc_get_sprg( const int which )
{
    word_t val;
    asm volatile( "mfsprg %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc_set_sprg( const int which, word_t val )
{
    asm volatile( "mtsprg %0, %1" : : "i" (which), "r" (val) );
}

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

INLINE word_t ppc_get_sdr1( void )
{
    word_t val;
    asm volatile( "mfsdr1 %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_sdr1( word_t val )
{
    asm volatile( "mtsdr1 %0" : : "r" (val) );
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

INLINE word_t ppc_get_tbl( void )
{
    word_t val;
    asm volatile( "mftbl %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_tbl( word_t val )
{
    asm volatile( "mttbl %0" : : "r" (val) );
}

INLINE word_t ppc_get_tbu( void )
{
    word_t val;
    asm volatile( "mftbu %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_tbu( word_t val )
{
    asm volatile( "mttbu %0" : : "r" (val) );
}

INLINE word_t ppc_get_dabr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 1013" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dec( void )
{
    word_t val;
    asm volatile( "mfdec %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_dec( word_t val )
{
    asm volatile( "mtdec %0" : : "r" (val) );
}

INLINE word_t ppc_get_ear( void )
{
    word_t val;
    asm volatile( "mfspr %0, 282" : "=r" (val) );
    return val;
}

#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC__PPC_REGISTERS_H__ */

