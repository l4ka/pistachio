/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     ia32-syscall-init.cc
 * Description:   Relocation of syscall gates
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
 * $Id: ia32-syscall-init.cc,v 1.4 2003/09/24 19:06:28 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <l4/kdebug.h>
#include <l4/kip.h>

#define FIXUP(syscall)							\
do {									\
    void __L4_##syscall (void);						\
    *(L4_Word8_t *) (__L4_##syscall) = 0xe9;				\
    *(L4_Word32_t *) ((L4_Word_t) __L4_##syscall + 1) =			\
	(L4_Word_t) kip + kip->syscall - (L4_Word_t) __L4_##syscall - 5;\
} while (0)


extern char __L4_syscalls_start;
extern char __L4_syscalls_end;
extern char __L4_syscalls_copy_start;
extern char __L4_syscalls_copy_end;

extern "C" void __L4_copy_syscalls_out (void)
{
    char * s = &__L4_syscalls_start;
    char * e = &__L4_syscalls_end;
    char * d = &__L4_syscalls_copy_start;

    while (s < e)
	*d++ = *s++;
}

extern "C" void __L4_copy_syscalls_in (L4_Word_t dest)
{
    char * s = &__L4_syscalls_copy_start;
    char * e = &__L4_syscalls_copy_end;
    char * d = (char *) dest;

    while (s < e)
	*d++ = *s++;
}

extern "C" void __L4_init_syscalls (void)
{
    L4_KernelInterfacePage_t * kip = (L4_KernelInterfacePage_t *)
	L4_KernelInterface ();

    // Make copy before starting to modify.
    __L4_copy_syscalls_out ();

    //L4_KDB_Enter ("Fixup syscalls");
    
    FIXUP (Ipc);
    FIXUP (Lipc);
    FIXUP (ExchangeRegisters);
    FIXUP (ThreadControl);
    FIXUP (SystemClock);
    FIXUP (ThreadSwitch);
    FIXUP (Schedule);
    FIXUP (Unmap);
    FIXUP (SpaceControl);
    FIXUP (ProcessorControl);
    FIXUP (MemoryControl);
}
