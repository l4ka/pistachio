/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/context.h
 * Description:   Various context management classes
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
 * $Id: context.h,v 1.26 2004/12/02 00:05:27 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_MIPS64__CONTEXT_H__
#define __GLUE__V4_MIPS64__CONTEXT_H__

#include INC_PLAT(config.h)

#if !defined(ASSEMBLY)

#include <asid.h>

/* These functions deal with switching to a thread (and saving its context), and
 * the associated notification functions */

extern "C" void mips64_return_from_notify0 (void);
extern "C" void mips64_return_from_notify1 (void);
extern "C" void mips64_return_from_notify2 (void);
extern "C" void mips64_return_to_user (void);

class mips64_switch_stack_t {
public:
    word_t  s0; /* 0 */
    word_t  s1; /* 8 */
    word_t  s8; /* 16 */
    word_t  gp; /* 24 */
    word_t  ra; /* 32 */
};

#define MIPS64_SWITCH_STACK_SIZE (5*8)

/* must match #defines below */
class mips64_irq_context_t
{
public:
    word_t	epc;    /*-0 -*/
    word_t	sp;	/* 8  */ 
    word_t	status; /* 16 */
    word_t	ra;     /* 24 */
    word_t	v0;	/*-32-*/
    word_t	v1;     /* 40 */
    word_t	a1;     /* 48 */
    word_t	cause;  /* 56 */
    word_t	a2;     /*-64-*/
    word_t	a3;	/* 72 */
    word_t	t9;     /* 80 */
    word_t	a0;     /* 88 */
    word_t	gp;     /*-96-*/
    word_t	at;     /* 104 */
    word_t	t0;     /* 112 */
    word_t	t1;     /* 120 */
    word_t	hi;     /*-128-*/
    word_t	t2;     /* 136 */
    word_t	t3;     /* 144 */
    word_t	lo;     /* 152 */
    word_t	t4;     /*-160-*/
    word_t	t5;     /* 168 */
    word_t	t6;     /* 176 */
    word_t	t7;     /* 184 */
    word_t	t8;     /*-192-*/
    word_t	s0;     /* 200 */
    word_t	s1;     /* 208 */
    word_t	s2;     /* 216 */
    word_t	s3;     /*-224-*/
    word_t	s4;     /* 232 */
    word_t	s5;     /* 240 */
    word_t	s6;     /* 248 */
    word_t	s7;     /*-256-*/
    word_t	s8;     /* 264 */
    word_t	badvaddr;   /* 272 */
    word_t	x1;	/* 280 */   /* fills to make frame cache aligned */
};

#else

/* Context save / restore : check, this is cache optimised */

#include INC_ARCH(regdef.h)
#include INC_ARCH(mipsregs.h)

/**** Switch ****/

/* If changing this, modify class above */
#define SAVE_SWITCH_STACK			\
/* Save the Callee-saved registers:	*/	\
/* s0..s2 ($16..$18)			*/	\
/* gp     ($28)				*/	\
/* s8     ($30)                         */	\
/* ra     ($31)				*/	\
    dsubu   sp, sp, MIPS64_SWITCH_STACK_SIZE;	\
    sd	    s0, 0(sp);				\
    sd	    s1, 8(sp);				\
    sd	    s8, 16(sp);				\
    sd	    gp, 24(sp);				\
    sd	    ra, 32(sp);

#define RESTORE_SWITCH_STACK			\
    ld	    ra, 32(sp);				\
    ld	    s0, 0(sp);				\
    ld	    s1, 8(sp);				\
    ld	    s8, 16(sp);				\
    ld	    gp, 24(sp);				\
    daddu   sp, sp, MIPS64_SWITCH_STACK_SIZE;

/**** Full Context ****/

#define PT_EPC		0
#define PT_SP		8
#define PT_STATUS	16
#define PT_RA		24
#define PT_V0		32
#define PT_V1		40
#define PT_A1		48
#define PT_CAUSE	56
#define PT_A2		64
#define PT_A3		72
#define PT_T9		80
#define PT_A0		88
#define PT_GP		96
#define PT_AT		104
#define PT_T0		112
#define PT_T1		120
#define PT_HI		128
#define PT_T2		136
#define PT_T3		144
#define PT_LO		152
#define PT_T4		160
#define PT_T5		168
#define PT_T6		176
#define PT_T7		184
#define PT_T8		192
#define PT_S0		200
#define PT_S1		208
#define PT_S2		216
#define PT_S3		224
#define PT_S4		232
#define PT_S5		240
#define PT_S6		248
#define PT_S7		256
#define PT_S8		264
#define PT_BADVADDR	272
#define PT_X1		280

