/****************************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/except.S
 * Description:	PowerPC64 exception vectors.
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
 * $Id: except.S,v 1.14 2004/06/04 03:40:20 cvansch Exp $
 *
 ***************************************************************************/


#include INC_ARCH(vectors.h)
#include INC_ARCH(asm.h)
#include INC_ARCH(frame.h)
#include INC_ARCH(msr.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include <tcb_layout.h>

/*----------------------MACROS-----------------------*/

#define START_EXCEPT(off, name)		\
	.section ".except", "xa";	\
	.align	3;			\
	. = EXCEPT_OFFSET_##off;	\
_except_##name##:;

#define EXCEPTION(target)		\
9:	mtsprg	SPRG_TEMP0, r20;	/* Save r20 in TEMP0		*/  \
	mfsprg	r20, SPRG_LOCAL;	/* Get the local spill area	*/  \
	std	r1, LOCAL_R1(r20);	/* Save r1			*/  \
	std	r2, LOCAL_R2(r20);	/* Save r2			*/  \
	std	r3, LOCAL_R3(r20);	/* Save r3			*/  \
	li	r3, 9b@l-0x4000;	/* Load current address in r3 */    \
	clrrdi	r2, r20, 48;		/* Keep the Kernel Offset */	    \
	ori	r2, r2, (_virt_##target)@l;	/* Create jump address, assuming target < 16-bits */ \
	mfsrr0	r1;							    \
	std	r1, LOCAL_SRR0(r20);	/* Save SRR0 */			    \
	mfsrr1	r1;							    \
	mtsprg	SPRG_TEMP1, r1;		/* Save SRR1 */			    \
	mfmsr	r1;			/* Get the MSR */		    \
	rotldi	r1, r1, 4;						    \
	ori	r1, r1, 0x30B;		/* Set IR, DR, SF, ISF, HV    */    \
	rotldi	r1, r1, 60;		/* for generic handlers       */    \
	mtsrr0	r2;			/* Set the target jump */	    \
	mtsrr1	r1;			/* Set the target MSR */	    \
	rfid;

#define SYSCALL(target)			\
9:	mtsprg	SPRG_TEMP0, r26;	/* Save r26 in TEMP0		*/  \
	mfcr	r26;			/* Save condition register	*/  \
	cmpdi	r0, SYSCALL_last;	/* Check if its an L4 or other syscall */   \
	blt-	10f;			\
	cmpdi	r0, SYSCALL_base;	\
	bgt-	10f;			\
	mfsprg	r26, SPRG_LOCAL;	/* Get the local spill area	*/  \
	clrrdi	r26, r26, 48;		/* Keep the Kernel Offset */	    \
	ori	r26, r26, (_virt_##target)@l;	/* Create jump address, assuming target < 16-bits */ \
	mfsrr0	r27;							    \
	mfsrr1	r28;							    \
	mfmsr	r29;			/* Get the MSR */		    \
	rotldi	r29, r29, 4;						    \
	ori	r29, r29, 0x30B;	/* Set IR, DR, SF, ISF, HV    */    \
	rotldi	r29, r29, 60;		/* for generic handlers       */    \
	mtsrr0	r26;			/* Set the target jump */	    \
	mtsrr1	r29;			/* Set the target MSR */	    \
	rfid;								    \
	nop;								    \
10:	/* Not an L4 syscall	*/					    \
	mtsprg	SPRG_TEMP0, r20;	/* Save r20 in TEMP0		*/  \
	mfsprg	r20, SPRG_LOCAL;	/* Get the local spill area	*/  \
	std	r1, LOCAL_R1(r20);	/* Save r1			*/  \
	std	r2, LOCAL_R2(r20);	/* Save r2			*/  \
	std	r3, LOCAL_R3(r20);	/* Save r3			*/  \
	li	r3, 9b@l-0x4000;	/* Load current address in r3 */    \
	clrrdi	r2, r20, 48;		/* Keep the Kernel Offset */	    \
	ori	r2, r2, (_virt_syscall_other)@l;	/* Create jump address, assuming target < 16-bits */ \
	mfsrr0	r1;							    \
	std	r1, LOCAL_SRR0(r20);	/* Save SRR0 */			    \
	mfsrr1	r1;							    \
	mtsprg	SPRG_TEMP1, r1;		/* Save SRR1 */			    \
	mfmsr	r1;			/* Get the MSR */		    \
	rotldi	r1, r1, 4;						    \
	ori	r1, r1, 0x30B;		/* Set IR, DR, SF, ISF, HV    */    \
	rotldi	r1, r1, 60;		/* for generic handlers       */    \
	mtsrr0	r2;			/* Set the target jump */	    \
	mtsrr1	r1;			/* Set the target MSR */	    \
	rfid;


/*--------------------EXCEPTIONS---------------------*/

	.align	3 
	.section ".except", "xa"
	.globl	_except_start_
_except_start_:

/* Start of the exception vectors.
 * Theses vectors are entered with relocation turned off!
 * Note: We only have 64 instructions separating vectors
 * and 32 instructions around the slb vectors
 */

/*..............Reset.Exception..............*/
START_EXCEPT( SYSTEM_RESET, reset )
9:	mtsprg	SPRG_TEMP0, r20;	/* Save r20 in TEMP0		*/
	mfsprg	r20, SPRG_LOCAL;	/* Get the local spill area	*/
	std	r1, RESET_R1(r20);	/* Save r1			*/
	mfsprg	r1, SPRG_TEMP1
	std	r1, RESET_R2(r20);	/* Save old SRR1			*/
	ld	r1, LOCAL_SRR0(r20)
	std	r1, RESET_SRR0(r20);    /* Save old SRR0 */

	LD_ADDR(r1, __idle_tcb+0xF00)	/* arbitary stack */
	std	r1, LOCAL_R1(r20);	/* Save r1			*/
	std	r2, LOCAL_R2(r20);	/* Save r2			*/
	std	r3, LOCAL_R3(r20);	/* Save r3			*/
	li	r3, 9b@l-0x4000;	/* Load current address in r3 */
	clrrdi	r2, r20, 48;		/* Keep the Kernel Offset */
	ori	r2, r2, (_virt_unhandled)@l;	/* Create jump address, assuming target < 16-bits */
	mfsrr0	r1;
	std	r1, LOCAL_SRR0(r20);	/* Save SRR0 */
	mfsrr1	r1;
	ori	r1, r1, MSR_PR
	xori	r1, r1, MSR_PR
	mtsprg	SPRG_TEMP1, r1;		/* Save SRR1 */
	mfmsr	r1;			/* Get the MSR */
	rotldi	r1, r1, 4;
	ori	r1, r1, 0x30B;		/* Set IR, DR, SF, ISF, HV    */
	rotldi	r1, r1, 60;		/* for generic handlers       */
	mtsrr0	r2;			/* Set the target jump */
	mtsrr1	r1;			/* Set the target MSR */

#if CONFIG_POWERPC64_STAB
mfasr	r1
std	r1, RESET_R3(r20);	/* Save old ASR			*/
	li	r1, (kernel_space_segment_table)@higher
	sldi	r1, r1, 32
	oris	r1, r1, (kernel_space_segment_table)@h
	ori	r1, r1, (kernel_space_segment_table+1)@l
	isync
	mtasr	r1
	slbia
	isync
#endif

mfsprg	r1, SPRG_TCB
std	r1, RESET_TCB(r20)
	LD_ADDR(r1,__idle_tcb)
	mtsprg	SPRG_TCB, r1

	rfid;

/*...........Machine.Check.Exception.........*/
START_EXCEPT( MACHINE_CHECK, check )
	EXCEPTION(unhandled)

/*.............Data.Access.Fault.............*/
START_EXCEPT( DATA_ACCESS, data_access )
9:	mtsprg	SPRG_TEMP0, r20;	/* Save r20 in TEMP0		*/
	mfsprg	r20, SPRG_LOCAL;	/* Get the local spill area	*/
	std	r1, LOCAL_R1(r20);	/* Save r1			*/
	std	r2, LOCAL_R2(r20);	/* Save r2			*/
	std	r3, LOCAL_R3(r20);	/* Save r3			*/
#if CONFIG_POWERPC64_STAB
	mtsprg	SPRG_TEMP1, r4		/* Save r4 in TEMP1		*/
	mfdsisr	r4
	mfcr	r3
	rldicl. r4, r4, 64-21, 63	/* Extract bit 21 (STAB MISS)	*/
	beq+	1f			/* Not a hit			*/
	mfdar	r1
	ld	r2, LOCAL_VSID_ASID(r20)
	std	r5, LOCAL_SRR0(r20)
	mfasr	r4
	b	_handle_stab

1:	mtcr	r3
	mfsprg	r4, SPRG_TEMP1
#endif
	clrrdi	r2, r20, 48;		/* Keep the Kernel Offset */
	ori	r2, r2, (_virt_dsi)@l;	/* Create jump address, assuming target < 16-bits */

	li	r3, 9b@l-0x4000;	/* Load current address in r3 */
	mfsrr0	r1;
	std	r1, LOCAL_SRR0(r20);	/* Save SRR0 */
	mfsrr1	r1;
	mtsprg	SPRG_TEMP1, r1;		/* Save SRR1 */
	mfmsr	r1;			/* Get the MSR */
	rotldi	r1, r1, 4;
	ori	r1, r1, 0x30B;		/* Set IR, DR, SF, ISF, HV    */
	rotldi	r1, r1, 60;		/* for generic handlers       */
	mtsrr0	r2;			/* Set the target jump */
	mtsrr1	r1;			/* Set the target MSR */
	rfid;

#if CONFIG_POWERPC64_SLB
/*.............Data.SLB.Miss.Fault...........*/
/* Data SLB Exception, we reload the SLB */
START_EXCEPT( DATA_ACCESS_SLB, dslb )
	mtsprg	SPRG_TEMP0, r20		/* Save r20 in TEMP0		*/
	mtsprg	SPRG_TEMP1, r4		/* Save r4 in TEMP1		*/

	mfsprg	r20, SPRG_LOCAL		/* Get the local spill area	*/
	std	r1, LOCAL_R1(r20)	/* Save r1			*/
	std	r2, LOCAL_R2(r20)	/* Save r2			*/
	std	r3, LOCAL_R3(r20)	/* Save r3			*/

	mfcr	r20			/* Preserve the condition register	*/
	mfdar	r1			/* Get the faulting address		*/

	li	r4, 16			/* user start				*/
	li	r3, 63
	srdi.	r2, r1, 56		/* Get the top 8-bits of the address	*/
	beq+	1f			/* If user, we skip			*/
	li	r4, 1			/* kernel start				*/
	li	r3, 15

1:	slbmfee	r2, r4			/* Get SLB ESID part entry (r1)-> r2	*/
	rldicl. r2,r2,37,63		/* Extract the valid bit from the ESID part	*/
	beq-	_handle_slb		/* Valid bit clear?			*/
	addi	r4, r4, 1		/* Next slb entry			*/
	cmpld	r4, r3			/* Compare to index to 63/15		*/
	ble+	1b			/* Less than or equal to - continue	*/

	/* We did not find a free entry	*/
	srdi.	r2, r1, 56		/* Get the top 8-bits of the address	*/
	mftbl	r4			/* Get a random entry	XXX		*/
	beq+	2f			/* If user, we skip			*/

	andi.	r4, r4, 0x0f
	bne+	_handle_slb		/* Is r1 0 - don't replace entry 0	*/
	li	r4, 15			/* if 0 - set it to 15			*/
	b	_handle_slb		/* Goto generic SLB handler code	*/
2:
	andi.	r4, r4, 0x1f		/* Mask out				*/
	addi	r4, r4, 32
	b	_handle_slb		/* Goto generic SLB handler code	*/

#endif

/*........Instruction.Access.Fault...........*/
START_EXCEPT( INST_ACCESS, inst_access )
9:	mtsprg	SPRG_TEMP0, r20;	/* Save r20 in TEMP0		*/
	mfsprg	r20, SPRG_LOCAL;	/* Get the local spill area	*/
	std	r1, LOCAL_R1(r20);	/* Save r1			*/
	std	r2, LOCAL_R2(r20);	/* Save r2			*/
	std	r3, LOCAL_R3(r20);	/* Save r3			*/
#if CONFIG_POWERPC64_STAB
	mtsprg	SPRG_TEMP1, r4		/* Save r4 in TEMP1		*/
	mfsrr1	r4
	mfcr	r3
	rldicl. r4, r4, 64-21, 63	/* Extract bit 21 (STAB MISS)	*/
	beq+	1f			/* Not a hit			*/
	mfsrr0	r1
	ld	r2, LOCAL_VSID_ASID(r20)
	std	r5, LOCAL_SRR0(r20)
	mfasr	r4
	b	_handle_stab

1:	mtcr	r3
	mfsprg	r4, SPRG_TEMP1
#endif
	li	r3, 9b@l-0x4000;	/* Load current address in r3 */
	clrrdi	r2, r20, 48;		/* Keep the Kernel Offset */
	ori	r2, r2, (_virt_isi)@l;	/* Create jump address, assuming target < 16-bits */
	mfsrr0	r1;
	std	r1, LOCAL_SRR0(r20);	/* Save SRR0 */
	mfsrr1	r1;
	mtsprg	SPRG_TEMP1, r1;		/* Save SRR1 */
	mfmsr	r1;			/* Get the MSR */
	rotldi	r1, r1, 4;
	ori	r1, r1, 0x30B;		/* Set IR, DR, SF, ISF, HV    */
	rotldi	r1, r1, 60;		/* for generic handlers       */
	mtsrr0	r2;			/* Set the target jump */
	mtsrr1	r1;			/* Set the target MSR */
	rfid;


/*..........Instruction.SLB.Fault............*/
START_EXCEPT( INST_ACCESS_SLB, islb )
	mtsprg	SPRG_TEMP0, r20		/* Save r20 in TEMP0		*/
	mtsprg	SPRG_TEMP1, r4		/* Save r4 in TEMP1		*/

	mfsprg	r20, SPRG_LOCAL		/* Get the local spill area	*/
	std	r1, LOCAL_R1(r20)	/* Save r1			*/
	std	r2, LOCAL_R2(r20)	/* Save r2			*/
	std	r3, LOCAL_R3(r20)	/* Save r3			*/

	mfcr	r20			/* Preserve the condition register	*/

	mfsrr0	r1			/* Get the faulting address		*/

	li	r4, 16			/* user start				*/
	li	r3, 63
	srdi.	r2, r1, 56		/* Get the top 8-bits of the address	*/
	beq+	1f			/* If user, we skip			*/
	li	r4, 1			/* kernel start				*/
	li	r3, 15

1:	slbmfee	r2, r4			/* Get SLB ESID part entry (r1)-> r2	*/
	rldicl. r2,r2,37,63		/* Extract the valid bit from the ESID part	*/
	beq-	_handle_slb		/* Valid bit clear?			*/
	addi	r4, r4, 1		/* Next slb entry			*/
	cmpld	r4, r3			/* Compare to index to 63/15		*/
	ble+	1b			/* Less than or equal to - continue	*/

	/* We did not find a free entry	*/
	srdi.	r2, r1, 56		/* Get the top 8-bits of the address	*/
	mftbl	r4			/* Get a random entry	XXX		*/
	beq+	2f			/* If user, we skip			*/

	andi.	r4, r4, 0x0f
	bne+	_handle_slb		/* Is r1 0 - don't replace entry 0	*/
	li	r4, 14			/* if 0 - set it to 14			*/
	b	_handle_slb		/* Goto generic SLB handler code	*/
2:
	andi.	r4, r4, 0x1f		/* Mask out				*/
	addi	r4, r4, 32
	b	_handle_slb		/* Goto generic SLB handler code	*/

START_EXCEPT( HARDWARE_INT, interrupt )
	EXCEPTION(unhandled)

START_EXCEPT( ALIGNMENT, alignement )
	EXCEPTION(unhandled)

START_EXCEPT( PROGRAM_CHECK, program )
	EXCEPTION(program_check)

START_EXCEPT( FPU_UNAVAILABLE, fpu_unavail )
	EXCEPTION(fpu_unavailable)

START_EXCEPT( DECREMENTER, dec )
	EXCEPTION(decrementer)

START_EXCEPT( TRAP_0A, trap0a )
	EXCEPTION(unhandled)

START_EXCEPT( TRAP_0B, trap0b )
	EXCEPTION(unhandled)

START_EXCEPT( SYSTEM_CALL, syscall )
	SYSCALL(syscall)

START_EXCEPT( TRACE, trace )
	EXCEPTION(trace)

START_EXCEPT( TRAP_OE, trap0e )
	EXCEPTION(unhandled)

START_EXCEPT( PERFORMANCE, performance )
	EXCEPTION(unhandled)

START_EXCEPT( INSTRUCTION_BREAK, inst_break )
	EXCEPTION(unhandled)

START_EXCEPT( SOFT_PATCH, soft_patch )
	EXCEPTION(unhandled)

START_EXCEPT( MAINTENANCE, maintenance )
	EXCEPTION(unhandled)

START_EXCEPT( INSTRUMENTATION, instrumentation )
	EXCEPTION(unhandled)

/*--------------------SLB Handling---------------------*/

	/* From here, the index is in r4    */
	/* From here, the ea is in r1    */
	.align	2
_handle_slb:
	srdi.	r2, r1, 56		/* Get the top 8-bits of the address	*/
	mfasr	r3			/* load the ASID			*/
	beq+	3f			/* If user, we skip			*/
	li	r3, 0x0080		/* Load a zero ASID and set class bit	*/

	srdi	r2, r1, 48		/* Extract the top 16-bits (sign extending)	*/
	cmpldi	r2, 0xfffe		/* Are top 16-bits 0xFFFE			*/
	beq-	2f
	cmpldi	r2, 0xfffb		/* Are top 16-bits 0xFFFB			*/
	beq-	2f
	b	3f			/* Skip if not 0xFFFE or 0xFFFB			*/
2:	ori	r3, r3, 0x0100		/* If 0xFFFE or 0xFFFB - it is a large page	*/
3:
	/* Here we create the ESID part of the Segment Table Entry */
	clrrdi	r2, r1, 28		/* Clear the lower 28-bits		*/
	oris	r2, r2, 0x0800		/* Insert the valid bit			*/
	or	r2, r2, r4		/* Insert the index			*/

	/* Here we create the VSID part of the Segment Table Entry */
	rldicl	r1, r1, 36, 36		/* Extract the ESID, clearing the top 8-bits for the ASID   */
	sldi	r1, r1, 12		/* Shift extracted ESID left 12		*/
	or	r1, r1, r3		/* Insert the ASID part	 (+large page bit)	*/
	ori	r1, r1, 0x0400		/* Insert the kp bit			*/

	isync
	slbmte	r1, r2			/* Insert the SLB entry (VSID, ESID)	*/
	isync

	mtcr	r20			/* Restore the Condition Register	*/
	mfsprg	r20, SPRG_LOCAL		/* Get the local spill area	*/
	ld	r1, LOCAL_R1(r20)	/* Restore r1			*/
	ld	r2, LOCAL_R2(r20)	/* Restore r2			*/
	ld	r3, LOCAL_R3(r20)	/* Restore r3			*/

	mfsprg	r20, SPRG_TEMP0		/* Restore r20 from TEMP0	*/
	mfsprg	r4, SPRG_TEMP1		/* Restore r4 from TEMP1	*/

	rfid
	nop

/*--------------------STAB Handling---------------------*/

	/* From here, the asr is in r4    */
	/* From here, the ea is in r1    */
	/* From here, the vsid_asid is in r2    */
	/* From here, the ctr is in r3    */
	.align	2
_handle_stab:
	clrrdi	r4, r4, 12		/* Clear lower bits */

	srdi	r20, r1, 52		/* Extract the top 12-bits (sign extending)	*/
	cmpldi	r20, 0x0fff		/* Compare with 0xfff				*/
	bne+	1f			/* not the kernel? */

	/* kernel address */
	li	r2, 0			/* Kernel VSID ASID */

1:	rldicl	r20, r1, 64-28, 64-5	/* Get HASH	    */
	insrdi	r4, r20, 5, 64-12	/* Compute the STEG */

	li	r20, 0			/* Start of loop to check stab entries */
2:	ld	r5, 0(r4)		/* Load the ESID part */
	rldicl. r5, r5, 64-7, 63	/* Extract bit 7 (VALID)	*/
	beq	3f			/* not valid - we can use this entry */
	addi	r4, r4, 16		/* next STE	    */
	addi	r20, r20, 1		/* Count + 1	    */
	cmpldi	r20, 8			/* Check if count == 8 */
	bne	2b			/* loop		    */

	clrrdi	r4, r4, 12		/* Clear lower bits */
	rldicl	r20, r1, 64-28, 64-5	/* Get HASH	    */
	not	r20, r20		/* Get second hash  */
	insrdi	r4, r20, 5, 64-12	/* Compute the STEG */

	li	r20, 0			/* Start of loop to check stab entries */
4:	ld	r5, 0(r4)
	rldicl. r5, r5, 64-7, 63	/* Extract bit 7 (VALID)	*/
	beq	3f			/* not valid */
	addi	r4, r4, 16
	addi	r20, r20, 1
	cmpldi	r20, 8
	bne	4b

	/* no entry found */
	clrrdi	r4, r4, 12		/* Clear lower bits */
	rldicl	r20, r1, 64-28, 64-5	/* Get HASH	    */
	insrdi	r4, r20, 5, 64-12	/* Compute the STEG */
	mftbl	r20			/* get random	    */
	insrdi	r4, r20, 3, 64-7	/* Compute the STE  */

	isync				/* Force completion of tranlation */
	li	r20, 0			/* Get 0	    */
	std	r20, 0(r4)		/* Write 0 to the ESID part (make invalid) */
	isync				/* Sync		    */

3:	/* ste in r4 */

	clrrdi	r1, r1, 28		/* Clear lower bits (get ESID part) */
	srdi	r20, r1, 16		/* Get the VSID part of the EA	    */
	or	r2, r2, r20		/* Add it to the VSID		    */
	ori	r1, r1, 0x90		/* Set valid, kp    */

	std	r2, 8(r4)		/* Store VSID	    */
	eieio
	std	r1, 0(r4)		/* Store ESID part  */
	sync

	mtcr	r3
	mfsprg	r20, SPRG_LOCAL		/* Get the local spill area	*/
	ld	r1, LOCAL_R1(r20);	/* Restore r1			*/
	ld	r2, LOCAL_R2(r20);	/* Restore r2			*/
	ld	r3, LOCAL_R3(r20);	/* Restore r3			*/
	ld	r5, LOCAL_SRR0(r20)

	mfsprg	r20, SPRG_TEMP0
	mfsprg	r4, SPRG_TEMP1

	rfid
	nop
	.align	3 


	.globl	_except_end_
_except_end_:

/*--------------------Virtual Mode Code---------------------*/

	/* r20 is in SPRG_TEMP0, srr1 is in SPRG_TEMP1	*/
	/* r20 contains SPRG_LOCAL			*/
	/* r1, r2, r3, srr0 are on the spill stack	*/
#define SAVE_ALL_CONTEXTlr		\
	mfcr	r2;			/* Save the Condition Register	*/  \
	mfsprg	r1, SPRG_TEMP1;		/* Get the SRR1			*/  \
	andi.	r1, r1, MSR_PR;		/* Test the Problem State bit	*/  \
	beq-	1f;			/* If from kernel mode - jump	*/  \
	mfsprg	r1, SPRG_TCB;		/* Get the TCB address		*/  \
	addi	r1, r1, KTCB_SIZE-EXC_STACK_SIZE;   /* Make stack frame	*/  \
	b	2f;			/* Continue			*/  \
1:;									    \
	ld	r1, LOCAL_R1(r20);	/* Restore the kernel stack	*/  \
	subi	r1, r1, EXC_STACK_SIZE + 144;	/* Make space for the frame... */   \
	    /* avoiding leaf functions (144) and no floating point assuption	*/  \
2:;					\
	std	r2, PT_CR(r1);		\
	mfctr	r2;			\
	std	r2, PT_CTR(r1);		\
	mfdsisr	r2;			\
	std	r2, PT_DSISR(r1);	\
	mfdar	r2;			\
	std	r2, PT_DAR(r1);		\
	mfxer	r2;			\
	std	r2, PT_XER(r1);		\
	std	r0, PT_R0(r1);		\
	ld	r2, LOCAL_R3(r20);	\
	std	r2, PT_R3(r1);		\
	std	r4, PT_R4(r1);		\
	std	r5, PT_R5(r1);		\
	std	r6, PT_R6(r1);		\
	std	r7, PT_R7(r1);		\
	std	r8, PT_R8(r1);		\
	std	r9, PT_R9(r1);		\
	std	r10, PT_R10(r1);	\
	std	r11, PT_R11(r1);	\
	std	r12, PT_R12(r1);	\
	std	r13, PT_R13(r1);	\
	std	r14, PT_R14(r1);	\
	std	r15, PT_R15(r1);	\
	std	r16, PT_R16(r1);	\
	std	r17, PT_R17(r1);	\
	std	r18, PT_R18(r1);	\
	mfsprg	r2, SPRG_TEMP0;		\
	std	r19, PT_R19(r1);	\
	std	r2, PT_R20(r1);		\
	std	r21, PT_R21(r1);	\
	std	r22, PT_R22(r1);	\
	std	r23, PT_R23(r1);	\
	std	r24, PT_R24(r1);	\
	std	r25, PT_R25(r1);	\
	std	r26, PT_R26(r1);	\
	std	r27, PT_R27(r1);	\
	std	r28, PT_R28(r1);	\
	std	r29, PT_R29(r1);	\
	std	r30, PT_R30(r1);	\
	std	r31, PT_R31(r1);	\
	ld	r4, LOCAL_R1(r20);	\
	ld	r5, LOCAL_R2(r20);	\
	std	r4, PT_R1(r1);		\
	std	r5, PT_R2(r1);		\
	mfsrr0	r6;			\
	std	r6, PT_LR(r1);		\
	ld	r4, LOCAL_SRR0(r20);	\
	std	r4, PT_SRR0(r1);	\
	mfsprg	r5, SPRG_TEMP1;		\
	std	r5, PT_SRR1(r1);

#define RESTORE_ALL_CONTEXT		\
	ld	r2, PT_DSISR(r1);	\
	mtdsisr	r2;			\
	ld	r2, PT_DAR(r1);		\
	mtdar	r2;			\
	ld	r2, PT_XER(r1);		\
	mtxer	r2;			\
	ld	r2, PT_CR(r1);		\
	mtcr	r2;			\
	ld	r2, PT_CTR(r1);		\
	mtctr	r2;			\
	ld	r0, PT_R0(r1);		\
	ld	r3, PT_R3(r1);		\
	ld	r4, PT_R4(r1);		\
	ld	r5, PT_R5(r1);		\
	ld	r6, PT_R6(r1);		\
	ld	r7, PT_R7(r1);		\
	ld	r8, PT_R8(r1);		\
	ld	r9, PT_R9(r1);		\
	ld	r10, PT_R10(r1);	\
	ld	r11, PT_R11(r1);	\
	ld	r12, PT_R12(r1);	\
	ld	r13, PT_R13(r1);	\
	ld	r14, PT_R14(r1);	\
	ld	r15, PT_R15(r1);	\
	ld	r16, PT_R16(r1);	\
	ld	r17, PT_R17(r1);	\
	ld	r18, PT_R18(r1);	\
	ld	r19, PT_R19(r1);	\
	ld	r20, PT_R20(r1);	\
	ld	r21, PT_R21(r1);	\
	ld	r22, PT_R22(r1);	\
	ld	r23, PT_R23(r1);	\
	ld	r24, PT_R24(r1);	\
	ld	r25, PT_R25(r1);	\
	ld	r26, PT_R26(r1);	\
	ld	r27, PT_R27(r1);	\
	ld	r28, PT_R28(r1);	\
	ld	r29, PT_R29(r1);	\
	ld	r30, PT_R30(r1);	\
	ld	r31, PT_R31(r1);	\
	ld	r2, PT_LR(r1);		\
	mtlr	r2;			\
	ld	r2, PT_SRR0(r1);	\
	mtsrr0	r2;			\
	ld	r2, PT_SRR1(r1);    /* tcb.h - must make sure priviledge can not be raised */	\
	mtsrr1	r2;			\
	ld	r2, PT_R2(r1);		\
	ld	r1, PT_R1(r1);

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
	. = 0x3000
_virt_save_all_and_call:
	SAVE_ALL_CONTEXTlr
	blr

	.globl	_restore_all
_restore_all:
	RESTORE_ALL_CONTEXT
	rfid

/* Virtual addressing is turned on from here */

/* ------------- .Unhandled. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_unhandled:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,ppc64_except_unhandled)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.ppc64_except_unhandled
	nop
	b	_restore_all

/* ------------- .Data.Access. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_dsi:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,dsi_handler)
	ld	r2, 8(r2)

	mfdar	r3
	mfdsisr	r4
	mr	r5, r1
	bl	.dsi_handler
	nop
	b	_restore_all

/* ------------- .Instruction.Access. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_isi:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,isi_handler)
	ld	r2, 8(r2)

	mr	r3, r1
	bl	.isi_handler
	nop
	b	_restore_all

/* ------------- .Program.Check. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_program_check:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,program_check_handler)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.program_check_handler
	nop
	b	_restore_all

/* ------------- .Floating.Point.Unavailable. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_fpu_unavailable:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,fpu_unavailable_handler)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.fpu_unavailable_handler
	nop
	b	_restore_all

/* ------------- .Decrementer. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_decrementer:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,decrementer_handler)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.decrementer_handler
	nop
	b	_restore_all

/* ------------- .System.Call. -------------- */

#define t0	r24
#define t1	r25
#define t2	r26
#define t3	r29
#define srr0	r27
#define srr1	r28

	/* srr0 is in r27 */
	/* srr1 is in r28 */
	.align	3 
	.section ".except", "xa"
_virt_syscall:
	andi.	t0, srr1, MSR_PR	/* Test the Problem State bit	*/
	beq-	20f			/* If from kernel mode - jump	*/ 
	mfsprg	t0, SPRG_TCB		/* Get the TCB address		*/
	addi	t0, t0, KTCB_SIZE-EXC_STACK_SIZE   /* Make stack frame	*/
	std	r1, PT_R1(t0)
	std	r2, PT_R2(t0)
	mflr	t3
	std	t3, PT_LR(t0)
	std	srr0, PT_SRR0(t0)
	std	srr1, PT_SRR1(t0)
	std	r30, PT_R30(t0)
	std	r31, PT_R31(t0)
	mr	r1, t0

	/* Load the TOC */
	LD_ADDR(r2,sys_ipc)
	ld	r2, 8(r2)

	cmpdi	r0, SYSCALL_ipc
	bne-	1f
	mfsprg	t1, SPRG_TCB
	ld	t2, OFS_TCB_UTCB(t1)
	std	r14, OFS_UTCB_MR+0(t2)
	std	r15, OFS_UTCB_MR+8(t2)
	std	r16, OFS_UTCB_MR+16(t2)
	std	r17, OFS_UTCB_MR+24(t2)
	std	r18, OFS_UTCB_MR+32(t2)
	std	r19, OFS_UTCB_MR+40(t2)
	std	r20, OFS_UTCB_MR+48(t2)
	std	r21, OFS_UTCB_MR+56(t2)
	std	r22, OFS_UTCB_MR+64(t2)
	std	r23, OFS_UTCB_MR+72(t2)
	bl	.sys_ipc
	nop
	mfsprg	t1, SPRG_TCB
	ld	t2, OFS_TCB_UTCB(t1)
	ld	r14, OFS_UTCB_MR+0(t2)
	ld	r15, OFS_UTCB_MR+8(t2)
	ld	r16, OFS_UTCB_MR+16(t2)
	ld	r17, OFS_UTCB_MR+24(t2)
	ld	r18, OFS_UTCB_MR+32(t2)
	ld	r19, OFS_UTCB_MR+40(t2)
	ld	r20, OFS_UTCB_MR+48(t2)
	ld	r21, OFS_UTCB_MR+56(t2)
	ld	r22, OFS_UTCB_MR+64(t2)
	ld	r23, OFS_UTCB_MR+72(t2)
	b	_restore_syscall
	nop

/* These still need to be optimised */
1:	cmpdi	r0, SYSCALL_thread_switch
	bne-	1f
	bl	.sys_thread_switch
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_thread_control
	bne-	1f
	bl	.sys_thread_control
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_exchange_registers
	bne-	1f
	bl	.sys_exchange_registers
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_schedule
	bne-	1f
	bl	.sys_schedule
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_unmap
	bne-	1f
	bl	.sys_unmap
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_space_control
	bne-	1f
	bl	.sys_space_control
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_processor_control
	bne-	1f
//	bl	.sys_
	trap
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_memory_control
	bne-	1f
	bl	.sys_memory_control
	nop
	b	_restore_syscall
	nop

1:	cmpdi	r0, SYSCALL_system_clock
	bne-	1f
//	bl	.sys_
	trap
	nop
	b	_restore_syscall
	nop

#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3)
1:	cmpdi	r0, SYSCALL_rtas_call
	bne-	1f
	bl	.sys_rtas_call
	nop
	b	_restore_syscall
	nop
#endif

1:	trap
	nop

	.globl	_restore_all_ipc
_restore_all_ipc:
	ld	r13, PT_R13(r1)
	.globl	_restore_syscall
_restore_syscall:
	ld	r30, PT_R30(r1)
	ld	r31, PT_R31(r1)
	ld	r2, PT_R2(r1)
	ld	srr0, PT_SRR0(r1)
	ld	srr1, PT_SRR1(r1)
	ld	t3, PT_LR(r1)
	mtlr	t3
	ld	r1, PT_R1(r1)
	mtsrr0	srr0
	mtsrr1	srr1
	rfid
	nop

20:	trap
	nop

/* ------------- .Trace. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_trace:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,ppc64_except_trace)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.ppc64_except_trace
	nop
	b	_restore_all


/* ------------- .Non.L4.Syscall. -------------- */

	/* r20 is in SPRG_TEMP0			*/
	/* r1, r2, r3 are on the spill stack	*/
	.align	3 
	.section ".except", "xa"
_virt_syscall_other:
	mflr	r2
	mtsrr0	r2
	bl	_virt_save_all_and_call

	/* Load the TOC */
	LD_ADDR(r2,ppc64_except_syscall)
	ld	r2, 8(r2)

	mr	r4, r1
	bl	.ppc64_except_syscall
	nop
	b	_restore_all


