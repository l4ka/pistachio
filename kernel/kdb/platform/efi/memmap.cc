/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/platform/efi/memmap.cc
 * Description:   EFI memory map dump command
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
 * $Id: memmap.cc,v 1.4 2003/09/24 19:05:18 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include INC_PLAT(memory_map.h)



/**
 * cmd_efi_memmap: dump the EFI memory map
 */
DECLARE_CMD (cmd_efi_memmap, arch, 'm', "memmap", "dump EFI memory map");

CMD(cmd_efi_memmap, cg)
{
    efi_memory_desc_t * desc;

    static const char * maptypes[] = {
	"reserved",
	"loader code",
	"loader data",
	"boot services code",
	"boot services data",
	"runtime services code",
	"runtime services data",
	"conventional memory",
	"unusuable memory",
	"ACPI reclaim memory",
	"ACPI memory nvs",
	"memory mapped I/O",
	"memory mapped I/O port space",
	"pal code"
    };

    printf ("EFI memory map:\n");

    efi_memmap.reset ();
    while ((desc = efi_memmap.next ()) != NULL)
    {
	// Name
	int n = 30 - printf ("  %s", maptypes[desc->type ()]);
	while (n-- > 0) printf (" ");

	// Phyiscal location
	printf ("  0x%p - 0x%p  ",
		desc->physical_start (),
		(word_t) desc->physical_start () +
		(desc->number_of_pages () * 4096));

	// Size
	word_t size = desc->number_of_pages () * 4096;
	if (size >= 1024*1024*1024)
	    printf ("%5dGB  ", size / (1024*1024*1024));
	else if (size >= 1024*1024)
	    printf ("%5dMB  ", size / (1024*1024));
	else
	    printf ("%5dKB  ", size / 1024);

	// Attributes
	u64_t a = desc->attribute ();
	if (a & EFI_MEMORY_UC) printf (" UC");
	if (a & EFI_MEMORY_WC) printf (" WC");
	if (a & EFI_MEMORY_WT) printf (" WT");
	if (a & EFI_MEMORY_WB) printf (" WB");
	if (a & EFI_MEMORY_WP) printf (" WP");
	if (a & EFI_MEMORY_RP) printf (" RP");
	if (a & EFI_MEMORY_XP) printf (" XP");
	if (a & EFI_MEMORY_RUNTIME) printf (" RUNTIME");

	printf ("\n");
    }

    return CMD_NOQUIT;
}
