/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     platform/sb1/smp.h
 * Description:   mips64 sibyte MP implementation
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
 * $Id: smp.h,v 1.3 2006/03/01 14:10:32 ud3 Exp $
 *                
 ********************************************************************/

#ifndef __PLATFORM__SB1__SMP_H__
#define __PLATFORM__SB1__SMP_H__

#include INC_API(types.h)
#include INC_GLUE(smp.h)
#include INC_PLAT(sb1250_regs.h)
#include INC_PLAT(sb1250_scd.h)
#include INC_PLAT(sb1250_int.h)
#include INC_ARCH(addrspace.h)

#define IMR_POINTER(cpu,reg) \
    ((volatile word_t*)(MIPS64_ADDR_K1(A_IMR_REGISTER(cpu,reg))))

INLINE void mips64_send_ipi(cpuid_t cpu)
{
    *IMR_POINTER(cpu, R_IMR_MAILBOX_SET_CPU) = 1;
}

INLINE void mips64_clear_ipi(cpuid_t cpu)
{
    *IMR_POINTER(cpu, R_IMR_MAILBOX_CLR_CPU) = ~(0ul);
}

#endif
