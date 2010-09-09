/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	lib/l4/powerpc64-syscalls.c
 * Description:	PowerPC64 syscall pointers.
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
 * $Id: powerpc64-syscalls.c,v 1.4 2003/10/29 22:47:34 cvansch Exp $
 *
 ***************************************************************************/
#include <l4/kip.h>

#if !defined(NULL)
#define NULL 0
#endif

__L4_Ipc_t __L4_Ipc = NULL;
__L4_Lipc_t __L4_Lipc = NULL;
__L4_Unmap_t __L4_Unmap = NULL;
__L4_Schedule_t __L4_Schedule = NULL;
__L4_ThreadSwitch_t __L4_ThreadSwitch = NULL;
__L4_SystemClock_t __L4_SystemClock = NULL;
__L4_ExchangeRegisters_t __L4_ExchangeRegisters = NULL;

__L4_ThreadControl_t __L4_ThreadControl = NULL;
__L4_SpaceControl_t __L4_SpaceControl = NULL;
__L4_ProcessorControl_t __L4_ProcessorControl = NULL;
__L4_MemoryControl_t __L4_MemoryControl = NULL;

__L4_RtasCall_t __L4_RtasCall = NULL;

void __L4_Init( void )
{
    L4_KernelInterfacePage_t *kip;
    
    kip = L4_KernelInterface( NULL, NULL, NULL );

#define KIP_RELOC(a) ((L4_Word_t)kip + a)

    __L4_Ipc = (__L4_Ipc_t) KIP_RELOC(kip->Ipc);
    __L4_Lipc = (__L4_Lipc_t) KIP_RELOC(kip->Lipc);
    __L4_Unmap = (__L4_Unmap_t) KIP_RELOC(kip->Unmap);
    __L4_Schedule = (__L4_Schedule_t) KIP_RELOC(kip->Schedule);
    __L4_ThreadSwitch = (__L4_ThreadSwitch_t) KIP_RELOC(kip->ThreadSwitch);
    __L4_SystemClock = (__L4_SystemClock_t) KIP_RELOC(kip->SystemClock);
    __L4_ExchangeRegisters = (__L4_ExchangeRegisters_t) KIP_RELOC(kip->ExchangeRegisters);

    __L4_ThreadControl = (__L4_ThreadControl_t) KIP_RELOC(kip->ThreadControl);
    __L4_SpaceControl = (__L4_SpaceControl_t) KIP_RELOC(kip->SpaceControl);
    __L4_ProcessorControl = (__L4_ProcessorControl_t) KIP_RELOC(kip->ProcessorControl );
    __L4_MemoryControl = (__L4_MemoryControl_t) KIP_RELOC(kip->MemoryControl);

    if (kip->ArchSyscall0)
	__L4_RtasCall = (__L4_RtasCall_t) KIP_RELOC(kip->ArchSyscall0);
#undef KIP_RELOC
}

