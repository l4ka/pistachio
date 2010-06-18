/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-hs/schedule.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_HS__SCHEDULE_H__
#define __API__V4__SCHED_HS__SCHEDULE_H__

#define DELAY_PREEMPT_TICKS	1
typedef u64_t period_cycles_t;
class schedule_req_t;

#include INC_API(smp.h)

EXTERN_KMEM_GROUP(kmem_sched);

class prio_queue_t
{
public:
    void enqueue( tcb_t * tcb, bool head=false)
        {
            ASSERT(tcb);
            ASSERT(tcb != get_idle_tcb());
            
            sched_ktcb_t *sktcb = &tcb->sched_state;
            prio_t prio = sktcb->get_priority();
            word_t stride = sktcb->get_stride();
            
            if (tcb->queue_state.is_set(queue_state_t::ready))
            {
                if( sktcb->get_pass() < global_pass )
                    sktcb->set_pass(global_pass + stride);
                return;
            }
    
            sktcb->set_pass(global_pass + stride);

            ASSERT(sktcb);

            if ( head )
                ENQUEUE_LIST_HEAD (prio_queue[prio], tcb, sched_state.ready_list);
            else
                ENQUEUE_LIST_TAIL (prio_queue[prio], tcb, sched_state.ready_list);
    	
            tcb->queue_state.set(queue_state_t::ready);
            max_prio = max((s16_t) prio, max_prio);
	    count++;
	    
        }


    bool dequeue(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    ASSERT(tcb != get_idle_tcb());

	    sched_ktcb_t *sktcb = &tcb->sched_state;
	    prio_t prio = sktcb->get_priority();

	    if (tcb->queue_state.is_set(queue_state_t::ready))
	    {	    
		DEQUEUE_LIST(prio_queue[prio], tcb, sched_state.ready_list);
		tcb->queue_state.clear(queue_state_t::ready);
		count--;
	    }
            return count;

	}

    void start_timeslice(u64_t current_time)
	{
	    this->timeslice_start = current_time;
	}

    void end_timeslice(u64_t current_time)
	{
	    this->period_cycles += period_cycles_t(current_time - this->timeslice_start);
	    this->timeslice_start = current_time;
	}

    void reset_period_cycles(u64_t current_time)
	{
	    this->period_cycles = 0;
	    this->timeslice_start = current_time;
	    this->poll_window = current_time;
	}

    period_cycles_t get_period_cycles()
	{
	    return this->period_cycles;
	}

    period_cycles_t get_poll_window()
	{
	    return this->poll_window;
	}


    tcb_t * get_domain_tcb()
	{
    	    return domain_tcb;
	}
    
    u64_t get_global_pass()
	{
    	    return global_pass;
	}

    void set_global_pass(u64_t pass)
	{
    	    global_pass = pass;
	}

    void set_depth(prio_queue_t *parent_queue)
	{
	    this->depth = parent_queue->get_depth() + 1;
	}

    word_t get_depth()
	{ return this->depth; }

    bool is_same_domain( prio_queue_t *prio_queue )
	{
#if defined(CONFIG_SMP)
    	    return prio_queue->cpu_head == this->cpu_head;
#else
    	    return this == prio_queue;
#endif
       	}
    
    prio_queue_t *domain_partner(cpuid_t cpu);
    prio_queue_t *add_prio_domain(schedule_ctrl_t prio_control);


    word_t prio_tickets( tcb_t *search, word_t *search_tickets )
	{
            ASSERT(search);
	    tcb_t *start = this->get(search->sched_state.get_priority());

	    tcb_t *tcb = start;
	    word_t stride_sum = 0;
            
	    do
	    {
                ASSERT(tcb);
		stride_sum += tcb->sched_state.get_stride();
		tcb = tcb->sched_state.ready_list.next;
	    } while( tcb != start );

	    *search_tickets = 0;
	    float tot_tickets = 0;
	    do
	    {
		float tickets = (float)stride_sum / (float)tcb->sched_state.get_stride();
		tot_tickets += tickets;
		if( search == tcb )
		    *search_tickets = (word_t)tickets;
		tcb = tcb->sched_state.ready_list.next;
	    } while( tcb != start );

	    return (word_t)tot_tickets;
	}

    void init ( tcb_t *dtcb )
        {
   
            for( int i = 0; i < MAX_PRIORITY; i++ )
                prio_queue[i] = NULL;
            
            global_pass = 0;
            max_prio = -1;
            refcnt = 0;
            domain_tcb = dtcb;
            depth = 0;
	    count = 0;
	    
            ON_CONFIG_SMP(cpu_link = cpu_head = NULL);
            reset_period_cycles(get_cpu_cycles());
        }

