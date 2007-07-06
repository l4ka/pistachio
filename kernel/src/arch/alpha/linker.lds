/* check ALIGN(4K) before use !!! */

OUTPUT_FORMAT("elf64-alpha")

#include INC_GLUE(offsets.h)

ENTRY(_start)

_start_text = (KERNEL_OFFSET + CONFIG_ALPHA_CONSOLE_RESERVE);

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
	.got : AT (ADDR(.got) - KERNEL_OFFSET)
	{
		*(.got)
	}
	. = ALIGN(8K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		. = ALIGN(4K);
		*(.user.*)
	}
	. = ALIGN(8K);

	_memory_descriptors_offset = memory_descriptors - kip;
        _memory_descriptors_size =
                (8K - ((memory_descriptors - kip) & (8K-1))) / 16;

	.data . :  AT (ADDR(.data) - KERNEL_OFFSET)
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
	}

#include <generic/linkersets.ldi>

	_end_text = ALIGN(8K);
	_end_text_phys = _end_text - KERNEL_OFFSET;
	_start_text_phys = _start_text - KERNEL_OFFSET;
	. = ALIGN(8K);

	.init _end_text : AT(ADDR(.init) - KERNEL_OFFSET)
	{
		*(.init)
		*(.init.*)
		*(.roinit)
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
