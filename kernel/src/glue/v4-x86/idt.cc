/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/idt.cc
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


/* trap definition */
#include INC_ARCH(traps.h)
#include INC_ARCH(segdesc.h)

#include INC_GLUE(syscalls.h)
#include INC_GLUE(idt.h)
#include INC_GLUE(traphandler.h)

#include <debug.h>
#include <ctors.h>

/**
 * idt: the global IDT (see: IA32 Vol 3)
 */
idt_t idt UNIT("x86.idt") CTORPRIO(CTORPRIO_GLOBAL, 3);


void SECTION(".init.system") 
    idt_t::init_gate(word_t index, idt_t::type_e type, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    
    switch (type)
    {
    case interrupt:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::interrupt, 0);
	break;
    case syscall:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::interrupt, 3);
	break;
    case trap:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::trap, 0);
	break;
    }	
}

void idt_t::add_gate(word_t index, idt_t::type_e type, void (*address)())
{
    ASSERT(index < IDT_SIZE);
    
    
    switch (type)
    {
    case interrupt:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::interrupt, 0);
	break;
    case syscall:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::interrupt, 3);
	break;
    case trap:
	descriptors[index].set(X86_KCS, address, x86_idtdesc_t::trap, 0);
	break;
    }	
}



/**
 * idt_t::activate: activates the previously set up IDT
 */
void idt_t::activate()
{
    x86_descreg_t idt((word_t) descriptors, sizeof(descriptors)-1);
    idt.setdescreg(x86_descreg_t::idtr);
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
	add_gate(i, interrupt, (func_exc) &exc_catch_all[i]);
    }
    
    /* setup the exception gates */
#if defined(CONFIG_DEBUG)
    init_gate(X86_EXC_DEBUG, interrupt, exc_debug);
    init_gate(X86_EXC_NMI, interrupt, exc_nmi);
    init_gate(X86_EXC_BREAKPOINT, syscall,exc_breakpoint);
#endif
    init_gate(X86_EXC_INVALIDOPCODE, interrupt, exc_invalid_opcode);
    init_gate(X86_EXC_NOMATH_COPROC, interrupt, exc_nomath_coproc);
    init_gate(X86_EXC_GENERAL_PROTECTION, interrupt, exc_gp);
    init_gate(X86_EXC_PAGEFAULT, interrupt, exc_pagefault);
    // 15 reserved

#if defined(CONFIG_SUBARCH_X32)
    // syscalls
    init_gate(0x30, syscall, exc_user_sysipc);
    init_gate(0x31, syscall, exc_user_syscall);
#endif
}
