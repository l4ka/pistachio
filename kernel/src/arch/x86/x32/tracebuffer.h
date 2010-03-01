/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008, 2010,  Karlsruhe University
 *                
 * File path:     arch/x86/x32/tracebuffer.h
 * Description:   IA32 specific tracebuffer
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
#ifndef __ARCH__X86__X32__TRACEBUFFER_H__
#define __ARCH__X86__X32__TRACEBUFFER_H__

#include <tcb_layout.h>

#define TRACEBUFFER_MAGIC       0x143acebf
#define TRACEBUFFER_PGENTSZ        pgent_t::size_4m


/*
 * Access to stack pointer, timestamp, and performance monitoring counters
 */
    
#define TBUF_SP     "   mov     %%esp, %%fs:3*%c9(%0)           \n"     \


/* Registers:
 *      EAX unused
 *      EBX unused, not preserved
 *      ECX unused
 *      EDX unused
 *      ESI unused, not preserved
 *      EDI TB record address
 *      EBP unused, not preserved 
 */

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

#define X86_PMC_TSC                        (0) 
#define X86_PMC_TSC_SHIFT                  6
#define X86_PMC_UC                         (0) 
#define X86_PMC_RB                         (5) 
#define X86_PMC_MR                         (13)          
#define X86_PMC_LDM                        (14)

#define TBUF_RDPMC_WEIGHTED(nr, weight, op)                             \
    "   mov $"MKSTR(nr)", %%ecx      \n" /* select pmc            */    \
    "  "#op"                         \n" /* pmc/tsc in %edx:%eax  */    \
    "   mov  %%edx, %%esi            \n" /* save pmc_hi           */    \
    "   mov  $"MKSTR(weight)", %%ecx \n" /* load weight           */    \
    "   mul  %%ecx                   \n" /* r1 = pmc_lo * weight  */    \
    "   mov  %%edx, %%ebx            \n" /* save r1_hi            */    \
    "   xchg %%eax, %%ecx            \n" /* r1_lo <-> weight      */    \
    "   mul  %%esi                   \n" /* r2 = weight * pmc_hi  */    \
    "   add  %%ebx, %%eax            \n" /* r2_lo += r1_hi        */    \
    "   addl %%ecx, %%fs:4*%c9(%0)   \n" /* result_lo             */    \
    "   adcl %%eax, %%fs:5*%c9(%0)   \n" /* result_hi             */    



#define TBUF_RDPMCS                                                     \
    /* Get config into rbx/ebx  */                                      \
    "   push  %%ebx                   \n" /* save ebx           */      \
    "   movl  %%fs:4*%c9, %%ebx       \n"                               \
    "   test $2, %%ebx                \n"                               \
    "   jz 5f                         \n" /* no pmon, rdtsc only */     \
    "   test $4, %%ebx                \n"                               \
    "   jz 4f                         \n" /* P2/K8 pmon */              \
    "   test $8, %%ebx                \n"                               \
    "   jz 3f                         \n" /* P4 pmon */                 \
    /* P4 PerfMon Energy  */                                            \
    "   push %%ebx                    \n" /* save ebx           */      \
    "   push %%esi                    \n" /* save esi           */      \
    "   movl $0, %%fs:4*%c9(%0)       \n" /* result_lo          */      \
    "   movl $0, %%fs:5*%c9(%0)       \n" /* result_hi          */      \
    TBUF_RDPMC_WEIGHTED(X86_PMC_UC,  X86_PMC_UC_WEIGHT,  rdpmc)         \
    TBUF_RDPMC_WEIGHTED(X86_PMC_RB,  X86_PMC_RB_WEIGHT,  rdpmc)         \
    TBUF_RDPMC_WEIGHTED(X86_PMC_MR,  X86_PMC_MR_WEIGHT,  rdpmc)         \
    TBUF_RDPMC_WEIGHTED(X86_PMC_LDM, X86_PMC_LDM_WEIGHT, rdpmc)         \
    TBUF_RDPMC_WEIGHTED(X86_PMC_TSC, X86_PMC_TSC_WEIGHT, rdtsc)         \
    "   pop %%esi                     \n" /* restore esi           */   \
    "   pop %%ebx                     \n" /* restore ebx           */   \
    "   rdtsc                         \n"                               \
    "   shrd    $"MKSTR(X86_PMC_TSC_SHIFT)", %%edx, %%eax\n"            \
    "   mov     %%eax, %%fs:2*%c9(%0) \n"                               \
    "   jmp 6f                        \n"                               \
    "3:                               \n"                               \
    /* P4 PerfMon  */                                                   \
    "   mov     $12, %1               \n"                               \
    "   rdpmc                         \n"                               \
    "   movl    %%eax, %%fs:4*%c9(%0) \n"                               \
    "   add     $2, %1                \n"                               \
    "   rdpmc                         \n"                               \
    "   movl    %%eax, %%fs:5*%c9(%0) \n"                               \
    "   jmp 5f                        \n"                               \
    "4:                               \n"                               \
    /* P2/K8 PerfMon  */                                                \
    "   xor  %1, %1                   \n"                               \
    "   rdpmc                         \n"                               \
    "   movl    %%eax, %%fs:4*%c9(%0) \n"                               \
    "   mov  $1, %1                   \n"                               \
    "   rdpmc                         \n"                               \
    "   movl    %%eax, %%fs:5*%c9(%0) \n"                               \
    "5:                               \n"                               \
    "   rdtsc                         \n"                               \
    "   mov     %%eax, %%fs:2*%c9(%0) \n"                               \
    "6:                               \n"                               \
    "   pop %%ebx                     \n" /* restore ebx           */   \

#endif /* !__ARCH__X86__X32__TRACEBUFFER_H__ */
