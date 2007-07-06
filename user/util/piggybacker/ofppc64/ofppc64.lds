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
