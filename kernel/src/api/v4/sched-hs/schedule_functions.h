/*********************************************************************
 *                
 * Copyright (C) 2007-2010, 2012,  Karlsruhe University
 *                
 * File path:     api/v4/sched-hs/schedule_functions.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_HS__SCHEDULE_FUNCTIONS_H__
#define __API__V4__SCHED_HS__SCHEDULE_FUNCTIONS_H_

EXTERN_TRACEPOINT(SCHEDULE_PM_DELAYED);
EXTERN_TRACEPOINT(SCHEDULE_PM_DELAY_REFRESH);
EXTERN_TRACEPOINT(SCHEDULE_IDLE);

INLINE void hs_sched_ktcb_t::account_pass() 
{
    tcb_t *tcb = addr_to_tcb(this);
    while (tcb->get_global_id() !=  IDLETHREAD )
    {
	ASSERT(tcb->is_local_cpu());
        sched_ktcb_t *sktcb = &tcb->sched_state; 

        TRACEPOINT (SCHEDULE_DETAILS, "account pass %t %llu += %d\n",
                  tcb, sktcb->get_pass(), sktcb->get_stride());
        
	sktcb->set_pass(sktcb->get_pass() + sktcb->get_stride());

	// Account for the parent queue too.
	tcb = sktcb->get_prio_queue()->get_domain_tcb();
    }
}


INLINE void hs_sched_ktcb_t::set_prio_queue( prio_queue_t *q )
{
    // TODO: delete priority subdomains when they become empty. 
    prio_queue = q;

    if( flags.is_set(is_schedule_domain))
	get_domain_prio_queue()->set_depth(prio_queue);
   
}

INLINE void hs_sched_ktcb_t::migrate_prio_queue(prio_queue_t *nq)
{
    tcb_t *tcb = addr_to_tcb(this);
    scheduler_t *scheduler = get_current_scheduler();
    
    ASSERT(tcb->get_cpu() == get_current_cpu());
    ASSERT(nq->get_domain_tcb()->get_cpu() == get_current_cpu());

    scheduler->dequeue_ready(tcb);
    set_prio_queue(nq);
    if( tcb->get_state().is_runnable() )
        scheduler->enqueue_ready(tcb);
}


INLINE bool hs_sched_ktcb_t::delay_preemption(tcb_t *dtcb)
{	
    tcb_t *stcb = addr_to_tcb(this);
    
    if (stcb == dtcb)
	return true;
    
    hs_sched_ktcb_t *ssktcb = &stcb->sched_state;
    hs_sched_ktcb_t *dsktcb = &dtcb->sched_state;

    prio_queue_t *sprio_queue = ssktcb->get_prio_queue();
    prio_queue_t *dprio_queue = dsktcb->get_prio_queue();
            
    while (sprio_queue != dprio_queue)
    {
        ASSERT(sprio_queue && dprio_queue);
        ASSERT(sprio_queue->get_domain_tcb() && dprio_queue->get_domain_tcb());
                
        if (sprio_queue->get_depth() >= dprio_queue->get_depth())
        {
            ssktcb = &sprio_queue->get_domain_tcb()->sched_state;
            sprio_queue = ssktcb->get_prio_queue();
        }
        if (sprio_queue->get_depth() <= dprio_queue->get_depth())
        {
            dsktcb = &dprio_queue->get_domain_tcb()->sched_state;
            dprio_queue = dsktcb->get_prio_queue();
        }
                
    }
            
    TRACEPOINT (SCHEDULE_DETAILS, "dpm %t (d %t prio %x pass %d) - %t (d %t prio %x pass %d)\n",
                stcb, addr_to_tcb(ssktcb), ssktcb->get_priority(), (word_t) ssktcb->get_pass(),
                dtcb, addr_to_tcb(dsktcb), dsktcb->get_priority(), (word_t) dsktcb->get_pass());

    bool ret; 

    if ( ssktcb->sensitive_prio < dsktcb->priority )
        ret = false;
    else 
        ret = (current_max_delay > 0);
    
    TRACEPOINT (SCHEDULE_DETAILS, "dpm %ssuccessful %t (d %t send prio %x delay %dus) - %t (d %t prio %x)\n",
                (ret ? "un" : ""),
                stcb, addr_to_tcb(ssktcb), ssktcb->sensitive_prio, current_max_delay,
                dtcb, addr_to_tcb(dsktcb), dsktcb->get_priority());

    return ret;
}

INLINE void sched_ktcb_t::init(sktcb_type_e type)
{
    init_timeslice(DEFAULT_TIMESLICE_LENGTH);
    init_total_quantum(DEFAULT_TOTAL_QUANTUM);
    prio_queue = get_current_scheduler()->get_prio_queue();
    
   switch (type)
    {
    case sktcb_user:
        set_priority(DEFAULT_PRIORITY);
        break;
    case sktcb_root:
        set_priority(ROOT_PRIORITY);
        break;
    case sktcb_irq:
    case sktcb_hi:
        set_priority(MAX_PRIORITY);
        break;
    case sktcb_lo:
        set_priority(0);
        break;
    default: 
        ASSERT(false);
    }
    // defacto disable delayed preemptions
    set_sensitive_prio(priority);
    set_maximum_delay (0);
    set_stride(DEFAULT_STRIDE);
    
#if defined(CONFIG_SMP)
    requeue = NULL;
#endif
#if defined(CONFIG_X_EVT_LOGGING)
    /* set domain */
    set_logid(type == sktcb_root ? ROOTSERVER_LOGID : IDLE_LOGID);
