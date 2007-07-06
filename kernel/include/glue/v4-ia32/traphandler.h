/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/traphandler.h
 * Description:   trap handler for IA32-traps
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
 * $Id: traphandler.h,v 1.7 2004/05/31 14:59:25 stoess Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__TRAPHANDLER_H__
#define __GLUE__V4_IA32__TRAPHANDLER_H__

/* debugging exceptions */
extern "C" void exc_debug(void);
extern "C" void exc_nmi(void);
extern "C" void exc_breakpoint(void);

/* gp and pagefault */
extern "C" void exc_gp(void);
extern "C" void exc_pagefault(void);

/* math */
extern "C" void exc_nomath_coproc(void);
extern "C" void exc_fpu_fault(void);
extern "C" void exc_simd_fault(void);

/* system */
extern "C" void exc_catch(void);
extern "C" void exc_invalid_opcode(void);

/* catcher for invalid interrupts */
typedef void (*func_exc)(void);
extern u64_t exc_catch_all[IDT_SIZE] UNIT("ia32.exc_all");
extern "C" void exc_catch_common_wrapper(void) UNIT("ia32.exc_common");
extern "C" void exc_catch_common(void) UNIT("ia32.exc_common");

#endif /* !__GLUE__V4_IA32__TRAPHANDLER_H__ */
