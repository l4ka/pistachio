/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/efi/types.h
 * Description:   EFI specific types
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
 * $Id: types.h,v 1.4 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__EFI__TYPES_H__
#define __PLATFORM__EFI__TYPES_H__

typedef char		char16[2];
typedef void *		efi_handle_t;
typedef struct {}	efi_interface_t;
typedef u8_t		efi_bool_t;


class efi_table_header_t
{
public:
    u64_t	signature;
    u32_t	revisions;
    u32_t	header_size;
    u32_t	crc32;
    u32_t	__reserved;
};



/**
 * efi_guid_t: unique identifier value for EFI objects and tables
 */
class efi_guid_t
{
public:
    union{
	u8_t		raw8[16];
	u32_t		raw32[4];
	u64_t		raw64[2];
	struct {
	    u32_t	data1;
	    u16_t	data2;
	    u16_t	data3;
	    u8_t	data4[8];
	} x;
    };

    inline bool operator == (efi_guid_t r)
	{ return (raw64[0] == r.raw64[0]) && (raw64[1] == r.raw64[1]); }
};


/*
 * Identifier for various EFI configuration tables.
 */

#define MPS_TABLE_GUID						\
    ((efi_guid_t) {{ x : { 0xeb9d2d2f, 0x2d88, 0x11d3, {	\
	0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} } }})

#define ACPI_TABLE_GUID						\
    ((efi_guid_t) {{ x : { 0xeb9d2d30, 0x2d88, 0x11d3, {	\
	0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} } }})

#define ACPI_20_TABLE_GUID  					\
    ((efi_guid_t) {{ x : { 0x8868e871, 0xe4f1, 0x11d3, {	\
	0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} } }})

#define SMBIOS_TABLE_GUID    					\
    ((efi_guid_t) {{ x : { 0xeb9d2d31, 0x2d88, 0x11d3, {	\
	0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} } }})

#define SAL_SYSTEM_TABLE_GUID					\
    ((efi_guid_t) {{ x : { 0xeb9d2d32, 0x2d88, 0x11d3, {	\
	0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} } }})

#define HCDP_TABLE_GUID						\
    ((efi_guid_t) {{ x : { 0xf951938d, 0x620b, 0x42ef, {	\
	0x82, 0x79, 0xa8, 0x4b, 0x79, 0x61, 0x78, 0x98} } }})


#endif /* !__PLATFORM__EFI__TYPES_H__ */
