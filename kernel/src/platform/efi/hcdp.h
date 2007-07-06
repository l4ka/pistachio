/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/efi/hcdp.h
 * Description:   EFI Headless Console and Debug Port table
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
 * $Id: hcdp.h,v 1.2 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__EFI__HCDP_H__
#define __PLATFORM__EFI__HCDP_H__

#include <acpi.h>

#define HCDP_DEV_CONSOLE	0
#define HCDP_DEV_DEBUG		1

class hcdp_dev_t
{
public:
    u8_t	type;
    u8_t	bits;
    u8_t	parity;
    u8_t	stop_bits;
    u8_t	pci_seg;
    u8_t	pci_bus;
    u8_t	pci_dev;
    u8_t	pci_func;
    u64_t	baud;
    acpi_gas_t	base_addr;
    u16_t	pci_dev_id;
    u16_t	pci_vendor_id;
    u32_t	global_int;
    u32_t	clock_rate;
    u8_t	pci_prog_intfc;
private:
    u8_t	reserved;
} __attribute__((packed));

class hcdp_table_t
{
    acpi_thead_t	header;
public:
    u32_t		num_entries;
    hcdp_dev_t		hcdp_dev[1];

    hcdp_dev_t *find(u8_t type) {
	for (u32_t i = 0; i < num_entries; i++) {
	    if (hcdp_dev[i].type == type)
		return &hcdp_dev[i];
	}
	return NULL;
    }
} __attribute__((packed));


#endif /* !__PLATFORM__EFI__HCDP_H__ */
