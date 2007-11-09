/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia32/debugregs.h
 * Description:   ia32 debug registers
 *                see: IA-32 Vol 3, Debugging and Performance Mon.
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
 * $Id: debugregs.h,v 1.2 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X32__DEBUGREGS_H__
#define __ARCH__X86__X32__DEBUGREGS_H__

INLINE void load_debug_regs(u32_t * dbg)
{
    __asm__ __volatile__ (
	"	mov 0(%0), %%eax	\n"
	"	mov %%eax, %%dr0	\n"
	"	mov 4(%0), %%eax	\n"
	"	mov %%eax, %%dr1	\n"
	"	mov 8(%0), %%eax	\n"
	"	mov %%eax, %%dr2	\n"
	"	mov 12(%0), %%eax	\n"
	"	mov %%eax, %%dr3	\n"
	"	mov 24(%0), %%eax	\n"
	"	mov %%eax, %%dr6	\n"
	"	mov 28(%0), %%eax	\n"
	"	mov %%eax, %%dr7	\n"
	:
	:"r"(dbg)
	:"eax");
}

INLINE void save_debug_regs(u32_t * dbg)
{
    __asm__ __volatile__ (
	"	mov %%dr0, %%eax	\n"
	"	mov %%eax, 0(%0)	\n"
	"	mov %%dr1, %%eax	\n"
	"	mov %%eax, 4(%0)	\n"
	"	mov %%dr2, %%eax	\n"
	"	mov %%eax, 8(%0)	\n"
	"	mov %%dr3, %%eax	\n"
	"	mov %%eax, 12(%0)	\n"
	"	mov %%dr6, %%eax	\n"
	"	mov %%eax, 24(%0)	\n"
	"	mov %%dr7, %%eax	\n"
	"	mov %%eax, 28(%0)	\n"
	:
	:"r"(dbg)
	:"eax");
}

#endif /* !__ARCH__X86__X32__DEBUGREGS_H__ */
