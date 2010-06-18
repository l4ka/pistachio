/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007-2010,  Karlsruhe University
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

enum sched_flags_e 
{
    sched_chk_flag	   = 0, // run scheduling policy
    sched_ds1_flag	   = 1, // schedule dest1
    sched_ds2_flag	   = 2, // schedule dest2
    sched_c2r_flag	   = 3, // current was running
    sched_timeout_flag	   = 4, // cancel timeout
    /* rr specific flags */
    rr_tsdonate_flag	   = 5, // round-robin TS donation
    /* pm specific flags */
    pm_chk_preemption_flag = 6, // PM-scheduling check
};

typedef u8_t sched_flags_t;


const sched_flags_t sched_default = FLAGFIELD2(sched_chk_flag, sched_c2r_flag);
const sched_flags_t sched_current = FLAGFIELD2(sched_ds1_flag, sched_c2r_flag);
const sched_flags_t sched_dest =    FLAGFIELD2(sched_ds2_flag, sched_c2r_flag); 
const sched_flags_t sched_handoff = FLAGFIELD1(sched_ds2_flag); 

/* IPC default flags */
const sched_flags_t sched_sndonly = FLAGFIELD3(sched_chk_flag, sched_c2r_flag, sched_timeout_flag); 
const sched_flags_t sched_ipcblk  = FLAGFIELD3(sched_ds2_flag, rr_tsdonate_flag, pm_chk_preemption_flag);
const sched_flags_t sched_rcverr  = FLAGFIELD3(sched_ds2_flag, rr_tsdonate_flag, pm_chk_preemption_flag); 
const sched_flags_t sched_rplywt  = FLAGFIELD3(sched_chk_flag, rr_tsdonate_flag, sched_timeout_flag); 


class schedule_req_t 
{
public:

    schedule_ctrl_t time_control;
    schedule_ctrl_t prio_control;
    schedule_ctrl_t preemption_control;
    schedule_ctrl_t processor_control;
    tcb_t* tcb;					 
    bool valid;
    
    void init() { time_control = 0; prio_control = 0; preemption_control = 0;  processor_control = 0; valid = false; } 
    schedule_req_t (void) { init();  }
};

class schedule_request_queue_t 
{
public:
    static const word_t schedule_queue_len = 128;
    schedule_req_t entries[schedule_queue_len];
    word_t first_alloc;
    word_t first_free;
    spinlock_t lock;
    
public:
    char pad2[CACHE_LINE_SIZE - sizeof(spinlock_t)];

    schedule_request_queue_t (void) { lock.init(); first_alloc = first_free = 0; }
    
    schedule_req_t *reserve_request() 
	{ 
	    lock.lock();
	    if ( ((first_free + 1) % schedule_queue_len) == first_alloc )
	    {
		lock.unlock();
		return NULL;
	    }
	    word_t idx = first_free;
	    first_free = (first_free + 1) % schedule_queue_len;
	    return &entries[idx];
	}
    
    schedule_req_t process_request ()
	{
	    ASSERT(!is_empty());
	    
	    lock.lock();
	    schedule_req_t req = entries[first_alloc];
	    entries[first_alloc].valid = false;
	    first_alloc = (first_alloc + 1) % schedule_queue_len;
	    lock.unlock();
	    
	    return req;
	}


    void commit_request ()
	{
	    lock.unlock();
	}

    
    bool is_empty() { return  (first_alloc == first_free); };
    
};

#include INC_API_SCHED(schedule.h)


class scheduler_t : public policy_scheduler_t
{
public:
    
    /**
     * initializes the scheduler, must be called before init
     */
    void init(bool bootcpu = true );

    /**
     * starts the scheduling, does not return
     * @param cpu processor the scheduler starts on
     */
    void start(cpuid_t cpu = 0);

    /**
     * dispatches a thread 
     * @param tcb the thread control block of the thread to be dispatched
     */
    void dispatch_thread(tcb_t * tcb);


    /**
     * sets the thread currently accounted
     * @param tcb the thread
     */
    void set_accounted_tcb(tcb_t *tcb);
    
    /**
     * delivers the thread currently accounted
     * @return the thread
     */
    tcb_t *get_accounted_tcb();
    
    /**
     * check if a thread is allowed to schedule another thread 
     * @param tcb the next control block
     * @param dest_tcb the destination control block
     * @return true if next was scheduled, false otherwise
     */
    bool is_scheduler(tcb_t *tcb, tcb_t *dest_tcb);

    
    /**
     * schedule a runnable thread
     * @return true if a runnable thread was found, false otherwise
     */
    bool schedule();
    
    /**
     * schedule a thread 
     * @param next the potential next control block
     * @param flags policy-specific flags
     * @return true if next was scheduled, false otherwise
     */
    bool schedule(tcb_t *dest, const sched_flags_t flags=sched_default);

