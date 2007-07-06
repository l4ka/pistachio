SECTIONS
{
	. = 0x180000;
        .text : {
        *(.mb_header)
        *(.text)
        *(.*data*)
        _edata = .;
        }
        _end = .;
}
