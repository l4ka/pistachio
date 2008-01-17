/*********************************************************************
 *                
 * Copyright (C) 2002-2006, 2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/x32comp/user.h
 * Description:   user system calls for Compatibility Mode
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
 * $Id: user.h,v 1.1 2006/10/20 17:32:57 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__X64__X32COMP__USER_H__
#define __GLUE__V4_X86__X64__X32COMP__USER_H__

extern "C" void SECTION(".user.syscall_32.ipc") user_ipc_32();
extern "C" void SECTION(".user.syscall_32.lipc") user_lipc_32();
extern "C" void SECTION(".user.syscall_32.exregs") user_exchange_registers_32();
extern "C" void SECTION(".user.syscall_32.threadctrl") user_thread_control_32();
extern "C" void SECTION(".user.syscall_32.sysclock") user_system_clock_32();
extern "C" void SECTION(".user.syscall_32.threadswtch") user_thread_switch_32();
extern "C" void SECTION(".user.syscall_32.schedule") user_schedule_32();
extern "C" void SECTION(".user.syscall_32.unmap") user_unmap_32();
extern "C" void SECTION(".user.syscall_32.spacectrl") user_space_control_32();
extern "C" void SECTION(".user.syscall_32.procctrl") user_processor_control_32();
extern "C" void SECTION(".user.syscall_32.memctrl") user_memory_control_32();

#endif /* !__GLUE__V4_X86__X64__X32COMP__USER_H__ */
