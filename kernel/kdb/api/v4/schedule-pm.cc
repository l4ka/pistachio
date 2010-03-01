/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:    api/v4/schedule.cc 
 * Description:  debugging of scheduling related stuff
1 *
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
 * $Id: schedule.cc,v 1.7 2004/12/09 01:27:24 cvansch Exp $
 *
 *********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <sync.h>

#include INC_API(tcb.h)
#include INC_API(schedule.h)

tcb_t * global_present_list UNIT("kdebug") = NULL;
spinlock_t present_list_lock;

DECLARE_CMD(cmd_show_sched, root, 'q', "showqueue",  "show scheduling queue");

CMD(cmd_show_sched, cg)
{
    present_list_lock.lock();
    for (cpuid_t cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
    {
	bool print_cpu_header = false;
	for (word_t level = 0; level < MAX_SCHEDULING_LEVEL; level++)
	{
	    /* check whether we have something for this prio */
	    tcb_t* walk = global_present_list;
	    bool print_level_header = false;
	    do {
		if (walk->sched_state.get_level() == level && walk->get_cpu() == cpu) 
		{
		    /* if so, print */
		    if (!print_cpu_header)
		    {
			pgent_t *pgent;
			pgent_t::pgsize_e pgsize;
			space_t *kspace = get_kernel_space();
			addr_t rsched_addr = get_current_scheduler();
			bool valid = kspace->lookup_mapping(rsched_addr, &pgent, &pgsize, cpu);
			ASSERT(valid);
			
			scheduler_t *rsched = (scheduler_t *) 
			    addr_offset(phys_to_virt(pgent->address(kspace, pgsize)),
					addr_mask(rsched_addr, page_mask (pgsize)));
			printf("\n\nCPU %d root %t\n", cpu, rsched->get_root_scheduler());
			print_cpu_header = true;
		    }
		    if (!print_level_header)
		    {		    
			printf("\n[%03d]:", level);
			print_level_header = true;
		    }		    
		    switch (walk->get_state())
		    {
		    case thread_state_t::polling:
			printf(" <%t>", walk);
			break;
		    case thread_state_t::locked_running:
		    case thread_state_t::locked_waiting:
			printf(" (%t)", walk);
			break;
		    case thread_state_t::waiting_forever:
			printf(" [%t]", walk);
			break;
		    case thread_state_t::halted:
		    case thread_state_t::aborted:
			printf(" *%t*", walk);
			break;
		    default:
			printf(" %t", walk);
			break;
		    }
	    
		}
		walk = walk->present_list.next;
	    } while (walk != global_present_list);
	}
    }
    printf("\nidle : %t\n\n", get_idle_tcb());
    present_list_lock.unlock();
    return CMD_NOQUIT;
}
