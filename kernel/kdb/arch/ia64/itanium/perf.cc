/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/itanium/perf.cc
 * Description:   Itanium performance monitoring functionality
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
 * $Id: perf.cc,v 1.8 2004/04/30 03:42:41 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(itanium_perf.h)
#include INC_GLUE(context.h)
#include INC_API(tcb.h)
#include INC_API(schedule.h)

#include <sync.h>


bool kdb_get_perfctr (pmc_itanium_t * pmc, word_t * pmc_mask);


/**
 * Set performance counter register.
 */
DECLARE_CMD (cmd_ia64_perf_setreg, ia64_perfmon, 's', "setreg",
	     "set performance register");

CMD (cmd_ia64_perf_setreg, cg)
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
    pmc.oi = 0;		// no overflow interrupt
    pmc.pm = 1;		// allow user access
    pmc.threshold = 0;	// sum up
    pmc.ism = 0;	// monitor IA32 and IA64 instructions

    printf("perfctr: es=%x, umask=%x, raw = %x\n", 
	   pmc.es, pmc.umask, *(word_t *) &pmc);

    switch (get_choice ("Priviledge level", "User/Kernel/All", 'a'))
    {
    case 'u': pmc.plm = pmc_t::user; break;
    case 'k': pmc.plm = pmc_t::kernel; break;
    case 'a': pmc.plm = pmc_t::both; break;
    default:
	return CMD_NOQUIT;
    }

    set_pmc (reg, pmc);
    set_pmd (reg, 0);

    frame->ipsr.pp = user_frame->ipsr.pp = 1;

    return CMD_NOQUIT;
}

extern tcb_t * global_present_list UNIT("kdebug");
extern spinlock_t present_list_lock;

/**
 * Enable performance counters for all threads.
 */
DECLARE_CMD (cmd_ia64_perf_enable_all, ia64_perfmon, 'E', "enableall",
	     "set performance monitoring for all threads");

CMD (cmd_ia64_perf_enable_all, cg)
{
    int abort = 1000000;
    present_list_lock.lock();
    scheduler_t * scheduler = get_current_scheduler();
    printf("\n");

    /* check whether we have something for this prio */
    tcb_t* walk = global_present_list;
    do {
     	ia64_exception_context_t * frame =
		(ia64_exception_context_t *) walk->get_stack_top () - 1;
    	frame->ipsr.pp = 1;

	printf(walk->queue_state.is_set(queue_state_t::ready) ? " %p" : "(%p)", walk);
        printf("\n");
        walk = walk->present_list.next;

	if (abort-- == 0)
	{
	    // huha -- something fucked up?
	    printf("present-list fucked???\n");
	    return CMD_NOQUIT;
	}

    } while (walk != global_present_list);

    present_list_lock.unlock();

    cr_dcr_t dcr = cr_get_dcr ();

    dcr.pp = 1;

    __asm__ ("mov cr.dcr = %0 " :: "r" (dcr.raw));

    return CMD_NOQUIT;
}

/**
 * Disable performance counters for all threads.
 */
DECLARE_CMD (cmd_ia64_perf_disable_all, ia64_perfmon, 'D', "disableall",
	     "disable performance monitoring for all threads");

CMD (cmd_ia64_perf_disable_all, cg)
{
    int abort = 1000000;
    present_list_lock.lock();
    scheduler_t * scheduler = get_current_scheduler();
    printf("\n");

    /* check whether we have something for this prio */
    tcb_t* walk = global_present_list;
    do {
     	ia64_exception_context_t * frame =
		(ia64_exception_context_t *) walk->get_stack_top () - 1;
    	frame->ipsr.pp = 0;

	printf(walk->queue_state.is_set(queue_state_t::ready) ? " %p" : "(%p)", walk);
        printf("\n");
        walk = walk->present_list.next;

	if (abort-- == 0)
	{
	    // huha -- something fucked up?
	    printf("present-list fucked???\n");
	    return CMD_NOQUIT;
	}

    } while (walk != global_present_list);

    present_list_lock.unlock();

    cr_dcr_t dcr = cr_get_dcr ();

    dcr.pp = 0;

    __asm__ ("mov cr.dcr = %0 " :: "r" (dcr.raw));

    return CMD_NOQUIT;
}
