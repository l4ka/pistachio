/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-hs/schedule.cc
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
DECLARE_TRACEPOINT(SCHEDULE_PM_DELAY_REFRESH);
DECLARE_TRACEPOINT(SCHEDULE_PRIO_DOMAIN);

DECLARE_KMEM_GROUP(kmem_sched);

FEATURESTRING ("hscheduling");

volatile u64_t hs_scheduler_t::current_time = 0;
	
prio_queue_t * prio_queue_t::add_prio_domain(schedule_ctrl_t prio_control)
{
    word_t num_cpus = cpu_t::count;
    
    ASSERT( (sizeof(tcb_t) + sizeof(prio_queue_t)) < sizeof(whole_tcb_t) );
    ASSERT( get_depth() < sizeof(word_t) );
    
    // Allocate dummy tcbs for the scheduling domain.
    whole_tcb_t *domain_tcbs = (whole_tcb_t *)kmem.alloc( kmem_sched, sizeof(whole_tcb_t) * num_cpus );
    
    if( domain_tcbs == NULL )
        return NULL;
    
    // Initialize a dummy tcb for the domain, which serves as a schedulable entity.
    for( cpuid_t cpu = 0; cpu < num_cpus; cpu++ )
    {
        tcb_t *domain_tcb = (tcb_t *) &domain_tcbs[cpu];
        
        domain_tcb->create_inactive(threadid_t::nilthread(), threadid_t::nilthread(), sktcb_user);
        domain_tcb->sched_state.flags += sched_ktcb_t::is_schedule_domain;

    	// Use the prio queue within the TCB's unused stack area.
	prio_queue_t *domain_queue = domain_tcb->sched_state.get_domain_prio_queue();
        
       	domain_queue->init(domain_tcb);
        
#if defined(CONFIG_SMP)
	// Link the domain queues for each CPU.  Point the head at CPU 0.
	domain_queue->cpu_head = ((tcb_t *) domain_tcbs)->sched_state.get_domain_prio_queue();
	// Point the link at the domain queue for the next CPU.
        domain_queue->cpu_link = ((tcb_t *)&domain_tcbs[(cpu+1) % num_cpus])->sched_state.get_domain_prio_queue();
#endif

	if( prio_control != schedule_ctrl_t::nilctrl() )
	{
	    // Set stride and priority of the domain.
	    if( prio_control.stride )
		domain_tcb->sched_state.set_stride(prio_control.stride);
	    domain_tcb->sched_state.set_priority(prio_control.prio);
#if defined(CONFIG_X_EVT_LOGGING)
            if ((word_t) prio_control.logid > 0 &&
                (word_t) prio_control.logid < MAX_LOGIDS)
                domain_tcb->sched_state.set_logid(prio_control.logid);
#endif

	}
    }

    // Install the tcb into the parent prio queue.  Must be done after
    // all other cpu state is finalized.
    for( cpuid_t cpu = 0; cpu < num_cpus; cpu++ )
    {
	tcb_t *domain_tcb = (tcb_t *)&domain_tcbs[cpu];
	ASSERT(domain_tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain));

	domain_tcb->sched_state.set_prio_queue(this);
 	if( cpu != get_current_cpu() )
	    domain_tcb->migrate_to_processor(cpu);
        
        TRACEPOINT (SCHEDULE_PRIO_DOMAIN, "new prio domain tcb %t, cpu %d, prio %d, stride %d\n", 
                    domain_tcb, cpu, domain_tcb->sched_state.get_priority(), domain_tcb->sched_state.get_stride());
    }
    
    return ((tcb_t *)&domain_tcbs[get_current_cpu()])->sched_state.get_domain_prio_queue();
}

