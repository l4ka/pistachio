/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/fp.h
 * Description:   Handling of IA-65 floating-point registers
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
 * $Id: fp.h,v 1.1 2003/11/03 16:24:00 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__FP_H__
#define __ARCH__IA64__FP_H__

#include INC_ARCH(psr.h)


/*
 * Implemented in arch/ia64/subr_asm.S
 */
extern "C" void ia64_save_highfp (void * buf);
extern "C" void ia64_restore_highfp (void * buf);


/**
 * Container class for holding the upper (f32--f127) spilled floating
 * point registers.
 */
class high_fp_t
{
    // Each spilled fp register occupies 16 bytes.
    u64_t registers[96*2];

public:

    /**
     * Save high floating-point registers to memory.
     */
    void save (void)
	{ ia64_save_highfp ((void *) registers); }

    /**
     * Restore high floating-point registers from memory.
     */
    void restore (void)
	{ ia64_restore_highfp ((void *) registers); }
};    


/**
 * Enable high floating-point registers.
 */
INLINE void ia64_enable_fphigh (void)
{
    __asm__ __volatile__ ("rsm psr.dfh");
}


/**
 * Disable high floating-point registers.
 */
INLINE void ia64_disable_fphigh (void)
{
    __asm__ __volatile__ ("ssm psr.dfh");
}


#endif /* !__ARCH__IA64__FP_H__ */