    void set(prio_t prio, tcb_t * tcb)
	{ this->prio_queue[prio] = tcb; }

  
    tcb_t * get(prio_t prio)
	{ return prio_queue[prio]; }

    s16_t max_prio;
    
private:
    word_t  refcnt;
    tcb_t * domain_tcb;
    word_t depth;
    word_t count;
public:
    u64_t global_pass;	// Proportional share in queue
    period_cycles_t period_cycles;// number of CPU cycles used during the sampling period
    period_cycles_t poll_window;
    u64_t timeslice_start;
#if defined(CONFIG_SMP)
    prio_queue_t *cpu_link, *cpu_head;
#endif
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

class hs_scheduler_t 
{
public:
    prio_queue_t * get_prio_queue()
	{
	    return &this->root_prio_queue;
	}

    prio_queue_t * get_scheduled_queue()
	{
	    return scheduled_queue;
	}

    
    void enqueue_timeout(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    ASSERT(tcb->get_cpu() == get_current_cpu());
	    ENQUEUE_LIST_TAIL(wakeup_list, tcb, sched_state.wait_list);
	    tcb->queue_state.set(queue_state_t::wakeup);

	}

    void dequeue_timeout(tcb_t * tcb)
	{
	    ASSERT(tcb);
	    DEQUEUE_LIST(wakeup_list, tcb, sched_state.wait_list);
	    tcb->queue_state.clear(queue_state_t::wakeup);

	}
    void enqueue_ready(tcb_t * tcb, bool head = false)
	{
            while (tcb->get_global_id() !=  IDLETHREAD )
            {
                ASSERT(tcb);
                ASSERT(tcb->sched_state.get_prio_queue());
                TRACE_SCHEDULE_DETAILS("enqueue_ready %t () pq %t dtcb %t idle %p\n", tcb, 
                                       tcb->sched_state.get_prio_queue(), tcb->sched_state.get_prio_queue()->get_domain_tcb(),
                                       get_idle_tcb());
                
                tcb->sched_state.get_prio_queue()->enqueue(tcb, head);
                tcb = tcb->sched_state.get_prio_queue()->get_domain_tcb();
            }
        }
    
    void dequeue_ready(tcb_t * tcb)
	{
            ASSERT(tcb);
            while (tcb->get_global_id() !=  IDLETHREAD )
            {
                ASSERT(tcb);
                ASSERT(tcb->sched_state.get_prio_queue());
                if (tcb->sched_state.get_prio_queue()->dequeue(tcb))
                    break;
                tcb = tcb->sched_state.get_prio_queue()->get_domain_tcb();
            }
	}

   
    void end_of_timeslice (tcb_t * tcb);
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
    
    void policy_scheduler_init();
    
    tcb_t * parse_wakeup_queues(tcb_t * current);

    bool check_dispatch_thread(tcb_t * stcb, tcb_t *dtcb)
	{
            ASSERT(stcb != dtcb);
            sched_ktcb_t *ssktcb = &stcb->sched_state;
            sched_ktcb_t *dsktcb = &dtcb->sched_state;
	    
            prio_queue_t *sprio_queue = ssktcb->get_prio_queue();
            prio_queue_t *dprio_queue = dsktcb->get_prio_queue();
            
	    if (EXPECT_FALSE(stcb->get_preempt_flags().is_delayed() && ssktcb->get_maximum_delay() ))
		return !ssktcb->delay_preemption (dtcb);
	    
            ASSERT(sprio_queue && dprio_queue);
            
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
            
            TRACEPOINT (SCHEDULE_DETAILS, "cdt %t (d %t prio %x pass %d) - %t (d %t prio %x pass %d)\n",
                        stcb, addr_to_tcb(ssktcb), ssktcb->get_priority(), (word_t) ssktcb->get_pass(),
                        dtcb, addr_to_tcb(dsktcb), dsktcb->get_priority(), (word_t) dsktcb->get_pass());

            if (ssktcb->get_priority() == dsktcb->get_priority())
                return ssktcb->get_pass() < dsktcb->get_pass();
            else
                return ssktcb->get_priority() < dsktcb->get_priority();

	}

   
    void hs_extended_schedule(schedule_req_t *req);
    
    tcb_t * wakeup_list;
    static volatile u64_t current_time;
    s64_t current_timeslice;
    u64_t delayed_preemption_start;

    tcb_t * scheduled_tcb;
    prio_queue_t root_prio_queue;
    prio_queue_t *scheduled_queue;
    
    friend class sched_ktcb_t;
   
};

typedef class hs_scheduler_t policy_scheduler_t;
typedef prio_queue_t policy_sched_next_thread_t;


#endif /* !__API__V4__SCHED_HS__SCHEDULE_H__ */
