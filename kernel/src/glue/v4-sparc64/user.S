/*********************************************************************
 *                
 * Copyright (C) 2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/user.S
 * Description:  User-level entry points for system calls
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
 * $Id: user.S,v 1.1 2004/02/12 05:48:52 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(syscalls.h)

#define SYSCALL(name)						\
BEGIN_PROC(user_##name,".user."STR(name))			\
	ta	SYSCALL_##name;					\
	retl;							\
	 nop

#define SYSCALL_UNIMPLEMENTED(name)				\
BEGIN_PROC(user_##name,".user."STR(name))			\
	retl;							\
	 nop

SYSCALL(ipc)
SYSCALL(exchange_registers)
SYSCALL(thread_control)
SYSCALL(thread_switch)
SYSCALL(schedule)
SYSCALL(unmap)
SYSCALL(space_control)
SYSCALL(memory_control)

SYSCALL_UNIMPLEMENTED(system_clock)
SYSCALL_UNIMPLEMENTED(lipc)
SYSCALL_UNIMPLEMENTED(processor_control)