#define PT_SIZE		288

#define	SAVE_ALL_INT			\
	.set	push;			\
	.set	reorder;		\
	.set	noat;			\
	mfc0	k1, CP0_STATUS;		/* get STATUS register k1 */	\
	li	k0, 0xffffffffffffffe0;	/* clear IE, EXL, ERL, KSU */	\
	and	k0, k0, k1;		\
	mtc0	k0, CP0_STATUS;		/* Enter kernel mode */		\
	andi	k0, k1, ST_KSU;		/* Isolate KSU bits */		\
					\
	.set	noreorder;		\
	beq	k0, zero, 8f;		/* Branch if from KERNEL mode */	\
	lui	k0, %hi(K_STACK_BOTTOM);	\
	.set	reorder;			\
	sd	t2, %lo(K_TEMP2)(k0);		\
	sd	t0, %lo(K_TEMP0)(k0);		\
	sd	t1, %lo(K_TEMP1)(k0);		\
	sd	t3, %lo(K_TEMP3)(k0);		\
	sd	t4, %lo(K_TEMP4)(k0);		\
						\
	dmfc0	t2, CP0_EPC;			\
	mfc0	t3, CP0_CAUSE;			\
	dmfc0	t4, CP0_BADVADDR;		\
	ld	t0, %lo(K_STACK_BOTTOM)(k0);	/* Load saved stack */	\
	move	t1, k1;				\
	sd	t1, PT_STATUS-PT_SIZE(t0);	/* Save status */	\
	sd	sp, PT_SP-PT_SIZE(t0);		/* Save old stack */	\
	sd	t2, PT_EPC-PT_SIZE(t0);		/* Save EPC */		\
	sd	t3, PT_CAUSE-PT_SIZE(t0);	/* Save CAUSE */	\
	sd	t4, PT_BADVADDR-PT_SIZE(t0);	/* Save BADVADDR */	\
	dsub	sp, t0, PT_SIZE;		/* New stack pointer */	\
	lui	k0, %hi(K_STACK_BOTTOM);	\
	ld	t0, %lo(K_TEMP0)(k0);		\
	ld	t1, %lo(K_TEMP1)(k0);		\
	ld	t2, %lo(K_TEMP2)(k0);		\
	ld	t3, %lo(K_TEMP3)(k0);		\
	ld	t4, %lo(K_TEMP4)(k0);		\
	b	9f;				\
8:;						\
	sd	t3, %lo(K_TEMP3)(k0);		\
	sd	t2, %lo(K_TEMP2)(k0);		\
	sd	t1, %lo(K_TEMP1)(k0);		\
	sd	t0, %lo(K_TEMP0)(k0);		\
	lui	t2, 0x8000;			\
	and	t1, sp, t2;			\
	beq	t1, t2, 7f;			\
	li	t2, 4;				\
	dsll	t2, 60;				\
	and	t1, sp, t2;			\
	beq	t1, t2, 7f;			\
	li	AT, 2;				\
	break;					\
7:						\
	dmfc0	t2, CP0_EPC;			\
	mfc0	t3, CP0_CAUSE;			\
	dmfc0	t0, CP0_BADVADDR;		\
	move	t1, k1;				\
	sd	t1, PT_STATUS-PT_SIZE(sp);	/* Save status */	\
	sd	t3, PT_CAUSE-PT_SIZE(sp);	/* Save CAUSE */	\
	sd	sp, PT_SP-PT_SIZE(sp);		/* Save old stack */	\
	sd	t0, PT_BADVADDR-PT_SIZE(sp);	/* Save BADVADDR */	\
	dsub	sp, sp, PT_SIZE;		/* New stack pointer */	\
	sd	t2, PT_EPC(sp);			/* Save EPC */		\
	lui	k0, %hi(K_STACK_BOTTOM);	\
	ld	t0, %lo(K_TEMP0)(k0);		\
	ld	t1, %lo(K_TEMP1)(k0);		\
	ld	t2, %lo(K_TEMP2)(k0);		\
	ld	t3, %lo(K_TEMP3)(k0);		\
