/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/idt.cc
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
 * $Id: idt.cc,v 1.5 2006/10/19 22:57:39 ud3 Exp $ 
 *                
 ********************************************************************/
#include <ctors.h>
#include <debug.h>

#include INC_ARCH(cpu.h)
#include INC_ARCH(descreg.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCHX(x86,traps.h)

#include INC_GLUE(idt.h)
#include INC_GLUE(traphandler.h)
#include INC_GLUE(syscalls.h)


/**
 * idt: the global IDT (see: AMD64 Vol 3)
 */
idt_t idt UNIT("amd64.idt") CTORPRIO(CTORPRIO_GLOBAL, 3);



void SECTION(".init.system") 
    idt_t::add_int_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set_handler(X86_KCS, address, amd64_idtdesc_t::interrupt, 0);
}


void SECTION(".init.system") 
    idt_t::add_syscall_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set_handler(X86_KCS, address, amd64_idtdesc_t::interrupt, 3);
}

void SECTION(".init.system") 
    idt_t::add_trap_gate(word_t index, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    descriptors[index].set_handler(X86_KCS, address, amd64_idtdesc_t::trap, 0);
}

/**
 * idt_t::activate: activates the previously set up IDT
 */
void SECTION(".init.cpu") 
    idt_t::activate()
{
    amd64_descreg_t::setdescreg(amd64_descreg_t::idtr, (u64_t) descriptors, sizeof(descriptors));
}

idt_t::idt_t()
{
    
    for (int i=0;i<IDT_SIZE;i++){
	/* 
	 * Synthesize call to exc_catch_common
	 * 
	 * exc_catch_all[IDT_SIZE]
	 * exc_catch_common
	 *  
	 * e8 = Near call with 4 byte offset (5 byte)
	 * 
	 */
	exc_catch_all[i] = ( (sizeof(exc_catch_all) - i * sizeof(word_t) - 5) << 8) | 0xe8;
	add_int_gate(i, (func_exc) &exc_catch_all[i]);
    }
	
    /* setup the exception gates */
    add_int_gate(X86_EXC_DIVIDE_ERROR, exc_catch_diverr);
    add_int_gate(X86_EXC_DEBUG, exc_debug);
    add_int_gate(X86_EXC_NMI, exc_nmi);
    add_syscall_gate(X86_EXC_BREAKPOINT, exc_breakpoint);
    add_int_gate(X86_EXC_OVERFLOW, exc_catch_overflow);
    add_int_gate(X86_EXC_BOUNDRANGE, exc_catch_boundrange);
    add_int_gate(X86_EXC_INVALIDOPCODE, exc_invalid_opcode);
    add_int_gate(X86_EXC_NOMATH_COPROC, exc_nomath_coproc);
    add_int_gate(X86_EXC_DOUBLEFAULT, exc_catch_doublefault);
    add_int_gate(X86_EXC_COPSEG_OVERRUN, exc_catch_overrun);
    add_int_gate(X86_EXC_INVALID_TSS, exc_catch_invtss);
    add_int_gate(X86_EXC_SEGMENT_NOT_PRESENT, exc_catch_segnotpr);
    add_int_gate(X86_EXC_STACKSEG_FAULT, exc_catch_ss_fault);
    add_int_gate(X86_EXC_GENERAL_PROTECTION, exc_gp);
    add_int_gate(X86_EXC_PAGEFAULT, exc_pagefault);
    /* 15 reserved */
    add_int_gate(X86_EXC_FPU_FAULT, exc_fpu_fault);
    add_int_gate(X86_EXC_ALIGNEMENT_CHECK, exc_catch_ac);
    add_int_gate(X86_EXC_MACHINE_CHECK, exc_catch_mc);
    add_int_gate(X86_EXC_SIMD_FAULT, exc_simd_fault);
    
    /*  no syscalls in IDT (only via syscall/sysret) */
}
