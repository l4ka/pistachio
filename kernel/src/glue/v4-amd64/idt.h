/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/idt.h
 * Description:   Interrupt Descriptor Table
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
 * $Id: idt.h,v 1.3 2006/10/19 22:57:35 ud3 Exp $ 
 *                
 ********************************************************************/

#ifndef __GLUE__V4_AMD64__IDT_H__
#define __GLUE__V4_AMD64__IDT_H__

#include <debug.h>
#include INC_GLUE(config.h)
#include INC_ARCH(amd64.h)
#include INC_ARCH(segdesc.h)


class idt_t
{
public:
    idt_t() SECTION(".init.cpu");
    void add_int_gate(word_t index, void (*address)());
    void add_syscall_gate(word_t index, void (*address)());
    void add_trap_gate(word_t index, void (*address)());
    void activate();

    amd64_idtdesc_t get_descriptor(word_t index);

private:
    amd64_idtdesc_t descriptors[IDT_SIZE];
};

INLINE amd64_idtdesc_t idt_t::get_descriptor(word_t index)
{
    ASSERT(index < IDT_SIZE);
    return descriptors[index];
}

extern idt_t idt;


#endif /* !__GLUE__V4_AMD64__IDT_H__ */