#endif
}


INLINE void sched_ktcb_t::set_scheduler(const threadid_t tid)
{ 
    scheduler = tid;  
}

INLINE void sched_ktcb_t::set_timeout(u64_t absolute_time, const bool enqueue)
{
    ASSERT(this);
    /* a thread should not be in the wakeup queue */
    absolute_timeout = absolute_time;
    
    if (enqueue)
	get_current_scheduler()->enqueue_timeout(addr_to_tcb(this));
}

INLINE void sched_ktcb_t::set_timeout(time_t time)
    {
	if (time.is_point())
	    UNIMPLEMENTED();
	set_timeout(get_current_scheduler()->get_current_time() + time.get_microseconds());
    }


INLINE void sched_ktcb_t::cancel_timeout()
{
    if (EXPECT_TRUE( !addr_to_tcb(this)->queue_state.is_set(queue_state_t::wakeup)) )
	return;
	
    get_current_scheduler()->dequeue_timeout(addr_to_tcb(this));
}

INLINE void sched_ktcb_t::sys_thread_switch()
{
    scheduler_t *scheduler = get_current_scheduler();
    /* user cooperatively preempts */
    if (get_maximum_delay() < get_init_maximum_delay() )
    {
        addr_to_tcb(this)->set_preempt_flags( addr_to_tcb(this)->get_preempt_flags().clear_pending() );
	/* refresh max delay */
	set_maximum_delay(get_init_maximum_delay());
	TRACEPOINT(SCHEDULE_PM_DELAY_REFRESH, "delayed preemption refresh for %t\n", addr_to_tcb(this));
        
        tcb_t *atcb = get_current_scheduler()->get_accounted_tcb();
        if( atcb->is_local_cpu())
        {
            word_t delta = get_timestamp() - scheduler->delayed_preemption_start;
            if( delta > atcb->sched_state.max_delay )
            {
                delta = atcb->sched_state.max_delay;
                TRACEF("sched-hs: large delay penalty\n");
            }

            atcb->sched_state.delay_penalty += delta;
            if( atcb->sched_state.delay_penalty > get_timer_tick_length() )
            {
                atcb->sched_state.delay_penalty -= get_timer_tick_length();
                scheduler->current_timeslice = -get_timer_tick_length();
            }

        }

    }
    
    /* eat up timeslice - we get a fresh one */
    scheduler->current_timeslice = 0;
    scheduler->schedule();

}

