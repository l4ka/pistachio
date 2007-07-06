/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/cpuid.cc
 * Description:   IA-64 CPUID
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
 * $Id: cpuid.cc,v 1.3 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>

#include INC_ARCH(cpuid.h)


/**
 * cmd_dpuid: dump CPU ID
 */
DECLARE_CMD (cmd_cpuid, arch, 'C', "cpuid", "dump CPUID");

CMD (cmd_cpuid, cg)
{
    printf ("CPUID:\n");
    printf ("  Vendor:       %s\n"
	    "  Serial num:   %p\n",
	    cpuid_vendor_info (), cpuid_serial_number ());

    cpuid_version_info_t info = cpuid_version_info ();

    printf ("  Version:      %p "
	    "[Stepping %c%d,  Model %d,  Family %d,  Archrev %d]\n",
	    info.raw,
	    (info.revision >> 1) + 'A' - 1, (info.revision & 0x1),
	    info.model, info.family, info. archrev);


    printf ("  cpuid[4]:     %p [%s]\n", cpuid_get (4),
	    cpuid_get (4) & 1 ? "lb" : "");

    for (word_t i = 5; i <= info.number; i++)
    {
	printf ("  cpuid[%d]:     %p\n", i, cpuid_get (i));
    }

    return CMD_NOQUIT;
}
