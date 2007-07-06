/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/pleb2/linker.lds
 * Description:   PLEB2 Linker Script
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
 * $Id: linker.lds,v 1.1 2004/12/02 22:04:07 cvansch Exp $
 *
 ********************************************************************/

#include INC_PLAT(offsets.h)

ENTRY(_start)
_start_text		= VIRT_ADDR_BASE;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
		. = ALIGN(64K);
		*(.base)
		*(.text.startup)
		*(.text)
		*(.gnu.linkonce.*)
	}

	. = ALIGN(4K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		. = ALIGN(2K);
		*(.user.*)
	}
	. = ALIGN(4K);
	_end_kip = .;

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(2K - ((memory_descriptors - kip) & (4K-1))) / 16;

	. = ALIGN(32K);
	_start_data = .;
	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
	{
		*(.data.kspace)
		*(.sdata)
		*(.data)
		*(.data.*)
		*(.bss)
		*(.rodata)
		*(.rodata.*)
	}

	.kdebug . : AT(ADDR(.kdebug) - KERNEL_OFFSET)
	{
		*(.kdebug)
		*(.kdebug-bss)
	}

#include <generic/linkersets.ldi>

	_end_text = ALIGN(4K);

	. = ALIGN(4K);

	.init . : AT(ADDR(.init) - KERNEL_OFFSET)
	{
                *(.init.head)
		*(.init)
		*(.init.*)
		*(.roinit)
		*(.init.data)

		. = ALIGN(4K);
		_bootstack_bottom = .;
		. = . + 8K;
		_bootstack_top = .;
		_bootstack_top_phys = . - KERNEL_OFFSET;
	}

	/* special section that is discarded during linking
	   all unwanted sections should go here */

	/* Required for user code! */
	_end = ALIGN(8K);
	/DISCARD/ :
	{
/*		*(*)            <-- THIS CAUSES SEGFAULT!!! */
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
}
