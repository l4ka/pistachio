/*********************************************************************
 *                
 * Copyright (C) 2002, 2007,  Karlsruhe University
 *                
 * File path:     arch/x86/fpu.h
 * Description:   contains x86 specific fpu declarations
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
 ********************************************************************/
#ifndef __ARCH_X86_FPU_H__
#define __ARCH_X86_FPU_H__

#include INC_ARCH(cpu.h)

class x86_fpu_t
{
public:
    static void enable()
        { x86_cr0_mask(X86_CR0_TS); }
    static void disable()
        { x86_cr0_set(X86_CR0_TS); }

    static void enable_osfxsr()
        { x86_cr4_set(X86_CR4_OSFXSR); }
    static void disable_osfxsr()
        { x86_cr4_mask(X86_CR4_OSFXSR); }

    static void init()
        { __asm__ __volatile__ ("finit\n"); }

    static void save_state(addr_t fpu_state)
        {
        __asm__ __volatile__ (
#if !defined(CONFIG_X86_FXSR)
        "fnsave %0"
#else
        "fxsave %0"
#endif
        :
        : "m" (*(word_t*)fpu_state));
    }

    static void load_state(addr_t fpu_state)
    {
        __asm__ __volatile__ (
#if !defined(CONFIG_X86_FXSR)
        "frstor %0"
#else
        "fxrstor %0"
#endif
        :
        : "m" (*(word_t*)fpu_state));
    }

    static const word_t get_state_size()
    {
#if !defined (CONFIG_X86_FXSR)
        return 128;
#else
        return 512;
#endif
    }

};

#endif  /* __ARCH_X86_FPU_H__ */
