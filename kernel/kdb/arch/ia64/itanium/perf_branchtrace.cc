/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/itanium/perf_branchtrace.cc
 * Description:   Itanium Branch Trace Buffer
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
 * $Id: perf_branchtrace.cc,v 1.4 2003/09/24 19:05:07 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(itanium_perf_branchtrace.h)
#include INC_GLUE(context.h)
#include INC_API(tcb.h)


#if defined(CONFIG_KDB_DISAS)
extern "C" int disas (addr_t ip);
#define DISAS(ip) disas (ip)
#else
#define DISAS(ip)
#endif


/**
 * Enable/disable the Itanium branch trace buffer.
 */
DECLARE_CMD (cmd_ia64_perf_branchtrace, ia64_perfmon, 'B', "branchtrace",
	     "configure branch tracing");

CMD (cmd_ia64_perf_branchtrace, cg)
{
    pmc_branch_trace_config_t btc;

    ia64_exception_context_t * frame =
	(ia64_exception_context_t *) kdb.kdb_param;
    ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) kdb.kdb_current->get_stack_top () - 1;

    if (get_choice ("Branch trace", "Enable/Disable", 'e') == 'd')
    {
	btc.disable ();
	frame->ipsr.pp = user_frame->ipsr.pp = 0;
	return CMD_NOQUIT;
    }

    btc.pm = 1;
    btc.plm = pmc_t::both;

    switch (get_choice ("Branch outcome", "Taken/Not taken/All", 't'))
    {
    case 't': btc.tm = pmc_branch_trace_config_t::taken; break;
    case 'n': btc.tm = pmc_branch_trace_config_t::not_taken; break;
    case 'a': btc.tm = pmc_branch_trace_config_t::all; break;
    }

    switch (get_choice ("Target predication",
			"Predicted/Mispredicted/All", 'a'))
    {
    case 'p': btc.ptm = pmc_branch_trace_config_t::predicted_targets; break;
    case 'm': btc.ptm = pmc_branch_trace_config_t::mispredicted_targets; break;
    case 'a': btc.ptm = pmc_branch_trace_config_t::all_targets; break;
    }

    switch (get_choice ("Predicate predication",
			"Predicted/Mispredicted/All", 'a'))
    {
    case 'p': btc.ppm = pmc_branch_trace_config_t::predicted_preds; break;
    case 'm': btc.ppm = pmc_branch_trace_config_t::mispredicted_preds; break;
    case 'a': btc.ppm = pmc_branch_trace_config_t::all_targets; break;
    }

    btc.tar = get_choice ("Capture TAR predictions", "y/n", 'y') == 'y';
    btc.bpt = get_choice ("Capture TAC predictions", "y/n", 'y') == 'y';
    btc.bac = get_choice ("Capture BAC predictions", "y/n", 'y') == 'y';

    // Clear trace buffer
    pmd_branch_trace_index_t bti;
    bti.clear ();

    // Enable tracing
    btc.activate ();
    frame->ipsr.pp = user_frame->ipsr.pp = 1;

    return CMD_NOQUIT;
}


/**
 * Dump the branch trace buffer.
 */
DECLARE_CMD (cmd_branches_show, ia64_perfmon, 'b', "dumpbranches",
	     "dump last branches");

CMD (cmd_branches_show, cg)
{
    pmd_branch_trace_index_t bti;
    word_t n, k;

    if (bti.num_entries () == 0)
    {
	printf ("No branch trace.\n");
	return CMD_NOQUIT;
    }

    printf ("Branch trace (oldest first):\n");
    for (n = bti.oldest_idx (), k = bti.num_entries ();
	 k > 0;
	 k--, n = bti.next_idx (n))
    {
	pmd_branch_trace_t bt (n);

	if (bt.is_invalid ())
	    continue;
	else if (bt.is_target ())
	{
	    printf ("          ===>  %p ", bt.address ());
	    DISAS (bt.canonical_address ());
	    printf ("\n");
	}
	else
	{
	    printf ("%s  %p ",
		    bt.is_predicted () ? "(predicted)   " : "(mispredicted)",
		    bt.canonical_address ());
	    DISAS (bt.canonical_address ());
	    printf ("\n");
	}
    }

    return CMD_NOQUIT;
}

