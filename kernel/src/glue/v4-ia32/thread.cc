/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/thread.cc
 * Description:   
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
 * $Id: thread.cc,v 1.6 2003/09/24 19:05:34 skoglund Exp $
 *                
 ********************************************************************/
#include INC_GLUE(config.h)
#include INC_API(thread.h)
#include INC_API(tcb.h)
#include INC_ARCH(tss.h)

#define EXCEPTION_FRAME_SIZE ((sizeof(ia32_exceptionframe_t)/4) - 4)

extern "C" void return_to_user();
void return_to_user_wrapper()
{
    __asm__ (
	".globl return_to_user		\n"
	".type return_to_user,@function	\n"
	"return_to_user:		\n"
	"mov %0, %%eax			\n"
	"mov %%ax, %%ds			\n"
	"mov %%ax, %%es			\n"
#ifdef CONFIG_TRACEBUFFER
	"mov %1, %%eax			\n"
#endif
	"mov %%ax, %%fs			\n"
	"mov %2, %%eax			\n"
	"mov %%ax, %%gs			\n"
	"add %3, %%esp			\n"
	"iret				\n"
	:
	: "i"(IA32_UDS), "i"(IA32_TBS), "i"(IA32_UTCB), "i"(EXCEPTION_FRAME_SIZE * 4)
	);
}


static void push(u32_t * &stack, u32_t val)
{
    *(--stack) = val;
}

/**
 * we set up a frame which looks like a normal pagefault frame
 * to allow to ex-regs a newly created thread
 */
void tcb_t::create_startup_stack(void (*func)())
{
    init_stack();
    push(stack, IA32_UDS);
    push(stack, 0x12345678); /* sp */
    push(stack, IA32_USER_FLAGS);
    push(stack, IA32_UCS);
    push(stack, 0x87654321); /* ip */
    stack -= EXCEPTION_FRAME_SIZE;
    push(stack, (u32_t)return_to_user);
    push(stack, (u32_t)func);
}


