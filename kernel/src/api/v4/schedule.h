/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     api/v4/schedule.h
 * Description:   scheduling declarations
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
 * $Id: schedule.h,v 1.24 2006/10/19 22:57:34 ud3 Exp $
 *                
 ********************************************************************/

#ifndef __API__V4__SCHEDULE_H__
#define __API__V4__SCHEDULE_H__

#include INC_API(tcb.h)
#include INC_GLUE(schedule.h)
#include <kdb/tracepoints.h>


EXTERN_TRACEPOINT(SCHEDULE_DETAILS);

class prio_queue_t 
{
public:
    void enqueue( tcb_t * tcb, bool head )
	{
	    ASSERT(tcb != get_idle_tcb());
    
	    if (tcb->queue_state.is_set(queue_state_t::ready))
		return;
    
	    ASSERT(tcb->priority >= 0 && tcb->priority <= MAX_PRIO);

	    if ( head )
		ENQUEUE_LIST_HEAD (prio_queue[tcb->priority], tcb, ready_list);
	    else
		ENQUEUE_LIST_TAIL (prio_queue[tcb->priority], tcb, ready_list);
    
	    tcb->queue_state.set(queue_state_t::ready);
	    max_prio = max(tcb->priority, max_prio);
	}

    void dequeue(tcb_t * tcb)
	{
	    if (!tcb->queue_state.is_set(queue_state_t::ready))
		return;
	    ASSERT(tcb->priority >= 0 && tcb->priority <= MAX_PRIO);
	    DEQUEUE_LIST(prio_queue[tcb->priority], tcb, ready_list);
	    tcb->queue_state.clear(queue_state_t::ready);
	}

    void set(prio_t prio, tcb_t * tcb)
	{
	    ASSERT(prio >= 0 && prio <= MAX_PRIO);
	    this->prio_queue[prio] = tcb;
	}

    tcb_t * get(prio_t prio)
	{
	    ASSERT(prio >= 0 && prio <= MAX_PRIO);
	    return prio_queue[prio];
	}

    void init ()
	{
	        /* prio queues */
	    for(int i = 0; i <= MAX_PRIO; i++)
		prio_queue[i] = (tcb_t *)NULL;
	    max_prio = -1;
	}

    prio_t max_prio;
private:
    tcb_t * prio_queue[MAX_PRIO + 1];
public:
    tcb_t * timeslice_tcb;
};



#ifdef CONFIG_SMP

class smp_requeue_t 
{
    tcb_t* tcb_list;
    char cache_pad0[CACHE_LINE_SIZE - sizeof(tcb_t*)];
public:
    spinlock_t lock;
    char cache_pad1[CACHE_LINE_SIZE - sizeof(spinlock_t)];
	
    volatile bool is_empty() { return (tcb_list == NULL); }
	
    void enqueue_head(tcb_t *tcb)
	{
	    ASSERT(lock.is_locked());
	    ASSERT(tcb);
	    
	    TRACE_SCHEDULE_DETAILS("smp_requeue:enqueue_head %t (s=%s) cpu %d (head %t)", 
	    	   tcb, tcb->get_state().string(), tcb->get_cpu(), tcb_list);
	    tcb->requeue = tcb_list;
	    tcb_list = tcb;
		

	}
    tcb_t *dequeue_head()
	{
	    ASSERT(lock.is_locked());
	    ASSERT(!is_empty());
 
	    tcb_t * tcb = tcb_list;
	    tcb_list = tcb->requeue;
	    tcb->requeue = NULL;
		
	    TRACE_SCHEDULE_DETAILS("smp_requeue:dequeue_head %t (s=%s) cpu %d (head %t)", 
	    	    tcb, tcb->get_state().string(), tcb->get_cpu(), tcb_list);

	    return (tcb_t *) tcb;
		
	}
	
};
#endif /* defined(CONFIG_SMP) */


class scheduler_t
{
public:
    /**
     * initializes the scheduler, must be called before init
     */
    void init( bool bootcpu = true );

    /**
     * starts the scheduling, does not return
     * @param cpu processor the scheduler starts on
     */
    void start(cpuid_t cpu = 0);

    /**
     * schedule a runnable thread
     * @param current the current thread control block
     * @return true if a runnable thread was found, false otherwise
     */
    bool schedule(tcb_t * current);

    /**
     * Marks end of timeslice
     * @param tcb       TCB of thread whose timeslice ends
     */
    void end_of_timeslice (tcb_t * tcb);

