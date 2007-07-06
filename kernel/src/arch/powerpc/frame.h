/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/frame.h
 * Description:	Exception and system call register state.
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
 ***************************************************************************/

#ifndef __ARCH__POWERPC__FRAME_H__
#define __ARCH__POWERPC__FRAME_H__

#define EABI_STACK_SIZE	8

#if !defined(ASSEMBLY)

/* It is important that the general purpose registers occupy consecutive
 * memory locations (starting at r3), with lower addresses for lower registers,
 * to support the multi-store and multi-load instructions (stmw, lmw).
 */

struct except_regs_t
{
    word_t xer;
    word_t cr;
    word_t ctr;
    word_t r0;
    word_t r3;
    word_t r4;
    word_t r5;
    word_t r6;
    word_t r7;
    word_t r8;
    word_t r9;
    word_t r10;
    word_t r11;
    word_t r12;
    word_t r13;
    word_t r14;
    word_t r15;
    word_t r16;
    word_t r17;
    word_t r18;
    word_t r19;
    word_t r20;
    word_t r21;
    word_t r22;
    word_t r23;
    word_t r24;
    word_t r25;
    word_t r26;
    word_t r27;
    word_t r28;
    word_t r29;
    word_t r30;
    word_t r31;
    word_t r1_stack;
    word_t r2_local_id;
    word_t lr;
    word_t srr0_ip;
    word_t srr1_flags;
};

struct syscall_regs_t
{
    word_t _pad;
    word_t r30;
    word_t r31;
    word_t r1_stack;
    word_t r2_local_id;
    word_t lr;
    word_t srr0_ip;
    word_t srr1_flags;
};

struct except_info_t
{
    word_t exc_no;
    except_regs_t *regs;
    word_t dar;
    word_t dsisr;
};

#endif	/* !ASSEMBLY */

#endif	/* __ARCH__POWERPC__FRAME_H__ */
