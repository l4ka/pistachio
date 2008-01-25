/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008,  Karlsruhe University
 *                
 * File path:     api/v4/schedule.cc
 * Description:   Scheduling functions
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
 * $Id: schedule.cc,v 1.62 2006/11/23 20:22:48 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(interrupt.h)
#include INC_API(queueing.h)
#include INC_API(syscalls.h)
#include INC_API(smp.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(config.h)

#include <kdb/tracepoints.h>

/* global idle thread, we allocate a utcb to make accessing MRs etc easier */
whole_tcb_t __idle_tcb UNIT("cpulocal") __attribute__((aligned(sizeof(whole_tcb_t))));
utcb_t	    __idle_utcb UNIT("cpulocal") __attribute__((aligned(sizeof(utcb_t))));

/* global scheduler object */
scheduler_t scheduler UNIT("cpulocal");
volatile u64_t scheduler_t::current_time = 0;

#define TOTAL_QUANTUM_EXPIRED (~0ULL)

DECLARE_TRACEPOINT(SYSCALL_THREAD_SWITCH);
DECLARE_TRACEPOINT(TIMESLICE_EXPIRED);
DECLARE_TRACEPOINT(TOTAL_QUANTUM_EXPIRED);
DECLARE_TRACEPOINT(PREEMPTION_DELAYED);
DECLARE_TRACEPOINT(PREEMPTION_DELAY_OVERRULED);
DECLARE_TRACEPOINT(PREEMPTION_FAULT);
DECLARE_TRACEPOINT(PREEMPTION_DELAY_REFRESH);
DECLARE_TRACEPOINT(WAKEUP_TIMEOUT);

DECLARE_TRACEPOINT(SYSCALL_SCHEDULE);
DECLARE_TRACEPOINT_DETAIL(SCHEDULE_DETAILS);


#ifdef CONFIG_SMP
smp_requeue_t scheduler_t::smp_requeue_lists[CONFIG_SMP_MAX_CPUS];

void scheduler_t::smp_requeue(bool holdlock)
{
    smp_requeue_t * rq = &smp_requeue_lists[get_current_cpu()];
    
    if (!rq->is_empty() || holdlock)
    {
	rq->lock.lock();
	tcb_t *tcb = NULL;

	while (!rq->is_empty()) 
	{
	    tcb = rq->dequeue_head();
	    ASSERT( tcb->get_cpu() == get_current_cpu() );
	    
	    if (tcb->requeue_callback) 
	    {
		tcb->requeue_callback(tcb);
		tcb->requeue_callback = NULL;
	    }
	    else {
		cancel_timeout( tcb );
		enqueue_ready( tcb );
	    }
	}
	if (!holdlock) 
	    rq->lock.unlock();
	
    }
}

void scheduler_t::remote_enqueue_ready(tcb_t * tcb)
{
    if (!tcb->requeue)
    {
	cpuid_t cpu = tcb->get_cpu();
	smp_requeue_t * rq = &smp_requeue_lists[cpu];
	rq->lock.lock();
	if (tcb->get_cpu() != cpu) 
	{
	    // thread may have migrated meanwhile
	    TRACEF("curr=%p, %p, CPU#%d != CPU#%d\n", get_current_tcb(), 
		   tcb, cpu, tcb->get_cpu());
	    UNIMPLEMENTED();
	}
	//tcb->requeue_func = __builtin_return_address(0);
	rq->enqueue_head(tcb);
	rq->lock.unlock();
	smp_xcpu_trigger(cpu);
    }
}
#endif


/**
 * Compare two time_t values, either absoulte time points or time
 * points relative to current time.
 *
 * @param r		time to compare against
 *
 * @return true if current time point is earlier than R
 */