    /**
     * dispatches a thread 
     * @param tcb the thread control block of the thread to be dispatched
     */
    void dispatch_thread(tcb_t * tcb);

    /**
     * hierarchical scheduling prio queue 
     */
    prio_queue_t * get_prio_queue(tcb_t * tcb)
	{
	    return &this->root_prio_queue;
	}

    /**
     * delay preemption
     * @param current   current TCB
     * @param tcb       destination TCB
     * @return true if preemption was delayed, otherwise false
     */
    bool delay_preemption ( tcb_t * current, tcb_t * tcb );

    /**
     * scheduler-specific dispatch decision
     * @param current current thread control block
     * @param tcb tcb to be dispatched
     * @return true if tcb can be dispatched, false if a 
     * scheduling decission has to be made
     */
    bool check_dispatch_thread(tcb_t * current, tcb_t * tcb)
	{
	    if (EXPECT_FALSE( current->get_preempt_flags().is_delayed() &&
			      current->current_max_delay ))
		return !delay_preemption (current, tcb);
	    return (get_priority(current) < get_priority(tcb)); 
	}

    /**
     * scheduler preemption accounting remaining timeslice etc.
     */
    void preempt_thread(tcb_t * current, tcb_t * dest)
	{
	    ASSERT(current);
	    ASSERT(dest);
	    ASSERT(current->get_cpu() == dest->get_cpu());
	    ASSERT(current != get_idle_tcb()); // don't "preempt" idle

	    enqueue_ready(current, true);

	    /* now switch to timeslice of dest */
	    get_prio_queue(dest)->timeslice_tcb = dest;
	    
	}

    /**
     * sets the absolute timeout for a thread
     * @param tcb		pointer to the thread control block
     * @param absolute_time	time the timeout expires
     */
    void set_timeout(tcb_t * tcb, u64_t absolute_time)
    {
	ASSERT(tcb);
	/* a thread should not be in the wakeup queue */
	ASSERT( !tcb->queue_state.is_set(queue_state_t::wakeup) );

	tcb->absolute_timeout = absolute_time;
	ENQUEUE_LIST_TAIL(wakeup_list, tcb, wait_list);
	tcb->queue_state.set(queue_state_t::wakeup);
    }

    /**
     * sets a timeout for the thread specified in tcb
     * @param tcb	pointer to the thread control block
     * @param time	timeout value
     */
    void set_timeout(tcb_t * tcb, time_t time)
    {
	if (time.is_point())
	    UNIMPLEMENTED();
	set_timeout(tcb, current_time + time.get_microseconds());
    }

    /**
     * cancels a set timeout for tcb
     * @param tcb thread control block of the thread
     *
     * VU: DO NOT set the absolute timeout to zero!!! Used for 
     *     thread migration between processors
     */
    void cancel_timeout(tcb_t * tcb)
    {
	ASSERT(tcb);
	if (EXPECT_TRUE( !tcb->queue_state.is_set(queue_state_t::wakeup)) )
	    return;

	DEQUEUE_LIST(wakeup_list, tcb, wait_list);
	tcb->queue_state.clear(queue_state_t::wakeup);
    }

    /**
     * @return true if the timeout has expired
     */
    bool has_timeout_expired(tcb_t * tcb, u64_t time)
    {
	ASSERT(tcb);
	return (tcb->absolute_timeout <= time);
    }

    
    /**
     * Enqueues a TCB into the ready list
     *
     * @param tcb       thread control block to enqueue
     * @param head      Enqueue TCB at the head of the list when true
     */
    void enqueue_ready(tcb_t * tcb, bool head = false)
    {
	ASSERT(tcb);
	ASSERT(tcb->get_cpu() == get_current_cpu());
	get_prio_queue(tcb)->enqueue(tcb, head);
    }

    /**
     * dequeues tcb from the ready list (if the thread is in)
     * @param tcb thread control block
     */
    void dequeue_ready(tcb_t * tcb)
    {
	ASSERT(tcb);
	get_prio_queue(tcb)->dequeue(tcb);
    }

    /**
     * sets the total time quantum of the thread
     */
    void set_total_quantum(tcb_t * tcb, time_t quantum)
    {
	if (quantum.is_never())
	    tcb->total_quantum = 0;
	else
	{
	    // quantum can only be specified as a time period
	    ASSERT(quantum.is_period()); 
	    tcb->total_quantum = quantum.get_microseconds();
	}
	// give a fresh timeslice according to the spec
	tcb->current_timeslice = tcb->timeslice_length;
    }

