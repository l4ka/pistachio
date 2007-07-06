/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     api/v4/user.h
 * Description:   user system calls
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
 * $Id: user.h,v 1.4 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__USER_H__
#define __API__V4__USER_H__

extern "C" void SECTION(".user.syscall.ipc") user_ipc();
extern "C" void SECTION(".user.syscall.lipc") user_lipc();
extern "C" void SECTION(".user.syscall.exregs") user_exchange_registers();
extern "C" void SECTION(".user.syscall.threadctrl") user_thread_control();
extern "C" void SECTION(".user.syscall.sysclock") user_system_clock();
extern "C" void SECTION(".user.syscall.threadswtch") user_thread_switch();
extern "C" void SECTION(".user.syscall.schedule") user_schedule();
extern "C" void SECTION(".user.syscall.unmap") user_unmap();
extern "C" void SECTION(".user.syscall.spacectrl") user_space_control();
extern "C" void SECTION(".user.syscall.procctrl") user_processor_control();
extern "C" void SECTION(".user.syscall.memctrl") user_memory_control();

#endif /* !__API__V4__USER_H__ */
