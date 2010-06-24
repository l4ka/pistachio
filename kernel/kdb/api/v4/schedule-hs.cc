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
#include INC_API(cpu.h)

tcb_t * global_present_list UNIT("kdebug") = NULL;
spinlock_t present_list_lock;

DECLARE_CMD(cmd_show_sched, root, 'q', "showqueue",  "show scheduling queue");

static void depth_indent( int depth )
{
    for( int i = 0; i < depth; i++ )
	printf( "       " );
}


static bool is_subdomain( tcb_t *tcb )
{
    return (tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain) &&
            tcb->sched_state.get_prio_queue());
}

static void show_prio_queue( int depth, scheduler_t *scheduler, prio_queue_t *prio_queue, cpuid_t cpu )
{
    depth_indent( depth );
    
    tcb_t *domain_tcb = prio_queue->get_domain_tcb();
    
    printf( "priority queue %p pass %U domain tcb %p  depth %d\n", 
            prio_queue, prio_queue->get_global_pass(), domain_tcb, prio_queue->get_depth() );

    for (s16_t prio = MAX_PRIORITY; prio >= 0; prio--)
    {
        /* check whether we have something for this prio */
        tcb_t* walk = global_present_list;
        do {

            if (walk->sched_state.get_priority() == prio && 
                walk->sched_state.get_prio_queue() == prio_queue &&
                walk->get_cpu() == cpu)
            {
                /* if so, print */
                depth_indent( depth );
                printf("[%02x]:\n", walk->sched_state.get_priority());

                bool subdomain = false;
                
                do {
                    if (walk->sched_state.get_priority() == prio && 
                        walk->sched_state.get_prio_queue() == prio_queue &&
                        walk->get_cpu() == cpu)
                    {
                        bool ready = walk->queue_state.is_set(queue_state_t::ready);
                        
                        if (is_subdomain(walk)) 
                            subdomain = true;
                        
                        depth_indent( depth + 1 );
                        printf("  [%16U][%8u]", walk->sched_state.get_pass(),walk->sched_state.get_stride());
                        printf(ready ? " %t\n" : " (%t)\n", walk);
                    }
                    walk = walk->present_list.next;

                } while (walk != global_present_list);
                printf("\n");

                if( subdomain )
                {
                    // Print the subdomains.
                    walk = global_present_list;
                    do {
                        if (walk->sched_state.get_priority() == prio && 
                            walk->sched_state.get_prio_queue() == prio_queue &&
                            walk->get_cpu() == cpu && is_subdomain(walk) )
                        {
                            // We found a subdomain.
                            show_prio_queue( depth+1, scheduler, walk->sched_state.get_domain_prio_queue(), cpu);
                        }
                        walk = walk->present_list.next;
                    } while( walk != global_present_list );
                }
            }
            else
                walk = walk->present_list.next;

        } while (walk != global_present_list);
    }
}

CMD(cmd_show_sched, cg)
{
    present_list_lock.lock();
    printf("\n");
    
    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
    {
        scheduler_t *scheduler = get_on_cpu(cpu, get_current_scheduler());
        printf("\n\nCPU %d:  scheduled tcb %t, scheduled queue %p max_prio %d\n", 
               cpu, scheduler->get_accounted_tcb(), scheduler->get_scheduled_queue(),
               scheduler->get_prio_queue()->max_prio);
        printf("\n");

        show_prio_queue( 0, scheduler, scheduler->get_prio_queue(), cpu );
    }
    printf("idle : %p\n\n", get_idle_tcb());

    present_list_lock.unlock();
    return CMD_NOQUIT;
}


