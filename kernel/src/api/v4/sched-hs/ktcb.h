/*********************************************************************
 *                
 * Copyright (C) 2007-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sched-hs/ktcb.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SCHED_HS__SCHED_KTCB_H__
#define __API__V4__SCHED_HS__SCHED_KTCB_H__

#include <tcb_layout.h>
#include <generic/bitmask.h>

#include INC_GLUE(ipc.h)
#if defined(CONFIG_X_EVT_LOGGING)
#include INC_GLUE(logging.h)
#endif


#define DEFAULT_TIMESLICE_LENGTH	(time_t::period(625, 3)) /*  5ms */
#define DEFAULT_TOTAL_QUANTUM		(time_t::never())
#define DEFAULT_STRIDE			100
#define DEFAULT_PRIORITY		100
#define MAX_PRIORITY			255
#define ROOT_PRIORITY			MAX_PRIORITY


typedef u8_t prio_t;
typedef void (*requeue_callback_t)(tcb_t* tcb);
class prio_queue_t;

class hs_sched_ktcb_t
{
public:
    enum flags_e {
	is_schedule_domain	= 0,
    };
    
   
#if defined(CONFIG_SMP)
    tcb_t		*requeue;
    requeue_callback_t	requeue_callback;
#endif

    ringlist_t<tcb_t>	ready_list;
    ringlist_t<tcb_t>	wait_list;
    
    u64_t get_total_quantum()
	{ return total_quantum; }

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
    
    s64_t get_timeslice()
	{ return current_timeslice; } 
    void set_timeslice(u32_t t)
	{ current_timeslice = t; } 
    
    u64_t get_timeslice_length()
	{ return timeslice_length; } 
    
    void init_timeslice(time_t timeslice)
	{
	    ASSERT(timeslice.is_period()); 
	    ASSERT(this);
	    current_timeslice = timeslice_length = timeslice.get_microseconds();
	}

    void set_maximum_delay (u16_t usec)
	{ 
            current_max_delay = usec;
            delay_penalty = 0;
        }

    u16_t get_maximum_delay ()
	{ return current_max_delay; }

    void init_maximum_delay (u16_t usec)
	{ current_max_delay = max_delay = usec;	}

    u16_t get_init_maximum_delay ()
	{ return max_delay; }

    void add_delay_penalty (u16_t usec)
	{ delay_penalty = usec;	}

    void set_priority(prio_t prio)
	{
	    priority = prio;
	    /* keep sensitive and current prio in-sync to reduce checking overhead */
	    if (sensitive_prio < prio)
		set_sensitive_prio (prio);
	}
    
    prio_t get_priority() { return priority; };
    void set_sensitive_prio (prio_t prio) {sensitive_prio = prio;}
    prio_t get_sensitive_prio ()
	{ return sensitive_prio; }
    
    u64_t get_timeout() 
        { return absolute_timeout; } 
    bool has_timeout_expired(u64_t time) 
        { return (absolute_timeout <= time); }

    bool delay_preemption ( tcb_t * tcb );

    void set_stride(word_t s) { stride = s; }
    word_t get_stride() { return stride; }

    void set_pass(u64_t p) { pass = p; }
    u64_t get_pass() { return pass; }
    void account_pass();

    prio_queue_t *get_prio_queue() { return prio_queue; }
    void set_prio_queue(prio_queue_t *q);
    void migrate_prio_queue(prio_queue_t *nq);

    prio_queue_t * get_domain_prio_queue()
	{ 
#if !defined(BUILD_TCB_LAYOUT)
            return (prio_queue_t *) (((word_t) this & KTCB_MASK) + OFS_TCB_KERNEL_STACK);
#endif
        }
    

   
public:
    bitmask_t<word_t>	flags;

protected:
    u64_t		total_quantum;
    u64_t		timeslice_length;
    s64_t		current_timeslice;
    u64_t		absolute_timeout;
    
    u64_t		pass;
    word_t		stride;	

    prio_t		priority;
    prio_t		sensitive_prio;

    u16_t		current_max_delay;
    u16_t		max_delay;
    u16_t		delay_penalty;
    u16_t		reserved0;

    prio_queue_t	*prio_queue;

    friend class prio_queue_t;
    
};

typedef hs_sched_ktcb_t policy_sched_ktcb_t;

#endif /* !__API__V4__SCHED_HS__SCHED_KTCB_H__ */