prio_queue_t *prio_queue_t::domain_partner( cpuid_t cpu )
{
    ASSERT(domain_tcb);
    
    if( domain_tcb->get_cpu() == cpu )
        return this;

#if defined(CONFIG_SMP)
    
    prio_queue_t *partner_queue = cpu_head;
    
    ASSERT(partner_queue);
    // Look for the domain prio queue for the target CPU.
    while( partner_queue->domain_tcb->get_cpu() != cpu )
    {
        partner_queue = partner_queue->cpu_link;
        ASSERT(partner_queue);
    }
    return partner_queue;
#endif
    return NULL;
    
}

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
tcb_t * scheduler_t::find_next_thread(prio_queue_t * prio_queue)
{
    ASSERT(prio_queue);

#if defined(CONFIG_SMP)
    // requeue threads which got activated by other CPUs
    smp_requeue(false);
#endif

    //for (prio_t prio = prio_queue->max_prio; prio >= 0; prio--)
    for (s16_t prio = MAX_PRIORITY; prio >= 0; prio--)
    {
	// Proportional share stride scheduling search.
	tcb_t *search_tcb = NULL;
	tcb_t *tcb = prio_queue->get(prio);
	tcb_t *return_tcb = NULL;
	
	while( tcb && !search_tcb )
	{
           
	    while (tcb)
	    {
		if( tcb->get_state().is_runnable() || 
                    tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain))
		{
		    if ( !search_tcb || (tcb->sched_state.get_pass() < search_tcb->sched_state.get_pass()) )
			search_tcb = tcb;
		    
		    TRACEPOINT (SCHEDULE_DETAILS, "fnt search %t (pass %U) tcb %t (pass %U)\n",
				search_tcb, search_tcb->sched_state.get_pass(), 
				tcb, tcb->sched_state.get_pass());

		    tcb = tcb->sched_state.ready_list.next;
                    
		    if( tcb == prio_queue->get(prio) )
			tcb = NULL; // We wrapped around the list.
		}
		else 
		{
		    // Dequeue a blocked thread.
		    tcb_t *next_tcb = tcb->sched_state.ready_list.next;
		    prio_queue->dequeue(tcb);
		    tcb = (tcb == next_tcb) ? NULL : next_tcb;
		}
                
	    }

	    return_tcb = search_tcb;
	    if( search_tcb && search_tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain))
	    {
		// Subdomain.  We want to find a schedulable thread in the subdomain.
                TRACEPOINT (SCHEDULE_PRIO_DOMAIN, "fnt prio domain tcb %t, prio %d, stride %d\n", 
                            search_tcb, prio, search_tcb->sched_state.get_stride());
		prio_queue_t *domain_prio_queue = search_tcb->sched_state.get_domain_prio_queue();
		tcb_t *domain_tcb = find_next_thread( domain_prio_queue );

		if( domain_tcb == get_idle_tcb() )
		{
		    // Nothing found in the subdomain, so dequeue.
		    prio_queue->dequeue(search_tcb);

		    // Prepare to restart the search at the current prio.
		    search_tcb = NULL;
		    tcb = prio_queue->get(prio);
		}
		else
		    return_tcb = domain_tcb;
	    }
	}

	if( search_tcb )
	{
	    // We found a thread!  Account for the thread, whether it
	    // is a real thread or a subdomain.
            prio_queue->set_global_pass(search_tcb->sched_state.get_pass());
            prio_queue->set(prio, search_tcb);
    	    prio_queue->max_prio = prio;
	    // Now give the scheduler the newly scheduled thread's timeslice.
	    current_timeslice = return_tcb->sched_state.get_timeslice();
	    delayed_preemption_start = get_timestamp();
	    // Return the real thread (found in this domain or a subdomain).
	    return return_tcb;
	}
    }
    /* If we can't find a schedulable thread - switch to idle.  */
    prio_queue->max_prio = -1;
    return get_idle_tcb();
}

#if defined(CONFIG_SMP)
void do_xcpu_domain_reset_period(cpu_mb_entry_t * entry)
{
    prio_queue_t *queue = (prio_queue_t *) entry->param[0];
    ASSERT(queue);
    queue->reset_period_cycles(get_cpu_cycles());
}

static void do_xcpu_domain_stride(cpu_mb_entry_t * entry)
{
    tcb_t *domain_tcb = entry->tcb;
    word_t stride = entry->param[0];
    scheduler_t *scheduler = get_current_scheduler();

    ASSERT(domain_tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain));
    scheduler->dequeue_ready( domain_tcb );
    domain_tcb->sched_state.set_stride( stride );
    scheduler->enqueue_ready( domain_tcb );
}

#endif

void hs_scheduler_t::policy_scheduler_init()
{
    wakeup_list = NULL;
    scheduled_tcb = NULL;
    scheduled_queue = NULL;
    cpuid_t cpu = get_current_cpu();
    
    root_prio_queue.init(get_on_cpu(cpu, get_idle_tcb()));
#if defined(CONFIG_SMP)
    root_prio_queue.cpu_head = get_on_cpu(0, get_current_scheduler())->get_prio_queue();
    root_prio_queue.cpu_link = get_on_cpu((cpu+1) % cpu_t::count, get_current_scheduler())->get_prio_queue();
#endif
    
}

