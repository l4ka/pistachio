 /*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/ski/efi.cc
 * Description:   efi emulation for SKI
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
 * $Id: efi.cc,v 1.11 2005/01/10 05:47:47 cgray Exp $
 *                
 ********************************************************************/
#include <types.h>
#include INC_PLAT(bootinfo.h)
#include INC_PLAT(system_table.h)
#include INC_PLAT(runtime_services.h)
#include INC_PLAT(memory_map.h)
#include INC_ARCH(sal.h)
#include INC_ARCH(ski.h)
#include INC_GLUE(hwspace.h)

#if defined(__GNUC__)
#if (__GNUC__ < 3) || (__GNUC__ >= 3 && __GNUC_MINOR__ < 3)
#error Ski support only works with gcc version >= 3.3
#endif
#endif

extern "C" void pal_emulator (unsigned long index, unsigned long arg1,
			      unsigned long arg2, unsigned long arg3);
extern "C" long sal_emulator (long index, 
			      unsigned long in1, unsigned long in2,
			      unsigned long in3, unsigned long in4, 
			      unsigned long in5, unsigned long in6, 
			      unsigned long in7);


#define EFI_SYSTEM_TABLE_SIGNATURE	0x5453595320494249
#define EFI_SYSTEM_TABLE_REVISION	((1 << 16) | 00)
#define EFI_RUNTIME_SERVICES_SIGNATURE	0x5652453544e5552
#define EFI_RUNTIME_SERVICES_REVISION	0x00010000

#define NUM_MEM_DESCS	4

static efi_bootinfo_t bootinfo_ski;
static efi_system_table_t efi_system_table_ski;
static efi_runtime_services_t efi_runtime_services_ski;
static efi_config_table_t efi_config_table_ski;
static struct {
    sal_system_table_t sal_system_table;
    sal_entrypoint_desc_t sal_entrypoint_desc;
} sal_tabs;
static efi_memory_desc_t efi_memory_desc[NUM_MEM_DESCS];

#define EFI_FUNC(rettype, name, args...)				\
static word_t __##name[2];						\
static rettype name (args)

#define SET_EFI_FUNC_PHYS(entry, func)					\
do {									\
    efi_runtime_services_ski.entry =  (efi_func_t) &__##func;		\
    word_t * fptr_n = &__##func[0];					\
    word_t * fptr_o = virt_to_phys ((word_t *) &func);			\
    fptr_n[0] = virt_to_phys (fptr_o[0]);				\
    fptr_n[1] = virt_to_phys (fptr_o[1]);				\
} while (0)


EFI_FUNC (void, efi_reset_system,
	  efi_runtime_services_t::reset_type_e reset_type,
	  efi_runtime_services_t::status_e reset_status,
	  word_t data_size,
	  char16 * reset_data)
{
    ia64_ski_ssc(reset_status, 0, 0, 0, IA64_SKI_SSC_EXIT);
}

EFI_FUNC (word_t, efi_unimplemented,
	  word_t, word_t, word_t, word_t, word_t)
{
    return efi_runtime_services_t::unsupported;
}

EFI_FUNC (word_t, efi_set_virtual_address_map,
	  word_t memmap_size, word_t memdesc_size,
	  word_t memdesc_version, word_t memmap_addr, word_t)
{
    return efi_runtime_services_t::success;
}


static void strcpy(char * dest, const char * src)
{
    while(*src)
	*dest++ = *src++;
}