INLINE void sched_ktcb_t::delete_tcb()
{
#if defined(CONFIG_SMP)
    if (requeue) 
	get_current_scheduler()->smp_requeue(false);
#endif
    
}

   
#if defined(CONFIG_DEBUG)
INLINE void sched_ktcb_t::dump_priority() 
{ 
    printf("=== PRIO: %2d ===", priority); 
#if defined(CONFIG_X_EVT_LOGGING)
    printf("= L: %2d =", logid);
#endif
}

INLINE void sched_ktcb_t::dump_list1() 
{ 
    printf("wait : %wt:%-wt   ", wait_list.next, wait_list.prev); 
}

INLINE void sched_ktcb_t::dump_list2() 
{ 
    printf("ready: %wt:%-wt   ", ready_list.next, ready_list.prev); 
}

INLINE void sched_ktcb_t::dump(u64_t current_time)
{
    printf("total quant:    %wdus, ts length  :       %wdus, curr ts: %wdus\n",
           (word_t)total_quantum, (word_t)timeslice_length,
           (word_t)current_timeslice);
    printf("abs timeout:    %wdus, rel timeout:       %wdus, prio_queue %p [%c]\n",
           (word_t)absolute_timeout, absolute_timeout == 0 ? 0 : (word_t)(absolute_timeout -  current_time), 
           prio_queue, flags.is_set(is_schedule_domain) ? 'D' : 'd');
    printf("sens prio: %d, delay: max=%dus, curr=%dus, ",
           sensitive_prio, max_delay, current_max_delay);
    printf("stride : %wd  pass: %wd\n", stride, (word_t) pass, prio_queue);
}

#endif


INLINE u64_t scheduler_t::get_current_time() 
{ 
    return current_time; 
}


INLINE void scheduler_t::set_accounted_tcb(tcb_t *tcb) 
{ 
    ASSERT(tcb && tcb->is_local_cpu());
        
    if (scheduled_tcb == tcb)
        return;

    u64_t now = get_cpu_cycles();
    
    scheduled_queue->end_timeslice(now);
    scheduled_tcb->sched_state.set_timeslice(current_timeslice);
                
    if( tcb->sched_state.get_init_maximum_delay() < tcb->sched_state.get_maximum_delay())
    {
        word_t delta = get_timestamp() - delayed_preemption_start;
        if( delta > scheduled_tcb->sched_state.get_maximum_delay())
        {
            delta = scheduled_tcb->sched_state.get_maximum_delay();
            TRACEF( "blocked thread long delay\n" );
        }
        scheduled_tcb->sched_state.add_delay_penalty(delta);
    }
   
    scheduled_tcb = tcb;
    scheduled_queue = tcb->sched_state.get_prio_queue();
    scheduled_queue->start_timeslice(now);
    
    TRACEPOINT (SCHEDULE_DETAILS, "sat %t (q %t pass %llu now %llu))\n",
                scheduled_tcb, scheduled_queue, scheduled_tcb->sched_state.get_pass(), now);

}


INLINE tcb_t *scheduler_t::get_accounted_tcb() 
{ 
    return scheduled_tcb;
}

    
/**
 * selects the next runnable thread and activates it.
 * @return true if a runnable thread was found, false otherwise
 */
INLINE bool scheduler_t::schedule()
{
    tcb_t * tcb = find_next_thread (&root_prio_queue);
    tcb_t *current = get_current_tcb();
    
    ASSERT(tcb);
    ASSERT(current);

    
    // do not switch to ourself
    if (tcb == current)
	return false;

    if (current != get_idle_tcb())
	enqueue_ready(current);

    // the newly selected thread gets accounted
    set_accounted_tcb(tcb);
    current->switch_to(tcb);

    return true;
}


