/*********************************************************************
 *                
 * Copyright (C) 2004,  Karlsruhe University
 *                
 * File path:     kdb/generic/bootinfo.cc
 * Description:   Generic bootinfo dumping
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
 * $Id: bootinfo.cc,v 1.5 2006/10/22 19:43:31 reichelt Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)


#define L4_BOOTINFO_MAGIC		((word_t) 0x14b0021d)
#define L4_BOOTINFO_VERSION		1


/**
 * Generic bootinfo record.
 */
class bootrec_t
{
    word_t	_type;
    word_t	_version;
    word_t	_offset_next;

public:

    enum type_e {
	module		= 0x0001,
	simple_exec	= 0x0002,
	efitables	= 0x0101,
	multiboot	= 0x0102,
    };

    type_e type (void)
	{ return (type_e) _type; }

    word_t version (void)
	{ return _version; }

    bootrec_t * next (void)
	{ return (bootrec_t *) ((word_t) this + _offset_next); }
};


/**
 * Bootinfo record for simple binary file.
 */
class boot_module_t
{
public:
    word_t	type;			// 0x01
    word_t	version;		// 1
    word_t	offset_next;

    word_t	start;
    word_t	size;
    word_t	cmdline_offset;

    const char * commandline (void)
	{ return cmdline_offset ? (const char *) this + cmdline_offset : ""; }
};


/**
 * Bootinfo record for simple executable image loaded and relocated by
 * the bootloader.
 */
class boot_simpleexec_t
{
public:
    word_t	type;			// 0x02
    word_t	version;		// 1
    word_t	offset_next;

    word_t	text_pstart;
    word_t	text_vstart;
    word_t	text_size;
    word_t	data_pstart;
    word_t	data_vstart;
    word_t	data_size;
    word_t	bss_pstart;
    word_t	bss_vstart;
    word_t	bss_size;
    word_t	initial_ip;
    word_t	flags;
    word_t	label;
    word_t	cmdline_offset;

    const char * commandline (void)
	{ return cmdline_offset ? (const char *) this + cmdline_offset : ""; }
};


/**
 * Bootinfo record for EFI table information.
 */
class boot_efi_t
{
public:
    word_t	type;			// 0x101
    word_t	version;		// 1
    word_t	offset_next;

    word_t	systab;
    word_t	memmap;
    word_t	memmap_size;
    word_t	memdesc_size;
    word_t	memdesc_version;
};


/**
 * Bootinfo record for multiboot info.
 */
class boot_mbi_t
{
public:
    word_t	type;			// 0x102
    word_t	version;		// 1
    word_t	offset_next;

    word_t	address;
};


/**
 * Main structure for generic bootinfo.
 */
class bootinfo_t
{
    word_t	_magic;
    word_t	_version;
    word_t	_size;
    word_t	_first_entry;
    word_t	_num_entries;
    word_t	__reserved[3];

    word_t safe_get (word_t * fld)
	{ return kdb.kdb_current->get_space ()->readmem_phys (fld); }

public:

    bool is_valid (void)
	{ return safe_get (&_magic) == L4_BOOTINFO_MAGIC; }
    word_t size_safe (void)	{ return safe_get (&_size); }

    word_t magic (void)		{ return _magic; }
    word_t version (void)	{ return _version; }
    word_t size (void)		{ return _size; }
    word_t entries (void)	{ return _num_entries; }
    bootrec_t * first_entry (void)
	{ return (bootrec_t *) ((word_t) this + _first_entry); }
};



/**
 * Copy of bootinfo structure.  Used in order to simplify parsing (no
 * need to access some random physical memory location).
 */
bootinfo_t * bootinfo_copy;



/**
 * Dump generic bootinfo structure
 */
DECLARE_CMD (cmd_dump_bootinfo, root, 'B', "bootinfo",
	     "generic bootinfo");

