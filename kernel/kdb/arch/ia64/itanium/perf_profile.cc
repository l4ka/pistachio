/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/itanium/perf_profile.cc
 * Description:   Continous profiling for Itanium
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
 * $Id: perf_profile.cc,v 1.6 2004/02/24 23:54:52 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(itanium_perf.h)
#include INC_GLUE(context.h)
#include INC_GLUE(intctrl.h)
#include INC_API(tcb.h)


#define KNUTH_MAGIC		2654435761
#define PROFILE_SIZE		( CONFIG_PROFILE_SIZE)
#define PROFILE_INTERVAL	(-CONFIG_PROFILE_INTERVAL)


bool kdb_get_perfctr (pmc_itanium_t * pmc, word_t * pmc_mask);

static bool profiling_enabled = false;
static word_t profiling_counter = 0;

static word_t profile_table[PROFILE_SIZE];

static void profile_handler (word_t irq, ia64_exception_context_t * frame);


/**
 * Enable continous profiling.
 */
DECLARE_CMD (cmd_ia64_perf_profile, ia64_perfmon, 'p', "profile",
	     "enable continous profiling");

CMD (cmd_ia64_perf_profile, cg)
{
    ia64_exception_context_t * frame =
	(ia64_exception_context_t *) kdb.kdb_param;
     ia64_exception_context_t * user_frame =
	(ia64_exception_context_t *) kdb.kdb_current->get_stack_top () - 1;

    pmc_itanium_t pmc = 0;
    word_t i, reg, pmc_mask;

    if (! kdb_get_perfctr (&pmc, &pmc_mask))
	return CMD_NOQUIT;

    do {
	for (i = 1, reg = 0; (i & pmc_mask) == 0; i <<= 1, reg++)
	    ;
	reg = get_dec ("Select register", reg);
	if (reg == ABORT_MAGIC)
	    return CMD_NOQUIT;

    } while (((1UL << reg) & pmc_mask) == 0);

    pmc.ev = 0;		// no external visibility
    pmc.oi = 1;		// enable overflow interrupt
    pmc.pm = 1;		// allow user access
    pmc.threshold = 0;	// sum up
#ifdef CONFIG_CPU_IA64_ITANIUM2
    pmc.enabled = 1;
#endif
    pmc.ism = 0;	// monitor IA32 and IA64 instructions

    switch (get_choice ("Priviledge level", "User/Kernel/All", 'a'))
    {
    case 'u': pmc.plm = pmc_t::user; break;
    case 'k': pmc.plm = pmc_t::kernel; break;
    case 'a': pmc.plm = pmc_t::both; break;
    default:
	return CMD_NOQUIT;
    }

    if (profiling_enabled)
    {
	// Disable previous profiling counter.

	pmc_t old_pmc = get_pmc (profiling_counter);
	old_pmc.plm = pmc_t::none;
	set_pmc (profiling_counter, old_pmc);
    }

    profiling_counter = reg;
    profiling_enabled = true;

    // Initalise profile table

    for (i=0; i < PROFILE_SIZE; i++) {
	profile_table[i] = 0;
    }

    // Enable interrupt vector

    cr_ivec_t ivec;
    ivec.raw = 0;
    ivec.vector = IVEC_PERFMON;

    cr_set_pmv (ivec);

    get_interrupt_ctrl ()->register_handler (IVEC_PERFMON, profile_handler);

    // Initialize performance monitoring registers.

    set_pmc (reg, pmc);
    set_pmd (reg, ((word_t)PROFILE_INTERVAL & PMD_MASK));

#ifdef CONFIG_CPU_IA64_ITANIUM2
    set_pmc (0, 0);
#endif

    frame->ipsr.pp = user_frame->ipsr.pp = 1;

    return CMD_NOQUIT;
}


/**
 * Disable continous profiling.
 */
DECLARE_CMD (cmd_ia64_perf_profile_disable, ia64_perfmon, 'P', "noprofile",
	     "disable continous profiling");

CMD (cmd_ia64_perf_profile_disable, cg)
{
    if (profiling_enabled)
    {
	// Disable previous profiling counter.

	pmc_t old_pmc = get_pmc (profiling_counter);
	old_pmc.plm = pmc_t::none;
	set_pmc (profiling_counter, old_pmc);

	// Disable profiling interrupt.

	cr_ivec_t ivec;
	ivec.raw = 0;
	ivec.m = 1;

	cr_set_pmv (ivec);
    }
    profiling_enabled = false;

    return CMD_NOQUIT;
}



static void profile_handler (word_t irq, ia64_exception_context_t * frame)
{
    pmc_overflow_t overflow;
    word_t hash;
    // printf ("profile @ %p\n", frame->iip);

    hash = ((word_t)frame->iip * KNUTH_MAGIC) % PROFILE_SIZE;

    profile_table[hash] ++;

    set_pmd (profiling_counter, ((word_t)PROFILE_INTERVAL & PMD_MASK));

#ifdef CONFIG_CPU_IA64_ITANIUM2
    set_pmc (0, 0); // clear freeze bit and overflow bit
#endif

    overflow.clr_freeze ();
}


/**
 * Dump profile
 */
DECLARE_CMD (cmd_ia64_perf_profile_dump, ia64_perfmon, 'd', "dump",
	     "dump profile");

CMD (cmd_ia64_perf_profile_dump, cg)
{
    word_t i;

    set_pmc(0, 1);  // set freeze bit

    printf("Profile IP hash-table dump:\n");
    printf("Bucket\tCount\n");
    for (i=0; i < PROFILE_SIZE; i++) {
	if (profile_table[i] > 0) {
	    printf("%d\t%d\n", i, profile_table[i]);
	}
    }

    set_pmc(0, 0);  // clear freeze bit

    return CMD_NOQUIT;
}
