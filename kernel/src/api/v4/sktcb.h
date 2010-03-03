/*********************************************************************
 *                
 * Copyright (C) 2009-2010,  Karlsruhe University
 *                
 * File path:     api/v4/sktcb.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __API__V4__SKTCB_H__
#define __API__V4__SKTCB_H__

#include INC_API(ipc.h)

#define MAX_LOGIDS		__UL(32)
#define NULL_LOGID		(0xFFFFFFFF)
#define IDLE_LOGID		(0)
#define ROOTSERVER_LOGID	(1)

enum sktcb_type_e {
    sktcb_user	= 0,
    sktcb_hi	= 1,
    sktcb_lo	= 2,
    sktcb_root	= 3,
    sktcb_irq	= 4,
};

typedef u8_t prio_t;

#include INC_API_SCHED(ktcb.h)

class sched_ktcb_t : public policy_sched_ktcb_t
{
public:
    /**
     * sets the scheduler of the thread
     * @param scheduler the scheduler 
     * @param init if scheduler was set during system initialization
     */
    void set_scheduler(const threadid_t tid);
    
    
    /**
     * delivers the current scheduler of a thread
     * @return the scheduler
     */

    threadid_t get_scheduler()
        { return scheduler; }


    /**
     * hook invoked during sys_thread_switch
     */

    void sys_thread_switch();
    

    /**
     * hook invoked during delete_tcb()
     */
    void delete_tcb();

    /**
     * sets the absolute timeout for a thread
     * @param tcb		pointer to the thread control block
     * @param absolute_time	time the timeout expires
     */
    void set_timeout(u64_t absolute_time, const bool enqueue=true);
    
    /**
     * sets a timeout for the thread specified in tcb
     * @param time	timeout value
     */
    void set_timeout(time_t time);

    /**
     * cancels a set timeout for tcb
     * @param tcb thread control block of the thread
     *
     * VU: DO NOT set the absolute timeout to zero!!! Used for 
     *     thread migration between processors
     */
    void cancel_timeout();

    /**
     * initialize the scheduling parameters of a TCB
     * @param root_server       if TCB is a root server 
     */
    void init(sktcb_type_e type);

#if defined(CONFIG_X_EVT_LOGGING)
    /**
     * Get accounting domain of a thread
     * @return      log ID
     */
    u16_t get_logid()
	{ return this->logid; }

    /**
     * Set accounting logid of a thread
     * @param      logid ID
     */
    void set_logid(u16_t logid)
	{ this->logid = logid; }
    
#endif

#if defined(CONFIG_DEBUG)
    void dump_priority();
    void dump_list1();
    void dump_list2();
    void dump(u64_t current_time);
#endif

    /* do not delete this TCB_START_MARKER */

    threadid_t		scheduler;
#if defined(CONFIG_X_EVT_LOGGING)
    word_t              logid;
#endif
    /* TCB_END_MARKER */

};

#endif /* !__API__V4__SKTCB_H__ */