9:;					\
	sd	ra, PT_RA(sp);		\
	sd	v0, PT_V0(sp);		\
	sd	v1, PT_V1(sp);		\
	sd	a1, PT_A1(sp);		\
	sd	a2, PT_A2(sp);		\
	sd	a3, PT_A3(sp);		\
	sd	t9, PT_T9(sp);		\
	sd	a0, PT_A0(sp);		\
	sd	gp, PT_GP(sp);		\
	sd	$1, PT_AT(sp);		\
	.set	at;			\
	mfhi	v0;			\
	sd	t0, PT_T0(sp);		\
	sd	t1, PT_T1(sp);		\
	sd	v0, PT_HI(sp);		\
	mflo	v1;			\
	sd	t2, PT_T2(sp);		\
	sd	t3, PT_T3(sp);		\
	sd	v1, PT_LO(sp);		\
	sd	t4, PT_T4(sp);		\
	sd	t5, PT_T5(sp);		\
	sd	t6, PT_T6(sp);		\
	sd	t7, PT_T7(sp);		\
	sd	t8, PT_T8(sp);		\
	sd	s0, PT_S0(sp);		\
	sd	s1, PT_S1(sp);		\
	sd	s2, PT_S2(sp);		\
	sd	s3, PT_S3(sp);		\
	sd	s4, PT_S4(sp);		\
	sd	s5, PT_S5(sp);		\
	sd	s6, PT_S6(sp);		\
	sd	s7, PT_S7(sp);		\
	sd	s8, PT_S8(sp);

#define	SAVE_ALL_XTLB		\
	.set	push;		\
	.set	reorder;	\
	.set	noat;		\
	la	sp, XTLB_REFILL_STACK_end - PT_SIZE;	/* Static stack */	\
	sd	ra, PT_RA(sp);	\
	sd	k0, PT_X1(sp);	/* Save UTCB in k0 */		\
	sd	k1, PT_SP(sp);	/* Save old stack in k1 */	\
	dmfc0	k0, CP0_EPC;	\
	sd	v0, PT_V0(sp);	\
	sd	k0, PT_EPC(sp);	\
	sd	v1, PT_V1(sp);	\
	mfc0	k0, CP0_CAUSE;	\
	sd	a1, PT_A1(sp);	\
	sd	a2, PT_A2(sp);	\
	sd	k0, PT_CAUSE(sp);	\
	sd	a3, PT_A3(sp);	\
	sd	t9, PT_T9(sp);	\
	sd	a0, PT_A0(sp);	\
	sd	gp, PT_GP(sp);	\
	sd	$1, PT_AT(sp);	\
	.set	at;		\
	mfhi	v0;		\
	sd	t0, PT_T0(sp);	\
	sd	t1, PT_T1(sp);	\
	sd	v0, PT_HI(sp);	\
	mflo	v1;		\
	sd	t2, PT_T2(sp);	\
	sd	t3, PT_T3(sp);	\
	sd	v1, PT_LO(sp);	\
	sd	t4, PT_T4(sp);	\
	sd	t5, PT_T5(sp);	\
	sd	t6, PT_T6(sp);	\
	sd	t7, PT_T7(sp);	\
	sd	t8, PT_T8(sp);	\
	sd	s0, PT_S0(sp);	\
	sd	s1, PT_S1(sp);	\
	sd	s2, PT_S2(sp);	\
	sd	s3, PT_S3(sp);	\
	sd	s4, PT_S4(sp);	\
	sd	s5, PT_S5(sp);	\
	sd	s6, PT_S6(sp);	\
	sd	s7, PT_S7(sp);	\
	sd	s8, PT_S8(sp);

