/*********************************************************************
 *                
 * Copyright (C) 2004-2006,  Karlsruhe University
 *                
 * File path:     bootinfo.cc
 * Description:   generic bootinfo creation functions
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
 * $Id: bootinfo.cc,v 1.2 2006/10/22 19:38:53 reichelt Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>

#include "bootinfo.h"

#include "kickstart.h"
#include "lib.h"
#include "elf.h"


#if defined(BOOTINFO_32on64)
#define BI_NS BI32
#elif defined(BOOTINFO_64on32)
#define BI_NS BI64
#elif defined(L4_32BIT)
#define BI_NS BI32
#elif defined(L4_64BIT)
#define BI_NS BI64
#endif

namespace BI_NS
{

/**
 * Initialize bootinfo structure
 *
 * @param bi		Pointer to bootinfo
 *
 * Function initializes global part of bootinfo structure.
 *
 * @returns pointer to first bootinfo record
 */
L4_BootRec_t * init_bootinfo (L4_BootInfo_t * bi)
{
    bi->magic		= L4_BOOTINFO_MAGIC;
    bi->version		= L4_BOOTINFO_VERSION;
    bi->size		= sizeof (*bi);
    bi->first_entry	= sizeof (*bi);
    bi->num_entries	= 0;

    return L4_BootInfo_FirstEntry (bi);
}


/**
 * Record all MBI modules into bootinfo
 *
 * @param bi		Pointer to bootinfo
 * @param rec		Pointer to next free bootinfo record
 * @param mbi		Pointer to loaded MBI modules
 * @param orig_mbi_modules Array of undecoded versions of all modules
 * @param decode_count	Number of possibly ELF-decoded modules
 *
 * Function parses through all multiboot info modules and inserts them
 * into the bootinfo structure, either as a simple executable or as a
 * simple module record.
 *
 * @returns pointer to next free bootinfo record
 */
L4_BootRec_t * record_bootinfo_modules (L4_BootInfo_t * bi,
					L4_BootRec_t * rec,
					mbi_t * mbi,
					mbi_module_t orig_mbi_modules[],
					unsigned int decode_count)
{
    L4_Word_t sz;

    // XXX Make sure that we do not overflow the allocated memory for
    // the bootinfo structure.

    if (mbi->flags.mods)
    {
	for (unsigned int i = 1; i < mbi->modcount; i++)
	{
	    L4_Boot_SimpleExec_t * exec = (L4_Boot_SimpleExec_t *) rec;

	    if (i < decode_count &&
		elf_find_sections (orig_mbi_modules[i].start, exec))
	    {
		// Found en ELF module.  Copy commandline into
		// bootinfo.
		sz = sizeof (*exec);
		exec->cmdline_offset = sz;
		strcpy ((char *) exec + sz, mbi->mods[i].cmdline);
		sz += strlen (mbi->mods[i].cmdline) + 1;
		sz = align_up (sz, sizeof (L4_Word_t));
		exec->offset_next = sz;
	    }
	    else
	    {
		// Record module as a simple module.
		L4_Boot_Module_t * mod = (L4_Boot_Module_t *) rec;
		sz = sizeof (*mod);

		mod->type	 = L4_BootInfo_Module;
		mod->version	 = 1;
		mod->offset_next = sz;
		mod->start 	 = mbi->mods[i].start;
		mod->size 	 = mbi->mods[i].end - mbi->mods[i].start;

		// Copy command line to bootinfo.
		mod->cmdline_offset = sz;
		strcpy ((char *) exec + sz, mbi->mods[i].cmdline);
		sz += strlen (mbi->mods[i].cmdline) + 1;
		sz = align_up (sz, sizeof (L4_Word_t));
		mod->offset_next = sz;
	    }

	    rec = (L4_BootRec_t *) ((L4_Word_t) rec + sz);
	    bi->num_entries++;
	    bi->size += sz;
	}
    }

    return rec;
}


/**
 * Record MBI pointer into bootinfo
 *
 * @param bi		Pointer to bootinfo
 * @param rec		Pointer to next free bootinfo record
 * @param mbi		Pointer to installed MBI info
 *
 * Function records pointer to MBI in bootinfo.
 *
 * @returns pointer to next free bootinfo record
 */
L4_BootRec_t * record_bootinfo_mbi (L4_BootInfo_t * bi,
				    L4_BootRec_t * rec,
				    mbi_t * mbi)
{
    L4_Boot_MBI_t * bimbi = (L4_Boot_MBI_t *) rec;

    bimbi->type		= L4_BootInfo_Multiboot;
    bimbi->version	= 1;
    bimbi->offset_next	= sizeof (*bimbi);
    bimbi->address	= (L4_Word_t) mbi;

    bi->num_entries++;
    bi->size += sizeof (*bimbi);
    return L4_Next (rec);
}

}
