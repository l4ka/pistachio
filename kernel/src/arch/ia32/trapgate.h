/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007,  Karlsruhe University
 *                
 * File path:     arch/ia32/trapgate.h
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
 * $Id: trapgate.h,v 1.8 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA32__TRAPGATE_H__
#define __ARCH__IA32__TRAPGATE_H__

class ia32_exceptionframe_t
{
public:
    u32_t reason;
    u32_t es;
    u32_t ds;
    u32_t edi;
    u32_t esi;
    u32_t ebp;
    u32_t __esp;
    u32_t ebx;
    u32_t edx;
    u32_t ecx;
    u32_t eax;
    /* default trapgate frame */
    u32_t error;
    u32_t eip;
    u32_t cs;
    u32_t eflags;
    u32_t esp;
    u32_t ss;
};


/*
 * If KEEP_LAST_BRANCHES is enabled we should clear the LBR flag prior
 * to handling the exception so as to avoid overwriting the last
 * branch records by regular kernel code.
 */
#if defined(CONFIG_IA32_KEEP_LAST_BRANCHES)
#define clr_dbgctl_lbr()		\
	"mov	$0x1d9, %%ecx		\n"\
	"rdmsr				\n"\
	"andl	$0xfffffffe, %%eax	\n"\
	"wrmsr				\n"

#define set_dbgctl_lbr()		\
	"mov	$0x1d9, %%ecx		\n"\
	"rdmsr				\n"\
	"orl	$0x1, %%eax		\n"\
	"wrmsr				\n"
#else
#define clr_dbgctl_lbr()
#define set_dbgctl_lbr()
#endif

#if defined(CONFIG_IA32_SMALL_SPACES)
#define set_kds(reg)						\
	"mov $" MKSTR(X86_KDS) ", %%" #reg "	\n"		\
	"mov %%" #reg ", %%ds	    		\n"
#else
#define set_kds(reg)
#endif


/**
 * IA32_EXC_WITH_ERRORCODE: allows C implementation of 
 *   exception handlers and trap/interrupt gates with error 
 *   code.
 *
 * Usage: IA32_EXC_WITH_ERRORCODE(exc_gp)
 */
#define IA32_EXC_WITH_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(ia32_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"pusha				\n"			\
	"push	%%ds			\n"			\
	"push	%%es			\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	set_kds (eax)						\
	clr_dbgctl_lbr ()					\
	"call	%0			\n"			\
	set_dbgctl_lbr ()					\
	"addl	$8, %%esp		\n"			\
	"popl	%%es			\n"			\
	"popl	%%ds			\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(ia32_exceptionframe_t * frame)




#define IA32_EXC_NO_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(ia32_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"subl	$4, %%esp		\n"			\
	"pusha				\n"			\
	"push	%%ds			\n"			\
	"push	%%es			\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	set_kds (eax)						\
	clr_dbgctl_lbr ()					\
	"call	%0			\n"			\
	set_dbgctl_lbr ()					\
	"addl	$8, %%esp		\n"			\
	"popl	%%es			\n"			\
	"popl	%%ds			\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(ia32_exceptionframe_t * frame)


#endif /* !__ARCH__IA32__TRAPGATE_H__ */
