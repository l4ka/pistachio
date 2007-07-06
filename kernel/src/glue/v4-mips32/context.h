/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/context.h
 * Description:   Context save/restore for MIPS32
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
 * $Id: context.h,v 1.1 2006/02/23 21:07:40 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__CONTEXT_H__
#define __GLUE__V4_MIPS32__CONTEXT_H__

#if !defined(ASSEMBLY)

extern "C" void mips32_return_from_notify0 (void);
extern "C" void mips32_return_from_notify1 (void);
extern "C" void mips32_return_from_notify2 (void);
extern "C" void mips32_return_to_user (void);


class mips32_switch_stack_t {
  public:
    word_t  s0; /* 0 */
    word_t  s1; /* 4 */
    word_t  s8; /* 8 */
    word_t  gp; /* 12 */
    word_t  ra; /* 16 */
};

#define MIPS32_SWITCH_STACK_SIZE (5*4)


/* must match #defines below */
class mips32_irq_context_t {
public:
    word_t	epc;    /*-0-*/
    word_t	sp;	/* 4 */ 
    word_t	status; /* 8 */
    word_t	ra;     /* 12 */
    word_t	v0;	/*-16-*/
    word_t	v1;     /* 20 */
    word_t	a1;     /* 24 */
    word_t	cause;  /* 28 */
    word_t	a2;     /*-32-*/
    word_t	a3;	/* 36 */
    word_t	t9;     /* 40 */
    word_t	a0;     /* 44 */
    word_t	gp;     /*-48-*/
    word_t	at;     /* 52 */
    word_t	t0;     /* 56 */
    word_t	t1;     /* 60 */
    word_t	hi;     /*-64-*/
    word_t	t2;     /* 68 */
    word_t	t3;     /* 72 */
    word_t	lo;     /* 76 */
    word_t	t4;     /*-80-*/
    word_t	t5;     /* 84 */
    word_t	t6;     /* 88 */
    word_t	t7;     /* 92 */
    word_t	t8;     /*-96-*/
    word_t	s0;     /* 100 */
    word_t	s1;     /* 104 */
    word_t	s2;     /* 108 */
    word_t	s3;     /*-112-*/
    word_t	s4;     /* 116 */
    word_t	s5;     /* 120 */
    word_t	s6;     /* 124 */
    word_t	s7;     /*-128-*/
    word_t	s8;     /* 132 */
    word_t	x1;	/* 136 */   /* fills to make frame cache aligned */
    word_t	x2;	/* 140 */
};

#else

/* Context save / restore : check, this is cache optimised */

#include INC_ARCH(regdef.h)
#include INC_ARCH(cp0regs.h)

#define MIPS32_SWITCH_STACK_SIZE (5*4)

/**** Switch ****/

/* If changing this, modify class above */

#define SAVE_SWITCH_STACK			\
/* Save the Callee-saved registers:	*/	\
/* s0..s2 ($16..$18)			*/	\
/* gp     ($28)				*/	\
/* s8     ($30)                         */	\
/* ra     ($31)				*/	\
    subu    sp, sp, MIPS32_SWITCH_STACK_SIZE;	\
    sw	    s0, 0(sp);				\
    sw	    s1, 4(sp);				\
    sw	    s8, 8(sp);				\
    sw	    gp, 12(sp);				\
    sw	    ra, 16(sp);

#define RESTORE_SWITCH_STACK			\
    lw	    ra, 16(sp);				\
    lw	    s0, 0(sp);				\
    lw	    s1, 4(sp);				\
    lw	    s8, 8(sp);				\
    lw	    gp, 12(sp);				\
    addiu    sp, sp, MIPS32_SWITCH_STACK_SIZE;

/**** Full Context ****/


#define PT_EPC		0	
#define PT_SP		4	
#define PT_STATUS	8	
#define PT_RA		12
#define PT_V0		16
#define PT_V1		20
#define PT_A1		24
#define PT_CAUSE	28
#define PT_A2		32		
#define PT_A3		36
#define PT_T9		40
#define PT_A0		44
#define PT_GP		48
#define PT_AT		52
#define PT_T0		56
#define PT_T1		60
#define PT_HI		64
#define PT_T2		68
#define PT_T3		72
#define PT_LO		76
#define PT_T4		80
#define PT_T5		84
#define PT_T6		88
#define PT_T7		92
#define PT_T8		96
#define PT_S0		100
#define PT_S1		104
#define PT_S2		108
#define PT_S3		112
#define PT_S4		116
#define PT_S5		120
#define PT_S6		124
#define PT_S7		128
#define PT_S8		132
#define PT_X1		136
#define PT_X2		140

