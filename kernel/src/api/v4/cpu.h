/*********************************************************************
 *                
 * Copyright (C) 2003, 2007, 2010,  Karlsruhe University
 *                
 * File path:     api/v4/cpu.h
 * Description:   processor management
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
 * $Id: processor.h,v 1.2 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__CPU_H__
#define __API__V4__CPU_H__

typedef u16_t cpuid_t;
void init_cpu(cpuid_t processor, word_t external_freq, word_t internal_freq);

class cpu_t {
public:
    cpu_t() 
	{ id = ~0UL; }

    bool is_valid()
	{ return this->id < ~0UL; }

    void set_id(word_t id)
	{ this->id = id; }

    word_t get_id() 
	{ return id; }

private:
    word_t id;
    static cpu_t descriptors[CONFIG_SMP_MAX_CPUS];
public:
    static word_t count;
    static cpu_t * get(cpuid_t cpuid) {
	ASSERT(cpuid < CONFIG_SMP_MAX_CPUS);
	return &descriptors[cpuid];
    }

    static bool add_cpu(word_t id) {
	if (count >= CONFIG_SMP_MAX_CPUS)
	    return false;
	descriptors[count++].id = id;
	return true;
    }
};

#endif /* !__API__V4__CPU_H__ */