    /**
     * sets the timeslice length of a thread 
     * @param tcb thread control block of the thread
     * @param timeslice timeslice length (must be a time period)
     */
    void set_timeslice_length(tcb_t * tcb, time_t timeslice)
    {
	ASSERT(timeslice.is_period()); 
	ASSERT(tcb);
	tcb->current_timeslice = tcb->timeslice_length = timeslice.get_microseconds();
    }

    /**
     * sets the priority of a thread
     * @param tcb thread control block
     * @param prio priority of thread
     */
    void set_priority(tcb_t * tcb, prio_t prio)
    {
	ASSERT(prio <= MAX_PRIO);
	ASSERT(!tcb->queue_state.is_set(queue_state_t::ready));
	tcb->priority = prio;

	/* keep sensitive and current prio in-sync to reduce checking overhead */
	if (tcb->sensitive_prio < prio)
	    set_sensitive_prio (tcb, prio);
    }

    /**
     * delivers the current priority of a thread
     * @param tcb thread control block of the thread
     * @return the priority 
     */
    static prio_t get_priority(tcb_t * tcb)
    { 
	return tcb->priority; 
    }

    /**
     * sets sensitive prio for delayed preemption
     */
    void set_sensitive_prio (tcb_t * tcb, prio_t prio)
    {
	ASSERT(prio <= MAX_PRIO);
	ASSERT(tcb);
	tcb->sensitive_prio = prio;
    }

    prio_t get_sensitive_prio (tcb_t * tcb)
    {
	ASSERT(tcb);
	return tcb->sensitive_prio;
    }

    /**
     * sets maximum delay for delayed preemption
     */
    void set_maximum_delay (tcb_t * tcb, u16_t usec)
    {
	ASSERT(tcb);
	tcb->current_max_delay = tcb->max_delay = usec;
    }
	    
    /**
     * initialize the scheduling parameters of a TCB
     * @param tcb		pointer to TCB
     * @param prio		priority of the thread
     * @param total_quantum	initial total quantum
     * @param timeslice_length	length of time slice
     */
    void init_tcb(tcb_t * tcb, prio_t prio = DEFAULT_PRIORITY, 
	time_t total_quantum = DEFAULT_TOTAL_QUANTUM,
	time_t timeslice_length = DEFAULT_TIMESLICE_LENGTH)
    {
	set_timeslice_length(tcb, timeslice_length);
	set_total_quantum(tcb, total_quantum);
	set_priority(tcb, prio);
	// defacto disable delayed preemptions
	set_sensitive_prio(tcb, prio);
	set_maximum_delay (tcb, 0);
    }
    
    /**
     * handles the timer interrupt event, walks the wait lists 
     * and makes a scheduling decission
     */
    void handle_timer_interrupt();

    /**
     * delivers the current time relative to the system 
     * startup in microseconds
     * @return absolute time 
     */
    u64_t get_current_time()
    {
	return current_time;
    }

    /**
     * function is called when the total quantum of a thread expires 
     * @param tcb thread control block of the thread whose quantum expired
     */
    void total_quantum_expired(tcb_t * tcb);
    
#ifdef CONFIG_SMP
    void remote_enqueue_ready(tcb_t * tcb);
    void smp_requeue(bool holdlock);
    void unlock_requeue() 
	{ smp_requeue_lists[get_current_cpu()].lock.unlock(); }

private:
    static smp_requeue_t smp_requeue_lists[CONFIG_SMP_MAX_CPUS];

#endif


private:
    /**
     * parse the wakeup queues
     * @param current current thread control block
     * @return woken up thread with the highest priority. If no thread has a 
     * higher priority than current, current will be returned
     */
    tcb_t * parse_wakeup_queues(tcb_t * current);

    /**
     * searches the run queue for the next runnable thread
     * @return next thread to be scheduled
     */
    tcb_t * find_next_thread(prio_queue_t * prio_queue);

private:
    tcb_t * wakeup_list;
    prio_queue_t root_prio_queue;
    static volatile u64_t current_time;
};





/* global function declarations */

/**
 * @return the current scheduler 
 * the default implementation features exactly one scheduler at a time.
 */
INLINE scheduler_t * get_current_scheduler()
{
    extern scheduler_t scheduler;
    return &scheduler;
}

#endif /*__API__V4__SCHEDULE_H__*/

