/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/idt.cc
 * Description:   v4 specific idt implementation
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
 * $Id: idt.cc,v 1.11 2004/05/31 14:15:59 stoess Exp $
 *                
 ********************************************************************/

#include INC_GLUE(idt.h)
#include INC_ARCH(sysdesc.h)

/* trap definition */
#include INC_ARCHX(x86,traps.h)

#include INC_GLUE(traphandler.h)
#include INC_GLUE(syscalls.h)

#include <debug.h>

/**
 * idt: the global IDT (see: IA32 Vol 3)
 */
idt_t idt UNIT("ia32.idt");


void SECTION(".init.system") 
    idt_t::add_int_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::interrupt, 0);
}


void SECTION(".init.system") 
    idt_t::add_syscall_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::interrupt, 3);
}

void SECTION(".init.system") 
    idt_t::add_trap_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set(IA32_KCS, address, ia32_idtdesc_t::trap, 0);
}

/**
 * idt_t::activate: activates the previously set up IDT
 */
void SECTION(".init.cpu") 
    idt_t::activate()
{
    ia32_sysdesc_t idt_desc = {sizeof(idt_t), (u32_t) descriptors, 0};

    asm ("lidt %0"
	 :
	 : "m"(idt_desc)
	);
}

idt_t::idt_t()
{
    for (int i=0;i<IDT_SIZE;i++){
	/* 
	 * Synthesize call to exc_catch_common
	 * 
	 * idt
	 * exc_catch_all[IDT_SIZE]
	 * exc_catch_common
	 *  
	 * e8 = Near call with 4 byte offset (5 byte)
	 * 
	 */
	exc_catch_all[i] = ( (sizeof(exc_catch_all) - i * sizeof(u64_t) - 5) << 8) | 0xe8;
	add_int_gate(i, (func_exc) &exc_catch_all[i]);
    }
    
    /* setup the exception gates */
    add_int_gate(X86_EXC_DIVIDE_ERROR, exc_catch);
    add_int_gate(X86_EXC_DEBUG, exc_debug);
    add_int_gate(X86_EXC_NMI, exc_nmi);
    add_syscall_gate(X86_EXC_BREAKPOINT, exc_breakpoint);
    add_int_gate(X86_EXC_OVERFLOW, exc_catch);
    add_int_gate(X86_EXC_BOUNDRANGE, exc_catch);
    add_int_gate(X86_EXC_INVALIDOPCODE, exc_invalid_opcode);
    add_int_gate(X86_EXC_NOMATH_COPROC, exc_nomath_coproc);
    add_int_gate(X86_EXC_DOUBLEFAULT, exc_catch);
    add_int_gate(X86_EXC_COPSEG_OVERRUN, exc_catch);
    add_int_gate(X86_EXC_INVALID_TSS, exc_catch);
    add_int_gate(X86_EXC_SEGMENT_NOT_PRESENT, exc_catch);
    add_int_gate(X86_EXC_STACKSEG_FAULT, exc_catch);
    add_int_gate(X86_EXC_GENERAL_PROTECTION, exc_gp);
    add_int_gate(X86_EXC_PAGEFAULT, exc_pagefault);
    // 15 reserved
    add_int_gate(X86_EXC_FPU_FAULT, exc_fpu_fault);
    add_int_gate(X86_EXC_ALIGNEMENT_CHECK, exc_catch);
    add_int_gate(X86_EXC_MACHINE_CHECK, exc_catch);
    add_int_gate(X86_EXC_SIMD_FAULT, exc_simd_fault);

    // syscalls
    add_syscall_gate(0x30, exc_user_sysipc);
    add_syscall_gate(0x31, exc_user_syscall);
}