INLINE bool scheduler_t::schedule(tcb_t *dest, const sched_flags_t flags)
{
    tcb_t *current = get_current_tcb();
    
    TRACE_SCHEDULE_DETAILS("schedule %t (%s) flags [%C]\n", 
			   dest, dest->get_state().string(), flags_stringword(flags));
    
    ASSERT(current->get_cpu() == dest->get_cpu());
    ASSERT(FLAG_IS_SET(flags, sched_chk_flag) || 
	   FLAG_IS_SET(flags, sched_ds2_flag) || 
	   FLAG_IS_SET(flags, sched_ds1_flag));

    if (FLAG_IS_SET(flags, sched_ds2_flag) || 
	(FLAG_IS_SET(flags, sched_chk_flag) && check_dispatch_thread(current, dest)))
    {
	ASSERT(current != dest);
        set_accounted_tcb(dest);

	// make sure we are in the ready queue 
	if (FLAG_IS_SET(flags, sched_c2r_flag) && current != get_idle_tcb())
	    enqueue_ready(current, true);

        
	current->switch_to (dest); 
	return true;
    }
    else
    {
        ASSERT(dest);
	/* according to the scheduler should the current
	 * thread remain active so simply activate the other
	 * guy and return */
	if (dest != get_idle_tcb())
	    enqueue_ready(dest);
	
	if (FLAG_IS_SET(flags, sched_timeout_flag))
	    dest->sched_state.cancel_timeout();
	
	return false;
    }
    
}

INLINE bool scheduler_t::schedule(tcb_t *dest1, tcb_t *dest2, const sched_flags_t flags)
{
    tcb_t *current = get_current_tcb();
    tcb_t *dest;
    bool  ret;
    
    ASSERT(FLAG_IS_SET(flags, sched_chk_flag) || 
	   FLAG_IS_SET(flags, sched_ds1_flag) || 
	   FLAG_IS_SET(flags, sched_ds2_flag));
    ASSERT(current != dest1 && current != dest2 && dest1 != dest2);
    ASSERT(current->get_cpu() == dest1->get_cpu());
    
    TRACE_SCHEDULE_DETAILS("schedule %t (%s) or %t (%s) flags [%C]\n", 
			   dest1, dest1->get_state().string(), 
			   dest2, dest2->get_state().string(), 
			   flags_stringword(flags));
    
    if (FLAG_IS_SET(flags, sched_ds2_flag) || 
	(FLAG_IS_SET(flags, sched_chk_flag) && check_dispatch_thread(dest1, dest2)))
    {
	if (FLAG_IS_SET(flags, sched_timeout_flag))
	    dest1->sched_state.cancel_timeout();
	enqueue_ready(dest1);
	dest = dest2;
	ret = false;
    }
    else
    {
	enqueue_ready(dest2);
	dest = dest1;
	ret = true;
    }
     
    set_accounted_tcb(dest);
    
    // make sure we are in the ready queue 
    if (FLAG_IS_SET(flags, sched_c2r_flag) && current != get_idle_tcb())
	enqueue_ready(current, true);
    
    current->switch_to (dest); 
    return ret;

    
}

INLINE bool scheduler_t::schedule_interrupt(tcb_t *irq, tcb_t *handler)
{
    threadid_t irq_tid = irq->get_global_id();
    irq->set_tag(msg_tag_t::irq_tag());
    irq->set_partner(handler->get_global_id());
    irq->set_state(thread_state_t::polling);
    handler->lock();
    irq->enqueue_send(handler);
    handler->unlock();
    return true;
}


INLINE void scheduler_t::deschedule(tcb_t *tcb)
{
    dequeue_ready(tcb);
}


INLINE bool scheduler_t::is_scheduler(tcb_t *tcb, tcb_t *dest_tcb)
{
    ASSERT(tcb->exists());
    ASSERT(dest_tcb);
    
    if (is_privileged_space(tcb->get_space()))
	return true;

    // are we in the same address space as the scheduler of the thread?
    threadid_t scheduler_tid = dest_tcb->sched_state.get_scheduler();
    tcb_t * scheduler_tcb = tcb_t::get_tcb(scheduler_tid);
    
    if (tcb->get_global_id() != scheduler_tid || 
	(tcb->get_space()    != scheduler_tcb->get_space()))
	return false;
    
    return true;
}