bool time_t::operator< (time_t & r)
{
    u64_t curtime = get_current_scheduler ()->get_current_time ();
    u64_t l_to, r_to;

    // Calculate absolute time of current time value
    if (this->is_point ())
	UNIMPLEMENTED ();
    else if (this->is_never ())
	l_to = ~0UL;
    else
	l_to = curtime + this->get_microseconds ();

    // Calculate absolute time of right time value.
    if (r.is_point ())
	UNIMPLEMENTED ();
    else if (r.is_never ())
	r_to = ~0UL;
    else
	r_to = curtime + r.get_microseconds ();

    return l_to < r_to;
}


/**
 * sends preemption IPC to the scheduler thread that the total quantum
 * has expired */
void scheduler_t::total_quantum_expired(tcb_t * tcb)
{
    TRACEPOINT(TOTAL_QUANTUM_EXPIRED, "total quantum expired for %t\n", tcb);

#warning VU: total quantum IPC disabled
    /* Total quantum IPC is an open point.  The expiration may happen
     * in the wrong thread context and thus we have to tunnel the IPC.
     * Also it may happen in the middle of a long IPC which leads to
     * nesting of four.  More details in eSk's p-core posting.
     * Disabled for the time being.
     */
    //tcb->send_preemption_ipc(current_time);
}


/**
 * find_next_thread: selects the next tcb to be dispatched
 *
 * returns the selected tcb, if no runnable thread is available, 
 * the idle tcb is returned.
 */
tcb_t * scheduler_t::find_next_thread(prio_queue_t * prio_queue)
{
    ASSERT(prio_queue);

#ifdef CONFIG_SMP
    // requeue threads which got activated by other CPUs
    smp_requeue(false);
#endif

    for (prio_t prio = prio_queue->max_prio; prio >= 0; prio--)
    {
	//TRACEF("prio=%d, (max=%d)\n", prio, max_prio);
	tcb_t *tcb = prio_queue->get(prio);
	while (tcb) {
	    ASSERT(tcb->queue_state.is_set(queue_state_t::ready));

	    if ( tcb->get_state().is_runnable() /*&& tcb->current_timeslice > 0 */)
	    {
		prio_queue->set(prio, tcb);
		prio_queue->max_prio = prio;
		//TRACE("find_next: returns %t\n", tcb);
		return tcb;
	    }
	    else {
		if (tcb->get_state().is_runnable())
		    TRACEF("dequeueing runnable thread %t without timeslice\n", tcb);
		/* dequeue non-runnable thread */
		prio_queue->dequeue(tcb);
		tcb = prio_queue->get(prio);
	    }
	}
    }
    /* if we can't find a schedulable thread - switch to idle */
    //TRACE("find_next: returns idle\n");
    prio_queue->max_prio = -1;
    return get_idle_tcb();
}

