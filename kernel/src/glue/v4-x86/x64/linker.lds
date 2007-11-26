/*********************************************************************
 *                
 * Copyright (C) 2003-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/linker.lds
 * Description:   AMD64 PC99 Linker Script
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
 * $Id: linker.lds,v 1.5 2006/10/27 16:49:45 reichelt Exp $
 *                
 ********************************************************************/
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH("i386:x86-64")

#include INC_GLUE_SA(offsets.h)

BOOTMEM_SIZE	= 1024K;

/* #define KERNEL_OFFSET 0  */
_start_text_phys = 0x400000;
_start_text = _start_text_phys + KERNEL_OFFSET;


SECTIONS
{
	. = ALIGN(4k); 
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
	    *(.text); 
	    *(.gnu.linkonce.*);
	    *(.spinlock);
	}
	. = ALIGN(4k); 
	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
	    *(.rodata);
	    *(.rodata.*);
	}

	. = ALIGN(4K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#define KIP_SECTION kip
#include INC_API(kip.ldi)
#undef KIP_SECTION
	}

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(4K - ((memory_descriptors - kip) & (4K-1))) / 16;

	. = ALIGN(KERNEL_PAGE_SIZE);
	_start_syscalls = .;
	.syscalls .  : AT (ADDR(.syscalls) - KERNEL_OFFSET)
	{
		*(.user.syscall.ipc);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.lipc);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.exregs);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.threadctrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.sysclock);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.threadswtch);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.schedule);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.unmap);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.spacectrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.procctrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall.memctrl);
	}
	. = ALIGN(KERNEL_PAGE_SIZE);
	_end_syscalls = .;

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
	. = ALIGN(4K);
	.kip_32 .  : AT (ADDR(.kip_32) - KERNEL_OFFSET)
	{
#define KIP_SECTION kip_32
#include INC_API(kip.ldi)
#undef KIP_SECTION

		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.ipc);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.lipc);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.exregs);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.threadctrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.sysclock);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.threadswtch);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.schedule);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.unmap);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.spacectrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.procctrl);
		. = ALIGN(SYSCALL_ALIGN);
		*(.user.syscall_32.memctrl);
	}

	_memory_descriptors_offset_32 = memory_descriptors_32 - kip_32;
	_memory_descriptors_size_32 =
		(2K - ((memory_descriptors_32 - kip_32) & (2K-1))) / 8;
	_memory_descriptors_raw_32 = (_memory_descriptors_offset_32 << 16)
		+ _memory_descriptors_size_32;
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

	. = ALIGN(KERNEL_PAGE_SIZE);
	_start_cpu_local = .;
	.cpulocal . : AT (ADDR(.cpulocal) - KERNEL_OFFSET)
	{
		*(.data.cpulocal)
		*(.data.x86.cpulocal)
	}
	. = ALIGN(KERNEL_PAGE_SIZE);
	_end_cpu_local = .; 

	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
	{
	    *(.sdata);
	    *(.data);
	    *(.data.x86.idt);
	    *(.data.x86.exc_all);
	    *(.data.x86.exc_common);
	    *(.data.*);
	    _start_bss = .;	
	    *(.bss);
	    _end_bss = .;	
	}

	. = ALIGN(4K);
	.kdebug . : AT(ADDR(.kdebug) - KERNEL_OFFSET)
	{
	    *(.kdebug);
	    *(.kdebug-bss);
	    *(.comment);	
	}
	. = ALIGN(4K);
#include <generic/linkersets.ldi> 
	. = ALIGN(4K);
	_end_text = .;


	. = ALIGN(4K);
	_start_bootmem = .;
	. = . + BOOTMEM_SIZE;
	_end_bootmem = .;

	. = ALIGN(4K);
	 _start_init = . - KERNEL_OFFSET;	
	.init (. - KERNEL_OFFSET) : AT(ADDR(.init))
	{
	    *(.init.startup32);
	    . = ALIGN(1K);
	    *(.init.init32) FILL(0x90909090);
	    . = . + 1K;
            . = ALIGN(1K);
	    *(.init.init64);
	    *(.init);
	    *(.roinit);
	    *(.init.data);
	    *(.init.memory);
  	    *(.init.cpulocal);
	    *(.init.*);
#include <generic/mdb.ldi>
#include <generic/ctors.ldi>
	} 
	_end_init = .;
	/* special section that is discarded during linking
	   all unwanted sections should go here */
	
	. = ALIGN(4K);

#include <generic/debug.ldi>

/*	/DISCARD/ :
	{
	    *(*);
	    *(.eh_frame);
	    *(.note);
	    *(.comment); 
	} */	
}

/* physical addresses */
_end_text_phys = _end_text - KERNEL_OFFSET;
_start_syscalls_phys = _start_syscalls - KERNEL_OFFSET;
_end_syscalls_phys = _end_syscalls - KERNEL_OFFSET;
_start_bootmem_phys = _start_bootmem - KERNEL_OFFSET;
_end_bootmem_phys = _end_bootmem - KERNEL_OFFSET;
