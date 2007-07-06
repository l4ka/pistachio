/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/switch.S
 * Description:	PowerPC64 switch_to and notify code.
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
 * $Id: switch.S,v 1.4 2004/06/04 03:40:20 cvansch Exp $
 *
 ***************************************************************************/


#include INC_ARCH(vectors.h)
#include INC_ARCH(asm.h)
#include INC_ARCH(frame.h)


	.section ".text", "ax"
BEGIN_PROC(powerpc64_do_notify)
	mr	r3, r30		/* Set argument 1   */
	mr	r4, r31		/* Set argument 2   */
	mtlr	r13		/* Load function address	*/
	blrl			/* Jump to function */
	nop
	ld	r2, 24(r1)	/* Restore next frame	*/
	ld	r13, 32(r1)
	ld	r30, 40(r1)
	ld	r31, 48(r1)
	addi	r1, r1, 64	/* Switch stack size */
	ld	r3, 16(r1)
	mtlr	r3
	blr
	
	.long 0x00000001
	
END_PROC(powerpc64_do_notify)

#define RESTORE_INITIAL			\
	li	r2, 0;			\
	mtdsisr	r2;			\
	mtdar	r2;			\
	mtxer	r2;			\
	mtcr	r2;			\
	mr	r0, r2;			\
	mr	r3, r2;			\
	mr	r4, r2;			\
	mr	r5, r2;			\
	mr	r6, r2;			\
	mr	r7, r2;			\
	mr	r8, r2;			\
	mr	r9, r2;			\
	mr	r10, r2;		\
	mr	r11, r2;		\
	mr	r12, r2;		\
	ld	r13, PT_R13(r1);	\
	mr	r14, r2;		\
	mr	r15, r2;		\
	mr	r16, r2;		\
	mr	r17, r2;		\
	mr	r18, r2;		\
	mr	r19, r2;		\
	mr	r20, r2;		\
	mr	r21, r2;		\
	mr	r22, r2;		\
	mr	r23, r2;		\
	mr	r24, r2;		\
	mr	r25, r2;		\
	mr	r26, r2;		\
	mr	r27, r2;		\
	mr	r28, r2;		\
	mr	r29, r2;		\
	mr	r30, r2;		\
	mr	r31, r2;		\
	mtlr	r2;			\
	ld	r2, PT_SRR0(r1);	\
	mtsrr0	r2;			\
	ld	r2, PT_SRR1(r1);	\
	mtsrr1	r2;			\
	li	r2, 0;			\
	ld	r1, PT_R1(r1);

/* Jump to user space. */
	.section ".text", "ax"
BEGIN_PROC(powerpc64_initial_to_user)
    RESTORE_INITIAL
    rfid
END_PROC(powerpc64_initial_to_user)

