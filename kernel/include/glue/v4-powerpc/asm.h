/*********************************************************************
 *                
 * Copyright (C) 2002, 2003  Karlsruhe University
 *                
 * File path:     include/glue/v4-powerpc/asm.h
 * Description:   Assembler macros
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __GLUE__V4_POWERPC__ASM_H__
#define __GLUE__V4_POWERPC__ASM_H__

#include <asmsyms.h>

#define NILTHREAD	0
#define LOCAL_TID_MASK	0x3f

/**
 * tid_to_tcb: Converts a thread ID into a TCB offset within the KTCB area.
 * @param tcb Name of register to hold the output TCB offset.
 * @param tid Name of register holding the TID.
 */
.macro	tid_to_tcb tcb, tid
	rlwinm \tcb, \tid, 32 - (L4_GLOBAL_VERSION_BITS - KTCB_BITS), (L4_GLOBAL_VERSION_BITS - KTCB_BITS), 31 - KTCB_BITS
.endm


/**
 * stack_to_tcb: Converts the stack pointer into a TCB pointer.
 * @param tcb Name of register to hold the output TCB pointer.
 */
.macro	stack_to_tcb tcb
	extlwi	\tcb, %r1, 32-KTCB_BITS, 0
.endm


.macro	stack_alloc size
	addi	%r1, %r1, -\size
.endm

/**
 * grab_sym: Encodes a symbol in the instruction stream as
 *   two 16-bit immediates, and combines into a dst register.
 * @param dst Name of the register to hold the symbol.
 * @param sym The symbol.
 */
.macro	grab_sym dst, sym
	lis	\dst, \sym @ha
	la	\dst, \sym @l(\dst)
.endm


/**
 * globalize_tid: Detect whether a TID is a local TID, and convert to
 *   a global TID as necessary.  Can handle the nil-TID, but not the
 *   any-TID.
 * @param tid Name of register containing the TID.
 * @return The default condition register is set if the TID was a
 *   local TID.
 */
.macro	globalize_tid tid
	cmplwi	cr1, \tid, NILTHREAD		/* Compare TID to the nil TID. */
	andi.	%r11, \tid, LOCAL_TID_MASK	/* Is the TID a local TID?     */
	/* Negate NIL result and AND with local TID result. */
	crandc	4*cr0+eq, 4*cr0+eq, 4*cr1+eq
	bne	0f
	lwz	\tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (\tid)
0:
.endm


/**
 * globalize_any_tid: Detect whether a TID is a local TID, and convert to
 *   a global TID as necessary.  Can handle the nil-TID and the any-TID.
 * @param tid Name of register containing the TID.
 * @return The default condition register is set if the TID was a
 *   local TID.
 */
.macro	globalize_any_tid tid
	cmplwi	cr1, \tid, NILTHREAD		/* Compare TID to the nil TID. */
	andi.	%r11, \tid, LOCAL_TID_MASK	/* Is the TID a local TID?     */
	/* Negate NIL result and AND with local TID result. */
	crandc	4*cr1+eq, 4*cr0+eq, 4*cr1+eq
	/* OR a bunch of 1's with the TID, to see if it becomes the any TID. */
	/* Invert the result to see whether it equals 0. */
	li	%r11, LOCAL_TID_MASK & 0xffffffff
	nor.	%r11, %r11, \tid
	/* Negate the any thread result, and AND with the local TID result. */
	crandc	4*cr0+eq, 4*cr1+eq, 4*cr0+eq
	bne	0f
	lwz	\tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (\tid)
0:
.endm



#endif	/* __GLUE__V4_POWERPC__ASM_H__ */