tcb_t * scheduler_t::parse_wakeup_queues(tcb_t * current)
{
    if (!wakeup_list)
	return current;

    // use thread owning the current timeslice
    tcb_t * highest_wakeup = get_prio_queue(current)->timeslice_tcb;
    tcb_t * tcb = wakeup_list;

    ASSERT(highest_wakeup);

    // check if the current timeslice holder is higher prio than timeslice owner
    if (! check_dispatch_thread(current, highest_wakeup))
	highest_wakeup = current;

    bool list_head_changed = false;

    do
    {
	list_head_changed = false;
	if ( has_timeout_expired(tcb, current_time) )
	{
	    /*
	     * We might try to wake up a thread which is waiting
	     * forever.  This can happen if:
	     *
	     *  1) we have issued a timeout IPC, an IPC fast path 
	     * 	   reply occured before the timeout triggered, and an
	     * 	   infinite timeout IPC was issued.  Since we're doing
	     * 	   fast IPC we will do the dequeing lazily (i.e.,
	     * 	   doing it now instead).
	     *
	     *  2) this is an xfer timeout, in which case we should
	     *     let the timeout trigger.
	     */
	    if (tcb->get_state().is_waiting_forever () &&
		! tcb->flags.is_set (tcb_t::has_xfer_timeout))
	    {
		tcb_t * tmp = tcb->wait_list.next;
		cancel_timeout (tcb);
		list_head_changed = true;
		tcb = tmp;
		continue;
	    }

	    /* we have to wakeup the guy */
	    if (check_dispatch_thread(highest_wakeup, tcb))
		highest_wakeup = tcb;

	    tcb_t * tmp = tcb->wait_list.next;
	    cancel_timeout(tcb);
	    list_head_changed = true;

	    tcb->flags -= tcb_t::has_xfer_timeout;

	    if (tcb->get_state().is_sending() ||
		tcb->get_state().is_receiving())
	    {
		/*
		 * The thread must invoke function which handles the
		 * IPC timeout.  The handler returns directly to user
		 * level with an error code.  As such, no special
		 * timeout code is needed in the IPC path.
		 */
		tcb->notify (handle_ipc_timeout, (word_t) tcb->get_state ());
	    }
	    
	    /* set it running and enqueue into ready-queue */
	    tcb->set_state(thread_state_t::running);
	    enqueue_ready(tcb);

	    TRACEPOINT(WAKEUP_TIMEOUT, "wakeup timeout (curr=%t wu=%p) Current time = %ld\n",
		       current, tcb, current_time);

	    /* was preempted -- give him a fresh timeslice */
	    //tcb->current_timeslice = tcb->timeslice_length;
	    tcb = tmp;
	}
	else
	    tcb = tcb->wait_list.next;
    } while ( wakeup_list && (tcb != wakeup_list || list_head_changed) );

    if (highest_wakeup == get_prio_queue(current)->timeslice_tcb)
	highest_wakeup = current;

    return highest_wakeup;
}

/**
 * selects the next runnable thread and activates it.
 * @return true if a runnable thread was found, false otherwise
 */
bool scheduler_t::schedule(tcb_t * current)
{
    tcb_t * tcb = find_next_thread (&root_prio_queue);
    
    ASSERT(tcb);
    ASSERT(current);

    // the newly selected thread gets accounted
    get_prio_queue(tcb)->timeslice_tcb = tcb;
    
    // do not switch to ourself
    if (tcb == current)
	return false;

    if (current != get_idle_tcb())
	enqueue_ready(current);

    current->switch_to(tcb);

    return true;
}

/**
 * selects the next runnable thread in a round-robin fashion
 */
void scheduler_t::end_of_timeslice (tcb_t * tcb)
{
    spin(74, get_current_cpu());
    ASSERT(tcb);
    ASSERT(tcb != get_idle_tcb()); // the idler never yields

    prio_queue_t * prio_queue = get_prio_queue ( tcb );
    ASSERT(prio_queue);

    tcb_t * timeslice_tcb = prio_queue->timeslice_tcb;
    ASSERT(timeslice_tcb);
    ASSERT(get_prio_queue(timeslice_tcb) == prio_queue);

    /*
     * if the timeslice TCB is in the prio queue perform RR
     * scheduling, otherwise the thread gets enqueued later on
     */
    if (prio_queue->timeslice_tcb->queue_state.is_set(queue_state_t::ready))
	prio_queue->set(get_priority( timeslice_tcb ), timeslice_tcb->ready_list.next);

    /*
     * make sure we are in the ready list, enqueue at tail to give
     * others a chance to run; if still in the ready list the thread
     * position is maintained
     */
    enqueue_ready (tcb, false);

    /* renew timeslice of accounted TCB */
    timeslice_tcb->current_timeslice += timeslice_tcb->timeslice_length;

    /* clear the accounted TCB */
    prio_queue->timeslice_tcb = NULL;
}


