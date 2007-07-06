/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/thread.cc
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
 * $Id: thread.cc,v 1.8 2006/10/20 21:24:25 reichelt Exp $
 *                
 ********************************************************************/


#include <debug.h>
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

/* 
 * EXC_REMAINDER_FRAME_SIZE (in quadwords)
 * exception_frame - interrupt_stack_frame (=5)
 */
#define EXC_REMAINDER_FRAME_SIZE ((sizeof(amd64_exceptionframe_t)/8) - 5)

/* Have  naked function */
extern "C" void return_to_user();
void return_to_user_wrapper()
{
    /* 
     * TODO: perhaps setting ds, es, ss to 0 
     * is not needed
     */
    __asm__ (
        ".globl return_to_user          \n"
        ".type return_to_user,@function \n"
        "return_to_user:                \n"
	"mov %0, %%eax			\n"
        "mov %%eax, %%ds                \n" 
        "mov %%eax, %%es                \n"
#if defined(CONFIG_TRACEBUFFER)
        "mov %1, %%eax                  \n"
#else
        "mov %%eax, %%fs                \n"
#endif
//      "mov  %2, %%eax			\n"
//	"mov  %%eax, %%gs               \n"
        "addq %3, %%rsp                 \n"
        "iretq                          \n"
        :
        : "i"(AMD64_UDS), "i" (AMD64_TBS), "i"(AMD64_UTCBS), "i"(EXC_REMAINDER_FRAME_SIZE * 8)
        );
}

static inline void push(u64_t * &stack, u64_t val)
{
    *(--stack) = val;
}

/**
 * Setup TCB to execute a function when switched to
 * @param func pointer to function
 *
 * The old stack state of the TCB does not matter.
 */

void tcb_t::create_startup_stack(void (*func)())
{
    init_stack();

    push(stack, AMD64_UDS);		/* ss (rpl = 3) */
    push(stack, 0x1234567812345678);	/* sp */
    push(stack, AMD64_USER_FLAGS);	
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
    if (resource_bits.have_resource(COMPATIBILITY_MODE))
	push(stack, AMD64_UCS32);	/* cs */
    else
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */
	push(stack, AMD64_UCS);		/* cs */
    push(stack, 0x8765432187654321);	/* ip */
    stack -= EXC_REMAINDER_FRAME_SIZE;  
    push(stack, (u64_t)return_to_user);
    push(stack, (u64_t)func);
}
