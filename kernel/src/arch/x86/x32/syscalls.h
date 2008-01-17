/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     pistachio/kernel/include/arch/ia32/syscalls.h
 * Description:   syscall macros
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
 * $Id: syscalls.h,v 1.3 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH_X86_X32_SYSCALL_H__
#define __ARCH_X86_X32_SYSCALL_H__

#include INC_API(ipc.h)

/*
 * register allocation for system calls 
 */
#define IPC_MR0		"a"
#define IPC_MR1		"S"
#define IPC_MR2		"D"
#define IPC_UTCB	"c"
#define IPC_TO		"d"
#define IPC_FROM	"D"
#define IPC_TIMEOUT	"S"


#define return_ipc_error()				\
{							\
    asm("mov %1,%%esp	\n"				\
        "ret		\n"				\
	:						\
	: IPC_MR0(IPC_MR0_ERROR), 			\
	"r"(__builtin_frame_address(0))			\
	);						\
}


#endif /*__ARCH_X86_X32_SYSCALL_H__*/
