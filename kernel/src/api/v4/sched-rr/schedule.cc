/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-rr/schedule.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#include <debug.h>
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(interrupt.h)
#include INC_API(queueing.h)
#include INC_API(syscalls.h)
#include INC_API(smp.h)
#include INC_API(cpu.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(config.h)

#define TOTAL_QUANTUM_EXPIRED (~0ULL)
DECLARE_TRACEPOINT(TOTAL_QUANTUM_EXPIRED);
DECLARE_TRACEPOINT(SCHEDULE_WAKEUP_TIMEOUT);
DECLARE_TRACEPOINT(TIMESLICE_EXPIRED);
DECLARE_TRACEPOINT(SCHEDULE_PM_DELAYED);
DECLARE_TRACEPOINT(SCHEDULE_PM_DELAY_OVERRULED);
DECLARE_TRACEPOINT(SCHEDULE_PM_FAULT);
DECLARE_TRACEPOINT(SCHEDULE_PM_DELAY_REFRESH);

volatile u64_t rr_scheduler_t::current_time = 0;

/**
 * the idle thread checks for runnable threads in the run queue
 * and performs a thread switch if possible. Otherwise, it 
 * invokes a system sleep function (which should normally result in
 * a processor halt)
 */
void scheduler_t::idle()
{
    TRACE_INIT("\tIdle thread started on CPU %d\n", get_current_cpu());

    while(1)
    {
	if (!schedule())
	{
            spin(78, get_current_cpu());
	    processor_sleep();
	}
    }
}

/**
 * find_next_thread: selects the next tcb to be dispatched
 *
 * returns the selected tcb, if no runnable thread is available, 
 * the idle tcb is returned.
 */
tcb_t * scheduler_t::find_next_thread(policy_sched_next_thread_t *)
{
    prio_queue_t * prio_queue = get_prio_queue();
    ASSERT(prio_queue);
    
#if defined(CONFIG_SMP)
    // requeue threads which got activated by other CPUs
    smp_requeue(false);
#endif

    ASSERT(prio_queue->max_prio <= MAX_PRIORITY);
    
    for (s16_t prio = prio_queue->max_prio; prio >= 0; prio--)
    {
	tcb_t *tcb = prio_queue->get(prio);
	while (tcb) 
        {
	    ASSERT(tcb->queue_state.is_set(queue_state_t::ready));
            
	    if ( tcb->get_state().is_runnable() /*&& tcb->current_timeslice > 0 */)
	    {
		prio_queue->set(prio, tcb);
		prio_queue->max_prio = prio;
		TRACE_SCHEDULE_DETAILS("fnt next thread %t max_prio %d", tcb, prio_queue->max_prio);
		return tcb;
	    }
	    else 
	    {
		if (tcb->get_state().is_runnable())
		    TRACEF("dequeueing runnable thread %t without timeslice\n", tcb);
		/* dequeue non-runnable thread */
		prio_queue->dequeue(tcb);
		tcb = prio_queue->get(prio);
	    }
	}
    }
    /* if we can't find a schedulable thread - switch to idle */
    prio_queue->max_prio = -1;
    return get_idle_tcb();
}

/**
 * sends preemption IPC to the scheduler thread that the total quantum
 * has expired */
void rr_scheduler_t::total_quantum_expired(tcb_t * tcb)
{
    TRACEPOINT(TOTAL_QUANTUM_EXPIRED, "total quantum expired for %t\n", tcb);
    
    enter_kdebug("total quantum IPC unimplemented");
    UNIMPLEMENTED();
    /* Total quantum IPC is an open point.  The expiration may happen
     * in the wrong thread context and thus we have to tunnel the IPC.
     * Also it may happen in the middle of a long IPC which leads to
     * nesting of four.  More details in eSk's p-core posting.
     * Disabled for the time being.
     */
    //tcb->send_preemption_ipc(current_time);
}


