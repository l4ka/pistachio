/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/mips64/head.S
 * Description:    The kernels entry point 
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
 * $Id: head.S,v 1.15 2003/12/24 03:22:35 cvansch Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(context.h)
#include INC_PLAT(config.h)

	.section .text.startup
BEGIN_PROC(_start)
	dla	sp, _bootstack_top
#ifndef CONFIG_SMP
	sd      sp, K_STACK_BOTTOM
#endif

	jal startup_system
END_PROC(_start)

#define  KVAR(x)	\
	.globl x;	\
x:;			\
	.space 8;

	.section .base
	/* Kernel BASE information (64K) aligned */
	.globl KERNEL_BASE
KERNEL_BASE:
	KVAR(K_STACK_BOTTOM)
	KVAR(K_TEMP0)
	KVAR(K_TEMP1)
	KVAR(K_TEMP2)
	KVAR(K_TEMP3)
	KVAR(K_TEMP4)

	.section .init
	.set at
	.set noreorder
BEGIN_PROC(init_cpu)

	/* Disable Caches */
	mfc0	t0, CP0_CONFIG
	and	t0, ~(CONFIG_CACHE_MASK)
#if defined (CONFIG_SB1_PASS1_WORKAROUNDS)
	or	t0, CONFIG_CACHABLE_COW
#else
	or	t0, CONFIG_NOCACHE
#endif		
	mtc0	t0, CP0_CONFIG
	nop

	/* Setup STATUS Regiser : These can be optimised */
	mfc0	t0, CP0_STATUS

	li	t1, INIT_CP0_STATUS_SET
	li	t2, ~INIT_CP0_STATUS_CLEAR

	or	t0, t1
	and	t0, t2

	srl	t0, 5;	    	/* clear IE, EXL, ERL, KSU */
	sll	t0, 5;

	mtc0	t0, CP0_STATUS  /* set new status */
	nop
	nop
	nop
	mtc0    zero, CP0_TAGLO
	nop

	mtc0    zero, CP0_CAUSE /* clear cause register */
	nop

	j	ra
	nop
END_PROC(init_cpu)