void scheduler_t::handle_timer_interrupt()
{
    spin(77, get_current_cpu());

    if (kdebug_check_interrupt())
	return;

    if (get_current_cpu() == 0)
    {
	/* update the time global time*/
	current_time += get_timer_tick_length();
    }

#if defined(CONFIG_SMP)
    process_xcpu_mailbox();
    smp_requeue(false);
#endif

    tcb_t * current = get_current_tcb();
    tcb_t * wakeup = parse_wakeup_queues(current);

    /* the idle thread schedules itself so no point to do it here.
     * Furthermore, it should not be preempted on end of timeslice etc.
     */
    if (current == get_idle_tcb())
	return;

    // tick timeslice
    bool reschedule = false;

    tcb_t * timeslice_tcb = get_prio_queue(current)->timeslice_tcb;
    ASSERT(timeslice_tcb);

    /* Check for not infinite timeslice and expired */
    if ( EXPECT_TRUE( timeslice_tcb->timeslice_length != 0 ) &&
	 EXPECT_FALSE( (timeslice_tcb->current_timeslice -=
			 get_timer_tick_length()) <= 0 ) )
    {
	ASSERT(current->get_utcb());

	if( current->get_preempt_flags().is_delayed() &&
	    this->delay_preemption(current, wakeup) )
	{
	    // We have a delayed preemption.  Perform accounting, etc.
	    TRACEPOINT(PREEMPTION_DELAYED, "delayed preemption thread=%t, time=%dus\n", 
		       current, current->current_max_delay);

    	    /* VU: should we give max_delay? */
    	    timeslice_tcb->current_timeslice += current->current_max_delay;
    	    current->current_max_delay = 0;
    	    current->set_preempt_flags( 
		    current->get_preempt_flags().set_pending() );
	}
	else
	{
	    // We have end-of-timeslice.
	    TRACEPOINT(TIMESLICE_EXPIRED, "timeslice expired for %t\n", current);

	    if( EXPECT_FALSE(current->get_preempt_flags().is_delayed()) )
	    {
		if( current->current_max_delay == 0 )
		{
		    TRACEPOINT(PREEMPTION_FAULT, "delay preemption fault by %t\n", current);
		}
		else
		{
		    TRACEPOINT(PREEMPTION_DELAY_OVERRULED, "delayed preemption overruled for %t by %t, "
			       "current prio %08x, sensitive prio %08x, target prio %08x\n",
			       current, wakeup, get_priority(current), get_sensitive_prio(current), 
			       get_priority(wakeup));
		}
	    }

	    end_of_timeslice ( current );
	    reschedule = true;
	}
    }

    /* a higher priority thread was woken up - switch to him.
     * Note: wakeup respects delayed preemption flags */
    if (!reschedule)
    {
	if ( wakeup == current )
	    return;

	ASSERT(wakeup);
	//printf("wakeup preemption %t->%t\n", current, wakeup);
	preempt_thread(current, wakeup);
	current->switch_to(wakeup);
	return;
    }

    /* time slice expired */
    if (EXPECT_FALSE (timeslice_tcb->total_quantum))
    {
	/* we have a total quantum - so do some book-keeping */
	if (timeslice_tcb->total_quantum == TOTAL_QUANTUM_EXPIRED)
	{
	    /* VU: must be revised. If a thread has an expired time quantum
	     * and is activated with switch_to his timeslice will expire
	     * and the event will be raised multiple times */
	    total_quantum_expired (timeslice_tcb);
	}
	else if (timeslice_tcb->total_quantum <= timeslice_tcb->timeslice_length)
	{
	    /* we are getting close... */
	    timeslice_tcb->current_timeslice += timeslice_tcb->total_quantum;
	    timeslice_tcb->total_quantum = TOTAL_QUANTUM_EXPIRED;
	}
	else
	{
	    // account this time slice
	    timeslice_tcb->total_quantum -= timeslice_tcb->timeslice_length;
	}
    }

    /* schedule the next thread */
    enqueue_ready(current);
    schedule(current);
}

bool scheduler_t::delay_preemption( tcb_t * current, tcb_t * tcb )
{
    // we always allow ourself to delay our preemption
    if (current == tcb)
	return true;
    
    if ( get_sensitive_prio (current) < get_priority (tcb) )
	return false;

    return current->current_max_delay > 0;
}
	

