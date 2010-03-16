/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-rr/ktcb.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_RR__SCHED_KTCB_H__
#define __API__V4__SCHED_RR__SCHED_KTCB_H__

#include INC_GLUE(ipc.h)
#if defined(CONFIG_X_EVT_LOGGING)
#include INC_GLUE(logging.h)
#endif


#define DEFAULT_TIMESLICE_LENGTH	(time_t::period(625, 4))
#define DEFAULT_TOTAL_QUANTUM		(time_t::never())
#define DEFAULT_PRIORITY		100
#define MAX_PRIORITY			255
#define ROOT_PRIORITY			MAX_PRIORITY

typedef u8_t prio_t;
typedef void (*requeue_callback_t)(tcb_t* tcb);

class rr_sched_ktcb_t
{
public:

    /**
     * delivers the current priority of a thread
     * @return the total quantum
     */

    u64_t get_total_quantum()
	{ return total_quantum; }

    /**
     * sets the total time quantum of the thread
     * @param quantum the total quantum
     */
    
    void set_total_quantum(u64_t quantum)
	{ total_quantum = quantum; }

    u64_t account_quantum(u32_t t)
	{ 
	    total_quantum -= t; 
	    return total_quantum;
	} 
    
    void init_total_quantum(time_t quantum)
	{
	    if (quantum.is_never())
		total_quantum = 0;
	    else
	    {
		// quantum can only be specified as a time period
		ASSERT(quantum.is_period()); 
		total_quantum = quantum.get_microseconds();
	    }
	    // give a fresh timeslice according to the spec
	    current_timeslice = timeslice_length;
	}
    
    /**
     * delivers the current timeslice of a thread
     * @return the timeslice
     */
    
    s64_t get_timeslice()
	{ return current_timeslice; } 

    s64_t account_timeslice(u32_t t)
	{ 
	    current_timeslice -= t; 
	    return current_timeslice;
	} 
    
    void renew_timeslice(u32_t t)
	{ current_timeslice += t; } 
    
    /**
     * delivers the current timeslice length of a thread
     * @return the timeslice
     */
    
    u64_t get_timeslice_length()
	{ return timeslice_length; } 
    
    /**
     * initializes the timeslice and timeslice length of a thread 
     * @param timeslice timeslice length (xmust be a time period)
     */

    void init_timeslice(time_t timeslice)
	{
	    ASSERT(timeslice.is_period()); 
	    ASSERT(this);
	    current_timeslice = timeslice_length = timeslice.get_microseconds();
	}

    /**
     * sets maximum delay for delayed preemption
     */

    void set_maximum_delay (u16_t usec)
	{ current_max_delay = usec; }

    /**
     * delivers the current maximum for delayed preemption of a thread
     * @return the delay 
     */
    
    u16_t get_maximum_delay ()
	{ return current_max_delay; }


    /**
     * initialize the current maximum for delayed preemption of a thread
     * @param usec length of delay
     */
    void init_maximum_delay (u16_t usec)
	{ current_max_delay = max_delay = usec;	}

    /**
     * delivers the initia value of maximum for delayed preemption of a thread
     * @param usec length of delay
     */
    u16_t get_init_maximum_delay ()
	{ return max_delay; }

    
    /**
     * sets the priority of a thread
     * @param prio priority of thread
     */

    void set_priority(prio_t prio)
	{
	    priority = prio;
	    /* keep sensitive and current prio in-sync to reduce checking overhead */
	    if (sensitive_prio < prio)
		set_sensitive_prio (prio);
	}
    
    
    /**
     * delivers the current priority of a thread
     * @return the priority 
     */
    prio_t get_priority() 
        { return priority; };

    /**
     * sets sensitive prio for delayed preemption
     */
    
    void set_sensitive_prio (prio_t prio)
	{ sensitive_prio = prio; }

    /**
     * delivers the current sensitive priority of a thread
     * @return the sensitive priority 
     */

    prio_t get_sensitive_prio ()
	{ return sensitive_prio; }


    /**
     * delivers the current timeout of a thread
     * @return the timeout 
     */
    u64_t get_timeout() 
          { return absolute_timeout; } 


    /**
     * @return true if the timeout has expired
     */
    bool has_timeout_expired(u64_t time)
	{
	    return (absolute_timeout <= time);
	}
    
    /**
     * delay preemption
     * @param current   current TCB
     * @param tcb       destination TCB
     * @return true if preemption was delayed, otherwise false
     */
    bool delay_preemption ( tcb_t * tcb );
  
    
#if defined(CONFIG_SMP)
    tcb_t		*requeue;
    requeue_callback_t	requeue_callback;
#endif

    /* scheduling lists  */
    ringlist_t<tcb_t>	ready_list;
    ringlist_t<tcb_t>	wait_list;
    

protected:
   
    u64_t		total_quantum;
    u64_t		timeslice_length;
    s64_t		current_timeslice;
    u64_t		absolute_timeout;

    prio_t		priority;
    /* delayed preemption */
    prio_t		sensitive_prio;
    u16_t		current_max_delay;
    u16_t		max_delay;
    
    friend class prio_queue_t;
    
   
   
};

typedef rr_sched_ktcb_t policy_sched_ktcb_t;

#endif /* !__API__V4__SCHED_RR__SCHED_KTCB_H__ */
