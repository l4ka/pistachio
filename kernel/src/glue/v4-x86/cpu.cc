/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2005-2003, 2006-2008, 2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/cpu.cc
 * Description:   X86 CPU implementation
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
 * $Id: smp.cc,v 1.6 2003/09/24 19:05:34 skoglund Exp $
 *                
 ********************************************************************/
#include INC_GLUE(config.h)
#include INC_GLUE(idt.h)
#include INC_API(smp.h)
#include INC_API(cpu.h)
#include INC_ARCH(apic.h)
#include INC_ARCH(trapgate.h)
#include <debug.h>

DECLARE_TRACEPOINT_DETAIL(X86_APIC_IPI);

cpu_t cpu_t::descriptors[CONFIG_SMP_MAX_CPUS];
word_t cpu_t::count;

#if defined(CONFIG_SMP)

static local_apic_t<APIC_MAPPINGS_START> apic;

X86_EXCNO_ERRORCODE(smp_trigger_ipi, 0)
{
    // ack early - we may switch
    apic.EOI();

    TRACEPOINT(X86_APIC_IPI, "IPI frame %x IP %x SP %x FLAGS %x\n",
	       frame, 
	       frame->regs[x86_exceptionframe_t::ipreg],
	       frame->regs[x86_exceptionframe_t::spreg],
	       frame->regs[x86_exceptionframe_t::freg]);

    // now handle the request
    process_xcpu_mailbox();
    

}


void smp_xcpu_trigger(cpuid_t cpu)
{
    TRACEPOINT(X86_APIC_IPI, "send IPI to CPU %d\n", cpu);
    apic.send_ipi(cpu_t::get(cpu)->get_id(), IDT_LAPIC_XCPU_IPI);
}

void init_xcpu_handling ()
{
    idt.add_gate(IDT_LAPIC_XCPU_IPI, idt_t::interrupt, smp_trigger_ipi);
}

#endif /* defined(CONFIG_SMP) */
