/*********************************************************************
 *                
 * Copyright (C) 2003, 2006,  Karlsruhe University
 *                
 * File path:     platform/pc99/acpi.h
 * Description:   ACPI support code for IA-PCs (PC99)
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
 * $Id: acpi.h,v 1.3 2006/05/24 09:31:03 stoess Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__PC99__ACPI_H__
#define __PLATFORM__PC99__ACPI_H__

#include <acpi.h>

/* ACPI 2.0 Specification, 5.2.4.1
   Finding the RSDP on IA-PC Systems */

#define ACPI20_PC99_RSDP_START	0x0e0000
#define ACPI20_PC99_RSDP_END	0x100000
#define ACPI20_PC99_RSDP_SIZE   (ACPI20_PC99_RSDP_END - ACPI20_PC99_RSDP_START)

INLINE acpi_rsdp_t* acpi_rsdp_t::locate(addr_t addr)
{
    /** @todo checksum, check version */
    for (addr_t p = addr;
	 p < addr_offset(addr, ACPI20_PC99_RSDP_SIZE);
	 p = addr_offset(p, 16))
    {
	acpi_rsdp_t* r = (acpi_rsdp_t*) p;
	if (r->sig[0] == 'R' &&
	    r->sig[1] == 'S' &&
	    r->sig[2] == 'D' &&
	    r->sig[3] == ' ' &&
	    r->sig[4] == 'P' &&
	    r->sig[5] == 'T' &&
	    r->sig[6] == 'R' &&
	    r->sig[7] == ' ')
	    return r;
    };
    /* not found */
    return NULL;
};

#endif /* !__PLATFORM__PC99__ACPI_H__ */
