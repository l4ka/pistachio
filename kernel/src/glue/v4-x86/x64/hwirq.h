/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/hwirq.h
 * Description:   Macros to define interrupt handler stubs for AMD64
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
 * $Id: hwirq.h,v 1.3 2006/10/19 22:57:35 ud3 Exp $ 
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X64__HWIRQ_H__
#define __GLUE_V4_X86__X64__HWIRQ_H__

#define HW_IRQ(num)						\
extern "C" void hwirq_##num();					\
void hwirq_##num##_wrapper()					\
{								\
    __asm__ (							\
        ".global hwirq_"#num"				\n"	\
	"\t.type hwirq_"#num",@function			\n"	\
	"hwirq_"#num":					\n"	\
	"pushq %%rax					\n"	\
   	"pushq %%rcx					\n"	\
	"pushq %%rbx					\n"	\
	"pushq %%rdx					\n"	\
	"pushq %%rbp					\n"	\
    	"pushq %%rsi					\n"	\
    	"pushq %%rdi					\n"	\
    	"pushq %%r8         			        \n"	\
    	"pushq %%r9					\n"	\
    	"pushq %%r10					\n"	\
    	"pushq %%r11					\n"	\
    	"pushq %%r12					\n"	\
    	"pushq %%r13					\n"	\
    	"pushq %%r14					\n"	\
    	"pushq %%r15					\n"	\
	"movq $"#num", %%rsi	/* irq number	*/	\n"	\
	"jmp	hwirq_common	/* common stuff	*/	\n"	\
	:							\
	:							\
	);							\
}								\

#define HW_IRQ_COMMON()						\
__asm__ (							\
    ".text						\n"	\
    "	.globl hwirq_common				\n"	\
    "	.type hwirq_common ,@function			\n"	\
    "hwirq_common:					\n"	\
    "movq $intctrl, %rdi	/* this pointer */	\n"	\
    "callq	intctrl_t_handle_irq			\n"	\
    "popq %r15						\n"	\
    "popq %r14						\n"	\
    "popq %r13						\n"	\
    "popq %r12						\n"	\
    "popq %r11						\n"	\
    "popq %r10						\n"	\
    "popq %r9						\n"	\
    "popq %r8						\n"	\
    "popq %rdi						\n"	\
    "popq %rsi						\n"	\
    "popq %rbp						\n"	\
    "popq %rdx						\n"	\
    "popq %rbx						\n"	\
    "popq %rcx						\n"	\
    "popq %rax						\n"	\
    "iretq						\n"	\
    );


#endif /* !__GLUE_V4_X86__X64__HWIRQ_H__ */
