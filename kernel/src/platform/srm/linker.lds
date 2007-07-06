/* check ALIGN(4K) before use !!! */

OUTPUT_FORMAT("elf64-alpha")

/* Evil */
#include INC_API(config.h)

ENTRY(_start)
#if CONFIG_ALPHA_ADDRESS_BITS == 43
KERNEL_OFFSET = 0xfffffc0000000000;
#else
KERNEL_OFFSET = 0xffff800000000000;
#endif /* CONFIG_ALPHA_ADDRESS_BITS */

_start_text = (KERNEL_OFFSET + CONFIG_ALPHA_CONSOLE_RESERVE);
BOOTMEM_SIZE	= 1024K;

SECTIONS
{
	.text _start_text : AT (ADDR(.text) - KERNEL_OFFSET)
	{
		*(.text)
		*(.gnu.linkonce.*)
	}

	_pal_code_start = .;	
	.text_pal . : AT (ADDR(.text_pal) - KERNEL_OFFSET)
	{
		*(.pal_code)
	}
	_pal_code_end = .;	

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
		*(.user.*)
	}
	. = ALIGN(8K);

	_memory_descriptors_offset = memory_descriptors - kip;
        _memory_descriptors_size =
                (8K - ((memory_descriptors - kip) & (8K-1))) / 8;

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
