/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2010,  Karlsruhe University
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
#include INC_API(cpu.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(config.h)

#include <kdb/tracepoints.h>

/* global idle thread, we allocate a utcb to make accessing MRs etc easier */
whole_tcb_t __whole_idle_tcb UNIT("cpulocal") __attribute__((aligned(sizeof(whole_tcb_t))));
utcb_t	    __idle_utcb UNIT("cpulocal") __attribute__((aligned(sizeof(utcb_t))));
const tcb_t *__idle_tcb = (const tcb_t *) &__whole_idle_tcb;	    

/* global scheduler object */
scheduler_t scheduler UNIT("cpulocal");
schedule_request_queue_t scheduler_t::schedule_request_queue[CONFIG_SMP_MAX_CPUS];


DECLARE_TRACEPOINT(SYSCALL_THREAD_SWITCH);
DECLARE_TRACEPOINT(SYSCALL_SCHEDULE);
DECLARE_TRACEPOINT(SCHEDULE_IDLE);
DECLARE_TRACEPOINT_DETAIL(SCHEDULE_DETAILS);
EXTERN_TRACEPOINT(INTERRUPT_DETAILS);


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


void SECTION(".init") init_all_threads(void)
{
    init_interrupt_threads();
    init_kernel_threads();
    init_root_servers();

#if defined(CONFIG_KDB_ON_STARTUP)
    enter_kdebug ("System started (press 'g' to continue)");
#endif
}

#if defined(CONFIG_SMP)
void do_xcpu_send_irq(cpu_mb_entry_t * entry)
{
    tcb_t * handler_tcb = entry->tcb;
    word_t irq = entry->param[0];
    threadid_t irq_tid = threadid_t::irqthread(irq);
    tcb_t * irq_tcb = tcb_t::get_tcb(irq_tid);
	
    if (!handler_tcb->is_local_cpu())
    {
	TRACEF("xcpu-IRQ forward (%d->%t %d)", irq, handler_tcb, handler_tcb->get_cpu());
	enter_kdebug("Untested");
	xcpu_request( handler_tcb->get_cpu(), do_xcpu_send_irq, handler_tcb, irq);
	return;
    }

    if ((handler_tcb->get_state().is_waiting() || 
	 handler_tcb->get_state().is_locked_waiting()) &&  
	(handler_tcb->get_partner().is_anythread() ||
	 handler_tcb->get_partner() == threadid_t::irqthread(irq)))
    {
	// ok, thread is waiting -- deliver IRQ
	TRACE_IRQ_DETAILS("irq %d xcpu delivery (%d->%t)", irq, handler_tcb);
	handler_tcb->set_tag(msg_tag_t::irq_tag());
	handler_tcb->set_partner(threadid_t::irqthread(irq));
	handler_tcb->set_state(thread_state_t::running);
	get_current_scheduler()->schedule(handler_tcb, sched_current);
    }
    else
    {
	TRACEF("irq %d xcpu handler not ready %t s=%s", 
	       irq, handler_tcb, handler_tcb->get_state().string());
	enter_kdebug("UNTESTED");
	irq_tcb->set_tag(msg_tag_t::irq_tag());
	irq_tcb->set_partner(handler_tcb->get_global_id());	
	irq_tcb->set_state(thread_state_t::polling);
	handler_tcb->lock();
	irq_tcb->enqueue_send(handler_tcb);
	handler_tcb->unlock();
    }
}
#endif

static void idle_thread()
{
    get_current_scheduler()->set_accounted_tcb(get_current_tcb());
#if defined(CONFIG_X_EVT_LOGGING)
    get_idle_tcb()->sched_state.set_logid(IDLE_LOGID);
#endif
    get_current_scheduler()->idle();
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
	tcb_t * dest_tcb = tcb_t::get_tcb(dest);

	if ( dest_tcb == current )
	    return_thread_switch();

	if ( dest_tcb->get_state().is_runnable() &&
	     dest_tcb->myself_global == dest &&
	     dest_tcb->is_local_cpu() )
	{
	    scheduler->schedule(dest_tcb, sched_ds2_flag + rr_tsdonate_flag);
	    return_thread_switch();
	}
    }
    
    current->sched_state.sys_thread_switch();
    return_thread_switch();
}


#if defined(CONFIG_SMP)
static void do_xcpu_schedule(cpu_mb_entry_t * entry)
{
    get_current_scheduler()->process_schedule_requests();
}
#endif

word_t scheduler_t::add_schedule_request(schedule_req_t &req)
{
    ASSERT(req.tcb);
    
    cpuid_t cpu = get_current_cpu();
    cpuid_t reqcpu = req.tcb->get_cpu();
    schedule_req_t *qreq = NULL;


    if (req.tcb->flags.is_set(tcb_t::schedule_in_progress))
    {
	TRACE_SCHEDULE_DETAILS("schedule %t on cpu %d still in progress, skip\n", req.tcb, reqcpu, req.tcb);
	return EINVALID_PARAM;
    }
    
    do 
    {
	if ((qreq = schedule_request_queue[reqcpu].reserve_request()))
	    break;
	
	TRACE_SCHEDULE_DETAILS("schedule request queue cpu %d full, empty it\n", reqcpu);
	
	if (reqcpu == cpu)
	    process_schedule_requests();
#if defined(CONFIG_SMP)
	else 
	{
	    xcpu_request(reqcpu, do_xcpu_schedule);
	    while (schedule_requests_pending(reqcpu))
		; /* spin */
	}
#endif
	
	TRACE_SCHEDULE_DETAILS("schedule request queue cpu %d empty again, retry reservation\n", reqcpu);
	
    } while(!qreq);
    
    
    TRACE_SCHEDULE_DETAILS("add %t cpu %d to requeust queue on cpu %d", req.tcb, req.tcb->get_cpu(), reqcpu);
    qreq->tcb = req.tcb;
    qreq->tcb->flags += tcb_t::schedule_in_progress;
    qreq->prio_control = req.prio_control;
    qreq->time_control = req.time_control;
    qreq->preemption_control = req.preemption_control;
    qreq->processor_control = req.processor_control;
    qreq->valid = 1;	
	
    schedule_request_queue[reqcpu].commit_request();
    
    return EOK;

}


