/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/psr.h
 * Description:   Processor status register
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
 * $Id: psr.h,v 1.8 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__PSR_H__
#define __ARCH__IA64__PSR_H__

#if defined(ASSEMBLY)
#define __UL(x) x
#else
#define __UL(x) x##UL
#endif

/*
 * Bit field definitions for PSR.
 */

#define PSR_BIG_ENDIAN				(__UL(1) << 1)
#define PSR_USER_PERFMON			(__UL(1) << 2)
#define PSR_ALIGNMENT_CHECK			(__UL(1) << 3)
#define PSR_LOWER_FP_USED			(__UL(1) << 4)
#define PSR_UPPER_FP_USED			(__UL(1) << 5)
#define PSR_INT_COLLECTION			(__UL(1) << 13)
#define PSR_INT_ENABLE				(__UL(1) << 14)
#define PSR_PROTECTION_KEY_ENABLE		(__UL(1) << 15)
#define PSR_DATA_TRANSLATION			(__UL(1) << 17)
#define PSR_LOWER_FP_DISABLE			(__UL(1) << 18)
#define PSR_UPPER_FP_DISABLE			(__UL(1) << 19)
#define PSR_SECURE_PERFMON			(__UL(1) << 20)
#define PSR_PRIV_PERFMON_ENABLE			(__UL(1) << 21)
#define PSR_INSTR_SET_TRANSITION		(__UL(1) << 22)
#define PSR_SECURE_INTERVAL_TIMER		(__UL(1) << 23)
#define PSR_DEBUG_BREAKPOINT			(__UL(1) << 24)
#define PSR_LOWER_PRIV_TRANSFER_TRAP		(__UL(1) << 25)
#define PSR_TAKEN_BRANCH_TRAP			(__UL(1) << 26)
#define PSR_REGISTER_STACK_TRANSLATION		(__UL(1) << 27)
#define PSR_CURRENT_PRIVILEGE_0			(__UL(0) << 32)
#define PSR_CURRENT_PRIVILEGE_3			(__UL(3) << 32)
#define PSR_INSTRUCTION_SET			(__UL(1) << 34)
#define PSR_MACHINE_CHECK_ABORT_MASK		(__UL(1) << 35)
#define PSR_INSTRUCTION_TRANSLATION		(__UL(1) << 36)
#define PSR_INSTRUCTION_DEBUG_DISABLE		(__UL(1) << 37)
#define PSR_DATA_ACCESS_FAULTS			(__UL(1) << 38)
#define PSR_DATA_DEBUG_FAULTS_DISABLE		(__UL(1) << 39)
#define PSR_SINGLE_STEP_ENABLE			(__UL(1) << 40)
#define PSR_EXCEPTION_DEFERRAL			(__UL(1) << 43)
#define PSR_REGISTER_BANK_0			(__UL(0) << 44)
#define PSR_REGISTER_BANK_1			(__UL(1) << 44)
#define PSR_INSTRUCTION_ACCESS_FAULT_DISABLE	(__UL(1) << 45)



#if !defined(ASSEMBLY)

/**
 * psr_t: the IA-64 processor status register
 */
class psr_t
{
public:
    union {
	struct {
	    word_t __rv1	: 1;
	    word_t be		: 1;
	    word_t up		: 1;
	    word_t ac		: 1;
	    word_t mfl		: 1;
	    word_t mfh		: 1;
	    word_t __rv2	: 7;
	    word_t ic		: 1;
	    word_t i		: 1;
	    word_t pk		: 1;

	    word_t __rv3	: 1;
	    word_t dt		: 1;
	    word_t dfl		: 1;
	    word_t dfh		: 1;
	    word_t sp		: 1;
	    word_t pp		: 1;
	    word_t di		: 1;
	    word_t si		: 1;
	    word_t db		: 1;
	    word_t lp		: 1;
	    word_t tb		: 1;
	    word_t rt		: 1;
	    word_t __rv4	: 4;

	    word_t cpl		: 2;
	    word_t is		: 1;
	    word_t mc		: 1;
	    word_t it		: 1;
	    word_t id		: 1;
	    word_t da		: 1;
	    word_t dd		: 1;
	    word_t ss		: 1;
	    word_t ri		: 2;
	    word_t ed		: 1;
	    word_t bn		: 1;
	    word_t ia		: 1;
	    word_t __rv5	: 18;
	};
	word_t raw;
    };
};


