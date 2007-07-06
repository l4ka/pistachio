/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/efi/memory_map.h
 * Description:   EFI memory map interface
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
 * $Id: memory_map.h,v 1.5 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__EFI__MEMORY_MAP_H__
#define __PLATFORM__EFI__MEMORY_MAP_H__


/*
 * Memory descriptor types.
 */

typedef enum {
    EFI_RESERVED_MEMORY_TYPE,
    EFI_LOADER_CODE,
    EFI_LOADER_DATA,
    EFI_BOOT_SERVICES_CODE,
    EFI_BOOT_SERVICES_DATA,
    EFI_RUNTIME_SERVICES_CODE,
    EFI_RUNTIME_SERVICES_DATA,
    EFI_CONVENTIONAL_MEMORY,
    EFI_UNUSUABLE_MEMORY,
    EFI_ACPI_RECLAIM_MEMORY,
    EFI_ACPI_MEMORY_NVS,
    EFI_MEMORY_MAPPED_IO,
    EFI_MEMORY_MAPPED_IO_PORT_SPACE,
    EFI_PAL_CODE,
    EFI_MAX_MEMORY_TYPE
} efi_memory_type_t;



/*
 * Memory descriptor attributes.
 */

/* Caching attributes */
#define EFI_MEMORY_UC		0x0000000000000001
#define EFI_MEMORY_WC		0x0000000000000002
#define EFI_MEMORY_WT		0x0000000000000004
#define EFI_MEMORY_WB		0x0000000000000008

/* Memory protecyion */
#define EFI_MEMORY_WP		0x0000000000001000
#define EFI_MEMORY_RP		0x0000000000002000
#define EFI_MEMORY_XP		0x0000000000004000

/* Requires runtime mapping */
#define EFI_MEMORY_RUNTIME	0x8000000000000000




/*
 * Memory descriptor.
 */

#define EFI_MEMORY_DESC_VERSION	1

class efi_memory_desc_t
{
    u32_t	_type;
    addr_t	_physical_start;
    addr_t	_virtual_start;
    u64_t	_number_of_pages;
    u64_t	_attribute;

public:
    efi_memory_type_t type (void);
    addr_t physical_start (void);
    addr_t virtual_start (void);
    u64_t number_of_pages (void);
    u64_t attribute (void);

    void set_virtual_start (addr_t addr);

    void set(efi_memory_type_t type, 
	     addr_t physical_start, addr_t virtual_start,
	     u64_t number_of_pages, u64_t attribute);
	    
};

/**
 * Query the type of the given memory region.
 * @return type of memory region
 */
INLINE efi_memory_type_t
efi_memory_desc_t::type (void)
{
    return (efi_memory_type_t) _type;
}

/**
 * Query physical start address of given memory region.
 * @return physical start address of memory region
 */
INLINE addr_t
efi_memory_desc_t::physical_start (void)
{
    return _physical_start;
}

/**
 * Query virtual start address of given memory region.  Note that
 * virtual start address will not be valid unless it has been set
 * using set_virtual_start().
 * @return virtual start address of memory region
 */
INLINE addr_t
efi_memory_desc_t::virtual_start (void)
{
    return _virtual_start;
}

/**
 * Query number of 4KB pages occupied by given memory region.
 * @return size of memory region (in 4KB pages)
 */
INLINE u64_t
efi_memory_desc_t::number_of_pages (void)
{
    return _number_of_pages;
}

/**
 * Query attributes for given memory region.
 * @return 64-bit word of attribute flags
 */
INLINE u64_t
efi_memory_desc_t::attribute (void)
{
    return _attribute;
}

/**
 * Modify virtual location of memory region.  It is necessary to
 * modify the virtual location of all memory regions before invocation
 * of efi_runtime_services_t::set_virtual_address_map().
 * @param addr		virtual location of memory region
 */
INLINE void
efi_memory_desc_t::set_virtual_start (addr_t addr)
{
    _virtual_start = addr;
}

/**
 * Initializes the memory descriptor. This function should not be
 * used on real hardware, but only if there the EFI must be emulated.
 * @param type			type of efi memory
 * @param physical_start	physical start address
 * @param virtual_start		virtual start address
 * @param number_of_pages	number of pages (4K)
 * @param attribute		attribute flags
 */
INLINE void 
efi_memory_desc_t::set(efi_memory_type_t type, 
		       addr_t physical_start, addr_t virtual_start,
		       u64_t number_of_pages, u64_t attribute)
{
    _type = type;
    _physical_start = physical_start;
    _virtual_start = virtual_start;
    _number_of_pages = number_of_pages;
    _attribute = attribute;
}



/**
 * Memory map for Extensible Firmware Interface.
 */
class efi_memory_map_t
{
    word_t _base;
    word_t _size;
    word_t _desc_size;
    word_t _curptr;

public:
    void init (addr_t base, word_t map_size, word_t desc_size);
    void reset (void);
    efi_memory_desc_t * next (void);
};


/**
 * Initialize memory map object.
 * @param base		virtual address of memory map
 * @param map_size	size of memory map (in bytes)
 * @param desc_size	size of memory descriptor (in bytes)
 */
INLINE void
efi_memory_map_t::init (addr_t base, word_t map_size, word_t desc_size)
{
    _base = (word_t) base;
    _size = map_size;
    _desc_size = desc_size;
    _curptr = _base;
}

/**
 * Reset memory map iterator so that next invokation of next() will
 * return first memory map descriptor.
 */
INLINE void
efi_memory_map_t::reset (void)
{
    _curptr = _base;
}

/**
 * Iterate to next memory map descritor.
 * @return pointer to next memory map descriptor, or NULL if the whole
 * set of descriptors has been iterater over
 */
INLINE efi_memory_desc_t *
efi_memory_map_t::next (void)
{
    efi_memory_desc_t * ret = (efi_memory_desc_t *) _curptr;

    if ((word_t) _curptr - (word_t) _base >= _size)
	return NULL;

    _curptr += _desc_size;
    return ret;
}




/**
 * EFI memory map (after exiting boot services).  Must be initialized
 * by the kernel using efi_memory_map_t::init().
 */
extern efi_memory_map_t efi_memmap;


#endif /* !__PLATFORM__EFI__MEMORY_MAP_H__ */
