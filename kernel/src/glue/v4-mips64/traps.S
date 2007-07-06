/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/traps.S
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   Kernel entry points for various traps 
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
 * $Id: traps.S,v 1.31 2005/07/12 07:36:55 cgray Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(context.h)
#include <tcb_layout.h>

//#define DEBUG_TRAPS

#define _mips64_debug	    \
1:	.set	noat;	    \
	li	AT, 2;	    \
	break;		    \
	.set	at;	    \
	b 1b;

	.set noat
	.set reorder
BEGIN_PROC(__mips64_tlb_refill) /* we could be from user space 0x0 */

	j	_mips64_xtlb_miss
	move	k1, sp

END_PROC(__mips64_tlb_refill)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_xtlb_refill)

	j	_mips64_xtlb_miss
	move	k1, sp

END_PROC(__mips64_xtlb_refill)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_cache_error)

#ifdef	CONFIG_DEBUG
	dla	sp, 0xffffffff80008000
	dla	a0, CACHE_STR
	jal	printf
	nop
#endif

	_mips64_debug

END_PROC(__mips64_cache_error)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_extra_vector)
	eret
END_PROC(__mips64_extra_vector)

	.set reorder
BEGIN_PROC(_mips64_restore_user)
	RESTORE_ALL
	eret
END_PROC(_mips64_restore_user)

	.set reorder
BEGIN_PROC(_mips64_interrupt)
	SAVE_ALL_INT

	mfc0    t1, CP0_CAUSE
	mfc0    t0, CP0_STATUS
	li	a0, 7		/* Interrupt source - start at 7 */
	and     t1, t1, t0

	.set	noreorder
	andi	t0, t1, INT_IRQ5
	bnez	t0, 1f
	andi	t0, t1, INT_IRQ4
	bnez	t0, 1f
	li	a0, 6
	andi	t0, t1, INT_IRQ3
	bnez	t0, 1f
	li	a0, 5
	andi	t0, t1, INT_IRQ2
	bnez	t0, 1f
	li	a0, 4
	andi	t0, t1, INT_IRQ1
	bnez	t0, 1f
	li	a0, 3
	andi	t0, t1, INT_IRQ0
	bnez	t0, 1f
	li	a0, 2
	andi	t0, t1, INT_SW0
	bnez	t0, 1f
	li	a0, 0
	andi	t0, t1, INT_SW1
	bnez	t0, 1f
	li	a0, 1
	/* No interrupt occured, can happen on R4000 (see refman 5.3.20) */
	b	_mips64_restore_user
	nop

	.set	reorder
1:
	sll	a1, a0, 3
	ld      t0, interrupt_handlers(a1)
	move	a1, sp
	jal     t0

	j	_mips64_restore_user
END_PROC(_mips64_interrupt)


	.set reorder
BEGIN_PROC(_mips64_exception)
	SAVE_ALL_INT

	move	a0, sp
	jal	mips64_exception

	j	_mips64_restore_user
END_PROC(_mips64_exception)


	.set reorder
BEGIN_PROC(_mips64_xtlb_miss)
	SAVE_ALL_XTLB

	/* Here we have space to insert NOPs to change the instruction count
	 * of this xtlb handler. This is because currently the length of the
	 * C code depends on your compiler and if it's a multiple of 48 cycles
	 * you can hard-lock. It sux. CEG
	 */
	nop
	
#ifdef DEBUG_TRAPS
	mfc0	a3, CP0_STATUS
	dmfc0	a2, CP0_BADVADDR
	dmfc0	a1, CP0_EPC
	dla	a0, XTLB_STR
	jal	printf
#endif

	dmfc0	a0, CP0_BADVADDR
	move	a1, sp
	jal	handle_xtlb_miss

	RESTORE_XTLB
	eret
END_PROC(_mips64_xtlb_miss)

	.set reorder
BEGIN_PROC(_mips64_xtlb_fall_through)
	RESTORE_XTLB
END_PROC(_mips64_xtlb_fall_through)

BEGIN_PROC(_mips64_stlb_miss)
	SAVE_ALL_INT

#ifdef DEBUG_TRAPS
	ld	a3, PT_STATUS(sp)
	ld	a2, PT_BADVADDR(sp)
	dmfc0	a1, CP0_EPC
	dla	a0, STLB_STR
	jal	printf
#endif

	ld	a0, PT_BADVADDR(sp)
	move	a1, sp
	jal	handle_stlb_miss

	j	_mips64_restore_user
END_PROC(_mips64_stlb_miss)

	.set reorder
BEGIN_PROC(_mips64_tlb_mod)
	SAVE_ALL_INT

#ifdef DEBUG_TRAPS
	ld	a2, PT_BADVADDR(sp)
	dmfc0	a1, CP0_EPC
	dla	a0, MTLB_STR
	jal	printf
	nop
#endif

	ld	a0, PT_BADVADDR(sp)
	move	a1, sp
	dla	t0, handle_tlb_mod
	jal	t0

	j	_mips64_restore_user
END_PROC(_mips64_tlb_mod)


#ifndef CONFIG_SMP
	.data
#else
	.section .base
#endif
	.align 3
XTLB_REFILL_STACK:
	.space 1024
XTLB_REFILL_STACK_end:

	.data

#ifdef DEBUG_TRAPS
XTLB_STR:
	.string ":XTLB Refill: EPC=0x%16lx VA=0x%16lx STATUS=0x%16lx\n"
STLB_STR:
	.string ":STLB Refill: EPC=0x%16lx VA=0x%16lx STATUS=0x%16lx\n"
MTLB_STR:
	.string ":TLB Mod: EPC=0x%16lx VA=0x%16lx\n"
#endif

CACHE_STR:
	.string ":Cache error occured:\n"
