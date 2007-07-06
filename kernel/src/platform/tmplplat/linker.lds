/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/tmplplat/linker.lds
 * Description:	Linker script template.
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
 * $Id: linker.lds,v 1.4 2006/10/27 17:35:18 reichelt Exp $
 *
 ***************************************************************************/


/* check ALIGN(4K) before use !!! */

BOOTMEM_SIZE	= 512K;
KERNEL_OFFSET	= 0;
_start_text	= 0;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
		*(.text)
		*(.gnu.linkonce.*)
	}

	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(4K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		. = ALIGN(4K);
		*(.user.*)
	}
	. = ALIGN(4K);

	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
	{
		*(.sdata)
		*(.data)
		*(.data.*)
		*(.bss)
	}

	.kdebug . : AT(ADDR(.kdebug) - KERNEL_OFFSET)
	{
		*(.kdebug)
		*(.kdebug-bss)
	}

	.sets . : AT(ADDR(.sets) - KERNEL_OFFSET)
	{
		_start_setlist = .;
		*(.setlist)
		_end_setlist = .;
		_start_sets = .;
		*(SORT(set_*))
		_end_sets = .;
	}

	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - KERNEL_OFFSET;

	. = ALIGN(4K);
	_start_bootmem = .;
	_start_bootmem_phys = _start_bootmem - KERNEL_OFFSET;
	. = . + BOOTMEM_SIZE;
	_end_bootmem = .;
	_end_bootmem_phys = _end_bootmem - KERNEL_OFFSET;

	.init _end_bootmem :
	{
		*(.init)
		*(.roinit)
		*(.init.data)
	}

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