    /**
     * deschedule a thread 
     * @param next the potential next control block
     * @param flags hint, which tcb should run next
     */
    void deschedule(tcb_t *tcb);

    /**
     * perform scheduling decision between  two runnable threads
     * @param dest1 the 1st potential next thread control block
     * @param dest2 the 2nd potential next thread control block
     * @param flags policy-specific flags
     * @return true if next1 was scheduled, false otherwise
     */
    bool schedule(tcb_t *dest1, tcb_t *dest2, const sched_flags_t flags=sched_default);

    /**
     * schedule an interrupt (if handler is not waiting)
     * @param irq the irq's thread control block
     * @param handler the handler's thread control block
     * @return true if next was scheduled, false otherwise
     */
    bool schedule_interrupt(tcb_t *irq, tcb_t *handler);

#if defined(CONFIG_SMP)
    /**
     * schedule a runnable thread on a different cpu
     * @param tcb the current thread control block
     */
    void remote_schedule(tcb_t * tcb);
    
    /**
     * migrate a runnable thread to a different cpu
     * @param tcb the current thread control block
     * @param cpu the destination cpu
     */
    void move_tcb(tcb_t *tcb, cpuid_t cpu);
#endif
    

    /**
     * handles the timer interrupt event, walks the wait lists 
     * and makes a scheduling decission
     */
    void handle_timer_interrupt();

    /**
     * Idle thread function
     */
    void idle();


    /**
     * Function called when threads execute a halting instruction
     * @return if call could be handled
     */
    bool idle_hlt();

    /**
     * delivers the current time relative to the system 
     * startup in microseconds
     * @return absolute time 
     */
    u64_t get_current_time();

    /**
     * add a scheduling request on that processor
     * @param tcb the destination thread control block
     * @param prio_control	  prio control word
     * @param time_control	  time control word
     * @param preemption_control  preemption control word
     * @param processor_control	  processor control word
     * @return error word
     * 
     */
    word_t add_schedule_request(schedule_req_t &req);
    
    /**
     * check if scheduling requests are pending on that cpu
     * 
     * @param cpu
     * @return if requests are pending
     */
    bool schedule_requests_pending(cpuid_t cpu) 
	{ 
	    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
	    return !schedule_request_queue[cpu].is_empty(); 
	}
   
    /**
     * process all scheduling request local to the scheduler 
     */
    void process_schedule_requests();

    /**
     * calculate scheduler return values
     */
    word_t return_schedule_parameter(word_t num, schedule_req_t &req);

    /**
     * check if a schedule parameters of a request are valid 
     * @param scheduler	  the issueing scheduler
     * @param req	  the schedule request
     * @return		  error word
     * 
     */
    word_t check_schedule_parameters(tcb_t *scheduler, schedule_req_t &req);

private:
    /**
     * searches the for the next runnable thread
     * @param p    policy specific param
     * 
     * @return next thread to be scheduled
     */
    tcb_t * find_next_thread(policy_sched_next_thread_t *p=NULL);
    
    
    /**
     * commit schedule parameters of a request 
     * @param req	  the schedule request
     * 
     */
    void commit_schedule_parameters(schedule_req_t &req);

    static schedule_request_queue_t schedule_request_queue[CONFIG_SMP_MAX_CPUS];
};

/**
 * @return the current scheduler 
 * the default implementation features exactly one scheduler at a time.
 */
INLINE scheduler_t * get_current_scheduler()
{
    extern scheduler_t scheduler;
    return &scheduler;
}

/* global declarations */
extern void init_all_threads(void);

#if defined(CONFIG_SMP)
#include INC_API(smp.h)
extern void do_xcpu_send_irq(cpu_mb_entry_t * entry);
#endif

#if defined(CONFIG_DEBUG)
INLINE word_t flags_stringword(sched_flags_t f)
{
    word_t ret = 0;
    char *s = (char *) &ret;
    
    s[0] = FLAG_IS_SET(f,sched_chk_flag) ? 'C' : 
	(FLAG_IS_SET(f,sched_ds1_flag) ? '1' :
	 (FLAG_IS_SET(f,sched_ds2_flag) ? '2' : '~'));
    s[1] = FLAG_IS_SET(f,sched_c2r_flag) ? 'R' : '~';
    s[2] = FLAG_IS_SET(f,sched_timeout_flag) ? 'T' : '~';
#if defined(CONFIG_SCHED_RR)
    s[3] = FLAG_IS_SET(f,rr_tsdonate_flag) ? 'O' : '~';
#endif
    return ret;
}
#endif

#include INC_API_SCHED(schedule_functions.h)




#endif /*__API__V4__SCHEDULE_H__*/