#define RESTORE_ALL		\
	.set	push;		\
	.set	reorder;	\
	ld	a2, PT_A2(sp);	\
	ld	a3, PT_A3(sp);	\
	ld	t9, PT_T9(sp);	\
	ld	a0, PT_A0(sp);	\
	ld	gp, PT_GP(sp);	\
	.set	noat;		\
	ld	$1, PT_AT(sp);	\
	ld	t0, PT_T0(sp);	\
	ld	t1, PT_T1(sp);	\
	ld	s0, PT_HI(sp);	\
	ld	t2, PT_T2(sp);	\
	ld	t3, PT_T3(sp);	\
	ld	s1, PT_LO(sp);	\
	mthi	s0;		\
	ld	t4, PT_T4(sp);	\
	ld	t5, PT_T5(sp);	\
	ld	t6, PT_T6(sp);	\
	mtlo	s1;		\
	ld	t7, PT_T7(sp);	\
	ld	t8, PT_T8(sp);	\
	ld	s0, PT_S0(sp);	\
	ld	s1, PT_S1(sp);	\
	ld	s2, PT_S2(sp);	\
	ld	s3, PT_S3(sp);	\
	ld	s4, PT_S4(sp);	\
	ld	s5, PT_S5(sp);	\
	ld	s6, PT_S6(sp);	\
	ld	s7, PT_S7(sp);	\
	ld	s8, PT_S8(sp);	\
	\
	mfc0	a1, CP0_STATUS;	/* Status in v1 */	\
	ld	v1, PT_EPC(sp);	/* is out of order bad for cache? (still same cache line?) */\
	ld	v0, PT_STATUS(sp);	\
	/* XXX - NOTE, Status register updates are not ATOMIC!!!, Interrupt Mask bits can change */ \
	ori	a1, a1, ST_EXL; /* set Exception Level */	\
	mtc0	a1, CP0_STATUS;	/* to disable interrupts, we now can set EPC */		\
	li	ra, CONFIG_MIPS64_STATUS_MASK; /* compute new status register */	\
	\
	and	a1, ra, a1;	\
	nor	ra, zero, ra;	\
	and	v0, ra, v0;	\
	ld	ra, PT_RA(sp);	\
	or	k1, a1, v0;	\
	dmtc0	v1, CP0_EPC;	/* restore EPC */	\
	ld	v0, PT_V0(sp);	\
	dsrl	k0, sp, 12;	\
	ld	v1, PT_V1(sp);	\
	ld	a1, PT_A1(sp);	\
	ld	sp, PT_SP(sp);	/* restore stack */	\
	mtc0	k1, CP0_STATUS;	/* new status value */	\
	dsll	k0, k0, 12;	/* Get TCB pointer */	\
	ld	k0, OFS_TCB_MYSELF_LOCAL(k0);	/* Load UTCB into k0 */	\
	nop


#define RESTORE_XTLB		\
	.set	push;		\
	.set	reorder;	\
	ld	a2, PT_A2(sp);	\
	ld	a3, PT_A3(sp);	\
	ld	t9, PT_T9(sp);	\
	ld	a0, PT_A0(sp);	\
	ld	gp, PT_GP(sp);	\
	.set	noat;		\
	ld	$1, PT_AT(sp);	\
	ld	t0, PT_T0(sp);	\
	ld	t1, PT_T1(sp);	\
	ld	s0, PT_HI(sp);	\
	ld	t2, PT_T2(sp);	\
	ld	t3, PT_T3(sp);	\
	ld	s1, PT_LO(sp);	\
	mthi	s0;		\
	ld	t4, PT_T4(sp);	\
	ld	t5, PT_T5(sp);	\
	ld	t6, PT_T6(sp);	\
	mtlo	s1;		\
	ld	t7, PT_T7(sp);	\
	ld	t8, PT_T8(sp);	\
	ld	s0, PT_S0(sp);	\
	ld	s1, PT_S1(sp);	\
	ld	s2, PT_S2(sp);	\
	ld	s3, PT_S3(sp);	\
	ld	s4, PT_S4(sp);	\
	ld	s5, PT_S5(sp);	\
	ld	s6, PT_S6(sp);	\
	ld	s7, PT_S7(sp);	\
	ld	s8, PT_S8(sp);	\
	\
	ld	a1, PT_EPC(sp);	\
	ld	v0, PT_V0(sp);	\
	dmtc0	a1, CP0_EPC;	/* restore EPC */   \
	ld	v1, PT_V1(sp);	\
	ld	ra, PT_RA(sp);	\
	ld	a1, PT_A1(sp);	\
	ld	k0, PT_X1(sp);	/* Restore UTCB in k0 */	\
	ld	sp, PT_SP(sp);	/* restore stack */

#endif

#endif /* __GLUE__V4_MIPS64__CONTEXT_H__ */
