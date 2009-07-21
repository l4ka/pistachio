/*********************************************************************
 *                
 * Copyright (C) 1999-2005, 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/linker-pc99.lds
 * Description:   Generic linkser script for x86.
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
 * $Id: linker-pc99.lds,v 1.3 2005/01/28 14:11:31 skoglund Exp $
 *                
 ********************************************************************/


#include INC_GLUE(config.h)

ENTRY(_start)

BOOTMEM_SIZE = 128K;

_start_text_phys = 0x00100000 + 0x200;
_start_text = _start_text_phys + KERNEL_OFFSET;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
		*(.mb_header)
		*(.text)
		*(.gnu.linkonce.*)
		*(.spinlock)
	}

	/* The user-level accessible sysexit trampoline code */
	. = ALIGN(4K);
	_start_utramp = .;
	. = UTRAMP_MAPPING;
	.utramp . : AT (_start_utramp - KERNEL_OFFSET)
	{
		*(.utramp)
	}
	. = _start_utramp + 4K;

	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata*)
	}
	. = ALIGN(4K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		. = ALIGN(2K);
		*(.user.*)
	}

	. = ALIGN(4K);
	_start_cpu_local = .;
	.cpulocal . : AT (ADDR(.cpulocal) - KERNEL_OFFSET)
	{
		*(.data.cpulocal)
		*(.data.x86.cpulocal)
#if defined(CONFIG_DEBUG)
		. = . + 8;
#endif
		*(.data.x86.cpulocal.tss)

	}
	_end_cpu_local = .;

	. = ALIGN(4K);
	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
	{
		*(.data)
	        *(.data.x86.idt);
	        *(.data.x86.exc_all);
	        *(.data.x86.exc_common);
		*(.data.*)
		_start_bss = .;
		*(.bss)
		_end_bss = .;
	}

	. = ALIGN(4K);
	.kdebug . : AT(ADDR(.kdebug) - KERNEL_OFFSET)
	{
		*(.kdebug)
		*(.kdebug-bss)
	}

#include <generic/linkersets.ldi>

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(2K - ((memory_descriptors - kip) & (2K-1))) / 8;

	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - KERNEL_OFFSET;

	. = ALIGN(4K);
	_start_bootmem = .;
	. = . + BOOTMEM_SIZE;
	_end_bootmem = .;

	_start_init = . - KERNEL_OFFSET;
	.init (. - KERNEL_OFFSET) : AT (ADDR(.init))
	{
		*(.init)
		*(.roinit)
		*(.init.data)
		*(.init.smp)
		*(.init.*)
#include <generic/mdb.ldi>
#include <generic/ctors.ldi>
	}
	. = ALIGN(4K);
	_end_init = .;

#include <generic/debug.ldi>

	/* special section that is discarded during linking
	   all unwanted sections should go here */
/*
	/DISCARD/ :
	{
		*(*)
		*(.eh_frame)
		*(.note)
		*(.comment)
	}*/

}
/* all the physical address calculations */
_end_text_phys = _end_text - KERNEL_OFFSET;
_start_bootmem_phys = _start_bootmem - KERNEL_OFFSET;
_end_bootmem_phys = _end_bootmem - KERNEL_OFFSET;
_start_utramp_p = _start_utramp - KERNEL_OFFSET;