void hs_scheduler_t::hs_extended_schedule(schedule_req_t *req)
{

    /* Extended HS scheduler control:
     * params:
     *  dest                = target
     *  time_control        = domain
     *  processor_control   = domain queue
     *  prio_control        = prio
     */

    word_t control = req->preemption_control.hs_extended_ctrl;
    tcb_t *dest = req->tcb;
    prio_queue_t *domain_queue = (prio_queue_t *) req->processor_control.raw;

    TRACE_SCHEDULE_DETAILS("extended HS schedule ctrl %x, domain: %x q %p, dest: %t, prio %d stride %d\n",
                           control, req->time_control.raw, domain_queue, dest, req->prio_control.prio, req->prio_control.stride);

    // Target thread is the thread to put into a subdomain of the parent scheduling domain.
    if( control & 1 )
    {
        // Create a new domain as a child of the domain.
        prio_queue_t *sub_queue = domain_queue->add_prio_domain(req->prio_control);
        ASSERT(sub_queue && sub_queue->get_domain_tcb());
        ASSERT(sub_queue->get_domain_tcb()->is_local_cpu());
        
        TRACE_SCHEDULE_DETAILS("new domain %p for %t prio %d stride %d\n", sub_queue, dest, 
                               req->prio_control.prio, req->prio_control.stride);

        // Move the target thread to the new domain.
        dest->sched_state.migrate_prio_queue(sub_queue);
        return;
    }
    else if( control & 2 )
    {
        TRACE_SCHEDULE_DETAILS("migrate %t to domain queue %x\n", dest, domain_queue);
        dest->sched_state.migrate_prio_queue(domain_queue);
    }
    
    if( control & 8 )
    {
        
        TRACE_SCHEDULE_DETAILS( "reset period cycles queue %p, stride %u, cpu %d\n", 
                                scheduled_queue, get_current_cpu());
        UNTESTED();
    
        prio_queue_t *queue = scheduled_queue;
        queue->reset_period_cycles(get_cpu_cycles());
        
#if defined(CONFIG_SMP)
        queue = queue->cpu_head;
        do 
        {
            ASSERT(queue);
            ASSERT(queue->get_domain_tcb());
            cpuid_t cpu = queue->get_domain_tcb()->get_cpu();
            if( cpu != get_current_cpu() )
                xcpu_request( cpu, do_xcpu_domain_reset_period, NULL, (word_t) queue );
            queue = queue->cpu_link;
        } while( queue != queue->cpu_head );
#endif /* defined(CONFIG_SMP)*/
        return;
    }            

    if( control & 16 )
    {
        tcb_t *cpu_domain_tcb = domain_queue->get_domain_tcb();
        
        if (req->prio_control.stride)
        {

            TRACE_SCHEDULE_DETAILS( "restride queue %p domain tcb %t domain cpu tcb %t, stride %u, cpu %d\n", 
                                    domain_queue, req->time_control.raw, cpu_domain_tcb, req->prio_control.stride, get_current_cpu() );
	    
            dequeue_ready( cpu_domain_tcb );
            cpu_domain_tcb->sched_state.set_stride(req->prio_control.stride);
            enqueue_ready( cpu_domain_tcb );
	    
            
#if defined(CONFIG_SMP)
            prio_queue_t *queue = domain_queue->cpu_head;
            do 
            {
                ASSERT(queue);
                ASSERT(queue->get_domain_tcb());
                cpuid_t cpu = queue->get_domain_tcb()->get_cpu();
                
                if( cpu != get_current_cpu() )
                    xcpu_request( cpu, do_xcpu_domain_stride, queue->get_domain_tcb(), req->prio_control.stride );
                
                queue = queue->cpu_link;
            } while( queue != domain_queue->cpu_head );
#endif
        }
    }

}

/**
 * sends preemption IPC to the scheduler thread that the total quantum
 * has expired */
void hs_scheduler_t::total_quantum_expired(tcb_t * tcb)
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


tcb_t * hs_scheduler_t::parse_wakeup_queues(tcb_t * current)
{
    if (!wakeup_list)
        return current;

    tcb_t * highest_wakeup = current;
    tcb_t * tcb = wakeup_list;

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
		TRACEPOINT(SCHEDULE_WAKEUP_TIMEOUT, "removie bogus wakeup timeout wu=%p s=%s to=%ld time=%ld tmp=%t\n",
			   tcb,  tcb->get_state().string(), sched_state->get_timeout(), (word_t) current_time, tmp);
		
                continue;
            }

            TRACEPOINT(SCHEDULE_WAKEUP_TIMEOUT, "wakeup timeout wu=%t s=%s to=%ld time=%ld\n",
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

    return highest_wakeup;
}


