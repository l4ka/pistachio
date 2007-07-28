/*********************************************************************
 *                
 * Copyright (C) 2006-2007,  Karlsruhe University
 *                
 * File path:     platform/pc99/perfmon.h
 * Description:   Performance monitoring counter macros for IA32/AMD64 CPUS.
 *                
 * @LICENSE@
 *                
 * $Id: perfmon.h,v 1.1 2006/09/26 10:41:15 stoess Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__PC99__PMC_MSR_H__
#define __PLATFORM__PC99__PMC_MSR_H__

#include INC_ARCH(cpu.h)

/*********************************************************************
 * Pentium 3 processors
 *********************************************************************/

#if defined(CONFIG_CPU_IA32_I686) 

#define PMC_MSR_EVTSEL0		       0x186      /* Performance EVT0 */
#define PMC_MSR_EVTSEL1		       0x187      /* Performance EVT1 */

#define PMC_MSR_CTR0		       0xc1      /* Performance CTR0 */
#define PMC_MSR_CTR1		       0xc2      /* Performance CTR1 */

/*********************************************************************
 * Athlon and Opteron processors
 *********************************************************************/

#elif defined(CONFIG_CPU_AMD64_K8) || defined(CONFIG_CPU_IA32_K8)

#define PMC_MSR_EVTSEL0		       0xC0010000      /* Performance EVT0 */
#define PMC_MSR_EVTSEL1		       0xC0010001      /* Performance EVT1 */
#define PMC_MSR_EVTSEL2		       0xC0010002      /* Performance EVT2 */
#define PMC_MSR_EVTSEL3                0xC0010003      /* Performance EVT3 */

#define PMC_MSR_CTR0		       0xC0010004      /* Performance CTR0 */
#define PMC_MSR_CTR1		       0xC0010005      /* Performance CTR1 */
#define PMC_MSR_CTR2		       0xC0010006      /* Performance CTR2 */
#define PMC_MSR_CTR3		       0xC0010007      /* Performance CTR3 */

/*********************************************************************
 * P4, Pentium D and Xeon processors
 *********************************************************************/

#elif defined(CONFIG_CPU_IA32_P4) || defined(CONFIG_CPU_AMD64_P4)

#define PMC_MSR_BASE			0x300
#define PMC_MSR_CTR_NO(addr)		(addr - AMD64_MRS_PMC_BASE) 

#define PMC_MSR_BPU_COUNTER(x)		(0x300 + x) 
#define PMC_MSR_MS_COUNTER(x)		(0x304 + x) 
#define PMC_MSR_FLAME_COUNTER(x)	(0x308 + x) 
#define PMC_MSR_IQ_COUNTER(x)		(0x30C + x) 

#define PMC_MSR_BPU_CCCR(x)		(0x360 + x) 
#define PMC_MSR_MS_CCCR(x)		(0x364 + x) 
#define PMC_MSR_FLAME_CCCR(x)		(0x368 + x) 
#define PMC_MSR_IQ_CCCR(x)		(0x36C + x) 

#define PMC_MSR_BSU_ESCR0    		0x3A0 
#define PMC_MSR_BSU_ESCR1    		0x3A1 
#define PMC_MSR_FSB_ESCR0    		0x3A2 
#define PMC_MSR_FSB_ESCR1    		0x3A3 
#define PMC_MSR_FIRM_ESCR0   		0x3A4 
#define PMC_MSR_FIRM_ESCR1   		0x3A5 
#define PMC_MSR_FLAME_ESCR0  		0x3A6 
#define PMC_MSR_FLAME_ESCR1  		0x3A7 
#define PMC_MSR_DAC_ESCR0    		0x3A8 
#define PMC_MSR_DAC_ESCR1    		0x3A9 
#define PMC_MSR_MOB_ESCR0    		0x3AA 
#define PMC_MSR_MOB_ESCR1    		0x3AB 
#define PMC_MSR_PMH_ESCR0    		0x3AC 
#define PMC_MSR_PMH_ESCR1    		0x3AD 
#define PMC_MSR_SAAT_ESCR0   		0x3AE 
#define PMC_MSR_SAAT_ESCR1   		0x3AF 
#define PMC_MSR_U2L_ESCR0    		0x3B0 
#define PMC_MSR_U2L_ESCR1    		0x3B1 
#define PMC_MSR_BPU_ESCR0    		0x3B2 
#define PMC_MSR_BPU_ESCR1     		0x3B3
#define PMC_MSR_IS_ESCR0   		0x3B4
#define PMC_MSR_IS_ESCR1   		0x3B5
#define PMC_MSR_ITLB_ESCR0    		0x3B6
#define PMC_MSR_ITLB_ESCR1    		0x3B7
#define PMC_MSR_CRU_ESCR0 		0x3B8
#define PMC_MSR_CRU_ESCR1 	  	0x3B9
#define PMC_MSR_IQ_ESCR0	  	0x3BA
#define PMC_MSR_IQ_ESCR1	  	0x3BB
#define PMC_MSR_RAT_ESCR0 	  	0x3BC
#define PMC_MSR_RAT_ESCR1 	  	0x3BD
#define PMC_MSR_SSU_ESCR0 	  	0x3BE
#define PMC_MSR_MS_ESCR0 	  	0x3C0
#define PMC_MSR_MS_ESCR1 	  	0x3C1
#define PMC_MSR_TBPU_ESCR0    		0x3C2
#define PMC_MSR_TBPU_ESCR1    		0x3C3
#define PMC_MSR_TC_ESCR0 	  	0x3C4
#define PMC_MSR_TC_ESCR1            	0x3C5
#define PMC_MSR_IX_ESCR0 		0x3C8
#define PMC_MSR_IX_ESCR1 		0x3C9
#define PMC_MSR_ALF_ESCR0 		0x3CA
#define PMC_MSR_ALF_ESCR1 		0x3CB
#define PMC_MSR_CRU_ESCR2 		0x3CC
#define PMC_MSR_CRU_ESCR3 	 	0x3CD
#define PMC_MSR_CRU_ESCR4 	 	0x3E0
#define PMC_MSR_CRU_ESCR5 	 	0x3E1
#define PMC_MSR_TC_PRECISE_EVENT 	0x3FO