CMD (cmd_dump_bootinfo, cg)
{
    static word_t kip_bootinfo;
    bootinfo_t * bi = bootinfo_copy;

    if (bi == NULL)
    {
	kip_bootinfo = get_kip ()->boot_info;
	bi = (bootinfo_t *) kip_bootinfo;

	/*
	 * Do some sanity checking to see if this really is a valid
	 * generic BootInfo structure.
	 */

	if (bi == NULL || (word_t) bi > (word_t) GB (2))
	{
	    printf ("Doesn't look like a generic bootinfo structure "
		    "(bootinfo=%p)\n", bi);
	    return CMD_NOQUIT;
	}

	if (! bi->is_valid ())
	{
	    printf ("Not a generic bootinfo record (bootinfo=%p).\n", bi);
	    return CMD_NOQUIT;
	}

	/*
	 * OK.  Looks fine.  Make a local copy of the bootinfo
	 * structure (easier to parse).
	 */

	word_t size = (bi->size_safe () + sizeof (word_t) - 1) &
	    ~(sizeof (word_t) - 1);
	word_t alloc_size = (1 << 12);
	while (alloc_size < size)
	    alloc_size <<= 1;

	EXTERN_KMEM_GROUP (kmem_misc);
	bootinfo_copy = (bootinfo_t *) 
	    kmem.alloc (kmem_misc, (1UL << alloc_size));

	space_t * s = kdb.kdb_current->get_space ();
	word_t * src = (word_t *) bi;
	word_t * dst = (word_t *) bootinfo_copy;
	for (;size > 0; size -= sizeof (word_t), src++, dst++)
	    *dst = s->readmem_phys (src);

	bi = bootinfo_copy;
    }


    /*
     * We are here operating on a local copy of the bootinfo.
     */

    printf ("Generic BootInfo @ %p\n", kip_bootinfo);
    printf ("  magic:        0x%p\n"
	    "  version:      %d\n"
	    "  size:         0x%x\n"
	    "  num records:  %d\n\n",
	    bi->magic (), bi->version (), bi->size (), bi->entries ());

    word_t numrec = bi->entries ();
    bootrec_t * rec = bi->first_entry ();

    for (word_t n = 1; numrec-- > 0; n++, rec = rec->next ())
    {
	switch (rec->type ())
	{
	case bootrec_t::module:
	{
	    boot_module_t * b = (boot_module_t *) rec;
	    printf ("[%d] Simple module (version %d):\n"
		    "  start:     %p\n"
		    "  size:      %p\n"
		    "  cmdline:   %s\n\n",
		    n, b->version, b->start, b->size,
		    b->commandline ());
	    break;
	}

	case bootrec_t::simple_exec:
	{
	    boot_simpleexec_t * e = (boot_simpleexec_t *) rec;
	    printf ("[%d] Simple executable (version %d):\n"
		    "  text:      [paddr: %p, vaddr: %p, size: %p]\n"
		    "  data:      [paddr: %p, vaddr: %p, size: %p]\n"
		    "  bss:       [paddr: %p, vaddr: %p, size: %p]\n"
		    "  entry:     %p\n"
		    "  flags:     %p\n"
		    "  label:     %p\n"
		    "  cmdline:   %s\n\n",
		    n, e->version,
		    e->text_pstart, e->text_vstart, e->text_size,
		    e->data_pstart, e->data_vstart, e->data_size,
		    e->bss_pstart,  e->bss_vstart,  e->bss_size,
		    e->initial_ip, e->flags, e->label, e->commandline ());
	    break;
	}

	case bootrec_t::efitables:
	{
	    boot_efi_t * e = (boot_efi_t *) rec;
	    printf ("[%d] EFI Tables (version %d):\n"
		    "  systab:    %p\n"
		    "  memmap:    [addr: %p, size: 0x%x]\n"
		    "  memdesc:   [version: 0x%x, size: 0x%x]\n",
		    n, e->version, e->systab, e->memmap, e->memmap_size,
		    e->memdesc_version, e->memdesc_size);
	    break;
	}

	case bootrec_t::multiboot:
	{
	    boot_mbi_t * m = (boot_mbi_t *) rec;
	    printf ("[%d] Multiboot info (version %d):\n"
		    "  address:   %p\n",
		    n, m->version, m->address);
	    break;
	}

	default:
	    printf ("[%d] Unknown record (type: 0x%x,  version: %d)\n\n", 
		    n, rec->type (), rec->version ());
	}
    }

    return CMD_NOQUIT;
}
