/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/msr.h
 * Created: 
 * Description:	Macros which describe the Machine State Register, and
 * 		functions which manipulate the register.
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
 * $Id: msr.h,v 1.6 2005/01/18 13:22:54 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__MSR_H__
#define __ARCH__POWERPC64__MSR_H__

#ifdef ASSEMBLY
#define __MASK(x)	(1<<(x))
#else
#define __MASK(x)	(1ul<<(x))
#endif

/* Machine State Register (MSR) Fields */
#define MSR_SF_LG	63              /* Enable 64 bit mode */
#define MSR_ISF_LG	61              /* Interrupt 64b mode valid on Power3, Power4 not implemented */
#define MSR_HV_LG 	60              /* Hypervisor state */
#define MSR_VEC_LG	25	        /* Enable AltiVec */
#define SRR1_SXAR_LG	25	        /* Used in SRR1 to sync SIAR, SDAR */
#define MSR_POW_LG	18		/* Enable Power Management */
#define MSR_WE_LG	18		/* Wait State Enable */
#define MSR_TGPR_LG	17		/* TLB Update registers in use */
#define MSR_CE_LG	17		/* Critical Interrupt Enable */
#define MSR_ILE_LG	16		/* Interrupt Little Endian */
#define MSR_EE_LG	15		/* External Interrupt Enable */
#define MSR_PR_LG	14		/* Problem State / Privilege Level */
#define MSR_FP_LG	13		/* Floating Point enable */
#define MSR_ME_LG	12		/* Machine Check Enable */
#define MSR_FE0_LG	11		/* Floating Exception mode 0 */
#define MSR_SE_LG	10		/* Single Step */
#define MSR_BE_LG	9		/* Branch Trace */
#define MSR_DE_LG	9 		/* Debug Exception Enable */
#define MSR_FE1_LG	8		/* Floating Exception mode 1 */
//#define MSR_IP_LG	6		/* Exception prefix 0x000/0xFFF - Not Implemented */
#define MSR_IR_LG	5 		/* Instruction Relocate */
#define MSR_DR_LG	4 		/* Data Relocate */
//#define MSR_PE_LG	3		/* Protection Enable - Not Implemented */
#define MSR_PX_LG	2		/* Protection Exclusive Mode */
#define MSR_PMM_LG	2		/* Performance Monitor Mode - PPC970 */
#define MSR_RI_LG	1		/* Recoverable Exception */
//#define MSR_LE_LG	0 		/* Little Endian - Not Implemented*/

#define MSR_SF		__MASK(MSR_SF_LG)	/* Enable 64 bit mode */
#define MSR_ISF		__MASK(MSR_ISF_LG)	/* Interrupt 64b mode valid on Power3 */
#define MSR_HV 		__MASK(MSR_HV_LG)	/* Hypervisor state */
#define MSR_VEC		__MASK(MSR_VEC_LG)	/* Enable AltiVec */
#define MSR_POW		__MASK(MSR_POW_LG)	/* Enable Power Management */
#define MSR_WE		__MASK(MSR_WE_LG)	/* Wait State Enable */
#define MSR_TGPR	__MASK(MSR_TGPR_LG)	/* TLB Update registers in use */
#define MSR_CE		__MASK(MSR_CE_LG)	/* Critical Interrupt Enable */
#define MSR_ILE		__MASK(MSR_ILE_LG)	/* Interrupt Little Endian */
#define MSR_EE		__MASK(MSR_EE_LG)	/* External Interrupt Enable */
#define MSR_PR		__MASK(MSR_PR_LG)	/* Problem State / Privilege Level */
#define MSR_FP		__MASK(MSR_FP_LG)	/* Floating Point enable */
#define MSR_ME		__MASK(MSR_ME_LG)	/* Machine Check Enable */
#define MSR_FE0		__MASK(MSR_FE0_LG)	/* Floating Point Exception mode 0 */
#define MSR_SE		__MASK(MSR_SE_LG)	/* Single Step */
#define MSR_BE		__MASK(MSR_BE_LG)	/* Branch Trace */
#define MSR_DE		__MASK(MSR_DE_LG)	/* Debug Exception Enable */
#define MSR_FE1		__MASK(MSR_FE1_LG)	/* Floating Point Exception mode 1 */
//#define MSR_IP		__MASK(MSR_IP_LG)	/* Exception prefix 0x000/0xFFF */
#define MSR_IR		__MASK(MSR_IR_LG)	/* Instruction Relocate */
#define MSR_DR		__MASK(MSR_DR_LG)	/* Data Relocate */
//#define MSR_PE		__MASK(MSR_PE_LG)	/* Protection Enable */
#define MSR_PX		__MASK(MSR_PX_LG)	/* Protection Exclusive Mode */
#define MSR_PMM		__MASK(MSR_PMM_LG)	/* Performance Monitor Mode - PPC970 */
#define MSR_RI		__MASK(MSR_RI_LG)	/* Recoverable Exception */
//#define MSR_LE		__MASK(MSR_LE_LG)	/* Little Endian */

// Floating point exception mode
// FE1 FE0
//  0   0   -  Disabled
//  0   1   -  Imprecise recoverable
//  1   0   -  Imprecise non-recoverable
//  1   1   -  Precise

#define MSR_INIT_REAL_MODE	(MSR_SF|MSR_HV|MSR_ME)
#define MSR_INIT_KERNEL_MODE	(MSR_SF|MSR_HV|MSR_ME|MSR_IR|MSR_DR|MSR_RI)

#define MSR_REAL_MODE	(MSR_SF|MSR_ISF|MSR_HV|MSR_ME)
#define MSR_KERNEL_MODE	(MSR_SF|MSR_ISF|MSR_HV|MSR_ME|MSR_IR|MSR_DR|MSR_RI|(MSR_FE0|MSR_FE1))

#define MSR_USER_MODE	(MSR_SF|MSR_ISF|MSR_EE|MSR_PR|MSR_ME|MSR_IR|MSR_DR|MSR_RI|(MSR_FE0|MSR_FE1))

#define MSR_USER_MASK	(MSR_FE0|MSR_FE1|MSR_BE|MSR_SE)

#ifndef ASSEMBLY

INLINE word_t ppc64_get_msr( void )
{
    word_t msr;
    asm volatile("mfmsr %0" : "=r" (msr) );
    return msr;
}

INLINE void ppc64_set_msr( word_t msr )
{
    asm volatile("mtmsrd %0" :: "r" (msr) );
}

INLINE bool ppc64_is_real_mode( word_t msr )
{
    return !((msr & MSR_DR) || (msr & MSR_IR));
}

INLINE bool ppc64_is_kernel_mode( word_t msr )
{
    return ((msr & MSR_PR) == 0);
}

INLINE void ppc64_enable_interrupts()
{
    word_t msr = ppc64_get_msr();
    msr = msr | MSR_EE;
    ppc64_set_msr( msr );
}

INLINE void ppc64_disable_interrupts()
{
    word_t msr = ppc64_get_msr();
    msr = msr & (~MSR_EE);
    ppc64_set_msr( msr );
}

INLINE void ppc64_enable_fpu()
{
    word_t msr = ppc64_get_msr();
    msr = msr | MSR_FP;
    ppc64_set_msr( msr );
    isync();
}

#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC64__MSR_H__ */