#define PMC_MSR_BPU_CTR_BSU_ESCR	7
#define PMC_MSR_BPU_CTR_FSB_ESCR	6
#define PMC_MSR_BPU_CTR_MOB_ESCR	2
#define PMC_MSR_BPU_CTR_PMH_ESCR	4
#define PMC_MSR_BPU_CTR_BPU_ESCR	0
#define PMC_MSR_BPU_CTR_IS_ESCR		1
#define PMC_MSR_BPU_CTR_ITLB_ESCR	3
#define PMC_MSR_BPU_CTR_IX_ESCR		5

#define PMC_MSR_MS_CTR_MS_ESCR		0
#define PMC_MSR_MS_CTR_TBPU_ESCR	2
#define PMC_MSR_MS_CTR_TC_ESCR		1

#define PMC_MSR_FLAME_CTR_FIRM_ESCR	1
#define PMC_MSR_FLAME_CTR_FLAME_ESCR	0
#define PMC_MSR_FLAME_CTR_DAC_ESCR	5
#define PMC_MSR_FLAME_CTR_SAAT_ESCR	2
#define PMC_MSR_FLAME_CTR_U2L_ESCR	3

#define PMC_MSR_IQ_CTR_CRU_ESCR01	4
#define PMC_MSR_IQ_CTR_CRU_ESCR23	5
#define PMC_MSR_IQ_CTR_CRU_ESCR45	6
#define PMC_MSR_IQ_CTR_IQ_ESCR01	0
#define PMC_MSR_IQ_CTR_RAT_ESCR01	2
#define PMC_MSR_IQ_CTR_SSU_ESCR01	3
#define PMC_MSR_IQ_CTR_ALF_ESCR01	1

#endif /* defined(CONFIG_CPU_IA32_I686) */

#define arch_wrmsr			x86_wrmsr

INLINE void setup_perfmon_cpu(word_t cpuid)
{

#if defined(CONFIG_CPU_IA32_I686) || defined(CONFIG_CPU_AMD64_K8) || defined(CONFIG_CPU_IA32_K8)
    
    /* disable PerfEvents */
     arch_wrmsr(PMC_MSR_EVTSEL0, 0);
     arch_wrmsr(PMC_MSR_EVTSEL1, 0);
 
     /* clear PMCs */
     arch_wrmsr(PMC_MSR_CTR0, 0);
     arch_wrmsr(PMC_MSR_CTR1, 0);
 
     /* init PMCs */
     arch_wrmsr(PMC_MSR_EVTSEL0, 0x4100C0);  // ENABLE + USER + INST_RETIRED
     arch_wrmsr(PMC_MSR_EVTSEL1, 0x4200C0);  // ENABLE + KRNL + INST_RETIRED

     //x86_cr4_set(IA32_CR4_PCE); // allow rdpmc in user mode

#elif defined(CONFIG_CPU_IA32_P4) || defined(CONFIG_CPU_AMD64_P4)

     /* disable PMCs via CCCR*/
     arch_wrmsr(PMC_MSR_IQ_CCCR(0), 3 << 16);
     arch_wrmsr(PMC_MSR_IQ_CCCR(2), 3 << 16);
     
     /* clear PMCs */
     arch_wrmsr(PMC_MSR_IQ_COUNTER(0), 0);
     arch_wrmsr(PMC_MSR_IQ_COUNTER(2), 0);

     /* 
      * init ESCR0:
      * user
      * event mask (non-bogus tagged and non-tagged) 
      * event select (inst_retired)
      */
     arch_wrmsr(PMC_MSR_CRU_ESCR0, (1 << 2) | (3 << 9) | (2 << 25)); 
     
     /* 
      * init ESCR1:
      * kernel
      * event mask (non-bogus tagged and non-tagged) 
      * event select (inst_retired)
      */
     arch_wrmsr(PMC_MSR_CRU_ESCR1, (1 << 3) | (3 << 9) | (2 << 25)); 

     /* 
      * enable PMCs via CCCR:
      * enable + escr select + reserved
      */
     arch_wrmsr(PMC_MSR_IQ_CCCR(0), (1 << 12) | (PMC_MSR_IQ_CTR_CRU_ESCR01 << 13) | (3 << 16)); 
     arch_wrmsr(PMC_MSR_IQ_CCCR(2), (1 << 12) | (PMC_MSR_IQ_CTR_CRU_ESCR01 << 13) | (3 << 16));
     
#endif
 
}

#undef arch_wrmsr


#endif /* !__PLATFORM__PC99__PMC_MSR_H__ */