extern "C" efi_bootinfo_t * init_efi_ski (void)
{
    /* Initialize the system tables */
    efi_system_table_t * st = &efi_system_table_ski;

    st->hdr.signature = EFI_SYSTEM_TABLE_SIGNATURE;
    st->hdr.revisions  = EFI_SYSTEM_TABLE_REVISION;
    st->hdr.header_size = sizeof (efi_table_header_t);
    st->firmware_vendor = (char16 *)
	"H\0e\0w\0l\0e\0t\0t\0-\0P\0a\0c\0k\0a\0r\0d\0\0";
    st->firmware_revision = 1;
    st->runtime_services = &efi_runtime_services_ski;
    st->config_table.number_of_table_entries = 1;
    st->config_table.config_table = &efi_config_table_ski;

    /* Configure runtime services */
    efi_runtime_services_t * rts = &efi_runtime_services_ski;

    rts->hdr.signature = EFI_RUNTIME_SERVICES_SIGNATURE;
    rts->hdr.revisions = EFI_RUNTIME_SERVICES_REVISION;
    rts->hdr.header_size = sizeof (efi_table_header_t);

    SET_EFI_FUNC_PHYS (get_time_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (set_time_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (get_wakeup_time_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (set_wakeup_time_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (set_virtual_address_map_f, efi_set_virtual_address_map);
    SET_EFI_FUNC_PHYS (convert_pointer_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (get_variable_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (get_next_variable_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (set_variable_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (get_next_high_monotinic_count_f, efi_unimplemented);
    SET_EFI_FUNC_PHYS (reset_system_f, efi_reset_system);

    /* Configure config tables */
    efi_config_table_t * ct = &efi_config_table_ski;

    ct->vendor_guid = SAL_SYSTEM_TABLE_GUID;
    ct->vendor_table = &sal_tabs.sal_system_table;

    /* Fill in the SAL system table */
    sal_system_table_t * sal_st = &sal_tabs.sal_system_table;
    sal_entrypoint_desc_t * sal_ed = &sal_tabs.sal_entrypoint_desc;

    sal_st->signature.string[0] = 'S';
    sal_st->signature.string[1] = 'S';
    sal_st->signature.string[2] = 'T';
    sal_st->signature.string[3] = '_';
    sal_st->total_table_length = sizeof (sal_system_table_t);
    sal_st->sal_a_version.x.major = 0;
    sal_st->sal_a_version.x.minor = 1;
    sal_st->entry_count = 1;
    strcpy (sal_st->oem_id, "Hewlett-Packard");
    strcpy (sal_st->product_id, "HP-simulator");
    sal_ed->type = 0; // entry point descriptor

    /* Fixup of IA-64 style function pointers */
    word_t * pal_desc = virt_to_phys ((word_t *) &pal_emulator);
    word_t * sal_desc = virt_to_phys ((word_t *) &sal_emulator);

    sal_ed->pal_proc = virt_to_phys ((addr_t) pal_desc[0]);
    sal_ed->sal_proc = virt_to_phys ((addr_t) sal_desc[0]);
    sal_ed->sal_global_data = virt_to_phys ((addr_t) sal_desc[1]);

    unsigned char checksum = 0;
    for (unsigned char * cp = (unsigned char *) &sal_tabs;
	 cp < (unsigned char *) (&sal_tabs + 1);
	 ++cp)
	checksum += *cp;
    sal_st->checksum = -checksum;

    /* Simulate free memory at physical address zero */
    efi_memory_desc_t * md = &efi_memory_desc[0];

    md[0].set (EFI_BOOT_SERVICES_DATA, 
	       (addr_t) MB (0),
	       (addr_t) MB (0), 
	       MB (1) >> 12, EFI_MEMORY_WB);

    md[1].set (EFI_CONVENTIONAL_MEMORY, 
	       (addr_t) MB(2),
	       (addr_t) 0, 
	       MB (128) >> 12, EFI_MEMORY_WB);

    md[2].set (EFI_PAL_CODE, 
	       addr_mask ((addr_t) &pal_emulator, ~((1 << 20) - 1)), 
	       (addr_t) MB (1), 
	       MB (1) >> 12, EFI_MEMORY_WB);

    md[3].set (EFI_MEMORY_MAPPED_IO_PORT_SPACE, 
	       (addr_t) 0xb8000,
	       (addr_t) 0,
	       MB (8) >> 12, EFI_MEMORY_UC);

    /*
     * Initialize bootinfo structure
     */
    efi_bootinfo_t * bi = &bootinfo_ski;

    /* this stops the compiler loading the full virtual address of
     * a string before we have translation on.
     */
    bi->magic.string[0] = 'L';
    bi->magic.string[1] = '4';
    bi->magic.string[2] = 'K';
    bi->magic.string[3] = 'a';
    bi->magic.string[4] = ' ';
    bi->magic.string[5] = 'l';
    bi->magic.string[6] = 'd';
    bi->magic.string[7] = 'r';

    bi->version = EFI_BOOTINFO_VERSION;
    bi->systab = (word_t) &efi_system_table_ski;
    bi->memmap = (word_t) &efi_memory_desc[0];
    bi->memmap_size = NUM_MEM_DESCS * sizeof (efi_memory_desc_t);
    bi->memdesc_size = sizeof (efi_memory_desc_t);
    bi->memdesc_version = EFI_MEMORY_DESC_VERSION;

    return bi;
}
