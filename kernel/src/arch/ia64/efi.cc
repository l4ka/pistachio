/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/efi.cc
 * Description:   IA-64 specific EFI initialization
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
 * $Id: efi.cc,v 1.11 2003/09/24 19:05:27 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>

#include INC_PLAT(runtime_services.h)
#include INC_PLAT(memory_map.h)
#include INC_PLAT(system_table.h)

#include INC_ARCH(ioport.h)
#include INC_ARCH(pal.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)


/*
 * Memory map given to us by the EFI loader.
 */

efi_memory_map_t efi_memmap;


/*
 * Location of the EFI system table.
 */

efi_system_table_t * efi_systab;


/*
 * Location of EFI runtime services table.
 */

efi_runtime_services_t * efi_runtime_services;


/*
 * EFI configuration tables
 */

efi_config_table_ptr_t efi_config_table;



void ipanic (const char * str);


/**
 * init_efi: EFI specific initialization
 * @param memmap_addr		Virtual address of loader provided memory map
 * @param memmap_size		Size of loader provided memory map
 * @param memdesc_size		Size of a single memory map descriptor
 * @param memdesc_version	Version information for memory map descriptor
 * @param systab_addr		Physical address of EFI system table
 */
void SECTION (".init")
init_efi (addr_t memmap_addr,
	  word_t memmap_size,
	  word_t memdesc_size,
	  u32_t memdesc_version,
	  addr_t systab_addr)
{
    if (memdesc_version != EFI_MEMORY_DESC_VERSION)
	ipanic ("Invalid EFI memory descriptor version\n");

    /*
     * Map EFI memory map.
     */

    if (! dtrmap.is_mapped (memmap_addr))
    {
	translation_t tr (1, translation_t::write_back, 1, 1, 0,
			  translation_t::rwx, virt_to_phys (memmap_addr), 0);
	dtrmap.add_map (tr, memmap_addr, HUGE_PGSIZE, 0);
    }
   
    efi_memmap.init (memmap_addr, (word_t) memmap_size, (word_t) memdesc_size);

    /*
     * Get location of various memory regions (in particular the I/O
     * port space needed for doing printfs).
     */

    efi_memory_desc_t * desc;
    efi_memmap.reset ();
    while ((desc = efi_memmap.next ()) != NULL)
    {
	switch (desc->type ())
	{
	case EFI_MEMORY_MAPPED_IO_PORT_SPACE:
	    ia64_io_port_base = desc->physical_start ();
	    asm volatile ("mov ar.k0 = %0" :: "r" (ia64_io_port_base));
	    break;
	case EFI_PAL_CODE:
	    ia64_pal_code = desc->physical_start ();
	    break;
	default:
	    break;
	}

	/*
	 * Relocate virtual address to the kernel region.  Make sure
	 * that cacheability attributes are preserved.
	 */
	desc->set_virtual_start (desc->attribute () & EFI_MEMORY_WB ?
				 phys_to_virt (desc->physical_start ()) :
				 phys_to_virt_uc (desc->physical_start ()));
    }

    /*
     * Map I/O ports.
     */

    if (ia64_io_port_base == 0)
	ipanic ("No I/O port base\n");

    ia64_io_port_base = phys_to_virt_uc (ia64_io_port_base);
    if (! dtrmap.is_mapped (ia64_io_port_base))
    {
	translation_t tr (1, translation_t::uncacheable, 1, 1,
			  0, translation_t::rwx,
			  virt_to_phys (ia64_io_port_base), 0);
	dtrmap.add_map (tr, ia64_io_port_base, HUGE_PGSIZE, 0);
    }

    /*
     * Map EFI system table.
     */

    efi_systab = (efi_system_table_t *) systab_addr;
    if (! dtrmap.is_mapped (efi_systab))
    {
	translation_t tr (1, translation_t::write_back, 1, 1, 0,
			  translation_t::rwx, virt_to_phys (systab_addr), 0);
	dtrmap.add_map (tr, efi_systab, HUGE_PGSIZE, 0);
    }

    /*
     * EFI Spec v1.10 says SetVirtualAddressMap translates all
     * pointers for us, but it seems that some old versions of EFI
     * don't.  So we take copies of the needed pointers before
     * SetVirtualAddressMap and translate ourselves.
     */

    efi_runtime_services = phys_to_virt (efi_systab->runtime_services);
    efi_config_table = efi_systab->config_table;
    efi_config_table.config_table =
	phys_to_virt (efi_config_table.config_table);

    /*
     * Notify EFI firmware about virtual locations of memory regions
     * so that we can invoke EFI functions in virtual mode.
     */

    word_t status = call_efi_physical
	((word_t) efi_runtime_services->set_virtual_address_map_f,
	 memmap_size, memdesc_size, memdesc_version,
	 virt_to_phys ((word_t) memmap_addr), 0);

    if (status != efi_runtime_services_t::success)
	ipanic ("efi_set_virtual_address_map() failed\n");
}
