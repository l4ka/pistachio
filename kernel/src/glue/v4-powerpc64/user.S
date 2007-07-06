/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/syscalls.S
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   Kernel entry points for syscalls
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
 * $Id: user.S,v 1.5 2004/06/04 06:38:41 cvansch Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(frame.h)
#include INC_GLUE(syscalls.h)

#define BEGIN_SYS(name)			\
    .global name;			\
    .align 3;				\
    .type name,@function;		\
    .func name;				\
name:

#define END_SYS(name)			\
    .endfunc


#define SYSCALL(name)	    \
.section    .user.##name, "ax";	    \
BEGIN_SYS(user_##name);	    \
    li	r0, SYSCALL_##name;	    \
    sc;				    \
    blr;			    \
END_SYS(user_##name);		    \
.previous

#define SYSCALL_UNIMPLEMENTED(name) \
.section    .user.##name, "ax";	    \
BEGIN_SYS(user_##name);		    \
    blr;			    \
END_SYS(user_##name);		    \
.previous

SYSCALL(ipc)
SYSCALL(thread_switch)
SYSCALL(thread_control)
SYSCALL(exchange_registers)
SYSCALL(schedule)
SYSCALL(unmap)
SYSCALL(space_control)
SYSCALL_UNIMPLEMENTED(processor_control)
SYSCALL(memory_control)
SYSCALL_UNIMPLEMENTED(system_clock)
SYSCALL_UNIMPLEMENTED(lipc)

#if (CONFIG_PLAT_OFPOWER4 || CONFIG_PLAT_OFPOWER3)
SYSCALL(rtas_call)
#endif
