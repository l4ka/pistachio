/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-rr/schedule_functions.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_RR__SCHEDULE_FUNCTIONS_H__
#define __API__V4__SCHED_RR__SCHEDULE_FUNCTIONS_H_

#include <tcb_layout.h>

EXTERN_TRACEPOINT(SCHEDULE_PM_DELAYED);
EXTERN_TRACEPOINT(SCHEDULE_PM_DELAY_REFRESH);
EXTERN_TRACEPOINT(SCHEDULE_IDLE);

INLINE bool rr_sched_ktcb_t::delay_preemption(tcb_t *tcb)
{	
    bool ret; 
	    
    // we always allow ourself to delay our preemption
    if (addr_to_tcb(this) == tcb)
	ret = true;
    else if ( sensitive_prio < tcb->sched_state.get_priority() )
	ret = false;
    else 
	ret = (current_max_delay > 0);

    TRACEPOINT(SCHEDULE_PM_DELAYED, "%ssuccesful delayed preemption dest %t prio %d, sens_prio %d time=%dus\n", 
	       (ret ? "un" : ""), tcb, tcb->sched_state.get_priority(), sensitive_prio, current_max_delay);
    
    return ret;
}

INLINE void sched_ktcb_t::init(sktcb_type_e type)
{
    init_timeslice(DEFAULT_TIMESLICE_LENGTH);
    init_total_quantum(DEFAULT_TOTAL_QUANTUM);
            
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
    }
    // defacto disable delayed preemptions
    set_sensitive_prio(priority);
    set_maximum_delay (0);
#if defined(CONFIG_SMP)
    requeue = NULL;
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
    }
    
    /* eat up timeslice - we get a fresh one */
    sched_ktcb_t *tsched_state = &scheduler->get_accounted_tcb()->sched_state;
    tsched_state->account_timeslice(tsched_state->get_timeslice());
    scheduler->end_of_timeslice (addr_to_tcb(this));
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
    printf("abs timeout:    %wdus, rel timeout:       %wdus\n",
           (word_t)absolute_timeout,
           absolute_timeout == 0 ? 0 :
           (word_t)(absolute_timeout -  current_time));
    printf("sens prio: %d, delay: max=%dus, curr=%dus\n",
           sensitive_prio, max_delay, current_max_delay);

}

#endif

  
INLINE u64_t scheduler_t::get_current_time() 
{ 
    return current_time; 
}

INLINE void scheduler_t::set_accounted_tcb(tcb_t *tcb) 
{ 
    get_prio_queue()->set_timeslice_tcb(tcb);
}


INLINE tcb_t *scheduler_t::get_accounted_tcb() 
{ 
    return get_prio_queue()->get_timeslice_tcb();
}



INLINE bool scheduler_t::schedule()
{
    tcb_t * tcb = find_next_thread ();
    tcb_t *current = get_current_tcb();
    
    ASSERT(tcb);
    ASSERT(current);

    // the newly selected thread gets accounted
    set_accounted_tcb(tcb);
    
    // do not switch to ourself
    if (tcb == current)
	return false;

    if (current != get_idle_tcb())
	enqueue_ready(current);

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
	
	// during IPC, perform TS donation and lazy destination queueing
	if (!FLAG_IS_SET(flags, rr_tsdonate_flag))
	    set_accounted_tcb(dest);

	// make sure we are in the ready queue 
	if (FLAG_IS_SET(flags, sched_c2r_flag) && current != get_idle_tcb())
	    enqueue_ready(current, true);
	
	current->switch_to (dest); 
	return true;
    }
    else
    {
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
	(FLAG_IS_SET(flags, sched_chk_flag) && check_dispatch_thread ( dest1, dest2 )) )
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
     
    // during IPC, perform TS donation 
    if (!FLAG_IS_SET(flags, rr_tsdonate_flag))
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
    if (req.prio_control != schedule_ctrl_t::nilctrl() &&
	req.prio_control.prio > scheduler->sched_state.get_priority() &&
	!is_privileged_space(scheduler->get_space()))
	return ENO_PRIVILEGE;
	
    if (req.time_control != schedule_ctrl_t::nilctrl() &&
	(!req.time_control.total_quantum.is_period() || 
	 !req.time_control.timeslice.is_period()))
	return EINVALID_THREAD;
    
    /* only set sensitive prio if 
     *   _at most_  equal to the scheduler's prio 
     */
    if (req.preemption_control != schedule_ctrl_t::nilctrl() &&
	(prio_t) req.prio_control.sensitive_prio > scheduler->sched_state.get_priority())
	return ENO_PRIVILEGE;
    
    return EOK;

}


INLINE void scheduler_t::commit_schedule_parameters(schedule_req_t &req)
{
    if (req.prio_control != schedule_ctrl_t::nilctrl())
    {
        deschedule (req.tcb);
       
	if ((word_t) req.prio_control.prio <= MAX_PRIORITY &&
            req.prio_control.prio != req.tcb->sched_state.get_priority())
            req.tcb->sched_state.set_priority((prio_t) req.prio_control.prio);	
#if defined(CONFIG_X_EVT_LOGGING)
	if ((word_t) req.prio_control.logid > 0 &&
            (word_t) req.prio_control.logid < MAX_LOGIDS)
	    req.tcb->sched_state.set_logid(req.prio_control.logid);
#endif
      
        schedule(req.tcb, sched_current);
    }	
    
    if (req.preemption_control != schedule_ctrl_t::nilctrl())
    {
	req.tcb->sched_state.init_maximum_delay (req.preemption_control.max_delay);
	    
	/* only set sensitive prio if 
	 *   _at most_ equal to current prio 
	 */
	if ( (prio_t) req.preemption_control.sensitive_prio >=  req.tcb->sched_state.get_priority() )
	    req.tcb->sched_state.set_sensitive_prio(req.preemption_control.sensitive_prio);
	    
    }

    if (req.processor_control != schedule_ctrl_t::nilctrl())
	req.tcb->migrate_to_processor(req.processor_control.processor);
	
    if (req.time_control != schedule_ctrl_t::nilctrl())
    {
	req.tcb->sched_state.init_timeslice (req.time_control.timeslice);
	req.tcb->sched_state.set_total_quantum (req.time_control.total_quantum);
    }

}

INLINE word_t scheduler_t::return_schedule_parameter(word_t num, schedule_req_t &req)
{
    if (!req.tcb) return 0;
    
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
                ({ WARNING("invalid state  tcb %t (%x)\n", req.tcb, (word_t)state); 0;}));
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
    return false;
}



#endif /* !__API__V4__SCHED_RR__SCHEDULE_FUNCTIONS_H__ */
