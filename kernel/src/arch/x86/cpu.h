/*********************************************************************
 *
 * Copyright (C) 2001-2004, 2007,  Karlsruhe University
 *
 * File path:     arch/x86/cpu.h
 * Description:   x86 helper functions to access special instructions
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
#ifndef __ARCH__X86__CPU_H__
#define __ARCH__X86__CPU_H__

INLINE void x86_cpuid(word_t index,
		      u32_t* eax, u32_t* ebx, u32_t* ecx, u32_t* edx)
{
    __asm__ (
	"cpuid"
	: "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	: "a" (index)
	);
}

#include INC_ARCH_SA(cpu.h)

INLINE void x86_pause()
{

    __asm__ __volatile__ ("pause");
}


INLINE u64_t x86_rdpmc(const int ctrsel)
{
    u32_t eax, edx;

    __asm__ __volatile__ (
            "rdpmc"
            : "=a"(eax), "=d"(edx)
            : "c"(ctrsel));

    return (((u64_t)edx) << 32) | (u64_t)eax;
}


INLINE u64_t x86_rdtsc(void)
{
    u32_t eax, edx;

    __asm__ __volatile__ (
            "rdtsc"
            : "=a"(eax), "=d"(edx));

    return (((u64_t)edx) << 32) | (u64_t)eax;
}


INLINE u64_t x86_rdmsr(const u32_t reg)
{
    u32_t eax, edx;

    __asm__ __volatile__ (
            "rdmsr"
            : "=a"(eax), "=d"(edx)
            : "c"(reg)
    );

    return (((u64_t)edx) << 32) | (u64_t)eax;
}


INLINE void x86_wrmsr(const u32_t reg, const u64_t val)
{
    __asm__ __volatile__ (
            "wrmsr"
            :
            : "a"( (u32_t) val), "d" ( (u32_t) (val >> 32)), "c" (reg));
}


INLINE void x86_settsc(const u64_t val)
{
    x86_wrmsr(0x10, val);
}


INLINE void x86_wbinvd(void)
{
    __asm__ __volatile__("wbinvd\n" : : : "memory");
}


INLINE int x86_lsb (word_t w) __attribute__ ((const));
INLINE int x86_lsb (word_t w)
{
    int bitnum;
    __asm__ ("bsf %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}

#define lsb(w) x86_lsb(w)

INLINE int x86_msb (word_t w) __attribute__ ((const));
INLINE int x86_msb (word_t w)
{
    int bitnum;
    __asm__ ("bsr %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}


INLINE word_t x86_cr0_read(void)
{
    word_t tmp;
    __asm__ __volatile__ ("mov  %%cr0, %0   \n"
              : "=r"(tmp));
    return tmp;
}


INLINE void x86_cr0_set(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr0, %0   \n"
            "or   %1, %0      \n"
            "mov  %0, %%cr0   \n"
            : "=r"(tmp)
            : "ri"(val));
}


INLINE void x86_cr0_mask(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr0, %0   \n"
            "and  %1, %0      \n"
            "mov  %0, %%cr0   \n"
            : "=r"(tmp)
            : "ri"(~val));
}


INLINE void x86_cr2_write(const u32_t val)
{
    __asm__ __volatile__ ("movl %0, %%cr2   \n"
              :
              : "r"(val));
}


INLINE u32_t x86_cr2_read(void)
{
    u32_t tmp;
    __asm__ __volatile__ ("movl %%cr2, %0   \n"
              : "=r"(tmp) );
    return tmp;
}


INLINE word_t x86_cr3_read(void)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr3, %0   \n"
            : "=r"(tmp));
    return tmp;
}


INLINE void x86_cr3_write(const word_t val)
{
    __asm__ __volatile__ (
            "mov  %0, %%cr3   \n"
            :
            : "r"(val));
}


INLINE word_t x86_cr4_read(void)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr4, %0   \n"
            : "=r"(tmp));
    return tmp;
}


INLINE void x86_cr4_write(const word_t val)
{
    __asm__ __volatile__ (
            "mov  %0, %%cr4   \n"
            :
            : "r"(val));
}


INLINE void x86_cr4_set(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr4, %0   \n"
            "or   %1, %0      \n"
            "mov  %0, %%cr4   \n"
            : "=r"(tmp)
            : "ri"(val));
}


INLINE void x86_cr4_mask(const word_t val)
{
    word_t tmp;
    __asm__ __volatile__ (
            "mov  %%cr4, %0   \n"
            "and  %1, %0      \n"
            "mov  %0, %%cr4   \n"
            : "=r"(tmp)
            : "ri"(~val));
}


#define X86_GET_DR(num, reg)     __asm__ __volatile__ ("mov %%db"#num",%0" : "=r"(reg));
#define X86_SET_DR(num, reg)     __asm__ __volatile__ ("mov %0, %%db"#num : : "r"(reg));


INLINE word_t x86_dr_read(word_t dr)
{
    word_t val = 0;
    if (dr==0) X86_GET_DR(0, val);
    if (dr==1) X86_GET_DR(1, val);
    if (dr==2) X86_GET_DR(2, val);
    if (dr==3) X86_GET_DR(3, val);
    if (dr==6) X86_GET_DR(6, val);
    if (dr==7) X86_GET_DR(7, val);
    return val;
}

INLINE void x86_dr_write(word_t dr, word_t val)
{
    if (dr==0) X86_SET_DR(0, val);
    if (dr==1) X86_SET_DR(1, val);
    if (dr==2) X86_SET_DR(2, val);
    if (dr==3) X86_SET_DR(3, val);
    if (dr==6) X86_SET_DR(6, val);
    if (dr==7) X86_SET_DR(7, val);
}


INLINE void x86_enable_interrupts(void)
{
    __asm__ __volatile__ ("sti\n":);
}


INLINE void x86_disable_interrupts(void)
{
    __asm__ __volatile__ ("cli\n":);
}

INLINE void x86_sleep(void)
{
    __asm__ __volatile__(
            "sti   \n"
            "hlt   \n"
            "cli   \n"
	    ::: "memory"
            );
}


INLINE void x86_sleep_uninterruptible(void)
{
    __asm__ __volatile__(
	"pushf			\n\t"
	"cli			\n\t"
	"hlt			\n\t"
	"popf			\n\t"
	::: "memory"
	);
}

INLINE void x86_wait_cycles(u64_t cycles)
{
    u64_t now = x86_rdtsc();
    u64_t then = now;
    
    do 
        then = x86_rdtsc();
    while (then < now + cycles);
    
}

INLINE void x86_invlpg (word_t addr)
{
    __asm__ __volatile__ (
            "invlpg (%0)   \n"
            :
            : "r" (addr));
}

#if defined(CONFIG_X_X86_HVM)

INLINE bool x86_vmsucceed (word_t code)
{
    return ((code & (X86_FLAGS_CF | X86_FLAGS_ZF)) == 0);
}


INLINE word_t x86_vmread (word_t index)
{
    word_t val;

    __asm__ __volatile__ (
	 "vmread %1, %0		\n"
	 : "=r" (val)		// %0
	 : "r" (index));	// %1

    return val;
}

INLINE word_t x86_vmwrite (word_t index, word_t val)
{
    word_t errorcode = 0;

    __asm__ __volatile__ (
	 "vmwrite %2, %1	\n"
	 "pushf			\n"
	 "pop %0		\n"
	 : "=r" (errorcode)	// %0
	 : "r" (index),		// %1
	   "rm" (val)		// %2
	 : "memory");

    return errorcode;
}

INLINE word_t x86_vmptrld (u64_t vmcs)
{
    word_t errorcode = 0;

    __asm__ __volatile__ ("vmptrld %1		\n"
			  "pushf		\n"
			  "pop %0		\n"
			  : "=r" (errorcode)	// %0
			  : "m" (vmcs)		// %1
			  : "memory");

    return errorcode;
}

INLINE word_t x86_vmptrst (u64_t *vmcs)
{
    word_t errorcode = 0;

    __asm__ __volatile__ ("vmptrst %1		\n"
			  "pushf		\n"
			  "pop %0		\n"
			  : "=r" (errorcode)	// %0
			  : "m" (*vmcs)	// %1
			  : "memory");

    return errorcode;
}

INLINE word_t x86_vmclear (u64_t vmcs)
{
    word_t errorcode = 0;

    __asm__ __volatile__ ("vmclear %1		\n"
			  "pushf		\n"
			  "pop %0		\n"
			  : "=r" (errorcode)	// %0
			  : "m" (vmcs)		// %1
			  : "memory");

    return errorcode;
}

INLINE word_t x86_vmxon (u64_t vmcs)
{
    word_t errorcode = 0;
    __asm__ __volatile__ ("vmxon %1		\n"
			  "pushf		\n"
			  "pop %0		\n"
			  : "=r" (errorcode)	// %0
			  : "m" (vmcs));	// %1

    return errorcode;
}

INLINE word_t x86_vmxoff ()
{
    word_t errorcode = 0;

    __asm__ __volatile__ ("vmxoff			\n"
			  "pushf			\n"
			  "pop %0			\n"
			  : "=r" (errorcode)	// %0
			  :
			  : "memory");

    return errorcode;
}


INLINE bool x86_vmptrtest (u64_t vmcs)
{
    u64_t curr_vmcs_ptr;
    return (x86_vmsucceed (x86_vmptrst (&curr_vmcs_ptr)) &&
	    (curr_vmcs_ptr == vmcs));
}



#endif /* defined(CONFIG_X_X86_HVM) */


#endif /* !__ARCH__X86__CPU_H__ */
