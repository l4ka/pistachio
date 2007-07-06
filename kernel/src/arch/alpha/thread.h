/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     arch/alpha/thread.h
 * Created:       29/07/2002 17:42:50 by Simon Winwood (sjw)
 * Description:   Generic thread functions 
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
 * $Id: thread.h,v 1.9 2005/02/09 11:30:00 ud3 Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__THREAD_H__
#define __ARCH__ALPHA__THREAD_H__

#include INC_ARCH(types.h)

class alpha_switch_stack_t {
public:
    u64_t r9;
    u64_t r10;
    u64_t r11;
    u64_t r12;
    u64_t r13;
    u64_t r14;
    u64_t r15;
    u64_t ra;
};

/**
 * Interrupt context of a thread
 *
 * This class contains the interrupt context of a thread which is
 * pushed on the stack.
 **/
class alpha_context_t {
public:
    u64_t ps;
    u64_t pc;
    u64_t gp;
    u64_t a0;
    u64_t a1;
    u64_t a2;
    /* Then the gprs ... see traps.S */
};

/**
 * Process Control Block
 *
 * This class contains the interrupt context of a thread which is
 * pushed on the stack.
 **/
class alpha_pcb_t {
public:
    u64_t ksp;
    u64_t usp;
    u64_t ptbr;
    u32_t pcc;
    u32_t asn;
    u64_t unique;
    u64_t flags;
    u64_t reserved1;
    u64_t reserved2;

 public:
    void clear_fen(void) {
	this->flags &= ~(1ull);
    }
    
    void set_fen(void) {
	this->flags |= (1ull);
    }
};
/**
 * Exception Saved registers
 *
 **/

class alpha_savedregs_t {
 public:
    u64_t r0;
    u64_t r1;
    u64_t r2;
    u64_t r3;
    u64_t r4;
    u64_t r5;
    u64_t r6;
    u64_t r7;
    u64_t r8;
    u64_t r9;
    u64_t r10;
    u64_t r11;
    u64_t r12;
    u64_t r13;
    u64_t r14;
    u64_t r15;
    u64_t r19;
    u64_t r20;
    u64_t r21;
    u64_t r22;
    u64_t r23;
    u64_t r24;
    u64_t r25;
    u64_t r26;
    u64_t r27;
    u64_t r28;
};



/* These functions deal with switching to a thread (and saving its context), and
 * the associated notification functions */

extern "C" word_t alpha_switch_to(word_t pcb_paddr);
extern "C" void alpha_return_from_notify(void);
extern "C" void alpha_return_to_user(void);

#endif /* __ARCH__ALPHA__THREAD_H__ */