INLINE word_t scheduler_t::check_schedule_parameters(tcb_t *scheduler, schedule_req_t &req)
{

    
    /* only set sensitive prio if 
     *   _at most_  equal to the scheduler's prio 
     */
    if (req.preemption_control != schedule_ctrl_t::nilctrl())
    {

        /* Extended HS schedule control
         * control &  1 -> new domain
         * control &  2 -> migrate domain
         * control &  4 -> retrieve tickets of dest
         * control &  8 -> reset period cycles of current queue and retrieve utilization
         * control & 16 -> set stride 
         */

        if (req.preemption_control.hs_extended)
        {
            /* must be privileged */
            if (!is_privileged_space(scheduler->get_space()))
                return ENO_PRIVILEGE;
            
            if ((req.preemption_control.hs_extended_ctrl & 0xc) == req.preemption_control.hs_extended_ctrl)
                return EOK;
        
            tcb_t *domain_tcb = tcb_t::get_tcb(req.time_control.tid);
        
            /* Target and domain must be different. */

            if( domain_tcb->get_global_id() != req.time_control.tid ||
                domain_tcb->sched_state.get_prio_queue() == NULL)
                return EINVALID_THREAD;

            prio_queue_t *domain_queue = domain_tcb->sched_state.get_prio_queue();
            domain_queue = domain_queue->domain_partner(get_current_cpu());
        
            if( !domain_queue )
                return EINVALID_PARAM;

            TRACE_SCHEDULE_DETAILS( "control %x, domain tid: %t queue %p, first dest tid: %t, prioctrl %x\n",
                                    req.preemption_control.hs_extended_ctrl, 
                                    domain_tcb, domain_queue, req.tcb, req.prio_control.raw );
            
            if ((req.preemption_control.hs_extended_ctrl & 0x3) &&
                domain_tcb == req.tcb)
                return EINVALID_THREAD;
            
            if (( req.preemption_control.hs_extended_ctrl & 0x10) &&
                !domain_queue->get_domain_tcb())
                    return EINVALID_PARAM;
        
            req.time_control.raw = (word_t) domain_tcb;
            req.processor_control.raw = (word_t) domain_queue;
        
            return EOK;
        }
        
        if ((prio_t) req.prio_control.sensitive_prio > scheduler->sched_state.get_priority())
            return ENO_PRIVILEGE;

    }

    if (req.prio_control != schedule_ctrl_t::nilctrl() &&
	req.prio_control.prio > scheduler->sched_state.get_priority() &&
	!is_privileged_space(scheduler->get_space()))
	return ENO_PRIVILEGE;
	
    if (req.time_control != schedule_ctrl_t::nilctrl() &&
	(!req.time_control.total_quantum.is_period() || 
	 !req.time_control.timeslice.is_period()))
	return EINVALID_THREAD;
    

    if (req.processor_control != schedule_ctrl_t::nilctrl())
    {
        /* Can't move domain tcbs */
        if (req.tcb->sched_state.flags.is_set(sched_ktcb_t::is_schedule_domain))
            return EINVALID_THREAD;
    }

    return EOK;

}


