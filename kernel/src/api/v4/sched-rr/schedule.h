/*********************************************************************
 *                
 * Copyright (C) 2007-2009,  Karlsruhe University
 *                
 * File path:     api/v4/sched-rr/schedule.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_RR__SCHEDULE_H__
#define __API__V4__SCHED_RR__SCHEDULE_H__


class prio_queue_t
{
public:
    void enqueue( tcb_t * tcb, bool head )
	{
	    ASSERT(tcb);
	    ASSERT(tcb != get_idle_tcb());
    
	    if (tcb->queue_state.is_set(queue_state_t::ready))
		return;
    
	    sched_ktcb_t *sktcb = &tcb->sched_state;

	    if ( head )
		ENQUEUE_LIST_HEAD (prio_queue[sktcb->get_priority()], tcb, sched_state.ready_list);
	    else
		ENQUEUE_LIST_TAIL (prio_queue[sktcb->get_priority()], tcb, sched_state.ready_list);
            
	    tcb->queue_state.set(queue_state_t::ready);
	    max_prio = max((s16_t) sktcb->get_priority(), max_prio);

	}

    void dequeue(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    ASSERT(tcb != get_idle_tcb());
	    if (!tcb->queue_state.is_set(queue_state_t::ready))
		return;
	    
	    sched_ktcb_t *sktcb = &tcb->sched_state;
	    DEQUEUE_LIST(prio_queue[sktcb->get_priority()], tcb, sched_state.ready_list);
	    tcb->queue_state.clear(queue_state_t::ready);
	}

    void set(prio_t prio, tcb_t * tcb)
	{
	    this->prio_queue[prio] = tcb;
	}

  
    tcb_t * get(prio_t prio)
	{
	    return prio_queue[prio];
	}

    void init ()
	{
	    /* prio queues */
	    for(int i = 0; i <= MAX_PRIORITY; i++)
		prio_queue[i] = (tcb_t *)NULL;
	    max_prio = -1;
	}

    void set_timeslice_tcb(tcb_t *tcb) 
	{ timeslice_tcb = tcb; }

    tcb_t *get_timeslice_tcb() 
	{ return timeslice_tcb;	}
    
    s16_t max_prio;
    
private:
    tcb_t * timeslice_tcb;
    tcb_t * prio_queue[MAX_PRIORITY + 1];
};



#if defined(CONFIG_SMP)

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
	    tcb->sched_state.requeue = tcb_list;
	    tcb_list = tcb;
		

	}
    tcb_t *dequeue_head()
	{
	    ASSERT(lock.is_locked());
	    ASSERT(!is_empty());
 
	    tcb_t * tcb = tcb_list;
	    tcb_list = tcb->sched_state.requeue;
	    tcb->sched_state.requeue = NULL;
		
	    TRACE_SCHEDULE_DETAILS("smp_requeue:dequeue_head %t (s=%s) cpu %d (head %t)", 
	    	    tcb, tcb->get_state().string(), tcb->get_cpu(), tcb_list);

	    return (tcb_t *) tcb;
		
	}
	
};
#endif /* defined(CONFIG_SMP) */

class rr_scheduler_t 
{
public:
    /**
     * RR-specific functions
     */
    
    /**
     * hierarchical scheduling prio queue 
     */
    prio_queue_t * get_prio_queue()
	{
	    return &this->root_prio_queue;
	}

    
    /**
     * Enqueues a TCB into the timeout list
     *
     * @param tcb       thread control block to enqueue
     * @param head      Enqueue TCB at the head of the list when true
     */
    void enqueue_timeout(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    ASSERT(tcb->get_cpu() == get_current_cpu());
	    ENQUEUE_LIST_TAIL(wakeup_list, tcb, sched_state.wait_list);
	    tcb->queue_state.set(queue_state_t::wakeup);

	}

    /**
     * dequeues tcb from the timeout list (if the thread is in)
     * @param tcb thread control block
     */
    void dequeue_timeout(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    DEQUEUE_LIST(wakeup_list, tcb, sched_state.wait_list);
	    tcb->queue_state.clear(queue_state_t::wakeup);

	}



    /**
     * Marks end of timeslice
     * @param tcb       TCB of thread whose timeslice ends
     */
    void end_of_timeslice (tcb_t * tcb);


    /**
     * function is called when the total quantum of a thread expires 
     * @param tcb thread control block of the thread whose quantum expired
     */
    void total_quantum_expired(tcb_t * tcb);

    
    
#if defined(CONFIG_SMP)
    bool requeue_needed(cpuid_t cpu) { return !smp_requeue_lists[cpu].is_empty(); }
    void smp_requeue(bool holdlock);
    void unlock_requeue() 
	{ smp_requeue_lists[get_current_cpu()].lock.unlock(); }

protected:
    static smp_requeue_t smp_requeue_lists[CONFIG_SMP_MAX_CPUS];
#endif

protected:
    
    void policy_scheduler_init()
	{
	    /* wakeup list */
	    wakeup_list = NULL;
            root_prio_queue.init();

	}
    /**
     * parse the wakeup queues
     * @param current current thread control block
     * @return woken up thread with the highest priority. If no thread has a 
     * higher priority than current, current will be returned
     */
    tcb_t * parse_wakeup_queues(tcb_t * current);

    /**
     * scheduler-specific dispatch decision
     * @param tcb thread control block
     * @param dest tcb to be dispatched
     * @return true if dest can be scheduled
     */
    bool check_dispatch_thread(tcb_t * tcb, tcb_t * dest)
	{
	    if (EXPECT_FALSE(tcb->get_preempt_flags().is_delayed() &&
			      tcb->sched_state.get_maximum_delay() ))
		return !tcb->sched_state.delay_preemption (dest);
	    return (tcb->sched_state.get_priority() < dest->sched_state.get_priority()); 
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
	    ASSERT(tcb->is_local_cpu());
	    get_prio_queue()->enqueue(tcb, head);
	}

    /**
     * dequeues tcb from the ready list (if the thread is in)
     * @param tcb thread control block
     */
    void dequeue_ready(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    get_prio_queue()->dequeue(tcb);
	}

        
    tcb_t * wakeup_list;
    prio_queue_t root_prio_queue;
    static volatile u64_t current_time;

};

typedef class rr_scheduler_t policy_scheduler_t;
typedef void policy_sched_next_thread_t;


#endif /* !__API__V4__SCHED_RR__SCHEDULE_H__ */
