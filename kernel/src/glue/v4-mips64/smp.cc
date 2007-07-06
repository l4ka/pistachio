/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/smp.cc
 * Description:   mips64 MP implementation
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
 * $Id: smp.cc,v 1.5 2006/03/01 14:10:32 ud3 Exp $
 *                
 ********************************************************************/
#if defined(CONFIG_SMP)

#include INC_API(tcb.h)
#include INC_GLUE(intctrl.h)
#include INC_API(smp.h)

extern "C" void handle_ipi(word_t irq, mips64_irq_context_t * frame)
{
    cpuid_t cpu = get_current_cpu();
    // TRACEF("Mips 64 IPI (%d)\n", cpu);

    mips64_clear_ipi(cpu);

    process_xcpu_mailbox ();
}

void SECTION (".init") init_xcpu_handling (cpuid_t cpu)
{
    TRACE_INIT("IPI - mailbox setup %d\n", cpu);
    mips64_init_ipi(cpu);
}

#endif /* CONFIG_SMP */

