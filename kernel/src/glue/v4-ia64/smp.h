/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/smp.h
 * Description:   SMP definitions for ia64 V4.
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
 * $Id: smp.h,v 1.4 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__SMP_H__
#define __GLUE__V4_IA64__SMP_H__
#if defined(CONFIG_SMP)

void smp_startup_processor (cpuid_t cpu_id, word_t vector);
void smp_send_ipi (cpuid_t cpu_id, word_t vector);
bool smp_wait_for_processor (cpuid_t cpu);
void smp_processor_online (cpuid_t cpu);
bool smp_is_processor_online (cpuid_t cpu_id);
bool smp_is_processor_available (cpuid_t cpu_id);
cpuid_t smp_get_cpuid (void);

void SECTION (".init") init_xcpu_handling (void);

#endif /* CONFIG_SMP */
#endif /* !__GLUE__V4_IA64__SMP_H__ */
