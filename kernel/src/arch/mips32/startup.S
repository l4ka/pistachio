/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/startup.S
 * Description:   Kernel entry point for MIPS32
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
 * $Id: startup.S,v 1.1 2006/02/23 21:07:45 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(regdef.h)
#include INC_ARCH(cp0regs.h)


# -------------------------------------------------------------------------
# Data segment
# -------------------------------------------------------------------------

.section .base

	.globl K_STACK_BOTTOM
	.globl K_TEMP0
	.globl K_TEMP1
	.globl K_TEMP2
	.globl K_TEMP3
	.globl K_TEMP4
	
	
K_STACK_BOTTOM:	.word	0

K_TEMP0: .word 0
K_TEMP1: .word 0
K_TEMP2: .word 0
K_TEMP3: .word 0
K_TEMP4: .word 0

# -------------------------------------------------------------------------
# Code segment
# -------------------------------------------------------------------------
 

.section .text.startup
.set nomove
 
.globl	_start

.globl init_cpu 

_start:

	la      gp, 0x80000000
 	la      sp, _bootstack_top 
	sw      sp, K_STACK_BOTTOM
	
 	jal     startup_system
	nop

init_cpu:

	# status register: disable interrupts, clear EXL, IM, ERL, BEV

	mfc0    v0, CP0_STATUS		
 	and     v0, v0, 0xFFBF00E0
 	or      v0, v0, 0x10000000
 	mtc0    v0, CP0_STATUS

	# config register: enable caching in user segment

	mfc0    v0, CP0_CONFIG		
 	and     v0, v0, 0xFFFFFFF8
 	or      v0, v0, 0x00000005
 	mtc0    v0, CP0_CONFIG

	# clear cause register
	mtc0    zero, CP0_CAUSE

	# clear tag registers
	mtc0    zero, CP0_TAGLO
	mtc0    zero, CP0_TAGHI

	j       ra

	 