#define PT_SIZE		144

/*
 * SAVE_ALL_INT:
 *  * clear ie, exl, erl, um 
 *	    - in kernel mode
 *			* save all registers on current stack, decrease sp by frame size	
 *	    - in user mode
 *			* save all registers on stack saved in K_STACK_BOTTOM and let new sp point there => K_STACK_BOTTOM must point to tcb of current thread
 */  

#define	SAVE_ALL_INT											\
	.set	push;											\
	.set	reorder;										\
	.set	noat;											\
	mfc0	k1, CP0_STATUS;                 /* get STATUS register k1 */				\
	li	k0, 0xffffffe0;                 /* clear IE, EXL, ERL, UM */				\
	and	k0, k0, k1;										\
	mtc0	k0, CP0_STATUS;                 /* Enter kernel mode */					\
	andi	k0, k1, 0x10;                   /* Isolate UM bit */					\
													\
	.set	noreorder;										\
	beq	k0, zero, 8f;                   /* Branch if from KERNEL mode */			\
	lui	k0, %hi(K_STACK_BOTTOM);								\
													\
													\
	.set	reorder;                        /* save those registers for a sec */			\
	sw	t2, %lo(K_TEMP2)(k0);           /* to use them temporarily...	  */			\
	sw	t0, %lo(K_TEMP0)(k0);									\
	sw	t1, %lo(K_TEMP1)(k0);									\
	sw	t3, %lo(K_TEMP3)(k0);									\
	sw	t4, %lo(K_TEMP4)(k0);									\
													\
	mfc0	t2, CP0_EPC;										\
	mfc0	t3, CP0_CAUSE;										\
	mfc0	t4, CP0_BADVADDR;									\
	lw	t0, %lo(K_STACK_BOTTOM)(k0);	/* Load saved stack */					\
													\
	move	t1, k1;				/* Save some stuff on kernel stack: */			\
	sw	t1, PT_STATUS-PT_SIZE(t0);	/* Save status */					\
	sw	sp, PT_SP-PT_SIZE(t0);		/* Save old stack */					\
	sw	t2, PT_EPC-PT_SIZE(t0);		/* Save EPC */						\
	sw	t3, PT_CAUSE-PT_SIZE(t0);	/* Save CAUSE */					\
	mtc0	t4, CP0_BADVADDR;		/* whatever :/ */					\
	sub	sp, t0, PT_SIZE;		/* New stack pointer = kernel stack */			\
													\
													\
	lui	k0, %hi(K_STACK_BOTTOM);								\
	lw	t0, %lo(K_TEMP0)(k0);		/* and restore those registers... */			\
	lw	t1, %lo(K_TEMP1)(k0);		/* to save them later */				\
	lw	t2, %lo(K_TEMP2)(k0);									\
	lw	t3, %lo(K_TEMP3)(k0);									\
	lw	t4, %lo(K_TEMP4)(k0);									\
	b	9f;											\
8:;													\
	sw	t3, %lo(K_TEMP3)(k0);									\
	sw	t2, %lo(K_TEMP2)(k0);									\
	sw	t1, %lo(K_TEMP1)(k0);									\
	sw	t0, %lo(K_TEMP0)(k0);									\
	lui	t2, 0x8000;										\
	and	t1, sp, t2;										\
	beq	t1, t2, 7f;										\
	/*li	t2, 4;		*/									\
	/*sll	t2, 60;		*/									\
	/*and	t1, sp, t2;	*/									\
	/*beq	t1, t2, 7f;	*/									\
	li	AT, 2;											\
	break;												\
7:													\
	mfc0	t2, CP0_EPC;										\
	mfc0	t3, CP0_CAUSE;										\
	mfc0	t0, CP0_BADVADDR;									\
	move	t1, k1;											\
	sw	t1, PT_STATUS-PT_SIZE(sp);	/* Save status */					\
	sw	t3, PT_CAUSE-PT_SIZE(sp);	/* Save CAUSE */					\
	sw	sp, PT_SP-PT_SIZE(sp);		/* Save old stack */					\
	sub	sp, sp, PT_SIZE;		/* New stack pointer */					\
	sw	t2, PT_EPC(sp);			/* Save EPC */						\
	mtc0	t0, CP0_BADVADDR;									\
													\
	lui	k0, %hi(K_STACK_BOTTOM);	/* restore stuff */					\
	lw	t0, %lo(K_TEMP0)(k0);									\
	lw	t1, %lo(K_TEMP1)(k0);									\
	lw	t2, %lo(K_TEMP2)(k0);									\
	lw	t3, %lo(K_TEMP3)(k0);									\
