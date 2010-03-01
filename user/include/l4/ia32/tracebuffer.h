/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     l4/ia32/tracebuffer.h
 * Description:   Functions for accessing the tracebuffer
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
#ifndef __L4__IA32__TRACEBUFFER_H__
#define __L4__IA32__TRACEBUFFER_H__

#include <l4/types.h>

/* Turn preprocessor symbol definition into string */
#define MKSTR(sym)      MKSTR2(sym)
#define MKSTR2(sym)     #sym

#if defined(L4_64BIT)
# define __PLUS32       + 32
#else
# define __PLUS32
#endif


#define L4_TRACEBUFFER_MAGIC            (0x143acebf)
#define L4_TRACEBUFFER_NUM_ARGS         (9)

/*
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
typedef struct
{
    struct {
        L4_Word_t       utype   : 16;
        L4_Word_t       __pad0  : 16 __PLUS32;
        L4_Word_t       cpu     : 16;
        L4_Word_t       id      : 16 __PLUS32;
    } X;
    
    L4_Word_t   tsc;
    L4_Word_t   thread;
    L4_Word_t   pmc0;
    L4_Word_t   pmc1;
    L4_Word_t   str;
    L4_Word_t   data[9];
} L4_TraceRecord_t;


/*
 * Access to performance monitoring counters
 */

#include <l4/ia32/arch.h>

#define __L4_TBUF_RDPMC_WEIGHTED(nr, weight, op)                        \
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



#define __L4_TBUF_RDPMCS                                                \
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
    __L4_TBUF_RDPMC_WEIGHTED(L4_X86_PMC_UC,  L4_X86_PMC_UC_WEIGHT,  rdpmc)   \
    __L4_TBUF_RDPMC_WEIGHTED(L4_X86_PMC_RB,  L4_X86_PMC_RB_WEIGHT,  rdpmc)   \
    __L4_TBUF_RDPMC_WEIGHTED(L4_X86_PMC_MR,  L4_X86_PMC_MR_WEIGHT,  rdpmc)   \
    __L4_TBUF_RDPMC_WEIGHTED(L4_X86_PMC_LDM, L4_X86_PMC_LDM_WEIGHT, rdpmc)   \
    __L4_TBUF_RDPMC_WEIGHTED(L4_X86_PMC_TSC, L4_X86_PMC_TSC_WEIGHT, rdtsc)   \
    "   pop %%esi                     \n" /* restore esi           */   \
    "   pop %%ebx                     \n" /* restore ebx           */   \
    "   rdtsc                         \n"                               \
    "   shrd    $"MKSTR(L4_X86_PMC_TSC_SHIFT)", %%edx, %%eax\n"         \
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


/*
 * Tracebuffer access macros
 */

#define __L4_TBUF_INCREASE_COUNTER(ctr)                         \
do {                                                            \
    asm volatile (                                              \
        "   testl $1, %%fs:4*%c1               \n"              \
        "   jz 2f                              \n"              \
        "   lock;                              \n"              \
        "   inc     %%fs:8*%c1(%0)             \n"              \
        :                                                       \
        :                                                       \
        "r" ((ctr & 0x7) * 4),                                  \
        "i" (sizeof(L4_Word_t)));                               \
} while (0)


#define __L4_TBUF_GET_NEXT_RECORD(type, id)                             \
    ({                                                                  \
        L4_Word_t _dummy, _addr;                                        \
        asm volatile (                                                  \
            /* Check wheter to filter the event */                      \
            "   mov     %%fs:2*%c9, %3                  \n"             \
            "   and     %1, %3                          \n"             \
            "   jz      7f                              \n"             \
            "   or      %2, %1                          \n"             \
                                                                        \
            /* Get record offset into EDI */                            \
            "1: mov     %%fs:1*%c9, %3                  \n"             \
            "   mov     %8, %0                          \n"             \
            "   mov     %0, %2                          \n"             \
            "   add     %3, %0                          \n"             \
            "   and     %%fs:3*%c9, %0                  \n"             \
            "   cmovz   %2, %0                          \n"             \
            "   testl   $1, %%fs:4*%c9                  \n"             \
            "   jz 2f                                   \n"             \
            "   lock;                                   \n"             \
            "2:                                         \n"             \
            "   cmpxchg %0, %%fs:1*%c9                  \n"             \
            "   jnz     1b                              \n"             \
                                                                        \
                                                                        \
            /* Store type, cpu, id, thread, counters */                 \
            "   mov     %1, %2                         \n"              \
            "   movzx   %%cx, %%ecx                     \n"             \
            "   movl    %%ecx, %%fs:(%0)               \n"              \
            "   mov     %%gs:0, %1                      \n"             \
            "   movw    "MKSTR(__L4_TCR_PROCESSOR_NO)"*%c9(%1), %%dx\n" \
            "   movl    %%edx, %%fs:1*%c9(%0)   \n"                     \
            "   mov     "MKSTR(__L4_TCR_MY_GLOBAL_ID)"*%c9(%1), %2\n"   \
            "   mov     %2, %%fs:3*%c9(%0)              \n"             \
            __L4_TBUF_RDPMCS                                            \
            "7:                                         \n"             \
            :                                                           \
                "=D" (_addr),                    /* 0  */               \
                "=c" (_dummy),                   /* 1  */               \
                "=d" (_dummy),                   /* 2  */               \
                "=a" (_dummy)                    /* 3  */               \
            :                                                           \
                "0" (0),                         /* 4  */               \
                "1" (type & 0xffff),             /* 5  */               \
                "2" ((id & 0xffff)<<16),         /* 6  */               \
                "3" (0),                         /* 7  */               \
                "i" (sizeof (L4_TraceRecord_t)), /* 8  */               \
                "i" (sizeof(L4_Word_t))          /* 9  */               \
            );                                                          \
        _addr;                                                          \
    })


/**
 * Record (format) string into event buffer.
 *
 * @param addr          offset of event record
 * @param offset        string to be recorded
 */
#define __L4_TBUF_STORE_STR(addr, str)                          \
do {                                                            \
    * (volatile char *) str;                                    \
    asm volatile (                                              \
        "mov   %0, %%fs:6*%c2(%1)\n"                    \
        :                                                       \
        :                                                       \
        "r" (str),                                              \
        "D" (addr),                                             \
        "i" (sizeof(L4_Word_t)));                               \
} while (0)


/**
 * Record arguments into event buffer at indicated location.
 *
 * @param addr          offset of event record
 * @param offset        offset within event record
 * @param item          value to be recorded
 */
#define __L4_TBUF_STORE_DATA(addr, offset, item)                \
do {                                                            \
    L4_Word_t _dummy;                                           \
    asm volatile (                                              \
        "mov  %2, %%fs:(%1)\n"                          \
        :                                                       \
        "=D" (_dummy)                                           \
        :                                                       \
        "0" (addr + (7 + offset) * sizeof(L4_Word_t)),          \
        "r" (item));                                            \
} while (0)

/**
 * Record arguments into event buffer at indicated location.
 *
 * @param addr          offset of event record
 * @param offset        offset within event record
 * @param item          value to be recorded
 */
#define L4_TBUF_SET_TYPEMASK(mask)                              \
    do {                                                        \
        L4_Word_t _dummy;                                       \
        asm volatile (                                          \
            "mov  %0, %%fs:2*%c2                        \n"     \
            :                                                   \
            "=D" (_dummy)                                       \
            :                                                   \
            "0" (mask),                                         \
            "i" (sizeof(L4_Word_t)));                           \
    } while (0)

#undef __PLUS32


#endif /* !__L4__IA32__TRACEBUFFER_H__ */
