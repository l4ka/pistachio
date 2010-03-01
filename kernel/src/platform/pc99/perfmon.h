/*********************************************************************
 *                
 * Copyright (C) 2006-2008, 2010,  Karlsruhe University
 *                
 * File path:     platform/pc99/perfmon.h
 * Description:   Performance monitoring counter macros for IA32/AMD64 CPUS.
 *                
 * @LICENSE@
 *                
 * $Id: perfmon.h,v 1.1 2006/09/26 10:41:15 stoess Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__PC99__PERFMON_H__
#define __PLATFORM__PC99__PERFMON_H__

#include INC_ARCH(cpu.h)

/*********************************************************************
 * Pentium 3 processors
 *********************************************************************/

#if defined(CONFIG_CPU_X86_I686) 

#define X86_MSR_PMC_EVTSEL0		       0x186      /* Performance EVT0 */
#define X86_MSR_PMC_EVTSEL1		       0x187      /* Performance EVT1 */

#define X86_MSR_PMC_CTR0		       0xc1      /* Performance CTR0 */
#define X86_MSR_PMC_CTR1		       0xc2      /* Performance CTR1 */

/*********************************************************************
 * Athlon and Opteron processors
 *********************************************************************/

#elif defined(CONFIG_CPU_X86_K8) 

#define X86_MSR_PMC_EVTSEL0		       0xC0010000      /* Performance EVT0 */
#define X86_MSR_PMC_EVTSEL1		       0xC0010001      /* Performance EVT1 */
#define X86_MSR_PMC_EVTSEL2		       0xC0010002      /* Performance EVT2 */
#define X86_MSR_PMC_EVTSEL3		       0xC0010003      /* Performance EVT3 */

#define X86_MSR_PMC_CTR0		       0xC0010004      /* Performance CTR0 */
#define X86_MSR_PMC_CTR1		       0xC0010005      /* Performance CTR1 */
#define X86_MSR_PMC_CTR2		       0xC0010006      /* Performance CTR2 */
#define X86_MSR_PMC_CTR3		       0xC0010007      /* Performance CTR3 */

/*********************************************************************
 * P4, Pentium D and Xeon processors
 *********************************************************************/

#elif defined(CONFIG_CPU_X86_P4)

#define X86_MSR_PMC_BASE			0x300
#define X86_MSR_PMC_CTR_NO(addr)		(addr - X86_X64_MRS_PMC_BASE) 

#define X86_MSR_PMC_BPU_COUNTER(x)		(0x300 + x) 
#define X86_MSR_PMC_MS_COUNTER(x)		(0x304 + x) 
#define X86_MSR_PMC_FLAME_COUNTER(x)		(0x308 + x) 
#define X86_MSR_PMC_IQ_COUNTER(x)		(0x30C + x) 

#define X86_MSR_PMC_BPU_CCCR(x)			(0x360 + x) 
#define X86_MSR_PMC_MS_CCCR(x)		        (0x364 + x) 
#define X86_MSR_PMC_FLAME_CCCR(x)		(0x368 + x) 
#define X86_MSR_PMC_IQ_CCCR(x)			(0x36C + x) 

#define X86_MSR_PMC_BSU_ESCR(x)    		(0x3A0 + x) 
#define X86_MSR_PMC_FSB_ESCR(x)    		(0x3A2 + x)
#define X86_MSR_PMC_FIRM_ESCR(x)   		(0x3A4 + x) 
#define X86_MSR_PMC_FLAME_ESCR(x)  		(0x3A6 + x) 
#define X86_MSR_PMC_DAC_ESCR(x)    		(0x3A8 + x) 
#define X86_MSR_PMC_MOB_ESCR(x)    		(0x3AA + x) 
#define X86_MSR_PMC_PMH_ESCR(x)    		(0x3AC + x) 
#define X86_MSR_PMC_SAAT_ESCR(x)   		(0x3AE + x) 
#define X86_MSR_PMC_U2L_ESCR(x)    		(0x3B0 + x) 
#define X86_MSR_PMC_BPU_ESCR(x)    		(0x3B2 + x) 
#define X86_MSR_PMC_IS_ESCR(x)   		(0x3B4 + x)
#define X86_MSR_PMC_ITLB_ESCR(x)    		(0x3B6 + x)
#define X86_MSR_PMC_IQ_ESCR(x)			(0x3BA + x)
#define X86_MSR_PMC_RAT_ESCR(x) 	  	(0x3BC + x)
#define X86_MSR_PMC_SSU_ESCR(x) 	  	(0x3BE + x)
#define X86_MSR_PMC_MS_ESCR(x)			(0x3C0 + x)
#define X86_MSR_PMC_TBPU_ESCR(x)    		(0x3C2 + x)
#define X86_MSR_PMC_TC_ESCR(x)			(0x3C4 + x)
#define X86_MSR_PMC_IX_ESCR(x)			(0x3C8 + x)
#define X86_MSR_PMC_ALF_ESCR(x) 		(0x3CA + x)

