/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:    api/v4/schedule.cc 
 * Description:  debugging of scheduling related stuff
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
 * $Id: schedule.cc,v 1.7 2004/12/09 01:27:24 cvansch Exp $
 *
 *********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <sync.h>
#include INC_API(tcb.h)
#include INC_API(smp.h)
#include INC_API(schedule.h)
#include INC_API(cpu.h)

tcb_t * global_present_list UNIT("kdebug") = NULL;
spinlock_t present_list_lock;

DECLARE_CMD(cmd_show_sched, root, 'q', "showqueue",  "show scheduling queue");
DECLARE_CMD(cmd_show_sched_empty, root, 'Q', "showqueue",  "show scheduling queue with empty ones");

static void show_sched_queue(bool empty)
{
    int abort = 1000000;
    present_list_lock.lock();

    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
    {
	bool print_cpu_header = false;
        scheduler_t *scheduler = get_on_cpu(cpu, get_current_scheduler());

        if (empty)
        {
            printf("\n\nCPU %d:  accounted tcb %t, max_prio %d\n", 
                   cpu, scheduler->get_accounted_tcb(), scheduler->get_prio_queue()->max_prio);
            printf("\n");
            print_cpu_header = true;
        }

	for (s16_t prio = MAX_PRIORITY; prio >= 0; prio--)
	{
	    /* check whether we have something for this prio */
	    tcb_t* walk = global_present_list;

	    do {
		if (walk->sched_state.get_priority() == prio && walk->get_cpu() == cpu) 
		{
                    /* if so, print */
                    if (!print_cpu_header)
                    {
                        printf("\n\nCPU %d:  accounted tcb %t, max_prio %d\n", 
                               cpu, scheduler->get_accounted_tcb(), scheduler->get_prio_queue()->max_prio);
                        printf("\n");
                        print_cpu_header = true;
                    }

		    printf("\t[%03d]:", prio);
		    walk = global_present_list;
		
		    do {
			if (walk->sched_state.get_priority() == prio && walk->get_cpu() == cpu) 
			    printf(walk->queue_state.is_set(queue_state_t::ready) ? " %t" : " (%t)", walk);
			walk = walk->present_list.next;

		    } while (walk != global_present_list);
		    printf("\n");
		    break;
		}
		walk = walk->present_list.next;

		if (abort-- == 0)
		{
		    // huha -- something fucked up?
		    printf("present-list fucked???\n");
		    walk = global_present_list;
		    for (int i = 0; i < 200; i++)
		    {
			printf("%t (%t <-> %t) ", walk, walk->present_list.prev, walk->present_list.next);
			if (walk->present_list.prev->present_list.next != walk ||
			    walk->present_list.next->present_list.prev != walk)
			    printf("\n*** ERROR ***\n");
			walk = walk->present_list.next;
			if (walk == global_present_list)
			{
			    printf("---> END");
			    break;
			}
		    }
		    return;
		}

	    } while (walk != global_present_list);
	}
    }
    printf("idle : %t\n\n", get_idle_tcb());
    present_list_lock.unlock();
    return;
}

CMD(cmd_show_sched, cg)
{
    show_sched_queue(false);
    return CMD_NOQUIT;
}
    
CMD(cmd_show_sched_empty, cg)
{
    show_sched_queue(true);
    return CMD_NOQUIT;
}
