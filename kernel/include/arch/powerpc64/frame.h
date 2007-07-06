/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	platform/powerpc64/frame.h
 * Description:	Exception state structure, specific to the calling conventions
 * 		for powerpc64.
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
 * $Id: frame.h,v 1.6 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__FRAME_H__
#define __ARCH__POWERPC64__FRAME_H__

/* Stack offsets in units of word_t, relative to the top of the stack. 
 * It is important that the general purpose registers occupy consecutive
 * memory locations (starting at r3), with lower addresses for lower registers,
 * to support the multi-store and multi-load instructions (stmw, lmw).
 */
#define KSTACK_DSISR		(-40)
#define KSTACK_DAR		(-39)
#define KSTACK_XER		(-38)
#define KSTACK_CR		(-37)
#define KSTACK_CTR		(-36)
#define KSTACK_R0		(-35)
#define KSTACK_R3		(-34)
#define KSTACK_R4		(-33)
#define KSTACK_R5		(-32)
#define KSTACK_R6		(-31)
#define KSTACK_R7		(-30)
#define KSTACK_R8		(-29)
#define KSTACK_R9		(-28)
#define KSTACK_R10		(-27)
#define KSTACK_R11		(-26)
#define KSTACK_R12		(-25)
#define KSTACK_R13		(-24)
#define KSTACK_R14		(-23)
#define KSTACK_R15		(-22)
#define KSTACK_R16		(-21)
#define KSTACK_R17		(-20)
#define KSTACK_R18		(-19)
#define KSTACK_R19		(-18)
#define KSTACK_R20		(-17)
#define KSTACK_R21		(-16)
#define KSTACK_R22		(-15)
#define KSTACK_R23		(-14)
#define KSTACK_R24		(-13)
#define KSTACK_R25		(-12)
#define KSTACK_R26		(-11)
#define KSTACK_R27		(-10)
#define KSTACK_R28		(-9)
#define KSTACK_R29		(-8)
#define KSTACK_R30		(-7)
#define KSTACK_R31		(-6)
#define KSTACK_R1		(-5)
#define KSTACK_R2		(-4)
#define KSTACK_LR		(-3)
#define KSTACK_SRR0		(-2)
#define KSTACK_SRR1		(-1)

/* The stack bottoms must be 8-byte aligned!
 */ 
#define KSTACK_FRAME_BOTTOM	KSTACK_DSISR

/* Save space on the stack for system calls, and only allocate space for
 * some of the registers.  The kernel had better not try to access the
 * other registers saved in a normal exception frame!  If the kernel must,
 * then change SCSTACK_FRAME_BOTTOM to equal KSTACK_FRAME_BOTTOM.
 */
#define SCSTACK_FRAME_BOTTOM	KSTACK_R29

/* Convert the stack offset into a word_t array index. */
#define FRAME_IDX(a)		((a) - KSTACK_FRAME_BOTTOM)
#define SC_FRAME_IDX(a)		((a) - SCSTACK_FRAME_BOTTOM)

/* Convert the KSTACK word_t offset (which is relative to the top of the stack),
 * into a byte offset relative to the bottom of the stack.
 */
#define OF_BASE 48	/* Preserve room for LR + the back chain when
			 * calling kernel C code. */
#define KOFF(a)		(8*FRAME_IDX(a) + OF_BASE)
#define SCOFF(a)	(8*SC_FRAME_IDX(a) + OF_BASE)

#define EXC_STACK_SIZE	KOFF(0)
#define SC_STACK_SIZE	SCOFF(0)
#define RTAS_FRAME_SIZE EXC_STACK_SIZE

/* Byte offsets, relative to the bottom of the exception frame.
 */
#define PT_R0	KOFF(KSTACK_R0)
#define PT_R1	KOFF(KSTACK_R1)
#define PT_R2	KOFF(KSTACK_R2)
#define PT_R3	KOFF(KSTACK_R3)
#define PT_R4	KOFF(KSTACK_R4)
#define PT_R5	KOFF(KSTACK_R5)
#define PT_R6	KOFF(KSTACK_R6)
#define PT_R7	KOFF(KSTACK_R7)
#define PT_R8	KOFF(KSTACK_R8)
#define PT_R9	KOFF(KSTACK_R9)
#define PT_R10	KOFF(KSTACK_R10)
#define PT_R11	KOFF(KSTACK_R11)
#define PT_R12	KOFF(KSTACK_R12)
#define PT_R13	KOFF(KSTACK_R13)
#define PT_R14	KOFF(KSTACK_R14)
#define PT_R15	KOFF(KSTACK_R15)
#define PT_R16	KOFF(KSTACK_R16)
#define PT_R17	KOFF(KSTACK_R17)
#define PT_R18	KOFF(KSTACK_R18)
#define PT_R19	KOFF(KSTACK_R19)
#define PT_R20	KOFF(KSTACK_R20)
#define PT_R21	KOFF(KSTACK_R21)
#define PT_R22	KOFF(KSTACK_R22)
#define PT_R23	KOFF(KSTACK_R23)
#define PT_R24	KOFF(KSTACK_R24)
#define PT_R25	KOFF(KSTACK_R25)
#define PT_R26	KOFF(KSTACK_R26)
#define PT_R27	KOFF(KSTACK_R27)
#define PT_R28	KOFF(KSTACK_R28)
#define PT_R29	KOFF(KSTACK_R29)
#define PT_R30	KOFF(KSTACK_R30)
#define PT_R31	KOFF(KSTACK_R31)
#define PT_LR	KOFF(KSTACK_LR)
#define PT_XER	KOFF(KSTACK_XER)
#define PT_CR	KOFF(KSTACK_CR)
#define PT_CTR	KOFF(KSTACK_CTR)
#define PT_SRR0	KOFF(KSTACK_SRR0)
#define PT_SRR1	KOFF(KSTACK_SRR1)
#define PT_DSISR KOFF(KSTACK_DSISR)
#define PT_DAR	KOFF(KSTACK_DAR)