#define X86_MSR_PMC_CRU_ESCR0			0x3B8
#define X86_MSR_PMC_CRU_ESCR1			0x3B9
#define X86_MSR_PMC_CRU_ESCR2			0x3CC
#define X86_MSR_PMC_CRU_ESCR3			0x3CD
#define X86_MSR_PMC_CRU_ESCR4			0x3E0
#define X86_MSR_PMC_CRU_ESCR5			0x3E1
#define X86_MSR_PMC_TC_PRECISE_EVENT		0x3F0

#define X86_MSR_PMC_BPU_CTR_BSU_ESCR		7
#define X86_MSR_PMC_BPU_CTR_FSB_ESCR		6
#define X86_MSR_PMC_BPU_CTR_MOB_ESCR		2
#define X86_MSR_PMC_BPU_CTR_PMH_ESCR		4
#define X86_MSR_PMC_BPU_CTR_BPU_ESCR		0
#define X86_MSR_PMC_BPU_CTR_IS_ESCR		1
#define X86_MSR_PMC_BPU_CTR_ITLB_ESCR		3
#define X86_MSR_PMC_BPU_CTR_IX_ESCR		5

#define X86_MSR_PMC_MS_CTR_MS_ESCR		0
#define X86_MSR_PMC_MS_CTR_TBPU_ESCR		2
#define X86_MSR_PMC_MS_CTR_TC_ESCR		1

#define X86_MSR_PMC_FLAME_CTR_FIRM_ESCR		1
#define X86_MSR_PMC_FLAME_CTR_FLAME_ESCR	0
#define X86_MSR_PMC_FLAME_CTR_DAC_ESCR		5
#define X86_MSR_PMC_FLAME_CTR_SAAT_ESCR		2
#define X86_MSR_PMC_FLAME_CTR_U2L_ESCR		3

#define X86_MSR_PMC_IQ_CTR_CRU_ESCR01	4
#define X86_MSR_PMC_IQ_CTR_CRU_ESCR23	5
#define X86_MSR_PMC_IQ_CTR_CRU_ESCR45	6
#define X86_MSR_PMC_IQ_CTR_IQ_ESCR01	0
#define X86_MSR_PMC_IQ_CTR_RAT_ESCR01	2
#define X86_MSR_PMC_IQ_CTR_SSU_ESCR01	3
#define X86_MSR_PMC_IQ_CTR_ALF_ESCR01	1

#endif /* defined(CONFIG_CPU_X86_I686) */


