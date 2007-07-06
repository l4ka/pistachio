/*********************************************************************
 *                
 * Copyright (C) 2004,  Karlsruhe University
 *                
 * File path:     bench/pingpong/ia64.h
 * Description:   IA64 specific pingpong functions
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
 * $Id: ia64.h,v 1.1 2004/05/10 15:54:14 skoglund Exp $
 *                
 ********************************************************************/

#define HAVE_READ_CYCLES
#define HAVE_ARCH_IPC

L4_INLINE L4_Word_t read_cycles (void)
{
    L4_Word_t ret;
    asm volatile (";; mov %0 = ar.itc ;;" :"=r" (ret));
    return ret;
}

L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_ThreadId_t r_to			asm ("r14") = dest;
    register L4_ThreadId_t r_FromSpecifier	asm ("r15") = dest;
    register L4_Word_t r_Timeouts		asm ("r16") = 0;
    register L4_Word_t mr0 			asm ("out0") = untyped;

    // Make sure that we allocate 8 outputs on register stack
    register L4_Word_t mr7 asm ("out7");

    L4_Word_t ar_lc, ar_ec;
    asm volatile ("	;;			\n"
		  "	mov	%0 = ar.lc	\n"
		  "	mov	%1 = ar.ec	\n"
		  :
		  "=r" (ar_lc), "=r" (ar_ec));

    asm volatile (
	"/* pingpong_arch_ipc() */\n"
	__L4_ASM_CALL (__L4_Ipc)
	:
	"+r" (mr0), "=r" (mr7),
	"+r" (r_to), "+r" (r_FromSpecifier), "+r" (r_Timeouts)
	:
	:
	"r2",  "r3", "r8", "r10", "r11",
	"r17", "r18", "r19", "r20", "r21", "r22",
	"r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	"out1", "out2", "out3", "out4", "out5", "out6",
	__L4_CALLER_SAVED_FP_REGS, __L4_CALLER_SAVED_PREDICATE_REGS,
	__L4_CALLER_SAVED_BRANCH_REGS, __L4_CALLEE_SAVED_REGS);

    asm volatile (
	"	mov	ar.lc = %0	\n"
	"	mov	ar.ec = %1	\n"
	"	;;			\n"
	:
	:
	"r" (ar_lc), "r" (ar_ec));

    return mr0 & 0x3f;
}
