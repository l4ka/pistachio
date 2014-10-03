/*********************************************************************
 *                
 * Copyright (C) 1999-2004,  Karlsruhe University
 *                
 * File path:     platform/efi/linker.lds
 * Description:   Generic linker script for IA-64.
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
 * $Id: linker.lds,v 1.26 2004/03/10 15:38:47 skoglund Exp $
 *                
 ********************************************************************/

OUTPUT_FORMAT("elf64-ia64-little", "elf64-ia64-little", "elf64-ia64-little")
OUTPUT_ARCH(ia64)
ENTRY(_start_phys)

RR0 =		0x0000000000000000;
RR1 =		0x2000000000000000;
RR2 =		0x4000000000000000;
RR3 =		0x6000000000000000;
RR4 =		0x8000000000000000;
RR5 =		0xA000000000000000;
RR6 =		0xC000000000000000;
RR7 =		0xE000000000000000;

BOOTMEM_SIZE =		16M;
SYSCALL_STUBS_SIZE =	16K;
KIP_SIZE =		4K;

PHYSMEM_OFFSET = 	CONFIG_IA64_PHYSMEM_OFFSET;
KERNEL_OFFSET =		PHYSMEM_OFFSET;
CPULOCAL_MEM_START =	RR7 + PHYSMEM_OFFSET - 256M;
_start_text_phys =	0x05000000 + 0x200;
_start_text =		_start_text_phys + RR7 + PHYSMEM_OFFSET;
_start_phys =		_start - PHYSMEM_OFFSET;
_syscall_stubs =	CPULOCAL_MEM_START - 1M;

SECTIONS
{
	.init _start_text : AT (ADDR(.init) - KERNEL_OFFSET)
	{
		*(.init)
		*(.roinit)
		*(.init.data)
	}

	. = ALIGN(4K);

	.text . : AT (ADDR(.text) - KERNEL_OFFSET)
	{
		*(.ivt)
		*(.text)
		src/*/*/*/* (.gnu.linkonce.t.*)
		src/*/*/* (.gnu.linkonce.t.*)
		src/*/* (.gnu.linkonce.t.*)
	}

	. = ALIGN(SYSCALL_STUBS_SIZE);
	_syscall_stubs_mem = .;
	_syscall_stubs_phys = . - RR7 - KERNEL_OFFSET;
	. = _syscall_stubs;
	.syscallstubs . : AT (_syscall_stubs_mem - KERNEL_OFFSET)
	{
		*(.user.*)
	}
	. = ALIGN(SYSCALL_STUBS_SIZE);
	. = _syscall_stubs_mem + SYSCALL_STUBS_SIZE;

	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata)
		*(.rodata.*)
	}

	__gp = ALIGN(8) + 0x200000;
	.got : AT (ADDR(.got) - KERNEL_OFFSET) { *(.got.plt) *(.got) }

	. = ALIGN(KIP_SIZE);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
	}
	. = ALIGN(KIP_SIZE);

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(KIP_SIZE - ((memory_descriptors - kip) & (KIP_SIZE -1))) / 16;

	. = ALIGN(16K);
	_start_cpu_local_mem = .;
	. = CPULOCAL_MEM_START;
	_start_cpu_local = .;
	.cpulocal _start_cpu_local : AT (_start_cpu_local_mem - KERNEL_OFFSET)
	{
		*(.data.cpulocal)
	}

	_end_cpu_local = .;
	. = _start_cpu_local_mem + SIZEOF(.cpulocal);
	_end_cpu_local_mem = .;

	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
	{
		*(.sdata)
		*(.data)
		*(.data.kdebug)
		*(.sbss)
		*(.bss)
		*(.opd)
	}

	.kdebug . : AT (ADDR(.kdebug) - KERNEL_OFFSET)
	{
		*(.kdebug)
		kdb/*/*/*/* (.gnu.linkonce.t.*)
		kdb/*/*/* (.gnu.linkonce.t.*)
		kdb/*/* (.gnu.linkonce.t.*)
		*(.kdebug-bss)
	}

#include <generic/linkersets.ldi>

	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - RR7 - KERNEL_OFFSET;

	. = ALIGN(4K);
	_start_bootmem = .;
	_start_bootmem_phys = _start_bootmem - RR7 - KERNEL_OFFSET;
	. = . + BOOTMEM_SIZE;
	_end_bootmem = .;
	_end_bootmem_phys = _end_bootmem - RR7 - KERNEL_OFFSET;
	. = ALIGN(4K);

	/* special section that is discarded during linking
	   all unwanted sections should go here */

	/DISCARD/ :
	{
		*(*)
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
}