INLINE void setup_perfmon_cpu(word_t cpuid)
{

#if defined(CONFIG_CPU_X86_I686) || defined(CONFIG_CPU_X86_K8)

    /* disable PerfEvents */
    x86_wrmsr(X86_MSR_PMC_EVTSEL0, 0);
    x86_wrmsr(X86_MSR_PMC_EVTSEL1, 0);
 
    /* clear PMCs */
    x86_wrmsr(X86_MSR_PMC_CTR0, 0);
    x86_wrmsr(X86_MSR_PMC_CTR1, 0);
    
     /* init PMCs */
     x86_wrmsr(X86_MSR_PMC_EVTSEL0, 0x4100C0);  // ENABLE + USER + INST_RETIRED
     x86_wrmsr(X86_MSR_PMC_EVTSEL1, 0x4200C0);  // ENABLE + KRNL + INST_RETIRED


#elif defined(CONFIG_CPU_X86_P4)
#if defined(CONFIG_TBUF_PERFMON_ENERGY) || defined(CONFIG_X_EVT_LOGGING)
    u64_t val;

    // reset performance counters
    for (word_t addr=X86_MSR_PMC_BPU_CCCR(0); addr <= X86_MSR_PMC_IQ_CCCR(5); ++addr)
	x86_wrmsr(addr, 0x30000);

    for (word_t addr=X86_MSR_PMC_BPU_COUNTER(0); addr <= X86_MSR_PMC_IQ_COUNTER(5); ++addr)
	x86_wrmsr(addr, 0);

    // Configure ESCRs

    val = ((u64_t)0x0 << 32) | 0xFC00;
    x86_wrmsr(X86_MSR_PMC_TC_PRECISE_EVENT, val);

    // Enable Precise Event Based Sampling (accurate & low sampling overhead)
    val = ((u64_t)0x0 << 32) | 0x1000001;
    x86_wrmsr(X86_MSR_PEBS_ENABLE, val);

    // Also enabling PEBS
    val = ((u64_t)0x0 << 32) | 0x1;
    x86_wrmsr(X86_MSR_PEBS_MATRIX_VERT, val);

    // Count unhalted cycles
    val = ((u64_t)0x0 << 32) | 0x2600020C;
    x86_wrmsr(X86_MSR_PMC_FSB_ESCR(0), val);

    // Count load uops that are replayed due to unaligned addresses
    // and/or partial data in the Memory Order Valfer (MOB)
    val = ((u64_t)0x0 << 32) | 0x600740C;
    x86_wrmsr(X86_MSR_PMC_MOB_ESCR(0), val);

    // Count op queue writes
    val = ((u64_t)0x0 << 32) | 0x12000E0C;
    x86_wrmsr(X86_MSR_PMC_MS_ESCR(0), val);

    // Count retired branches
    val = ((u64_t)0x0 << 32) | 0x8003C0C;
    x86_wrmsr(X86_MSR_PMC_TBPU_ESCR(0), val);

    // Count x87_FP_uop 
    val = ((u64_t)0x0 << 32) | 0x900000C;
    x86_wrmsr(X86_MSR_PMC_FIRM_ESCR(0), val);

    // Count mispredicted
    val = ((u64_t)0x0 << 32) | 0x600020C;
    x86_wrmsr(X86_MSR_PMC_CRU_ESCR0, val);

    // Count memory retired
    val = ((u64_t)0x0 << 32) | 0x1000020C;
    x86_wrmsr(X86_MSR_PMC_CRU_ESCR2, val);

    // Count load miss level 1 data cache
    val = ((u64_t)0x0 << 32) | 0x1200020C;
    x86_wrmsr(X86_MSR_PMC_CRU_ESCR3, val);

    // uop type
    val = ((u64_t)0x0 << 32) | 0x4000C0C;
    x86_wrmsr(X86_MSR_PMC_RAT_ESCR(0), val);
    // Configure CCCRs

    // Store unhalted cycles
    val = ((u64_t)0x0 << 32) | 0x3D000;
    x86_wrmsr(X86_MSR_PMC_BPU_CCCR(0), val);

    // Store MOB load replay
    val = ((u64_t)0x0 << 32) | 0x35000;
    x86_wrmsr(X86_MSR_PMC_BPU_CCCR(1), val);

    // Store op queue writes
    val = ((u64_t)0x0 << 32) | 0x31000;
    x86_wrmsr(X86_MSR_PMC_MS_CCCR(0), val);

    // Store retired branches
    val = ((u64_t)0x0 << 32) | 0x35000;
    x86_wrmsr(X86_MSR_PMC_MS_CCCR(1), val);

    // Store x87_FP_uop
    val = ((u64_t)0x0 << 32) | 0x33000;
    x86_wrmsr(X86_MSR_PMC_FLAME_CCCR(0), val);

    // Store mispredicted branches
    val = ((u64_t)0x0 << 32) | 0x39000;
    x86_wrmsr(X86_MSR_PMC_IQ_CCCR(0), val);

    // Store memory retired
    val = ((u64_t)0x0 << 32) | 0x3B000;
    x86_wrmsr(X86_MSR_PMC_IQ_CCCR(1), val);

    // Store load miss level 1 data cache
    val = ((u64_t)0x0 << 32) | 0x3B000;
    x86_wrmsr(X86_MSR_PMC_IQ_CCCR(2), val);

    // Store uop type
    val = ((u64_t)0x0 << 32) | 0x35000;
    x86_wrmsr(X86_MSR_PMC_IQ_CCCR(4), val);

    // Setup complete

#else
     /* disable PMCs via CCCR*/
     x86_wrmsr(X86_MSR_PMC_IQ_CCCR(0), 3 << 16);
     x86_wrmsr(X86_MSR_PMC_IQ_CCCR(2), 3 << 16);
     
     /* clear PMCs */
     x86_wrmsr(X86_MSR_PMC_IQ_COUNTER(0), 0);
     x86_wrmsr(X86_MSR_PMC_IQ_COUNTER(2), 0);

     /* 
      * init ESCR0:
      * user
      * event mask (non-bogus tagged and non-tagged) 
      * event select (inst_retired)
      */
     x86_wrmsr(X86_MSR_PMC_CRU_ESCR0, (1 << 2) | (3 << 9) | (2 << 25)); 
     
     /* 
      * init ESCR1:
      * kernel
      * event mask (non-bogus tagged and non-tagged) 
      * event select (inst_retired)
      */
     x86_wrmsr(X86_MSR_PMC_CRU_ESCR1, (1 << 3) | (3 << 9) | (2 << 25)); 

     /* 
      * enable PMCs via CCCR:
      * enable + escr select + reserved
      */
     x86_wrmsr(X86_MSR_PMC_IQ_CCCR(0), (1 << 12) | (X86_MSR_PMC_IQ_CTR_CRU_ESCR01 << 13) | (3 << 16)); 
     x86_wrmsr(X86_MSR_PMC_IQ_CCCR(2), (1 << 12) | (X86_MSR_PMC_IQ_CTR_CRU_ESCR01 << 13) | (3 << 16));

#endif /* CONFIG_TBUF_PERFMON_ENERGY */
#endif /* CONFIG_CPU_X86_P4 */
 
}




#endif /* !__PLATFORM__PC99__PERFMON_H__ */
