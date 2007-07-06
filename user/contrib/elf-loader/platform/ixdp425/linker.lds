ENTRY(_start)

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x01000000;
  _text = .;
  .text : { 
	*(.text.start)
	*(.text) 
  }
  _etext = .;

  /* Global data */
  _data = .;
  .rodata : { 
	*(.rodata) 
	*(.rodata.*) 
	. = ALIGN(16);
	mod_kernel_start = .;
	*(.mod_kernel)
	mod_kernel_end = .;
	. = ALIGN(16);
	mod_sigma0_start = .;
	*(.mod_sigma0)
	mod_sigma0_end = .;
	. = ALIGN(16);
	mod_sigma1_start = .;
	*(.mod_sigma1)
	mod_sigma1_end = .;
	. = ALIGN(16);
	mod_root_start = .;	
	*(.mod_root)
	mod_root_end = .;	
  }
  .data    : { *(.data) *(.gnu.linkonce.d*) CONSTRUCTORS }
  _gp = ALIGN (16) + 0x7ff0;
  .got           : { *(.got.plt) *(.got) }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata     	: { *(.sdata) }
  _edata  =  .;

  _bss = .;
  .sbss      : { *(.sbss) *(.scommon) }
  .bss       : { *(.bss) *(COMMON) }
  . = ALIGN(64 / 8);
  _end = . ;

  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* These must appear regardless of  .  */
}
