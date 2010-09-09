/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/thread.cc
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
 ********************************************************************/
#include <debug.h>
#include INC_GLUE(config.h)
#include INC_API(thread.h)
#include INC_API(tcb.h)
#include INC_ARCH_SA(tss.h)

#if defined(CONFIG_IS_64BIT)
# define EXC_FRAME_SIZE ((sizeof(x86_exceptionframe_t)/BYTES_WORD) - 5)
# else
# define EXC_FRAME_SIZE ((sizeof(x86_exceptionframe_t)/BYTES_WORD) - 4)
#endif

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
        "    mov %0, %%eax              \n"
        "    mov %%eax, %%ds            \n"
        "    mov %%eax, %%es            \n"
        "    mov %%eax, %%fs            \n"

#if defined(CONFIG_IS_64BIT)
        "    add %1, %%rsp              \n"
        "    iretq                      \n"
#else
#if defined(CONFIG_X_CTRLXFER_MSG)
	"     addl   $16, %%esp		\n"
	"     popa			\n"
	"     addl   $4, %%esp		\n"
#else
	"     add %1, %%esp		\n"
#endif
	"     iret			\n"
#endif
        :
        : "i"(X86_UDS), "i"(EXC_FRAME_SIZE * BYTES_WORD)
        );
}


static inline void push(word_t * &stack, word_t val)
{
    *(--stack) = val;
}

#if defined(CONFIG_X_X86_HVM)
static void return_to_hvm()
{
    tcb_t *current = get_current_tcb();
    current->get_arch()->enter_hvm_loop();
}
#endif


/**
 * Setup TCB to execute a function when switched to
 * @param func pointer to function
 *
 * The old stack state of the TCB does not matter.
 */
void tcb_t::create_startup_stack(void (*func)())
{
    init_stack();

    word_t cs = X86_UCS;
    word_t flags = X86_USER_FLAGS;
    word_t return_ip = (word_t) return_to_user;
    
#if defined(CONFIG_X_X86_HVM)
    if (this->resource_bits.have_resource (HVM))
    {
	return_ip = (word_t) return_to_hvm;
	flags = X86_HVM_FLAGS;
    }
#endif    
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
    if (resource_bits.have_resource(COMPATIBILITY_MODE))
        cs = X86_UCS32;       /* cs */
#endif   

    push(stack, X86_UDS);               /* ss (rpl = 3) */
    push(stack, 0x12345678);            /* sp */
    push(stack, flags);			/* flags */
    push(stack, cs);			/* cs */
    push(stack, 0x87654321);            /* ip */
    stack -= EXC_FRAME_SIZE;
    push(stack, return_ip);
    push(stack, (word_t)func);
}

