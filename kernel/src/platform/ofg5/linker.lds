/****************************************************************************
 *
 * Copyright (C) 2003,2005  National ICT Australia
 *
 * File path:	platform/ofg5/linker.lds
 * FIXME
 * Description:	A linker script which generates binaries compatible with
 *		the G5 Open Firmware.
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
 * $Id: linker.lds,v 1.1 2005/01/18 13:49:46 cvansch Exp $
 *
 ***************************************************************************/

#include INC_GLUE(offsets.h)

ENTRY(_start)

phys_start	= KERNEL_PHYS_START;
kernel_offset	= KERNEL_OFFSET;
kernel_cpu_offset = KERNEL_CPU_OFFSET;

text_paddr	= phys_start;
text_vaddr	= text_paddr + kernel_offset;

PHDRS
{
	/*  List program headers in the order that matches their section
	 *  offsets in the file.
	 */
	lower	PT_LOAD;
	kernel	PT_LOAD;
	data	PT_LOAD;
	cpu	PT_LOAD;
}

SECTIONS
{
	. = text_vaddr;

	. = ALIGN(0x0100);
	.except . : AT (ADDR(.except) - kernel_offset) {
		*(.except)
	} : lower

	. = ALIGN(4K); 
	_start_kernel = .;
	_start_kernel_phys = . - kernel_offset;
	.init . : AT (ADDR(.init) - kernel_offset) {
		*(.init) *(.init.*) *(.roinit)
	} : kernel
	. = ALIGN(4K);
	.kdb . : AT (ADDR(.kdb) - kernel_offset) {
		/* Match all modules in the kdb subdirectory. */
		kdb/*(.text) kdb/*(.rodata) *(.kdebug) kdb/*(.gnu*)
	} : kernel
	.text . : AT (ADDR(.text) - kernel_offset) {
		*(.text) *(.rodata*) *(.gnu*)
		*(.got1)
	} : kernel

	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	/*  Align the data at the next 4K page.
	 */
	. = ALIGN(4K);
	_start_data = .;
	.data . : AT (ADDR(.data) - kernel_offset) {
		*(.data) *(.sdata) *(.sdata2) *(.data.kdebug)
		*(.data.rel*)
		*(.got.plt) *(.got)
	} : data

	.sets . : AT(ADDR(.sets) - KERNEL_OFFSET)
	{
		. = ALIGN(16);
		_start_setlist = .;
		*(.setlist)
		_end_setlist = .;
		. = ALIGN(16);
		_start_sets = .;
		*(SORT(set_*))
		_end_sets = .;
	} : data

	. = ALIGN(4K);
	.kip . : AT (ADDR(.kip) - kernel_offset) {
		/* Align the kip on a page boundary, while also adjusting
		 * its file offset.  Wastes some space in the binary.
		 */
#include INC_API(kip.ldi)
		/* Leave space for memory desriptors */
		. = ALIGN(2K);
		*(.user.*)
	} : data
	/* The kip section requires its own page.  So ensure that the
	 * next section lives on the next page.
	 */
	. = ALIGN(4K);

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(2K - ((memory_descriptors - kip) & (2K-1))) / 8;

	.toc . : AT (ADDR(.toc) - kernel_offset) {
	    *(.toc)
	} : data

	. = ALIGN(16);
	.toc1 . : AT (ADDR(.toc1) - kernel_offset) {
	    *(.toc1)
	} : data
	. = ALIGN(16);
	.opd . : AT (ADDR(.opd) - kernel_offset) {
	    *(.opd)
	} : data

	_edata = .;
	PROVIDE (edata = .);

	. = ALIGN(16);
	.bss . : AT (ADDR(.bss) - kernel_offset) {
		_start_bss = .;
		*(.bss) *(.sbss) *(.kdebug-bss)
		. = ALIGN(8);	/* Align by 8-bytes! */
		_end_bss = .;
	} : data

	. = ALIGN(4K);
	_end = .;
	__end = .;
	PROVIDE (end = .);

	_end_data_phys = . - kernel_offset;


	/* The end of the physical memory required by the kernel.
	 */
	. = ALIGN(4K);
	_end_kernel_phys = . - kernel_offset;


	/* Per-cpu area for the kernel. (dynamicly allocated)
	 */
	. = kernel_cpu_offset;
	_start_cpu_ = .;
	.cpu . : AT (_end_kernel_phys) {
		*(.data.cpulocal)
	} : cpu
	_end_cpu_ = .;


	/* Throw away all other sections. */
	/DISCARD/ : {
		*(*)
		*(.comment)
	}
}

