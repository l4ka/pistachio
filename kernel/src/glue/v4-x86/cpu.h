/*********************************************************************
 *                
 * Copyright (C) 2004, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/cpu.h
 * Description:   CPU management
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
#ifndef __GLUE__V4_X86__CPU_H__
#define __GLUE__V4_X86__CPU_H__

#include INC_API(processor.h)

class cpu_t {
public:
    cpu_t() 
	{ apicid = ~0UL;}

    void set_apic_id(word_t apicid)
	{ this->apicid = apicid; }

    word_t get_apic_id() 
	{ return apicid; }

    bool is_valid()
	{ return this->apicid < ~0UL; }

public:
    word_t apicid;

private:
    static cpu_t descriptors[CONFIG_SMP_MAX_CPUS];
public:
    static word_t count;
    static cpu_t * get(cpuid_t cpuid) {
	ASSERT(cpuid < CONFIG_SMP_MAX_CPUS);
	return &descriptors[cpuid];
    }

    static bool add_cpu(word_t apicid) {
	if (count >= CONFIG_SMP_MAX_CPUS)
	    return false;
	descriptors[count++].apicid = apicid;
	return true;
    }
};


#endif /* !__GLUE__V4_X86__CPU_H__ */