/**
 * Read processor status register and return it.
 */
INLINE psr_t get_psr (void)
{
    psr_t ret;
    __asm__ __volatile__ ("mov %0 = psr" : "=r" (ret));
    return ret;
}

INLINE void set_psr (psr_t psr)
{
	__asm__ __volatile__ (
	    "1:	mov	r15 = ip		\n"
	    "	mov	r14 = %[psr]		\n"
	    "	;;				\n"
	    "	add	r15 = 2f-1b,r15		\n"
	    "	rsm	psr.ic			\n"
	    "	;;				\n"
	    "	srlz.d 		 		\n"
	    "	;;				\n"
	    "	mov	cr.ipsr = r14		\n"
	    "	mov	cr.iip = r15		\n"
	    "	mov	cr.ifs = r0		\n"
	    "	;;				\n"
	    "	rfi				\n"
	    "2:					\n"
	    :
	    :
	    [psr] "r" (psr.raw)
	    :
	    "r14", "r15");
}

INLINE void set_psr_low (psr_t psr)
{
    __asm__ __volatile__ ("mov psr.l = %0" :: "r" (psr));
}

INLINE void disable_interrupts (void)
{
    __asm__ __volatile__ ("rsm psr.i");
}

INLINE void enable_interrupts (void)
{
    __asm__ __volatile__ ("ssm psr.i ;; srlz.d ;;");
}

INLINE void psr_clrbits_low (word_t mask)
{
    __asm__ __volatile__ (
	"	mov	r14 = psr		\n"
	"	;;				\n"
	"	andcm	r14 = r14, %0		\n"
	"	;;				\n"
	"	mov	psr.l = r14		\n"
	:
	:
	"r" (mask)
	:
	"r14");
}

INLINE void psr_clrbits (word_t mask)
{
    if (EXPECT_TRUE ((mask >> 32) == 0))
	psr_clrbits_low (mask);
    else
    {
	__asm__ __volatile__ (
	    "1:	mov	r15 = ip		\n"
	    "	mov	r14 = psr		\n"
	    "	;;				\n"
	    "	add	r15 = 2f-1b,r15		\n"
	    "	andcm	r14 = r14, %0		\n"
	    "	rsm	psr.ic			\n"
	    "	;;				\n"
	    "	or	r14 = r14, %1		\n"
	    "	srlz.i 		 		\n"
	    "	;;				\n"
	    "	mov	cr.ipsr = r14		\n"
	    "	mov	cr.iip = r15		\n"
	    "	mov	cr.ifs = r0		\n"
	    "	;;				\n"
	    "					\n"
	    "2:					\n"
	    :
	    :
	    "r" (mask), "r" (PSR_CURRENT_PRIVILEGE_0 | PSR_REGISTER_BANK_1)
	    :
	    "r14", "r15");
    }
}

INLINE void psr_setbits_low (word_t mask)
{
    __asm__ __volatile__ (
	"	mov	r14 = psr		\n"
	"	;;				\n"
	"	or	r14 = r14, %0		\n"
	"	;;				\n"
	"	mov	psr.l = r14		\n"
	:
	:
	"r" (mask)
	:
	"r14");
}

INLINE void psr_setbits (word_t mask)
{
    if (EXPECT_TRUE ((mask >> 32) == 0))
	psr_setbits_low (mask);
    else
    {
	__asm__ __volatile__ (
	    "1:	mov	r15 = ip		\n"
	    "	mov	r14 = psr		\n"
	    "	;;				\n"
	    "	add	r15 = 2f-1b,r15		\n"
	    "	or	r14 = r14, %0		\n"
	    "	rsm	psr.ic			\n"
	    "	;;				\n"
	    "	or	r14 = r14, %1		\n"
	    "	srlz.i 		 		\n"
	    "	;;				\n"
	    "	mov	cr.ipsr = r14		\n"
	    "	mov	cr.iip = r15		\n"
	    "	mov	cr.ifs = r0		\n"
	    "	;;				\n"
	    "	rfi				\n"
	    "2:					\n"
	    :
	    :
	    "r" (mask), "r" (PSR_CURRENT_PRIVILEGE_0 | PSR_REGISTER_BANK_1)
	    :
	    "r14", "r15");
    }
}

#endif /* !ASSEMBY */

#endif /* !__ARCH__IA64__PSR_H__ */
