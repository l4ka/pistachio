/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/x32/cpu.h
 * Description:   IA32 helper functions to access special registers
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
 * $Id: cpu.h,v 1.15 2004/09/15 18:05:06 jdoll Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X32__CPU_H__
#define __ARCH__X86__X32__CPU_H__

INLINE void x86_iret_self()
{
    __asm__ __volatile__(
	"pushf			\n\t"
	"push	%[kcs]		\n\t"
	"pushl	$1f		\n\t"
	"iret			\n\t"
	"1:			\n\t"	
	: /* No output */
	: [kcs]	  "r" ((word_t) X86_KCS)
	);
}	


INLINE bool x86_x32_has_cpuid()
{
    /* Iff bit 21 in EFLAGS can be set the CPU supports the CPUID
     * instruction */
    word_t eflags;
    __asm__ (
        // Save EFLAGS to the stack
	"pushfl                 \n"
        // Set bit 21 in EFLAGS image on stack
	"orl     %1,(%%esp)     \n"
        // Restore EFLAGS from stack.
	"popfl                  \n"
        // If supported, this has set bit 21
        // Save EFLAGS on stack to see if bit 21 was set or not
	"pushfl                 \n"
        // Move EFLAGS image to register for inspection
	"pop     %0             \n"
	: "=a" (eflags)
	: "i" (X86_FLAGS_ID)
	);
    return (eflags & X86_FLAGS_ID);
}

INLINE u32_t x86_x32_get_cpu_features()
{
    if (x86_x32_has_cpuid ())
    {
	u32_t features, dummy;
	x86_cpuid(1, &dummy, &dummy, &dummy, &features);
	return features;
    } else {
	/* If there is no CPUID instruction we just fabricate the
         * appropriate feature word.  Currently we only support
         * i486DX+ and therefore assume the FPU to be present */
	return X86_X32_FEAT_FPU;
    }
}


#endif /* !__ARCH__X86__X32__CPU_H__ */
