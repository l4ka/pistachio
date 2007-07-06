/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/perf.cc
 * Description:   IA64 performance monitoring functionailty
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
 * $Id: perf.cc,v 1.5 2003/09/24 19:05:06 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>

#include INC_ARCH(pal.h)
#include INC_ARCH(perf.h)


/* 
 * We read the perfmon info and mask on first time entering the
 * perfmon submenu to avoid doing it on each function
 */
pal_perf_mon_info_t perf_info;
pal_perf_mon_masks_t perf_masks;


/**
 * Command group for IA64 performance monitoring functionality.
 */
DECLARE_CMD_GROUP (ia64_perfmon);


/**
 * Enter IA64 performance monitoring menu.
 */
DECLARE_CMD (cmd_ia64_perfmon, arch, 'p', "perfmon", "performance monitoring");

CMD (cmd_ia64_perfmon, cg)
{
    static bool initialized = false;

    if (! initialized)
    {
	pal_status_e status;
	if ((status = pal_perf_mon_info (&perf_masks, &perf_info)) != PAL_OK)
	{
	    printf ("Error: PAL_PERF_MON_INFO => %d\n", (long) status);
	    return CMD_NOQUIT;
	}

	initialized = true;
    }

    return ia64_perfmon.interact (cg, "perfmon");
}


/**
 * Dump all performance monitoring configuration and data registers.
 */
DECLARE_CMD (cmd_dump_all_regs, ia64_perfmon, 'a', "dumpregs",
	     "dump all registers");

CMD (cmd_dump_all_regs, cg)
{
    printf ("Performance configuration registers:\n");
    for (word_t i = 0; i < 256; i++)
	if (perf_masks.is_pmc_implemented (i))
	    printf ("  pmc[%d]%s = %p %s%s\n",
		    i, i >= 100 ? "" : i >= 10 ? " " : "  ",
		    (word_t) get_pmc (i),
		    perf_masks.can_count_cycles (i) ? "(cycles) " : "",
		    perf_masks.can_count_retired_bundles (i) ?
		    "(retired bundles) " : "");

    printf ("\nPerformance data registers:\n");
    for (word_t i = 0; i < 256; i++)
	if (perf_masks.is_pmd_implemented (i))
	    printf ("  pmd[%d]%s = %p %s%s\n",
		    i, i >= 100 ? "" : i >= 10 ? " " : "  ",
		    (word_t) get_pmd (i),
		    perf_masks.can_count_cycles (i) ? "(cycles) " : "",
		    perf_masks.can_count_retired_bundles (i) ?
		    "(retired bundles) " : "");

    return CMD_NOQUIT;
}