#define SC_PT_R1	SCOFF(KSTACK_R1)
#define SC_PT_R2	SCOFF(KSTACK_R2)
#define SC_PT_R30	SCOFF(KSTACK_R30)
#define SC_PT_R31	SCOFF(KSTACK_R31)
#define SC_PT_SRR0	SCOFF(KSTACK_SRR0)
#define SC_PT_SRR1	SCOFF(KSTACK_SRR1)
#define SC_PT_LR	SCOFF(KSTACK_LR)

/* CPU Local Spill Area */

#define LOCAL_R1	0
#define LOCAL_SRR0	8
#define LOCAL_R2	16
#define LOCAL_R3	24
#define LOCAL_MSR	32

/* XXX - above 64 bytes - see init code */
#define RESET_R1	40
#define RESET_MSR	48
#define RESET_R2	56
#define RESET_R3	64
#define RESET_SRR0	72
#define RESET_TCB	80

#if CONFIG_POWERPC64_STAB
#define LOCAL_VSID_ASID	96
#endif

#if !defined(ASSEMBLY)

extern "C" char powerpc64_do_notify[];

/* This must be a multiple of 16 bytes */
class powerpc64_switch_stack_t {
public:
    word_t	back_chain;
    word_t	cr_save;
    word_t	lr_save;    /* Return address		*/
    word_t	temp0;	    /* Compiler double word	*/
    word_t	temp1;	    /* Link editor double word	*/
    word_t	temp2;	    /* TOC save area		*/
    word_t	temp3;	    /* Parameter save area	*/
    word_t	temp4;	    /* Local variable		*/
};

/* PowerPC64 Stack format
          +-> Back chain
          |   Floating point register save area
          |   General register save area
          |   Local variable space
          |   Parameter save area    (SP + 48)
          |   TOC save area          (SP + 40)
          |   link editor doubleword (SP + 32)
          |   compiler doubleword    (SP + 24)
          |   LR save area           (SP + 16)
          |   CR save area           (SP + 8)
SP  --->  +-- Back chain             (SP + 0)
*/

/* must match #defines above */
class powerpc64_irq_context_t
{
public:
    char	room[OF_BASE];
    word_t	dsisr;	/* 0  */
    word_t	dar;	/* 8  */ 
    word_t	xer;	/* 16 */
    word_t	cr;	/* 24 */
    word_t	ctr;	/* 32 */
    word_t	r0;	/* 40 */
    word_t	r3;	/* 48 */
    word_t	r4;	/* 56 */
    word_t	r5;	/* 64 */
    word_t	r6;	/* 72 */
    word_t	r7;	/* 80 */
    word_t	r8;	/* 88 */
    word_t	r9;	/* 96 */
    word_t	r10;	/* 104 */
    word_t	r11;	/* 112 */
    word_t	r12;	/* 120 */
    word_t	r13;	/* 128 */
    word_t	r14;	/* 136 */
    word_t	r15;	/* 144 */
    word_t	r16;	/* 152 */
    word_t	r17;	/* 160 */
    word_t	r18;	/* 168 */
    word_t	r19;	/* 176 */
    word_t	r20;	/* 184 */
    word_t	r21;	/* 192 */
    word_t	r22;	/* 200 */
    word_t	r23;	/* 208 */
    word_t	r24;	/* 216 */
    word_t	r25;	/* 224 */
    word_t	r26;	/* 232 */
    word_t	r27;	/* 240 */
    word_t	r28;	/* 248 */
    word_t	r29;	/* 256 */
    word_t	r30;	/* 264 */
    word_t	r31;	/* 272 */
    word_t	r1;	/* 280 */
    word_t	r2;	/* 288 */
    word_t	lr;	/* 296 */
    word_t	srr0;	/* 304 */
    word_t	srr1;	/* 312 */
};


#endif /* !defined(ASSEMBLY) */

#endif	/* __ARCH__POWERPC64__FRAME_H__ */
