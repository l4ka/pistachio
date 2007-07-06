/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	platform/ofpower3/call.S
 * Description: Calls to Open Firmware and the RTAS
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
 * $Id: call.S,v 1.3 2003/10/22 08:03:43 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(frame.h)
#include <asmsyms.h>


/*
 * On CHRP, the Run-Time Abstraction Services (RTAS) have to be
 * called with the MMU off.
 *
 * In addition, we need to be in 32b mode, at least for now.
 * 
 * Note: r3 is an input parameter to rtas, so don't trash it...
 */

BEGIN_PROC(__call_rtas)
	mflr	r0
	std	r0,16(r1)
        stdu	r1,-RTAS_FRAME_SIZE(r1)	/* Save SP and create stack space. */

	/* Because RTAS is running in 32b mode, it clobbers the high order half
	 * of all registers that it saves.  We therefore save those registers
	 * RTAS might touch to the stack.  (r0, r3-r13 are caller saved)
   	 */

	std	r2, PT_R1(r1)		/* Save the TOC */
	std	r13,PT_R13(r1)		/* Save the UTCB */

	/* Save non-volatile registers */
	std	r14,PT_R14(r1)
	std	r15,PT_R15(r1)
	std	r16,PT_R16(r1)
	std	r17,PT_R17(r1)
	std	r18,PT_R18(r1)
	std	r19,PT_R19(r1)
	std	r20,PT_R20(r1)
	std	r21,PT_R21(r1)
	std	r22,PT_R22(r1)
	std	r23,PT_R23(r1)
	std	r24,PT_R24(r1)
	std	r25,PT_R25(r1)
	std	r26,PT_R26(r1)
	std	r27,PT_R27(r1)
	std	r28,PT_R28(r1)
	std	r29,PT_R29(r1)
	std	r30,PT_R30(r1)
	std	r31,PT_R31(r1)

	/* Save special registers */
	mfcr	r4
	std	r4, PT_CR(r1)
	mfctr	r5
	std	r5, PT_CTR(r1)
	mfxer	r6
	std	r6, PT_XER(r1)
	mfdar	r7
	std	r7, PT_DAR(r1)
	mfdsisr	r8
	std	r8, PT_DSISR(r1)
	mfsrr0	r9
	std	r9, PT_SRR0(r1)
	mfsrr1	r10
	std	r10,PT_SRR1(r1)

	/* Unfortunately, the stack pointer and the MSR are also clobbered,
	 * so they are saved in the cpulocal which allows us to restore
	 * our original state after RTAS returns.
         */
	mfsprg	r4, SPRG_LOCAL
	std	r1, LOCAL_R1(r4)
	mfmsr	r5		/* MSR in r5, used below */
        std	r5, LOCAL_MSR(r4)

	/* Setup the RTAS return addr */
	LD_LABEL (r4,.rtas_return_loc)
	LD_CONST (r9, KERNEL_OFFSET)
	sub	r4, r4,	r9
       	mtlr	r4

	li	r0, 0
	ori	r0, r0,	MSR_EE|MSR_SE|MSR_BE|MSR_RI
	andc	r0, r5, r0		/* Mask MSR (Turn off interrupts) */
	
	/* Load the 64-bit flag */
        li      r9, 1
        rldicr  r9, r9,	MSR_SF_LG,(63-MSR_SF_LG)

	ori	r9, r9,	MSR_IR|MSR_DR|MSR_FE0|MSR_FE1|MSR_FP|MSR_RI
	andc	r6, r0,	r9		/* Mask out more MSR bits - Relocation, 64-bit, FPU */
	sync
	mtmsrd	r0			/* Disable interrupts */

	LD_ADDR (r4,rtas)
	ld	r5, RTAS_ENTRY(r4)	/* get the rtas->entry value */
	ld	r4, RTAS_BASE(r4)	/* get the rtas->base value */

	mtsrr0	r5			/* Load the RTAS entry */
	mtsrr1	r6			/* Load 32-bit mode */
	rfid				/* Jump to RTAS */

STATIC_PROC(rtas_return_loc)
	/* relocation is off at this point */
	mfsprg	r4, SPRG_LOCAL
	LD_CONST (r9, KERNEL_OFFSET)	// XXX should be - CPU offset
	sub	r4, r4,	r9

        ld	r1, LOCAL_R1(r4)	/* Restore our SP */
	LD_ADDR (r3,.rtas_restore_regs)
        ld	r4, LOCAL_MSR(r4)	/* Restore our MSR */

	mtsrr0	r3
	mtsrr1	r4
	rfid				/* Jump back to original CPU mode */

STATIC_PROC(rtas_restore_regs)
	/* relocation is on at this point */

	ld	r2, PT_R1(r1)		/* Restore the TOC */
	ld	r13,PT_R13(r1)		/* Restore the UTCB */

	/* Restore non-volatile registers */
	ld	r14,PT_R14(r1)
	ld	r15,PT_R15(r1)
	ld	r16,PT_R16(r1)
	ld	r17,PT_R17(r1)
	ld	r18,PT_R18(r1)
	ld	r19,PT_R19(r1)
	ld	r20,PT_R20(r1)
	ld	r21,PT_R21(r1)
	ld	r22,PT_R22(r1)
	ld	r23,PT_R23(r1)
	ld	r24,PT_R24(r1)
	ld	r25,PT_R25(r1)
	ld	r26,PT_R26(r1)
	ld	r27,PT_R27(r1)
	ld	r28,PT_R28(r1)
	ld	r29,PT_R29(r1)
	ld	r30,PT_R30(r1)
	ld	r31,PT_R31(r1)

	/* Restore special registers */
	ld	r4, PT_CR(r1)
	mtcr	r4
	ld	r5, PT_CTR(r1)
	mtctr	r5
	ld	r6, PT_XER(r1)
	mtxer	r6
	ld	r7, PT_DAR(r1)
	mtdar	r7
	ld	r8, PT_DSISR(r1)
	mtdsisr	r8
	ld	r9, PT_SRR0(r1)
	mtsrr0	r9
	ld	r10,PT_SRR1(r1)
	mtsrr1	r10

        addi	r1, r1,	RTAS_FRAME_SIZE	/* Unstack our frame */

	ld	r0, 16(r1)		/* get return address */

	mtlr    r0
        blr				/* return to caller */
END_PROC(__call_rtas)