static void SECTION(".init") init_all_threads(void)
{
    init_interrupt_threads();
    init_kernel_threads();
    init_root_servers();

#if defined(CONFIG_KDB_ON_STARTUP)
    enter_kdebug ("System started (press 'g' to continue)");
#endif
}

/**
 * the idle thread checks for runnable threads in the run queue
 * and performs a thread switch if possible. Otherwise, it 
 * invokes a system sleep function (which should normally result in
 * a processor halt)
 */
static void idle_thread()
{
    TRACE_INIT("Idle thread started on CPU %d\n", get_current_cpu());

    while(1)
    {
	if (!get_current_scheduler()->schedule(get_idle_tcb()))
	{
	    spin(78, get_current_cpu());
	    processor_sleep();
	}
    }
}

SYS_THREAD_SWITCH (threadid_t dest)
{
    /* Make sure we are in the ready queue to 
     * find at least ourself and ensure that the thread 
     * is rescheduled */
    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();

    TRACEPOINT( SYSCALL_THREAD_SWITCH, "SYS_THREAD_SWITCH current=%t, dest=%t\n",
		current, TID(dest));

    /* explicit timeslice donation */
    if (!dest.is_nilthread())
    {
	tcb_t * dest_tcb = get_current_space()->get_tcb(dest);

	if ( dest_tcb == current )
	    return_thread_switch();

	if ( dest_tcb->get_state().is_runnable() &&
	     dest_tcb->myself_global == dest &&
	     dest_tcb->is_local_cpu() )
	{
	    scheduler->enqueue_ready(current);
	    current->switch_to(dest_tcb);
	    return_thread_switch();
	}
    }

    scheduler->enqueue_ready(current);

    /* user cooperatively preempts */
    if ( current->current_max_delay < current->max_delay )
    {
	current->set_preempt_flags( current->get_preempt_flags().clear_pending() );
	/* refresh max delay */
	current->current_max_delay = current->max_delay;
	TRACEPOINT(PREEMPTION_DELAY_REFRESH, "delayed preemption refresh for %t\n", current);
    }


    /* eat up timeslice - we get a fresh one */
    scheduler->get_prio_queue(current)->timeslice_tcb->current_timeslice = 0;
    scheduler->end_of_timeslice (current);
    scheduler->schedule (current);
    return_thread_switch();
}


/* local part of schedule */
bool do_schedule(tcb_t * tcb, word_t time_control, word_t prio, 
		 word_t preemption_control )
{
    ASSERT(tcb->get_cpu() == get_current_cpu());

    scheduler_t * scheduler = get_current_scheduler();

    if ( (prio != (~0UL)) && ((prio_t)prio != scheduler->get_priority(tcb)) )
    {
	scheduler->dequeue_ready(tcb);
	scheduler->set_priority(tcb, (prio_t) (prio & 0xff));
	scheduler->enqueue_ready(tcb);
    }

    if ( time_control != (~0UL) )
    {
	time_t time;
	
	// total quantum
	time.set_raw ((u16_t)time_control);
	if ( time.is_period() )
	{
	    scheduler->set_total_quantum (tcb, time);
	    scheduler->enqueue_ready (tcb);
	}
	
	// timeslice length
	time.set_raw (time_control >> 16);
	if ( time.is_period() )
	    scheduler->set_timeslice_length (tcb, time);
    }

    if ( preemption_control != (~0UL) )
    {
#warning VU: limit upper bound of sensitive prio!
	scheduler->set_maximum_delay (tcb, preemption_control & 0xffff);
	    
	/* only set sensitive prio if _at least_ equal to current prio */
	prio_t sens_prio = (preemption_control >> 16) & 0xff;
	if ( sens_prio >= scheduler->get_priority(tcb) )
	    scheduler->set_sensitive_prio(tcb, sens_prio);
    }

    return true;
}

