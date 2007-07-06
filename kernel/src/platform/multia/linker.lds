/* check ALIGN(4K) before use !!! */

OUTPUT_FORMAT("elf64-alpha")

/* Evil */
#include INC_API(config.h)

ENTRY(_start)
#if 1
#if CONFIG_ALPHA_ADDRESS_BITS == 43
_start_text = (0xfffffc0000000000 + CONFIG_ALPHA_CONSOLE_RESERVE);
#else
_start_text = (0xffff800000000000 + CONFIG_ALPHA_CONSOLE_RESERVE);
#endif /* CONFIG_ALPHA_ADDRESS_BITS */
#endif

BOOTMEM_SIZE	= 1024K;
KERNEL_OFFSET   = 0;

SECTIONS
{
	_start_image = _start_text;
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
		*(.got) *(.got.plt)
	}
	. = ALIGN(8K);
	.kip .  : AT (ADDR(.kip) - KERNEL_OFFSET)
	{
#include INC_API(kip.ldi)
		*(.user.*)
	}
	. = ALIGN(8K);

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

	. = ALIGN(8K);
	_start_bootmem = .;
	_start_bootmem_phys = _start_bootmem - KERNEL_OFFSET;
	. = . + BOOTMEM_SIZE;
	_end_bootmem = .;
	_end_bootmem_phys = _end_bootmem - KERNEL_OFFSET;

	.init _end_bootmem : AT(ADDR(.init) - KERNEL_OFFSET)
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

	/DISCARD/ :
	{
		*(*)
		*(.eh_frame)
		*(.note)
		*(.comment)
	}
	_end_image = .;
}
