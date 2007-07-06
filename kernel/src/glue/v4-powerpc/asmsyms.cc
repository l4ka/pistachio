/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/asmsyms.cc
 * Description:	Various C++ constants converted into assembler compatible 
 * 		symbols.
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
 * $Id: asmsyms.cc,v 1.5 2004/06/03 15:02:51 joshua Exp $
 *
 ***************************************************************************/

#include <mkasmsym.h>

#include INC_API(threadstate.h)
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

MKASMSYM( TSTATE_RUNNING, (word_t) thread_state_t::running );
MKASMSYM( TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever );

MKASMSYM( OFS_TSWITCH_FRAME_IP, offsetof(tswitch_frame_t, ip) );
MKASMSYM( OFS_TSWITCH_FRAME_R30, offsetof(tswitch_frame_t, r30) );
MKASMSYM( OFS_TSWITCH_FRAME_R31, offsetof(tswitch_frame_t, r31) );
MKASMSYM( TSWITCH_FRAME_SIZE, sizeof(tswitch_frame_t) );

extern utcb_t *UTCB;
MKASMSYM( OFS_UTCB_MR, (((word_t) &UTCB->mr[0]) - ((word_t) UTCB)));
MKASMSYM( OFS_UTCB_EXCEPTION_HANDLER, (((word_t) &UTCB->exception_handler) - ((word_t) UTCB)));
MKASMSYM( OFS_UTCB_MY_GLOBAL_ID, (((word_t) &UTCB->my_global_id) - ((word_t) UTCB)));
MKASMSYM( OFS_UTCB_PROCESSOR_NO, (((word_t) &UTCB->processor_no) - ((word_t) UTCB)));

MKASMSYM( PROCDESC_SIZE, sizeof(procdesc_t) );
MKASMSYM( OFS_PROCDESC_INTERNAL_FREQ, offsetof(procdesc_t, internal_freq) );
MKASMSYM( OFS_PROCDESC_EXTERNAL_FREQ, offsetof(procdesc_t, external_freq) );

MKASMSYM( SYSCALL_FRAME_SIZE, sizeof(syscall_regs_t) );
MKASMSYM( OFS_SYSCALL_R1, offsetof(syscall_regs_t, r1_stack) );
MKASMSYM( OFS_SYSCALL_R2, offsetof(syscall_regs_t, r2_local_id) );
MKASMSYM( OFS_SYSCALL_R30, offsetof(syscall_regs_t, r30) );
MKASMSYM( OFS_SYSCALL_R31, offsetof(syscall_regs_t, r31) );
MKASMSYM( OFS_SYSCALL_LR, offsetof(syscall_regs_t, lr) );
MKASMSYM( OFS_SYSCALL_SRR0, offsetof(syscall_regs_t, srr0_ip) );
MKASMSYM( OFS_SYSCALL_SRR1, offsetof(syscall_regs_t, srr1_flags) );

MKASMSYM( EXCEPT_FRAME_SIZE, sizeof(except_regs_t) );
MKASMSYM( OFS_EXCEPT_SRR0, offsetof(except_regs_t, srr0_ip) );
MKASMSYM( OFS_EXCEPT_SRR1, offsetof(except_regs_t, srr1_flags) );
MKASMSYM( OFS_EXCEPT_LR,  offsetof(except_regs_t, lr) );
MKASMSYM( OFS_EXCEPT_CTR, offsetof(except_regs_t, ctr) );
MKASMSYM( OFS_EXCEPT_XER, offsetof(except_regs_t, xer) );
MKASMSYM( OFS_EXCEPT_CR,  offsetof(except_regs_t, cr) );
MKASMSYM( OFS_EXCEPT_R0,  offsetof(except_regs_t, r0) );
MKASMSYM( OFS_EXCEPT_R1,  offsetof(except_regs_t, r1_stack) );
MKASMSYM( OFS_EXCEPT_R2,  offsetof(except_regs_t, r2_local_id) );
MKASMSYM( OFS_EXCEPT_R3,  offsetof(except_regs_t, r3) );
MKASMSYM( OFS_EXCEPT_R4,  offsetof(except_regs_t, r4) );
MKASMSYM( OFS_EXCEPT_R5,  offsetof(except_regs_t, r5) );
MKASMSYM( OFS_EXCEPT_R6,  offsetof(except_regs_t, r6) );
MKASMSYM( OFS_EXCEPT_R7,  offsetof(except_regs_t, r7) );
MKASMSYM( OFS_EXCEPT_R8,  offsetof(except_regs_t, r8) );
MKASMSYM( OFS_EXCEPT_R9,  offsetof(except_regs_t, r9) );
MKASMSYM( OFS_EXCEPT_R10, offsetof(except_regs_t, r10) );
MKASMSYM( OFS_EXCEPT_R11, offsetof(except_regs_t, r11) );
MKASMSYM( OFS_EXCEPT_R12, offsetof(except_regs_t, r12) );
MKASMSYM( OFS_EXCEPT_R13, offsetof(except_regs_t, r13) );
MKASMSYM( OFS_EXCEPT_R14, offsetof(except_regs_t, r14) );
MKASMSYM( OFS_EXCEPT_R15, offsetof(except_regs_t, r15) );
MKASMSYM( OFS_EXCEPT_R16, offsetof(except_regs_t, r16) );
MKASMSYM( OFS_EXCEPT_R17, offsetof(except_regs_t, r17) );
MKASMSYM( OFS_EXCEPT_R18, offsetof(except_regs_t, r18) );
MKASMSYM( OFS_EXCEPT_R19, offsetof(except_regs_t, r19) );
MKASMSYM( OFS_EXCEPT_R20, offsetof(except_regs_t, r20) );
MKASMSYM( OFS_EXCEPT_R21, offsetof(except_regs_t, r21) );
MKASMSYM( OFS_EXCEPT_R22, offsetof(except_regs_t, r22) );
MKASMSYM( OFS_EXCEPT_R23, offsetof(except_regs_t, r23) );
MKASMSYM( OFS_EXCEPT_R24, offsetof(except_regs_t, r24) );
MKASMSYM( OFS_EXCEPT_R25, offsetof(except_regs_t, r25) );
MKASMSYM( OFS_EXCEPT_R26, offsetof(except_regs_t, r26) );
MKASMSYM( OFS_EXCEPT_R27, offsetof(except_regs_t, r27) );
MKASMSYM( OFS_EXCEPT_R28, offsetof(except_regs_t, r28) );
MKASMSYM( OFS_EXCEPT_R29, offsetof(except_regs_t, r29) );
MKASMSYM( OFS_EXCEPT_R30, offsetof(except_regs_t, r30) );
MKASMSYM( OFS_EXCEPT_R31, offsetof(except_regs_t, r31) );

