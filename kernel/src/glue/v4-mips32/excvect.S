/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/excvect.S
 * Description:   Exception handler entry and exit points for MIPS32
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
 * $Id: excvect.S,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(regdef.h)
#include INC_ARCH(cp0regs.h)
#include INC_ARCH(asm.h)
#include INC_GLUE(context.h)
#include <tcb_layout.h>

        .section .base

#.globl UTLB_REFILL_STACK
#.globl UTLB_REFILL_STACK_end

UTLB_REFILL_STACK:
	.space 1024
UTLB_REFILL_STACK_end:

				
        .text

        .globl __general_except
        .globl __general_except_end
        .globl __tlbmiss_except
        .globl __tlbmiss_except_end
        .globl __extra_except
        .globl __extra_except_end

        .globl mips32_exception
        .globl mips32_interrupt
        .globl mips32_syscall
        .globl mips32_tlb_invalid
        .globl  mips32_tlb_mod

# -------------------------------------------------------------------------
# On tlb miss: jump to tlbmiss exception handler.
# This stub is copied to the exception handler address 0xa0000000,
# -------------------------------------------------------------------------

        .set    noreorder
	.set    noat
__tlbmiss_except:

	j       _tlbmiss_except
        nop

__tlbmiss_except_end:
	.set    at
        .set    reorder
		
# -----------------------
# tlbmiss in user mode
# -----------------------
.align 4
_tlbmiss_except:

	SAVE_ALL_INT
	
	mfc0    a0, CP0_BADVADDR
	move    a1, sp
	subu    sp, sp, 0x10
	jal     tlbmiss_handler	
	nop
	addu    sp, sp, 0x10
	j       mips32_restore_user
	nop
	
# -----------------------
# tlbmiss in kernel mode
# -----------------------
.align 4
mips32_tlb_invalid:

	SAVE_ALL_INT

	mfc0    a0, CP0_BADVADDR
	move    a1, sp
	subu    sp, sp, 0x10
	jal     tlbmiss_handler				# jump into c routine
	nop
	addu    sp, sp, 0x10
	j       mips32_restore_user
	nop
	
		
.align 4
mips32_tlb_mod:

	SAVE_ALL_INT

	mfc0    a0, CP0_BADVADDR
	move    a1, sp
	subu    sp, sp, 0x10
	jal     tlbmod_handler				# jump into c routine
	nop
	addu    sp, sp, 0x10
	j       mips32_restore_user
	nop
	
.align 4
.globl mips32_restore_user_o32
mips32_restore_user_o32:
	addu	sp, sp, 0x10	#o32 calling convention hack

.globl mips32_restore_user
mips32_restore_user:
	RESTORE_ALL
	eret
	nop


# -------------------------------------------------------------------------
# This stub is copied to the exception handler address 0xa0000200,
# -------------------------------------------------------------------------

        .set    noreorder
	.set    noat
__extra_except:
	nop
	nop
	nop
	eret
	nop
__extra_except_end:
	.set    at
        .set    reorder



# -------------------------------------------------------------------------
# On exception/interrupt: jump to exception handler.
# This stub is copied to the exception handler address 0xa0000180,
# -------------------------------------------------------------------------

        .set    noreorder
	.set    noat
__general_except:

	mfc0    k1, CP0_CAUSE
	lui     k0, %hi(exception_handlers)
	andi    k1, k1, 0x7c
	add     k0, k0, k1
		
	lw      k0, %lo(exception_handlers)(k0)
	jr      k0
        nop

__general_except_end:
	.set    at
        .set    reorder


# default exception handler		
.align 4
mips32_exception:

	SAVE_ALL_INT

        move    a0, sp
	
        subu    sp, sp, 0x10
        jal     mips32_exception_handler
	nop
	addu    sp, sp, 0x10
	
        j       mips32_restore_user


.set reorder
.align 4
mips32_interrupt:

	SAVE_ALL_INT

	mfc0    t1, CP0_CAUSE
	mfc0    t0, CP0_STATUS
	li      a0, 7					/* Interrupt source - start at 7 */
	and     t1, t1, t0

	.set	noreorder
	andi    t0, t1, INT_IRQ5
	bnez    t0, 1f
	andi    t0, t1, INT_IRQ4
	bnez    t0, 1f
	li      a0, 6
	andi    t0, t1, INT_IRQ3
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
	li      a0, 1
	
	/* No interrupt occured, can happen on R4000 (see refman 5.3.20)... - and i've got no clue whether it's the same on R4Kc */
b	mips32_restore_user
	nop

	.set	reorder
1:
	sll     a1, a0, 2
	lw      t0, interrupt_handlers(a1)
	move    a1, sp
	subu    sp, sp, 0x10
	jal     t0
	nop
	addu    sp, sp, 0x10

        j       mips32_restore_user


.globl looop
looop:
	j       looop
	nop


#.align 4
#mips32_syscall:
#
#		.set noat
##		lui		k1, %hi(exception_stack)		# load kernel stack	
##		lw		k0, %lo(exception_stack)(k1)
##		addi k1,$at,0							# preserve $at register
#		.set at
#	
#		eret


		

        .globl  mips32_utlb_fall_through
        .align  4
mips32_utlb_fall_through:
#RESTORE_UTLB


