/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/trapgate.h
 * Description:   defines macros for implementation of trap and 
 *		  interrupt gates in C
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
 * $Id: trapgate.h,v 1.5 2006/10/19 22:57:34 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__TRAPGATE_H__
#define __ARCH__AMD64__TRAPGATE_H__

 
class amd64_exceptionframe_t
{
public:
    u64_t reason;
    u64_t r15;
    u64_t r14;
    u64_t r13;
    u64_t r12;
    u64_t r11;
    u64_t r10;
    u64_t r9;
    u64_t r8;
    u64_t rdi;
    u64_t rsi;
    u64_t rbp;
    u64_t rdx;
    u64_t rbx;
    u64_t rcx;
    u64_t rax;
    /* default trapgate frame */
    u64_t error;
    u64_t rip;
    u64_t cs;
    u64_t rflags;
    u64_t rsp;
    u64_t ss;
};

/**
 * AMD64_EXC_WITH_ERRORCODE: allows C implementation of 
 *   exception handlers and trap/interrupt gates with error 
 *   code.
 *
 * Usage: AMD64_EXC_WITH_ERRORCODE(exc_gp)
 */

#define AMD64_EXC_WITH_ERRORCODE(name, reason)			\
extern "C" void name##handler(amd64_exceptionframe_t *frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
        "pushq %%rax			\n"			\
	"pushq %%rcx			\n"			\
	"pushq %%rbx			\n"			\
	"pushq %%rdx			\n"			\
	"pushq %%rbp			\n"			\
    	"pushq %%rsi			\n"			\
    	"pushq %%rdi			\n"			\
    	"pushq %%r8			\n"			\
    	"pushq %%r9			\n"			\
    	"pushq %%r10			\n"			\
    	"pushq %%r11			\n"			\
    	"pushq %%r12			\n"			\
    	"pushq %%r13			\n"			\
    	"pushq %%r14			\n"			\
    	"pushq %%r15			\n"			\
	"pushq %0			\n"			\
	"movq  %%rsp, %%rdi		\n"			\
	"call "#name"handler		\n"			\
	"addq  $8, %%rsp		\n"			\
    	"popq  %%r15			\n"			\
    	"popq  %%r14			\n"			\
    	"popq  %%r13			\n"			\
    	"popq  %%r12			\n"			\
    	"popq  %%r11			\n"			\
    	"popq  %%r10			\n"			\
    	"popq  %%r9			\n"			\
    	"popq  %%r8			\n"			\
    	"popq  %%rdi			\n"			\
    	"popq  %%rsi			\n"			\
	"popq  %%rbp			\n"			\
	"popq  %%rdx			\n"			\
	"popq  %%rbx			\n"			\
	"popq  %%rcx			\n"			\
        "popq  %%rax			\n"			\
	"addq  $8, %%rsp		\n"			\
	"iretq				\n"			\
	:							\
	: "i"(reason)						\
	);							\
}								\
void name##handler(amd64_exceptionframe_t *frame)


/* JS: TODO use RIP relative addressing !!!*/
#define AMD64_EXC_NO_ERRORCODE(name, reason)		\
extern "C" void name (void);					\
extern "C" void name##handler(amd64_exceptionframe_t *frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"subq  $8, %%rsp		\n"			\
        "pushq %%rax			\n"			\
	"pushq %%rcx			\n"			\
	"pushq %%rbx			\n"			\
	"pushq %%rdx			\n"			\
	"pushq %%rbp			\n"			\
    	"pushq %%rsi			\n"			\
    	"pushq %%rdi			\n"			\
    	"pushq %%r8			\n"			\
    	"pushq %%r9			\n"			\
    	"pushq %%r10			\n"			\
    	"pushq %%r11			\n"			\
    	"pushq %%r12			\n"			\
    	"pushq %%r13			\n"			\
    	"pushq %%r14			\n"			\
    	"pushq %%r15			\n"			\
	"pushq %0			\n"			\
	"movq  %%rsp, %%rdi		\n"			\
	"call "#name"handler		\n"			\
	"addq  $8, %%rsp		\n"			\
    	"popq  %%r15			\n"			\
    	"popq  %%r14			\n"			\
    	"popq  %%r13			\n"			\
    	"popq  %%r12			\n"			\
    	"popq  %%r11			\n"			\
    	"popq  %%r10			\n"			\
    	"popq  %%r9			\n"			\
    	"popq  %%r8			\n"			\
    	"popq  %%rdi			\n"			\
    	"popq  %%rsi			\n"			\
	"popq  %%rbp			\n"			\
	"popq  %%rdx			\n"			\
	"popq  %%rbx			\n"			\
	"popq  %%rcx			\n"			\
        "popq  %%rax			\n"			\
	"addq  $8, %%rsp		\n"			\
	"iretq				\n"			\
	:							\
	: "i"(reason)						\
	);							\
}								\
void name##handler(amd64_exceptionframe_t *frame)


#endif /* !__ARCH__AMD64__TRAPGATE_H__ */
