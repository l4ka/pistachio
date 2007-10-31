/*********************************************************************
 *                
 * Copyright (C) 2003, 2007,  Karlsruhe University
 *                
 * File path:     api/v4/processor.cc
 * Description:   Processor Management
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
 * $Id: processor.cc,v 1.5 2003/09/24 19:05:24 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/tracepoints.h>
#include <sync.h>

#include INC_API(kernelinterface.h)
#include INC_API(syscalls.h)


DECLARE_TRACEPOINT(SYSCALL_PROCESSOR_CONTROL);

SYS_PROCESSOR_CONTROL (word_t processor_no, word_t internal_frequency,
		       word_t external_frequency, word_t voltage)
{
    TRACEPOINT(SYSCALL_PROCESSOR_CONTROL, "SYS_PROCESSOR_CONTROL (cpu=%d, ifreq=%d, efreq=%d, voltage=%d\n",
	       processor_no, internal_frequency, external_frequency, voltage);

    return_processor_control();
}


/**
 * Lock for access to Kernel Interface Page.
 */
static spinlock_t kiplock;


/**
 * Register processor in Kernel Interface Page.
 *
 * @param processor		processor number
 * @param external_freq		external frequency (in KHz)
 * @param internal_freq		internal frequency (in KHz)
 */
void SECTION(".init")
init_processor(cpuid_t processor, word_t external_freq, word_t internal_freq)
{
    TRACE_INIT("Registering processor %d in KIP (%dMHz, %dMHz)\n", 
	       processor, external_freq/1000, internal_freq/1000);
    kiplock.lock();

    procdesc_t * pdesc = get_kip()->processor_info.get_procdesc(processor);
    ASSERT (pdesc);

    pdesc->set_external_frequency(external_freq);
    pdesc->set_internal_frequency(internal_freq);

    // make processor available in KIP
    if ( get_kip()->processor_info.processors < processor )
	get_kip()->processor_info.processors = processor;
    
    kiplock.unlock();
}
