/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 *                
 * File path:     piggybacker/ofppc64/ofppc64.lds
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
OUTPUT_FORMAT("elf64-powerpc")
OUTPUT_ARCH(powerpc:common64)
ENTRY(_start)

SECTIONS
{
    . = 0x400000;
    .blah : {
	LONG(0)
    }
    . = 0x400080;

    __text_start = .;
    .text : { 
	*(.start) 
	*(.text) 
    }
    __text_end = .;

    . = ALIGN(4k);

    .rodata : {
	*(.rodata*)
    }
    . = ALIGN(4k);

    .data : {
	*(.data*)
	*(.sdata*)
    }

    . = ALIGN(32);
    __toc_start = .;
    __toc_value = . + 0x8000;
    .toc : {
	*(.toc)
    }
    __toc_end = .;

    . = ALIGN(4k);
    __bss_start = .;
    .bss : {
	*(.sbss)
	*(.scommon)
	*(.bss)
	*(COMMON)
    }
    . = ALIGN(4k);
    __bss_end = . ;

    _end = . ;
    PROVIDE (end = .);
}
