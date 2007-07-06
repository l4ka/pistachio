/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/linker.lds
 * Description:	A linker script which generates binaries compatible with
 *		the G3 Open Firmware and psim.  To avoid offending psim
 *		(and the bfd library), define many standard elf symbols
 *		such as _etext, _edata, etc.
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
 * $Id: linker.lds,v 1.27 2004/06/03 14:58:21 joshua Exp $
 *
 ***************************************************************************/

#include INC_GLUE(offsets.h)

ENTRY(_start)

phys_start	= 0x3000;
kernel_offset	= KERNEL_OFFSET;
kernel_cpu_offset = KERNEL_CPU_OFFSET;

text_paddr	= phys_start + SIZEOF_HEADERS;
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
	note	PT_NOTE;
}

SECTIONS
{
	. = text_vaddr;

	. = ALIGN(0x0100);
	.except . : AT (ADDR(.except) - kernel_offset) {
		*(.except)
	} : lower

	. = ALIGN(4K) + (. & (4K - 1));
	_start_kernel = .;
	.init . : AT (ADDR(.init) - kernel_offset) {
		*(.init) *(.init.*) *(.roinit)
	} : kernel
	.kdb . : AT (ADDR(.kdb) - kernel_offset) {
		/* Match all modules in the kdb subdirectory. */
		kdb/*(.text) kdb/*(.rodata*) *(.kdebug) kdb/*(.gnu*)
	} : kernel
	.text . : AT (ADDR(.text) - kernel_offset) {
		*(.text) *(.rodata*) *(.gnu*)
	} : kernel

	. = ALIGN(4K) + (. & (4K - 1));
	_start_syscall = .;
	.syscall . : AT (ADDR(.syscall) - kernel_offset) {
		*(.sc_ipc) *(.sys_ipc)
		*(.sc_schedule) *(.sys_schedule)
		*(.sc_thread_switch) *(.sys_thread_switch)
		*(.sc_xchg_registers) *(.sys_exchange_registers)
		*(.sc_unmap) *(.sys_unmap)
		*(.sc_system_clock)
		*(.sc_memory_ctrl) *(.sys_memory_control)
		*(.sc_processor_ctrl) *(.sys_processor_control)
		*(.sc_thread_ctrl) *(.sys_thread_control)
		*(.sc_space_ctrl) *(.sys_space_control)
		*(.sc_perf)
	} : kernel
	_end_syscall = .;

	PROVIDE (__etext = .);
	PROVIDE (_etext = .);
	PROVIDE (etext = .);

	/*  Align the data at the next 4K page.  It is OK for data
	 *  to overlap code in the BAT mappings, since they use separate
	 *  registers for code and data.
	 */
	. = ALIGN(4K) + (. & (4K - 1));
	_start_data = .;
	.data . : AT (ADDR(.data) - kernel_offset) {
		*(.data) *(.sdata) *(.sdata2) *(.data.kdebug)
	} : data

	.sets . : AT(ADDR(.sets) - kernel_offset) {
		_start_setlist = .;
		*(.setlist)
		_end_setlist = .;
		_start_sets = .;
		*(set_*)
		_end_sets = .;
	} : data

	.kip . : AT (ADDR(.kip) - kernel_offset) {
		/* Align the kip on a page boundary, while also adjusting
		 * its file offset.  Wastes some space in the binary.
		 */
		. = ALIGN(4K);
#include INC_API(kip.ldi)
	} : data
	/* The kip section requires its own page.  So ensure that the
	 * next section lives on the next page.
	 */
	. = ALIGN(4K);
	_end_kip = .;

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(4K - ((memory_descriptors - kip) & (4K-1))) / 8;

	_edata = .;
	PROVIDE (edata = .);

	.bss . : AT (ADDR(.bss) - kernel_offset) {
		_start_bss = .;
		*(.bss) *(.sbss) *(.kdebug-bss)
		. = ALIGN(4);	/* Align by 4-bytes! */
		_end_bss = .;
	} : data

	. = ALIGN(4K);
	_end = .;
	__end = .;
	PROVIDE (end = .);

	_end_data_phys = . - kernel_offset;


	/* Per-cpu data area for the kernel.
	 */
	_cpu_phys = (_end_data_phys + 128K) & ~(128K - 1);
	. = kernel_cpu_offset;
	.cpu . : AT (_cpu_phys) {
		*(.data.cpulocal)
	} : cpu

	/* The end of the physical memory required by the kernel.
	 */
	. = ALIGN(4K);
	_end_kernel_phys = _cpu_phys + (. - kernel_cpu_offset);


	/* Some OpenFirmware implementations want a .note section which
	 * describes the load environment.
	 */
	.note . : AT (_end_kernel_phys) {
		*(.note)
	} : note


	/* Throw away all other sections. */
	/DISCARD/ : {
		*(*)
		*(.comment)
	}
}