#ifdef CONFIG_SMP
static void do_xcpu_schedule(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;

    // meanwhile migrated? 
    if (tcb->get_cpu() != get_current_cpu())
	xcpu_request (tcb->get_cpu(), do_xcpu_schedule, tcb, 
		      entry->param[0], entry->param[1], entry->param[2]);
    else
	do_schedule (tcb, entry->param[0], entry->param[1], entry->param[2]);
}
#endif

SYS_SCHEDULE (threadid_t dest_tid, word_t time_control, 
	      word_t processor_control, word_t prio,
	      word_t preemption_control )
{
    
    TRACEPOINT(SYSCALL_SCHEDULE, 
	       "SYS_SCHEDULE: curr=%t, dest=%t, time_ctrl=%x, "
	       "proc_ctrl=%x, prio=%x, preemption_ctrl=%x\n",
	       get_current_tcb(), TID(dest_tid), time_control, 
	       processor_control, prio, preemption_control);

    tcb_t * dest_tcb = get_current_space()->get_tcb(dest_tid);
    
    // make sure the thread id is valid
    if (dest_tcb->get_global_id() != dest_tid)
    {
	get_current_tcb ()->set_error_code (EINVALID_THREAD);
	return_schedule(0, 0);
    }

    // Don't allow to raise priority above current thread's prio
    if ((prio != (~0UL)) &&
	((prio_t) prio >
	 get_current_scheduler ()->get_priority (get_current_tcb ())))
    {
	get_current_tcb ()->set_error_code (EINVALID_PARAM);
	return_schedule (0, 0);
    }

    // are we in the same address space as the scheduler of the thread?
    tcb_t * sched_tcb = get_current_space()->get_tcb(dest_tcb->get_scheduler());
    if (sched_tcb->get_global_id() != dest_tcb->get_scheduler() ||
	sched_tcb->get_space() != get_current_space())
    {
	get_current_tcb ()->set_error_code (ENO_PRIVILEGE);
	return_schedule(0, 0);
    }

#warning FIXME: Check sys_schedule parameters

    if ( dest_tcb->is_local_cpu() )
    {
	do_schedule ( dest_tcb, time_control, prio, preemption_control);
	
	if ( processor_control != ~0UL ) 
	    dest_tcb->migrate_to_processor (processor_control);
    }
#ifdef CONFIG_SMP
    else
    {
	xcpu_request(dest_tcb->get_cpu(), do_xcpu_schedule, 
		     dest_tcb, time_control, prio, preemption_control);

	if ( processor_control != ~0UL ) 
	    dest_tcb->migrate_to_processor (processor_control);
    }
#endif

    thread_state_t state = dest_tcb->get_state();

    return_schedule(state == thread_state_t::aborted	? 1 : 
		    state.is_halted()			? 2 :
		    state.is_running()			? 3 :
		    state.is_polling()			? 4 :
		    state.is_sending()			? 5 :
		    state.is_waiting()			? 6 :
		    state.is_receiving()		? 7 : ({ TRACEF("invalid state (%x)\n", (word_t)state); 0; }),
		    0);
}


/**********************************************************************
 *
 *                     Initialization
 *
 **********************************************************************/

void SECTION(".init") scheduler_t::start(cpuid_t cpuid)
{
    TRACE_INIT ("Switching to idle thread (CPU %d)\n", cpuid);
    get_idle_tcb()->set_cpu(cpuid);

    initial_switch_to(get_idle_tcb());
}

void SECTION(".init") scheduler_t::init( bool bootcpu )
{
    
    TRACE_INIT ("Initializing threading CPU %d\n", get_current_cpu());

    /* wakeup list */
    wakeup_list = NULL;
    root_prio_queue.init();

    get_idle_tcb()->create_kernel_thread(NILTHREAD, &__idle_utcb);
    /* set idle-magic */
    get_idle_tcb()->set_space(get_kernel_space());
    get_idle_tcb()->myself_global.set_raw((word_t)0x1d1e1d1e1d1e1d1eULL);
    get_idle_tcb()->create_startup_stack(idle_thread);
    if( bootcpu )
    	get_idle_tcb()->notify(init_all_threads);
    return;
}
