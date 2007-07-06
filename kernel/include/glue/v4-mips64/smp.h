/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     glue/v4-msip64/smp.h
 * Description:   SMP definitions for mips64 V4.
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
#ifndef __GLUE__V4_MIPS64__SMP_H__
#define __GLUE__V4_MIPS64__SMP_H__
#if defined(CONFIG_SMP)

#include INC_PLAT(smp.h)

void mips64_start_processor (cpuid_t cpu);
bool mips64_wait_for_processor (cpuid_t cpu);
bool mips64_is_processor_available (cpuid_t cpu);
void mips64_processor_online (cpuid_t cpu);
cpuid_t mips64_get_cpuid (void);
void mips64_init_ipi(cpuid_t cpu);
void mips64_clear_ipi(cpuid_t cpu);

void init_xcpu_handling(cpuid_t cpu);

INLINE void smp_xcpu_trigger (cpuid_t cpu)
{
    // TRACEF("Send IPI to (%d)\n", cpu);
    mips64_send_ipi(cpu);
}


#endif /* CONFIG_SMP */
#endif /* !__GLUE__V4_MIPS64__SMP_H__ */