INLINE void scheduler_t::commit_schedule_parameters(schedule_req_t &req)
{
    if (req.preemption_control != schedule_ctrl_t::nilctrl())
    {
        if (req.preemption_control.hs_extended)
        {
            hs_extended_schedule(&req);
            return;
        }

	req.tcb->sched_state.init_maximum_delay (req.preemption_control.max_delay);
	    
	/* only set sensitive prio if 
	 *   _at most_ equal to current prio 
	 */
	if ( (prio_t) req.preemption_control.sensitive_prio >=  req.tcb->sched_state.get_priority() )
	    req.tcb->sched_state.set_sensitive_prio(req.preemption_control.sensitive_prio);
        
    }
    
    if (req.prio_control != schedule_ctrl_t::nilctrl())
    {
        deschedule (req.tcb);
	
	if ((word_t) req.prio_control.prio <= MAX_PRIORITY &&
            (word_t) req.prio_control.prio != req.tcb->sched_state.get_priority())
            req.tcb->sched_state.set_priority(req.prio_control.prio);	
#if defined(CONFIG_X_EVT_LOGGING)
	if ((word_t) req.prio_control.logid > 0 &&
            (word_t) req.prio_control.logid < MAX_LOGIDS)
	    req.tcb->sched_state.set_logid(req.prio_control.logid);
#endif
        if (req.prio_control.stride > 0)
	    req.tcb->sched_state.set_stride(req.prio_control.stride);	

        schedule(req.tcb, sched_current);
                    
    }	
    
    if (req.processor_control != schedule_ctrl_t::nilctrl())
	req.tcb->migrate_to_processor(req.processor_control.processor);
	
    if (req.time_control != schedule_ctrl_t::nilctrl())
    {
	req.tcb->sched_state.init_timeslice (req.time_control.timeslice);
	req.tcb->sched_state.set_total_quantum (req.time_control.total_quantum.get_microseconds());
    }

}

INLINE word_t scheduler_t::return_schedule_parameter(word_t num, schedule_req_t &req)
{
    if (!req.tcb) return 0;

    if (req.preemption_control != schedule_ctrl_t::nilctrl()
        && req.preemption_control.hs_extended)
    {
        if( req.preemption_control.hs_extended_ctrl & 4 )
        {
            prio_queue_t *queue = req.tcb->sched_state.get_prio_queue();
            if( queue != &root_prio_queue )
            {
                // We want to look into the priority queue which contains the
                // domain of the current thread.
                req.tcb = queue->get_domain_tcb();
                queue = req.tcb->sched_state.get_prio_queue();
            }

            word_t current_tickets, tickets;
            tickets = queue->prio_tickets( req.tcb, &current_tickets );
        
            return (num == 0) ? tickets : current_tickets;
        }
        
        if( req.preemption_control.hs_extended_ctrl & 8 )
        {
            prio_queue_t *queue = scheduled_queue;
            period_cycles_t period_cycles = queue->get_period_cycles();
            
            ASSERT(queue->get_domain_tcb());
#if defined(CONFIG_SMP)
            queue = queue->cpu_head;
            do {
                if( queue->get_domain_tcb()->get_cpu() != get_current_cpu() )
                    period_cycles += queue->get_period_cycles();
                queue = queue->cpu_link;
            } while( queue != queue->cpu_head );
#endif /* defined(CONFIG_SMP)*/

            if (num == 0)
                return (word_t) (period_cycles * 1000 / queue->get_poll_window());
            else 
                return period_cycles;
        }
        
        return 1;
    }


    if (num == 0)
    {
        thread_state_t state = req.tcb->get_state();
        return (state == thread_state_t::aborted	? 1 : 
                state.is_halted()			? 2 :
                state.is_running()			? 3 :
                state.is_polling()			? 4 :
                state.is_sending()			? 5 :
                state.is_waiting()			? 6 :
                state.is_receiving()                    ? 7 : 
                state.is_xcpu_waiting()                 ? 6 :
                ({ WARNING("invalid state (%x)\n", (word_t)state); 0;}));
    }
    else 
    {
        word_t rem_ts = req.tcb->sched_state.get_timeslice();
        word_t rem_tq = req.tcb->sched_state.get_total_quantum();
        
        return (rem_ts << 16) | (rem_tq & 0xffff);
    }
}

INLINE bool scheduler_t::idle_hlt()
{
    TRACEPOINT(SCHEDULE_IDLE, "idle loop by user hlt");
    if ( is_privileged_space(get_current_tcb()->get_space()))
    {
        processor_sleep();
        return true;
    }
    return false;
}


#endif /* !__API__V4__SCHED_HS__SCHEDULE_FUNCTIONS_H__ */