9:;													\
	sw	ra, PT_RA(sp);										\
	sw	v0, PT_V0(sp);										\
	sw	v1, PT_V1(sp);										\
	sw	a1, PT_A1(sp);										\
	sw	a2, PT_A2(sp);										\
	sw	a3, PT_A3(sp);										\
	sw	t9, PT_T9(sp);										\
	sw	a0, PT_A0(sp);										\
	sw	gp, PT_GP(sp);										\
	sw	$1, PT_AT(sp);										\
	.set	at;											\
	mfhi	v0;											\
	sw	t0, PT_T0(sp);										\
	sw	t1, PT_T1(sp);										\
	sw	v0, PT_HI(sp);										\
	mflo	v1;											\
	sw	t2, PT_T2(sp);										\
	sw	t3, PT_T3(sp);										\
	sw	v1, PT_LO(sp);										\
	sw	t4, PT_T4(sp);										\
	sw	t5, PT_T5(sp);										\
	sw	t6, PT_T6(sp);										\
	sw	t7, PT_T7(sp);										\
	sw	t8, PT_T8(sp);										\
	sw	s0, PT_S0(sp);										\
	sw	s1, PT_S1(sp);										\
	sw	s2, PT_S2(sp);										\
	sw	s3, PT_S3(sp);										\
	sw	s4, PT_S4(sp);										\
	sw	s5, PT_S5(sp);										\
	sw	s6, PT_S6(sp);										\
	sw	s7, PT_S7(sp);										\
	sw	s8, PT_S8(sp);



#define RESTORE_ALL												\
	.set	push;												\
	.set	reorder;											\
	lw	a2, PT_A2(sp);											\
	lw	a3, PT_A3(sp);											\
	lw	t9, PT_T9(sp);											\
	lw	a0, PT_A0(sp);											\
	lw	gp, PT_GP(sp);											\
	.set	noat;												\
	lw	$1, PT_AT(sp);											\
	lw	t0, PT_T0(sp);											\
	lw	t1, PT_T1(sp);											\
	lw	s0, PT_HI(sp);											\
	lw	t2, PT_T2(sp);											\
	lw	t3, PT_T3(sp);											\
	lw	s1, PT_LO(sp);											\
	mthi	s0;												\
	lw	t4, PT_T4(sp);											\
	lw	t5, PT_T5(sp);											\
	lw	t6, PT_T6(sp);											\
	mtlo	s1;												\
	lw	t7, PT_T7(sp);											\
	lw	t8, PT_T8(sp);											\
	lw	s0, PT_S0(sp);											\
	lw	s1, PT_S1(sp);											\
	lw	s2, PT_S2(sp);											\
	lw	s3, PT_S3(sp);											\
	lw	s4, PT_S4(sp);											\
	lw	s5, PT_S5(sp);											\
	lw	s6, PT_S6(sp);											\
	lw	s7, PT_S7(sp);											\
	lw	s8, PT_S8(sp);											\
														\
	mfc0	a1, CP0_STATUS;			/* Status in v1 */						\
	lw	v1, PT_EPC(sp);			/* is out of order bad for cache? (still same cache line?) */	\
	lw	v0, PT_STATUS(sp);										\
	/* XXX - NOTE, Status register updates are not ATOMIC!!!, Interrupt Mask bits can change */		\
	ori	a1, a1, 0x2;			/* set Exception Level */					\
	mtc0	a1, CP0_STATUS;			/* to disable interrupts, we now can set EPC */			\
	li	ra, 0x0fffffe0;			/* compute new status register */				\
														\
	and	a1, ra, a1;			/* a1 = current STATUS & 0xe0 */				\
	nor	ra, zero, ra;			/* ra = 0xf000001f */						\
	and	v0, ra, v0;			/* v0 = last 5 bit of saved STATUS */				\
	lw	ra, PT_RA(sp);											\
	or	k1, a1, v0;			/* k1 = current STATUS with end of saves STATUS */		\
	mtc0	v1, CP0_EPC;			/* restore EPC */						\
	lw	v0, PT_V0(sp);											\
	srl	k0, sp, 12;											\
	lw	v1, PT_V1(sp);											\
	lw	a1, PT_A1(sp);											\
	lw	sp, PT_SP(sp);			/* restore stack */						\
	mtc0	k1, CP0_STATUS;			/* new status value */						\
	sll	k0, k0, 12;			/* Get TCB pointer */						\
	lw	k0, OFS_TCB_MYSELF_LOCAL(k0);	/* Load UTCB into k0 */						\
	nop

#endif


#endif /* !__GLUE__V4_MIPS32__CONTEXT_H__ */
