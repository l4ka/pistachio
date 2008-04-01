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
    __asm__ ("wbinvd\n" : : : "memory");
}


INLINE int x86_lsb (word_t w) __attribute__ ((const));
INLINE int x86_lsb (word_t w)
{
    int bitnum;
    __asm__ ("bsf %1, %0" : "=r" (bitnum) : "rm" (w));
    return bitnum;
}


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
            :);
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

INLINE void x86_invlpg (word_t addr)
{
    __asm__ __volatile__ (
            "invlpg (%0)   \n"
            :
            : "r" (addr));
}

#endif /* !__ARCH__X86__CPU_H__ */
