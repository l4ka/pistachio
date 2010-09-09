/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008, 2010,  Karlsruhe University
 *                
 * File path:     arch/x86/tracebuffer.h
 * Description:   X86 specific tracebuffer
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
 ********************************************************************/

#ifndef __ARCH__X86__TRACEBUFFER_H__
#define __ARCH__X86__TRACEBUFFER_H__

#include INC_ARCH(cpu.h)
#include <tcb_layout.h>

#if defined(CONFIG_TBUF_PERFMON_ENERGY)
#define TRACEBUFFER_SIZE        (32 * 1024 * 1024)
#else
#define TRACEBUFFER_SIZE        ( 4 * 1024 * 1024)
#endif

/**********************************************************************
 *                       Sample PMC energy weights
 **********************************************************************/
#if 0
/* P4 */
#define X86_PMC_TSC_WEIGHT                  (617)           
#define X86_PMC_UC_WEIGHT                   (712)
#define X86_PMC_MQW_WEIGHT                  (475)
#define X86_PMC_RB_WEIGHT                    (56)
#define X86_PMC_MB_WEIGHT                 (34046)
#define X86_PMC_MR_WEIGHT                   (173)                 
#define X86_PMC_MLR_WEIGHT                 (2996)
#define X86_PMC_LDM_WEIGHT                 (1355)
#else
/* Pentium D */
#define X86_PMC_TSC_WEIGHT                 (1418)  
#define X86_PMC_UC_WEIGHT                  (1285)  
#define X86_PMC_LDM_WEIGHT                 (881)   
#define X86_PMC_MR_WEIGHT                  (649)   
#define X86_PMC_MB_WEIGHT                  (23421) 
#define X86_PMC_MLR_WEIGHT                 (4320)       
#define X86_PMC_RB_WEIGHT                  (840)   
#define X86_PMC_MQW_WEIGHT                 (75)    
#endif

#define X86_PMC_TSC_SHIFT                  6
#define X86_PMC_UC                         (0) 
#define X86_PMC_MLR                        (1) 
#define X86_PMC_MQW                        (4) 
#define X86_PMC_RB                         (5) 
#define X86_PMC_MB                         (12) 
#define X86_PMC_MR                         (13)          
#define X86_PMC_LDM                        (14)

INLINE void tracerecord_t::store_arch(const traceconfig_t config)
{
    tsc = x86_rdtsc();
    
    if (config.pmon)
    {
        switch (config.pmon_cpu)
        {
        case 0:
            // P2/P3/K8
            pmc0 = x86_rdpmc(0);
            pmc1 = x86_rdpmc(1);
            break;
        case 1:
            // P4
            if (config.pmon_e)
            {
                u64_t pmce =
                    X86_PMC_TSC_WEIGHT *  x86_rdtsc() +
                    X86_PMC_UC_WEIGHT  *  x86_rdpmc(X86_PMC_UC)  +
                    X86_PMC_MLR_WEIGHT *  x86_rdpmc(X86_PMC_MLR) +
                    X86_PMC_MQW_WEIGHT *  x86_rdpmc(X86_PMC_MQW) +
                    X86_PMC_RB_WEIGHT  *  x86_rdpmc(X86_PMC_RB)  +
                    X86_PMC_MB_WEIGHT  *  x86_rdpmc(X86_PMC_MB)  +
                    X86_PMC_MR_WEIGHT  *  x86_rdpmc(X86_PMC_MR)  +
                    X86_PMC_LDM_WEIGHT *  x86_rdpmc(X86_PMC_LDM);
                
                pmc0 = (word_t) pmce;
                pmc1 = (word_t) (pmce >> 32);
            }
            else
            {
                pmc0 = (word_t) x86_rdpmc(12);
                pmc1 = (word_t) x86_rdpmc(14);
            }
            break;
        default:
            break;
        }
    }

}
   
INLINE void tracebuffer_t::initialize()
{
    magic = TRACEBUFFER_MAGIC;
    current = 0;
    mask = TBUF_DEFAULT_MASK;
    max = (TRACEBUFFER_SIZE/sizeof(tracerecord_t))-1;
    config.raw = 0;
#if defined(CONFIG_SMP)
    config.smp = 1;
#endif
#if defined(CONFIG_TBUF_PERFMON)
    config.pmon = 1;
#endif
#if defined(CONFIG_TBUF_PERFMON_ENERGY)
    config.pmon_e = 1;
#endif
#if defined(CONFIG_CPU_X86_P4)
    config.pmon_cpu = 1;
#endif

    printf("sz %d rec %d max %d\n", TRACEBUFFER_SIZE, sizeof(tracerecord_t), max);
}
#endif /* !__ARCH__X86__TRACEBUFFER_H__ */
