/* check ALIGN(4K) before use !!! */

#include INC_GLUE(offsets.h)

ENTRY(_start)
_start_text_phys	= 0x10000;
_start_text		= KERNEL_OFFSET + _start_text_phys;

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
	.rodata . : AT (ADDR(.rodata) - KERNEL_OFFSET)
	{
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(4K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		. = ALIGN(2K);
		*(.user.*)
	}
	. = ALIGN(4K);

	_memory_descriptors_offset = memory_descriptors - kip;
	_memory_descriptors_size =
		(2K - ((memory_descriptors - kip) & (4K-1))) / 16;

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

#include <generic/linkersets.ldi>

	_end_text = ALIGN(4K);
	_end_text_phys = _end_text - KERNEL_OFFSET;

	. = ALIGN(4K);
	.init _end_text : AT(ADDR(.init) - KERNEL_OFFSET)
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