tcb_t * rr_scheduler_t::parse_wakeup_queues(tcb_t * current)
{
    if (!wakeup_list)
	return current;

    // use thread owning the current timeslice
    tcb_t * highest_wakeup = get_prio_queue()->get_timeslice_tcb();
    tcb_t * tcb = wakeup_list;

    ASSERT(highest_wakeup);

    // check if the current timeslice holder is higher prio than timeslice owner
    if (! check_dispatch_thread(current, highest_wakeup))
	highest_wakeup = current;

    bool list_head_changed = false;

    do
    {
	sched_ktcb_t *sched_state = &tcb->sched_state;
	list_head_changed = false;
	if ( sched_state->has_timeout_expired(current_time) )
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
		tcb_t * tmp = tcb->sched_state.wait_list.next;
		sched_state->cancel_timeout ();
		list_head_changed = true;
		tcb = tmp;
		continue;
	    }

	    TRACEPOINT(SCHEDULE_WAKEUP_TIMEOUT, "wakeup timeout wu=%p s=%s to=%ld time=%ld\n",
		       tcb,  tcb->get_state().string(), sched_state->get_timeout(), (word_t) current_time);

	    /* we have to wakeup the guy */
	    if (check_dispatch_thread(highest_wakeup, tcb))
		highest_wakeup = tcb;

	    tcb_t * tmp = tcb->sched_state.wait_list.next;
	    sched_state->cancel_timeout ();
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

	    /* was preempted -- give him a fresh timeslice */
	    //tcb->current_timeslice = tcb->timeslice_length;
	    tcb = tmp;
	}
	else
	    tcb = tcb->sched_state.wait_list.next;
    } while ( wakeup_list && (tcb != wakeup_list || list_head_changed) );

    if (highest_wakeup == get_prio_queue()->get_timeslice_tcb())
	highest_wakeup = current;

    return highest_wakeup;
}


/**
 * selects the next runnable thread in a round-robin fashion
 */
void rr_scheduler_t::end_of_timeslice (tcb_t * tcb)
{
    spin(74, get_current_cpu());
    ASSERT(tcb);
    ASSERT(tcb != get_idle_tcb()); // the idler never yields

    prio_queue_t * prio_queue = get_prio_queue();
    ASSERT(prio_queue);

    tcb_t * timeslice_tcb = get_prio_queue()->get_timeslice_tcb();
    ASSERT(timeslice_tcb);

    /*
     * if the timeslice TCB is in the prio queue perform RR
     * scheduling, otherwise the thread gets enqueued later on
     */
    sched_ktcb_t *tsched_state = &timeslice_tcb->sched_state;
    
    if (timeslice_tcb->queue_state.is_set(queue_state_t::ready))
	prio_queue->set(tsched_state->get_priority(), tsched_state->ready_list.next);

    /*
     * make sure we are in the ready list, enqueue at tail to give
     * others a chance to run; if still in the ready list the thread
     * position is maintained
     */
    enqueue_ready (tcb, false);

    /* renew timeslice of accounted TCB */
    tsched_state->renew_timeslice(tsched_state->get_timeslice_length());

    /* clear the accounted TCB */
    prio_queue->set_timeslice_tcb(NULL);
}

#if defined(CONFIG_SMP)
smp_requeue_t rr_scheduler_t::smp_requeue_lists[CONFIG_SMP_MAX_CPUS];

void rr_scheduler_t::smp_requeue(bool holdlock)
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
	    
	    if (tcb->sched_state.requeue_callback) 
	    {
		tcb->sched_state.requeue_callback(tcb);
		tcb->sched_state.requeue_callback = NULL;
	    }
            else
            {
                tcb->sched_state.cancel_timeout ();
                if (tcb->get_state().is_runnable())
                    enqueue_ready( tcb );
            }
	}
	if (!holdlock) 
	    rq->lock.unlock();
        
        get_current_scheduler()->schedule();
    }
}

void scheduler_t::remote_schedule(tcb_t * tcb)
{
    if (!tcb->sched_state.requeue)
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
	rq->enqueue_head(tcb);
	rq->lock.unlock();
	smp_xcpu_trigger(cpu);
    }
}


/**
 * re-integrates a thread into the CPUs queues etc.
 */
static void xcpu_integrate_thread(tcb_t * tcb)
{
    ASSERT(!tcb->queue_state.is_set(queue_state_t::wakeup));
    ASSERT(tcb != get_current_tcb());

    TRACE_SCHEDULE_DETAILS("integrate %t (s=%s) current %t\n", tcb, tcb->get_state().string(), get_current_tcb());

    tcb->lock();

    // interrupt threads are handled specially
    if (tcb->is_interrupt_thread())
    {
	migrate_interrupt_end(tcb);
	tcb->unlock();
	return;
    }

    tcb->unlock();
    
    scheduler_t *scheduler = get_current_scheduler();
    sched_ktcb_t *sched_state = &tcb->sched_state;
    /* VU: the thread may have received an IPC meanwhile hence we
     * check whether the thread is already running again.  to make it
     * fully working the waiting timeout must be set more carefull! */
    
    
    if (tcb->get_state().is_runnable())
	scheduler->schedule(tcb);
    else if (sched_state->get_timeout() && tcb->get_state().is_waiting_with_timeout())
	sched_state->set_timeout(scheduler->get_current_time() + sched_state->get_timeout());
}

