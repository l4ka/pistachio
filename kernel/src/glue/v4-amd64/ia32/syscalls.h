/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/syscalls.h
 * Description:   syscall dispatcher for 32-bit programs
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
 * $Id: syscalls.h,v 1.1 2006/10/20 19:50:54 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__GLUE__V4_AMD64__IA32__SYSCALLS_H__
#define __API__GLUE__V4_AMD64__IA32__SYSCALLS_H__

extern "C" amd64_sysret_t syscall_dispatcher_32(word_t arg1,  /* RDI */
					        word_t arg2,  /* RSI */
					        word_t arg3,  /* RDX */
					        word_t uip,   /* RCX */
					        word_t arg4,  /* R08 */
					        word_t arg5,  /* R09 */
					        word_t arg6,  /* stack (RAX) */
					        word_t arg7   /* stack (RBX) */);

#endif /* !__API__GLUE__V4_AMD64__IA32__SYSCALLS_H__ */
