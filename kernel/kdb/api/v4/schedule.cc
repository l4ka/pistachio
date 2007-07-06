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
#include INC_API(schedule.h)

tcb_t * global_present_list UNIT("kdebug") = NULL;
spinlock_t present_list_lock;

DECLARE_CMD(cmd_show_ready, root, 'q', "showqueue",  "show scheduling queue");

CMD(cmd_show_ready, cg)
{
    int abort = 1000000;
    present_list_lock.lock();
    scheduler_t * scheduler = get_current_scheduler();
    printf("\n");
    for (prio_t prio = MAX_PRIO; prio >= 0; prio--)
    {
        /* check whether we have something for this prio */
        tcb_t* walk = global_present_list;
        do {
            if ( scheduler->get_priority(walk) == prio)
            {
                /* if so, print */
                printf("[%03d]:", prio);
                walk = global_present_list;
                do {
                    if (scheduler->get_priority(walk) == prio) 
		    {
#if !defined(CONFIG_SMP)
			printf(walk->queue_state.is_set(queue_state_t::ready) ? " %.wt" : " (%.wt)", walk);
#else
			printf(walk->queue_state.is_set(queue_state_t::ready) ? 
			       " %t:%d" : " (%t:%d)", walk, walk->get_cpu());
#endif
                    }
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
		return CMD_NOQUIT;
	    }

        } while (walk != global_present_list);
    }
    printf("idle : %t\n\n", get_idle_tcb());
    present_list_lock.unlock();
    return CMD_NOQUIT;
}