void scheduler_t::process_schedule_requests() 
{
    /* local part of schedule */

    cpuid_t cpu = get_current_cpu();
    
    TRACE_SCHEDULE_DETAILS("process schedule requests");
	

    while (!schedule_request_queue[cpu].is_empty())
    {
	schedule_req_t req = schedule_request_queue[cpu].process_request();
	
	if (!req.valid)
	    continue;	
	
	tcb_t *dest_tcb = req.tcb;

	TRACE_SCHEDULE_DETAILS("process_request: %t time=%x, prio=%x proc=%x, preempt=%x",
		   dest_tcb, req.time_control.get_raw(), req.prio_control.get_raw(), 
		   req.processor_control.get_raw(), req.preemption_control.get_raw());
	
	if (dest_tcb->get_cpu() != cpu)
	{
	    TRACEF(" wrong cpu %t cpu %d time=%x, prio=%x proc=%x, preempt=%x",
		   dest_tcb, dest_tcb->get_cpu(),
		   req.time_control.get_raw(), req.prio_control.get_raw(), 
		   req.processor_control.get_raw(), req.preemption_control.get_raw());
	    enter_kdebug("SCHEDULE BUG");
	}
	
	ASSERT(dest_tcb->get_cpu() == cpu);
	commit_schedule_parameters(req);
	dest_tcb->flags -= tcb_t::schedule_in_progress;
    }
	    
}

SYS_SCHEDULE (threadid_t dest_tid, word_t time_control, 
	      word_t processor_control, word_t prio_control,
	      word_t preemption_control )
{
    
    tcb_t * current = get_current_tcb();
    tcb_t * dest_tcb = tcb_t::get_tcb(dest_tid);
    scheduler_t *scheduler = get_current_scheduler();
    
    TRACEPOINT(SYSCALL_SCHEDULE, 
	       "SYS_SCHEDULE: curr=%t, dest=%t, time_ctrl=%x, "
	       "proc_ctrl=%x, prio_ctrl=%x, preemption_ctrl=%x\n",
	       current, TID(dest_tid), time_control, 
	       processor_control, prio_control, preemption_control);

    schedule_req_t req;
    req.tcb = dest_tcb;
    req.time_control = time_control;
    req.prio_control = prio_control;
    req.preemption_control = preemption_control;
    req.processor_control = processor_control; 

    word_t err, ret0 = 0 , ret1 = 0;
 
    if (dest_tid != threadid_t::nilthread())
    {
        // make sure the thread id is valid
	if (dest_tcb->get_global_id() != dest_tid)
	{
	    get_current_tcb ()->set_error_code (EINVALID_THREAD);
	    return_schedule(0, 0);
	}
    
	if (!scheduler->is_scheduler(current, dest_tcb))
	{
	    get_current_tcb ()->set_error_code (ENO_PRIVILEGE);
	    return_schedule(0, 0);
	}
    
	err = scheduler->check_schedule_parameters(current, req);
	if (err != EOK)
	{
	    get_current_tcb ()->set_error_code (err);
	    return_schedule (0, 0);
	}

        /* Calculate return values before operation */
        ret0 = scheduler->return_schedule_parameter(0, req);
        ret1 = scheduler->return_schedule_parameter(1, req); 
       
	err = scheduler->add_schedule_request(req);
	if (err != EOK)
	{
	    get_current_tcb ()->set_error_code (err);
	    return_schedule (0, 0);
	}
    
    }

    
    // Process our own requests
    scheduler->process_schedule_requests();

#if defined(CONFIG_SMP)
    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
    {
	if (scheduler->schedule_requests_pending(cpu))
	    xcpu_request(cpu, do_xcpu_schedule);
    }
#endif

    get_current_tcb ()->set_error_code (EOK);
    return_schedule(ret0, ret1);
}

/**********************************************************************
 *
 *                     Initialization
 *
 **********************************************************************/

void SECTION(".init") scheduler_t::start(cpuid_t cpuid)
{
    TRACE_INIT ("\tSwitching to idle thread (CPU %d)\n", cpuid);
    get_idle_tcb()->set_cpu(cpuid);

    initial_switch_to(get_idle_tcb());
}

typedef void (*func_ptr_t) (void);

void SECTION(".init") scheduler_t::init( bool bootcpu )
{
    
    TRACE_INIT ("\tInitializing threading (CPU %d)\n", get_current_cpu());
    policy_scheduler_init();
    

    /* set idle-magic */
    get_idle_tcb()->create_kernel_thread(NILTHREAD, &__idle_utcb, sktcb_lo);
    get_idle_tcb()->set_space(get_kernel_space());
    get_idle_tcb()->myself_global.set(IDLETHREAD);
    get_idle_tcb()->create_startup_stack(idle_thread);
    
    if( bootcpu )
    	get_idle_tcb()->notify(init_all_threads);
    
    
    return;
}
