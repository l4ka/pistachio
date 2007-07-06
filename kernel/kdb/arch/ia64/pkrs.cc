/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/pkrs.cc
 * Description:   Protection key register access
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
 * $Id: pkrs.cc,v 1.6 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_ARCH(pkr.h)
#include INC_ARCH(pal.h)
#include <kdb/cmd.h>
#include <kdb/kdb.h>


/**
 * cmd_pkrs: dump protection key registers
 */
DECLARE_CMD (cmd_pkrs, arch, 'P', "pkrs", "dump protection key registers");

CMD(cmd_pkrs, cg)
{
    pal_vm_summary_t info;
    pal_status_e status;

    /*
     * Get number of implemented registers.
     */

    if ((status = pal_vm_summary (&info)) != PAL_OK)
    {
	printf ("Error: PAL_VM_SUMMARY => %d\n", status);
	return CMD_NOQUIT;
    }

    int num_pkrs = info.max_pkr + 1;

    static const char * rights[] = {
	"no rights", "write-only", "read-only", "exec-only",
	"read/write", "read/exec", "write/exec", "read/write/exec"
    };

    printf ("Protection key registers:\n");
    for (int i = 0; i < num_pkrs; i++)
    {
	pkr_t pkr = get_pkr (i);
	printf ("  pkr[%d]%s = %s  key: 0x%5x,  %s\n",
		i, i < 10 ? " " : "",
		pkr.is_valid () ? "valid,  " : "invalid,", pkr.key (),
		rights[pkr.access_rights ()]);
    }

    return CMD_NOQUIT;
}
