/****************************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/msr.h
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
 * $Id: msr.h,v 1.14 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__MSR_H__
#define __ARCH__POWERPC__MSR_H__

#define MSR_BIT(msr,bit)	(((msr) >> (bit)) & 1)
#define MSR_SET(msr,bit)	((msr) | (1 << (bit)))
#define MSR_CLR(msr,bit)	((msr) & ~(1 << (bit)))

#define MSR_VEC	25	/* enable AltiVec */
#define MSR_POW	18	/* power management enable */
#define MSR_ILE	16	/* exception little-endian mode */
#define MSR_EE	15	/* extern interrupt enable */
#define MSR_PR	14	/* privilege level */
#define MSR_FP	13	/* floating point available */
#define MSR_ME	12	/* machine check enable */
#define MSR_FE0	11	/* floating point exception mode 0 */
#define MSR_SE	10	/* single-step trace enable */
#define MSR_BE	 9	/* brance trace enable */
#define MSR_FE1	 8	/* floating point exception mode 1 */
#define MSR_IP	 6	/* exception prefix */
#define MSR_IR	 5	/* instruction address translation */
#define MSR_DR	 4	/* data address translation */
#define MSR_PE	 3	/* protection enable */
#define MSR_PX	 2	/* protection exclusive mode */
#define MSR_RI	 1	/* recoverable exception */
#define MSR_LE	 0	/* little-endian mode enable */

#define MSR_POW_DISABLED	0
#define MSR_POW_ENABLED		1

#define MSR_EE_DISABLED	0
#define MSR_EE_ENABLED	1

#define MSR_PR_KERNEL	0
#define MSR_PR_USER	1

#define MSR_FP_DISABLED	0
#define MSR_FP_ENABLED	1

#define MSR_ME_DISABLED	0
#define MSR_ME_ENABLED	1

#define MSR_SE_DISABLED	0
#define MSR_SE_ENABLED	1

#define MSR_BE_DISABLED	0
#define MSR_BE_ENABLED	1

#define MSR_IP_RAM	0
#define MSR_IP_ROM	1

#define MSR_IR_DISABLED	0
#define MSR_IR_ENABLED	1

#define MSR_DR_DISABLED	0
#define MSR_DR_ENABLED	1

#define MSR_RI_NOT_RECOVERABLE	0
#define MSR_RI_IS_RECOVERABLE	1

#define MSR_LE_BIG_ENDIAN	0
#define MSR_LE_LITTLE_ENDIAN	1

#define MSR_KERNEL_INIT ((MSR_PR_KERNEL << MSR_PR) 			\
	| (MSR_FP_ENABLED << MSR_FP) 					\
	| (MSR_ME_ENABLED << MSR_ME) | (MSR_IP_RAM << MSR_IP)		\
	| (MSR_IR_ENABLED << MSR_IR) | (MSR_DR_ENABLED << MSR_DR))

#define MSR_KERNEL (MSR_KERNEL_INIT | (MSR_RI_IS_RECOVERABLE << MSR_RI))

#define MSR_EXCEPTION ((MSR_PR_KERNEL << MSR_PR) | (MSR_FP_DISABLED << MSR_FP) \
	| (MSR_ME_ENABLED << MSR_ME) | (MSR_IP_RAM << MSR_IP)		\
	| (MSR_IR_ENABLED << MSR_IR) | (MSR_DR_ENABLED << MSR_DR))

#define MSR_USER  ((MSR_EE_ENABLED << MSR_EE) | (MSR_PR_USER << MSR_PR)	\
	| (MSR_FP_DISABLED << MSR_FP) | (MSR_ME_ENABLED << MSR_ME)	\
	| (MSR_IP_RAM << MSR_IP) | (MSR_IR_ENABLED << MSR_IR)		\
	| (MSR_DR_ENABLED << MSR_DR) | (MSR_RI_IS_RECOVERABLE << MSR_RI))

#define MSR_REAL ((MSR_PR_KERNEL << MSR_PR) | (MSR_FP_ENABLED << MSR_FP) \
	| (MSR_IP_RAM << MSR_IP))

#define MSR_USER_MASK ((1 << MSR_LE) | (1 << MSR_FE1) | (1 << MSR_BE) \
	| (1 << MSR_SE) | (1 << MSR_FE0))


#ifndef ASSEMBLY
INLINE word_t ppc_get_msr( void )
{
	word_t msr;
	asm volatile("mfmsr %0" : "=r" (msr) );
	return msr;
}

INLINE void ppc_set_msr( word_t msr )
{
	asm volatile("mtmsr %0" : : "r" (msr) );
}

INLINE bool ppc_is_real_mode( word_t msr )
{
	return ((MSR_BIT(msr,MSR_IR) == MSR_IR_DISABLED) &&
		(MSR_BIT(msr,MSR_DR) == MSR_DR_DISABLED));
}

INLINE bool ppc_is_kernel_mode( word_t msr )
{
    return (MSR_BIT(msr,MSR_PR) == MSR_PR_KERNEL);
}

INLINE void ppc_enable_interrupts()
{
    word_t msr = ppc_get_msr();
    msr = MSR_SET(msr,MSR_EE);
    ppc_set_msr( msr );
}

INLINE void ppc_disable_interrupts()
{
    word_t msr = ppc_get_msr();
    msr = MSR_CLR(msr,MSR_EE);
    ppc_set_msr( msr );
}
#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC__MSR_H__ */