/**
 * selects the next runnable thread in a round-robin fashion
 */
void hs_scheduler_t::end_of_timeslice (tcb_t * tcb)
{
    spin(74, get_current_cpu());
    ASSERT(tcb);  
    ASSERT(scheduled_tcb);
    ASSERT(tcb != get_idle_tcb()); // the idler never yields

    enqueue_ready (tcb, true);

    if( scheduled_tcb->is_local_cpu() )
    {
        sched_ktcb_t *sktcb = &scheduled_tcb->sched_state;
        sktcb->set_timeslice(current_timeslice + sktcb->get_timeslice_length());
        sktcb->account_pass();
    }

}

#if defined(CONFIG_SMP)
smp_requeue_t hs_scheduler_t::smp_requeue_lists[CONFIG_SMP_MAX_CPUS];

void hs_scheduler_t::smp_requeue(bool holdlock)
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

    prio_queue_t *new_prio_queue = tcb->sched_state.get_prio_queue()->domain_partner(cpu);
    tcb->sched_state.set_pass(0);  // Force tcb to use the pass of the target domain.
    tcb->sched_state.set_prio_queue(new_prio_queue);


    if (need_xcpu) 
    {
        tcb->sched_state.requeue_callback = xcpu_integrate_thread;
        remote_schedule(tcb);
    }
    
    tcb->unlock();

    TRACE_SCHEDULE_DETAILS("move_tcb: %t (s=%s) cpu %d pq %p dtcb %t", tcb, 
                           tcb->get_state().string(), cpu, new_prio_queue, 
                           new_prio_queue->get_domain_tcb());

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
    current_timeslice -= get_timer_tick_length();

    
    sched_ktcb_t *cstcb = &current->sched_state;
    sched_ktcb_t *sstcb = &scheduled_tcb->sched_state;
    
    /* Check for not infinite timeslice and expired */
    if (EXPECT_FALSE(current_timeslice <= 0) )
    {
        ASSERT(current->get_utcb());

        if( current->get_preempt_flags().is_delayed() &&
            cstcb->delay_preemption(wakeup) )
        {
            /* VU: should we give max_delay? */
            cstcb->set_timeslice(cstcb->get_timeslice() + cstcb->get_maximum_delay());
            cstcb->set_maximum_delay(0);
            current->set_preempt_flags(current->get_preempt_flags().set_pending() );
        }
        else
        {
            // We have end-of-timeslice.
            TRACEPOINT(TIMESLICE_EXPIRED, "timeslice expired for %t\n", current);

            if( EXPECT_FALSE(current->get_preempt_flags().is_delayed()) )
            {
                /* penalize thread for its delayed time slice */
                current_timeslice = current->sched_state.get_maximum_delay() - current->sched_state.get_init_maximum_delay();
                /* refresh max delay */
                current->sched_state.set_maximum_delay (current->sched_state.get_init_maximum_delay());
                current->set_preempt_flags( current->get_preempt_flags().clear_pending() );
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
        {	
            this->delayed_preemption_start = get_timestamp();
            return;
        }

        ASSERT(wakeup && wakeup->get_state().is_runnable());
        TRACE_SCHEDULE_DETAILS("wakeup preemption %t %t", current, wakeup);
	
	if (wakeup->sched_state.get_prio_queue() !=
	    current->sched_state.get_prio_queue())
	{
	    schedule();
	}
	else
	{
	    if( scheduled_tcb->is_local_cpu())
		sstcb->set_timeslice(current_timeslice);
	    
	    /* now switch to timeslice of dest */
	    schedule(wakeup, sched_dest);
	}
	return;
    }

    /* time slice expired */
    if (EXPECT_FALSE (cstcb->get_total_quantum()))
    {
        /* we have a total quantum - so do some book-keeping */
        if (sstcb->get_total_quantum() == TOTAL_QUANTUM_EXPIRED)
        {
            /* VU: must be revised. If a thread has an expired time quantum
             * and is activated with switch_to his timeslice will expire
             * and the event will be raised multiple times */
            total_quantum_expired (scheduled_tcb);
        }
        else if (sstcb->get_total_quantum() <= sstcb->get_timeslice_length())
        {
            /* we are getting close... */
            sstcb->set_timeslice(sstcb->get_timeslice() + sstcb->get_total_quantum());
            sstcb->set_total_quantum(TOTAL_QUANTUM_EXPIRED);
        }
        else
        {
            // account this time slice
            sstcb->account_quantum(sstcb->get_timeslice_length());
        }
    }

    /* schedule the next thread */
    enqueue_ready(current);
    schedule();
}

