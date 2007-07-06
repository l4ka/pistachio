/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     arch/powerpc64/asm.h
 * Created:       Carl van Schaik
 * Description:   Assembler macros etc. 
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
 * $Id: asm.h,v 1.5 2004/12/09 01:01:54 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __L4__POWERPC64__ASM_H__
#define __L4__POWERPC64__ASM_H__

#define XGLUE(a,b) a##b
#define GLUE(a,b) XGLUE(a,b)

#define BEGIN_PROC(name)			\
    .global name;				\
    .global GLUE(.,name);			\
    .align 3;					\
    .section ".opd","aw";			\
name:						\
    .quad GLUE(.,name);				\
    .quad .TOC.@tocbase;			\
    .quad 0;					\
    .previous;					\
    .type GLUE(.,name),@function;		\
    .func GLUE(.,name);				\
GLUE(.,name):

#define STATIC_PROC(name)			\
    .align 3;					\
    .section ".opd","aw";			\
name:						\
    .quad GLUE(.,name);				\
    .quad .TOC.@tocbase;			\
    .quad 0;					\
    .previous;					\
    .type GLUE(.,name),@function;		\
GLUE(.,name):

#define END_PROC(name)				\
    .endfunc


/*
 * LD_ADDR ( reg, symbol )
 *   loads the address of symbol into reg
 */
#define	LD_ADDR(reg, symbol)		\
    lis	    reg, symbol##@highest;	\
    ori	    reg, reg, symbol##@higher;	\
    rldicr  reg, reg, 32,31;		\
    oris    reg, reg, symbol##@h;	\
    ori	    reg, reg, symbol##@l

#define LD_CONST(reg, value)			\
    lis	    reg,(((value)>>48)&0xFFFF);		\
    ori     reg,reg,(((value)>>32)&0xFFFF);	\
    rldicr  reg,reg,32,31;			\
    oris    reg,reg,(((value)>>16)&0xFFFF);	\
    ori     reg,reg,((value)&0xFFFF);

#define LD_LABEL(reg, label)		\
    lis     reg,(label)@highest;        \
    ori     reg,reg,(label)@higher;     \
    rldicr  reg,reg,32,31;              \
    oris    reg,reg,(label)@h;          \
    ori     reg,reg,(label)@l;

/* Condition Register Bit Fields */
#define	cr0	0
#define	cr1	1
#define	cr2	2
#define	cr3	3
#define	cr4	4
#define	cr5	5
#define	cr6	6
#define	cr7	7


/* General Purpose Registers (GPRs) */
#define	r0	0
#define	r1	1
#define	r2	2
#define	r3	3
#define	r4	4
#define	r5	5
#define	r6	6
#define	r7	7
#define	r8	8
#define	r9	9
#define	r10	10
#define	r11	11
#define	r12	12
#define	r13	13
#define	r14	14
#define	r15	15
#define	r16	16
#define	r17	17
#define	r18	18
#define	r19	19
#define	r20	20
#define	r21	21
#define	r22	22
#define	r23	23
#define	r24	24
#define	r25	25
#define	r26	26
#define	r27	27
#define	r28	28
#define	r29	29
#define	r30	30
#define	r31	31


/* Floating Point Registers (FPRs) */
#define	fr0	0
#define	fr1	1
#define	fr2	2
#define	fr3	3
#define	fr4	4
#define	fr5	5
#define	fr6	6
#define	fr7	7
#define	fr8	8
#define	fr9	9
#define	fr10	10
#define	fr11	11
#define	fr12	12
#define	fr13	13
#define	fr14	14
#define	fr15	15
#define	fr16	16
#define	fr17	17
#define	fr18	18
#define	fr19	19
#define	fr20	20
#define	fr21	21
#define	fr22	22
#define	fr23	23
#define	fr24	24
#define	fr25	25
#define	fr26	26
#define	fr27	27
#define	fr28	28
#define	fr29	29
#define	fr30	30
#define	fr31	31

/*
 * Register names
 */
#define	sp	r1
#define	toc	r2
#define	utcb	r13

#endif /* __L4__POWERPC64__ASM_H__ */
