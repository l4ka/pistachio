/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008, 2010,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/tracebuffer.h
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
#ifndef __ARCH__X86__X64__TRACEBUFFER_H__
#define __ARCH__X86__X64__TRACEBUFFER_H__

#include <tcb_layout.h>

#define TRACEBUFFER_MAGIC       0x1464b123acebf
#define TRACEBUFFER_PGENTSZ     pgent_t::size_2m

/*
 * Access to stack pointer, timestamp, and performance monitoring counters
 */

#define TBUF_SP         "       mov     %%rsp, %%fs:3*%c9(%0)           \n"     

#define X86_PMC_TSC_SHIFT                  0

#include INC_ARCH(tracebuffer.h)

#define TBUF_RDPMCS                                                     \
    /* Get config into rbx/ebx  */                                      \
    "   push %%rbx                    \n" /* save ebx           */      \
    "   movq    %%fs:4*%c9, %%rbx     \n"                               \
    "   test    $2, %%rbx             \n"                               \
    "   jz 4f                         \n" /* no pmon, rdtsc only */     \
    "   test    $4, %%rbx             \n"                               \
    "   jz 3f                         \n" /* P2/K8 pmon */              \
    /* P4 PerfMon  */                                                   \
    "   mov     $12, %1               \n"                               \
    "   rdpmc                         \n"                               \
    "   shl     $32, %%rdx            \n"                               \
    "   movl    %%eax, %%edx          \n"                               \
    "   mov     %2, %%fs:4*%c9(%0)    \n"                               \
    "   add     $2, %1                \n"                               \
    "   rdpmc                         \n"                               \
    "   shl     $32, %%rdx            \n"                               \
    "   movl    %%eax, %%edx          \n"                               \
    "   mov     %2, %%fs:5*%c9(%0)    \n"                               \
    "   jmp 4f                        \n"                               \
    "3:                               \n"                               \
    /* P2/K8 PerfMon  */                                                \
    "   xor  %1, %1                   \n"                               \
    "   rdpmc                         \n"                               \
    "   shl     $32, %%rdx            \n"                               \
    "   movl    %%eax, %%edx          \n"                               \
    "   mov     %2, %%fs:4*%c9(%0)    \n"                               \
    "   mov  $1, %1                   \n"                               \
    "   rdpmc                         \n"                               \
    "   shl     $32, %%rdx            \n"                               \
    "   movl    %%eax, %%edx          \n"                               \
    "   mov     %2, %%fs:5*%c9(%0)    \n"                               \
    "4:                               \n"                               \
    "   rdtsc                         \n"                               \
    "   shl     $32, %%rdx            \n"                               \
    "   movl    %%eax, %%edx          \n"                               \
    "   mov     %2, %%fs:2*%c9(%0)    \n"                               \
    "   pop %%rbx                     \n" /* restore ebx           */   \



#endif /* !__ARCH__X86__X64__TRACEBUFFER_H__ */
