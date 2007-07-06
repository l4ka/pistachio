/*********************************************************************
 *                
 * Copyright (C) 2002, 2006,  Karlsruhe University
 *                
 * File path:     platform/efi/acpi.cc
 * Description:   ACPI support code for EFI
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
 * $Id: acpi.cc,v 1.5 2006/05/24 09:43:56 stoess Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <acpi.h>
#include INC_PLAT(system_table.h)

/* ACPI 2.0 Specification, 5.2.4.2
   Finding the RSDP on EFI Enabled Systems */

acpi_rsdp_t* acpi_rsdp_t::locate(addr_t addr)
{
    acpi_rsdp_t *p;

    /* look for ACPI 2.0 RSDT pointer */
    p = (acpi_rsdp_t*) efi_config_table.find_table(ACPI_20_TABLE_GUID);
    if (p != NULL)
	return p;

    /* look for ACPI 1.0 RSDT pointer */
    p = (acpi_rsdp_t*) efi_config_table.find_table(ACPI_TABLE_GUID);
    if (p != NULL)
	return p;

    return NULL;
};
