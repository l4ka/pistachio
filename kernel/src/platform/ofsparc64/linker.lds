/****************************************************************************
 *
 * Copyright (C) 2003, University of New South Wales
 *
 * File path:	 platform/ofsparc64/linker.lds
 * Description:	 Kernel linker script for SPARC v9 architectures with OpenBoot
 *               (Open Firmware) running on them.
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
 * $Id: linker.lds,v 1.4 2004/02/12 02:02:25 philipd Exp $
 *
 ***************************************************************************/

OUTPUT_FORMAT("elf64-sparc")

ENTRY(_start)

#include INC_GLUE_API_PLAT(offsets.h)

_start_text = KERNEL_AREA_START;
_start_text_laddr = _start_text - KERNEL_OFFSET;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
#ifndef CONFIG_SMP
		*(.tsb)	      /* TSB (software TLB). */	
#endif /* !CONFIG_SMP */
		ttable = .;
		ttable_laddr = ttable - KERNEL_OFFSET;
#include INC_ARCH(ttable.ldi) /* Trap Table */
		*(.text)
		*(.gnu.linkonce.*)
	}
	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata)
		*(.rodata.*)
	}
	. = ALIGN(8K);
	.kip . : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
		kip_laddr = . - KERNEL_OFFSET;
#include INC_API(kip.ldi)
		. = ALIGN(2K);
		*(.user.*)
	}
	. = ALIGN(8K);

	_memory_descriptors_offset = memory_descriptors - kip;
        _memory_descriptors_size =
                (8K - ((memory_descriptors - kip) & (8K-1))) / 16;

	_start_data = .;

#ifdef CONFIG_SMP
#error awiggins (08-09-03): SMP does not work fix this and TRAPTYPE2ADDR().
	/* CPU local data section */
	_start_cpu_local = KERNEL_CPULOCAL_START;
	.cpulocal _start_cpu_local : AT(_end)
	{
		*(.tsb)           /* TSB (software TLB). */
		*(.data.cpulocal)
	}
	_end_cpu_local = .;

#endif /* CONFIG_SMP */

	.data _start_data : AT (ADDR(.data) - KERNEL_OFFSET)
	{
		*(.sdata)
		*(.data)
		*(.data.*)
		*(.bss)
		*(.sbss) 
	}

	.kdebug . : AT(ADDR(.kdebug) - KERNEL_OFFSET)
	{
		*(.kdebug)
		*(.kdebug-bss)

		. = ALIGN(8K);
		_kdebugstack_bottom = .;
		. = . + 8K;
		_kdebugstack_top = .;
	}

#include <generic/linkersets.ldi>

	. = ALIGN(8K);
	_end_text = .;

	.init _end_text : AT(ADDR(.init) - KERNEL_OFFSET)
	{
		*(.init)
		*(.init.*)
		*(.roinit)
	}

	. = ALIGN(8K);
	_end_init = .;
	_end_init_laddr = _end_init - KERNEL_OFFSET;

	/**
	 *  Boot area is a one-one mapping area used to kickstart
	 *  the kernel. We have to do this because OpenBoot refused
	 *  to map in the upper half of the virtual address space
         *  for us.
	 */
	.boot _end_init_laddr : AT(ADDR(.boot) - BOOT_OFFSET)
	{
		*(.boot)
		*(.boot.*)

		. = ALIGN(8k);
		_bootstack_bottom = .;
		. = . + 8K;
		_bootstack_top = .;
	}

	/* Required for user code! */
	_end = ALIGN(8K) - BOOT_OFFSET;

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