void scheduler_t::move_tcb(tcb_t *tcb, cpuid_t cpu)
{

    tcb->lock();

    /* VU: it is only necessary to notify the other CPU if the thread
     * is in one of the scheduling queues (wakeup, ready) or is an
     * interrupt thread */
    bool need_xcpu = tcb->get_state().is_runnable() || 
	(tcb->sched_state.get_timeout() != 0) || tcb->is_interrupt_thread();


    smp_requeue(true);
    ASSERT(tcb->sched_state.requeue == NULL);

    if (tcb->get_space())
	tcb->get_space()->move_tcb(tcb, get_current_cpu(), cpu);

    tcb->set_cpu(cpu);
    unlock_requeue();
    
    if (need_xcpu) 
    {
	tcb->sched_state.requeue_callback = xcpu_integrate_thread;
	remote_schedule(tcb);
    }
    tcb->unlock();


}    

#endif

void scheduler_t::handle_timer_interrupt()
{
    spin(77, get_current_cpu());

#if defined(CONFIG_DEBUG)
    if (kdebug_check_interrupt())
	return;
#endif
    
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

    tcb_t * timeslice_tcb = get_prio_queue()->get_timeslice_tcb();
    ASSERT(timeslice_tcb);
 
    sched_ktcb_t *tsched_state = &timeslice_tcb->sched_state;
    sched_ktcb_t *csched_state = &current->sched_state;
    
    /* Check for not infinite timeslice and expired */
    if ( EXPECT_TRUE(tsched_state->get_timeslice_length() != 0 ) &&
	 EXPECT_FALSE(tsched_state->account_timeslice(get_timer_tick_length()) <= 0))
    {
	ASSERT(current->get_utcb());

	if( current->get_preempt_flags().is_delayed() &&
	    csched_state->delay_preemption(wakeup) )
	{
    	    /* VU: should we give max_delay? */
    	    tsched_state->renew_timeslice(csched_state->get_maximum_delay());
	    csched_state->set_maximum_delay(0);
    	    current->set_preempt_flags(current->get_preempt_flags().set_pending() );
	}
	else
	{
	    // We have end-of-timeslice.
	    TRACEPOINT(TIMESLICE_EXPIRED, "timeslice expired for %t\n", current);

	    if( EXPECT_FALSE(current->get_preempt_flags().is_delayed()) )
	    {
		if( csched_state->get_maximum_delay() == 0 )
		{
		    TRACEPOINT(SCHEDULE_PM_FAULT, "delay preemption fault by %t\n", current);
		}
		else
		{
		    TRACEPOINT(SCHEDULE_PM_DELAY_OVERRULED, "delayed preemption overruled for %t by %t, "
			       "current prio %08x, sensitive prio %08x, target prio %08x\n",
			       current, wakeup, 
			       csched_state->get_priority(), 
			       csched_state->get_sensitive_prio(), 
			       wakeup->sched_state.get_priority());
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
 	TRACE_SCHEDULE_DETAILS("wakeup preemption %t %t", current, wakeup);
        schedule(wakeup, sched_dest);
	return;
    }

    /* time slice expired */
    if (EXPECT_FALSE (tsched_state->get_total_quantum()))
    {
	/* we have a total quantum - so do some book-keeping */
	if (tsched_state->get_total_quantum() == TOTAL_QUANTUM_EXPIRED)
	{
	    /* VU: must be revised. If a thread has an expired time quantum
	     * and is activated with switch_to his timeslice will expire
	     * and the event will be raised multiple times */
	    total_quantum_expired (timeslice_tcb);
	}
	else if (tsched_state->get_total_quantum() <= tsched_state->get_timeslice_length())
	{
	    /* we are getting close... */
	    tsched_state->renew_timeslice(tsched_state->get_total_quantum());
	    tsched_state->set_total_quantum(TOTAL_QUANTUM_EXPIRED);
	}
	else
	{
	    // account this time slice
	    tsched_state->account_quantum(tsched_state->get_timeslice_length());
	}
    }

    /* schedule the next thread */
    schedule();
}

