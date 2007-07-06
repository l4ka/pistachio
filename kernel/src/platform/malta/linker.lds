/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     platform/malta/linker.lds
 * Description:   Linker script for the Malta platform (mips32)
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
 * $Id: linker.lds,v 1.2 2006/10/27 17:34:57 reichelt Exp $
 *                
 ********************************************************************/


ENTRY(_start)

KERNEL_OFFSET = 0;

/* 0x0 - 0x400 = exception vectors */
_start_text_phys	= 0xA0000400; 

_start_text		= KERNEL_OFFSET + _start_text_phys;

SECTIONS
{
	.text _start_text : 
	{
		. = ALIGN(0x100);
		*(.text.startup)
		*(.text)
		*(.base)
		*(.gnu.linkonce.*)
	}

	. = ALIGN(4K);
	.rodata . :
	{
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(4K);
	.kip .  : 
	{
#include INC_API(kip.ldi)
		. = ALIGN(2K);
		*(.user.*)
	}
	. = ALIGN(4K);

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(2K - ((memory_descriptors - kip) & (4K-1))) / 16;

	.data . :
	{
		*(.sdata)
		*(.data)
		*(.data.*)
		*(.sbss)
		*(.bss)
	}

	.kdebug . :
	{
		*(.kdebug)
		*(.kdebug-bss)
	}


		.sets . :
        {
                . = ALIGN(16);
                _start_setlist = .;
                *(.setlist)
                _end_setlist = .;
                . = ALIGN(16);
                _start_sets = .;
                *(SORT(set_*))
                _end_sets = .;
        }

	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - KERNEL_OFFSET;

	. = ALIGN(4K);
	.init _end_text :
	{
		*(.init)
		*(.init.*)
		*(.roinit)
		*(.init.data)
	}

	. = ALIGN(8K);
	_bootstack_bottom = .;
	. = . + 8K;
	_bootstack_top = .;

	/* special section that is discarded during linking
	   all unwanted sections should go here */

	/* Required for user code! */
	_end = ALIGN(8K) - KERNEL_OFFSET;
	/DISCARD/ :
	{
		*(*)
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
}
