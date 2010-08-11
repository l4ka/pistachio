/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     api/v4/tcb.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __API__V4__TCB_H__
#define __API__V4__TCB_H__

#include <debug.h>

#include INC_API(cpu.h)
#include INC_API(types.h)
#include INC_API(queuestate.h)
#include INC_API(queueing.h)
#include INC_API(threadstate.h)
#include INC_API(space.h)
#include INC_API(resources.h)
#include INC_API(thread.h)
#include INC_API(preempt.h)
#include INC_API(fpage.h)
#include INC_API(ipc.h)
#include INC_API(sktcb.h)

/* implementation specific functions */
#if defined(CONFIG_X_CTRLXFER_MSG)
#include INC_GLUE(ipc.h)
class arch_ktcb_t;
typedef word_t (arch_ktcb_t::*get_ctrlxfer_regs_t)(word_t id, word_t mask, tcb_t *dst_utcb, word_t &dst_mr);
typedef word_t (arch_ktcb_t::*set_ctrlxfer_regs_t)(word_t id, word_t mask, tcb_t *src_utcb, word_t &src_mr);
#endif

/* implementation specific functions */
#include INC_GLUE(ktcb.h)
#include INC_GLUE(utcb.h)
#if defined(CONFIG_X_CTRLXFER_MSG)
#include INC_GLUE(ipc.h)
#endif


class space_t;

/**
 * tcb_t: kernel thread control block
 */
class tcb_t
{
public:
    enum unwind_reason_e {
	abort		= 1,
	timeout		= 2,
    };

    /*
     * Generic flags go from bit zero and upwards.  Architecture
     * specific flags should go from most significant bits and
     * downwards.
     */
    enum flags_e {
	has_xfer_timeout	= 0,
	schedule_in_progress	= 1,
#if defined(CONFIG_X_CTRLXFER_MSG)
	kernel_ctrlxfer_msg	= 2,
#endif
    };

    /* public functions */
    bool activate(void (*startup_func)(), threadid_t pager);
    
    void create_inactive(threadid_t dest, threadid_t scheduler, sktcb_type_e type);
    void create_kernel_thread(threadid_t dest, utcb_t * utcb, sktcb_type_e type);
    
    void delete_tcb();
    bool migrate_to_space(space_t * space);
    bool migrate_to_processor(cpuid_t processor);
    
    bool exists() 
	{ return space != NULL; }
    bool is_activated()
	{ return utcb != NULL; }

    void unwind (unwind_reason_e reason);
    
    /* queue manipulations */
    void enqueue_send(tcb_t * tcb, const bool head=false);
    void dequeue_send(tcb_t * tcb);
    void enqueue_present();
    void dequeue_present();

    /* thread id access functions */
    void set_global_id(threadid_t tid);
    threadid_t get_global_id();

    threadid_t get_local_id();
    bool check_utcb_location (void);
    bool check_utcb_location (word_t utcb_location);
    void set_utcb_location (word_t utcb_location);
    word_t get_utcb_location();

    void set_error_code (word_t err);
    word_t get_error_code (void);

    /* thread state */
    void set_state(thread_state_t state);
    thread_state_t get_state();
    arch_ktcb_t * get_arch();
    void save_state (void);
    void restore_state (void);

    /* ipc */
    void set_partner(threadid_t tid);
    threadid_t get_partner();
    tcb_t * get_partner_tcb();

    time_t get_xfer_timeout_snd ();
    time_t get_xfer_timeout_rcv ();
    void set_actual_sender(threadid_t tid);
    threadid_t get_virtual_sender();
    threadid_t get_intended_receiver();

    msg_tag_t get_tag();
    void set_tag(msg_tag_t tag);
    word_t get_mr(word_t index);
    void set_mr(word_t index, word_t value);
    void copy_mrs(tcb_t * dest, word_t start, word_t count);
    word_t get_br(word_t index);
    void set_br(word_t index, word_t value);

    msg_tag_t do_ipc(threadid_t to_tid, threadid_t from_tid, timeout_t timeout);
    void send_pagefault_ipc(addr_t addr, addr_t ip, space_t::access_e access);
    bool send_preemption_ipc();
    void return_from_ipc (void);
    void return_from_user_interruption (void);

#if defined(CONFIG_X_CTRLXFER_MSG)
    void set_fault_ctrlxfer_items(word_t fault, ctrlxfer_mask_t mask);
    ctrlxfer_mask_t get_fault_ctrlxfer_items(word_t fault);
    word_t append_ctrlxfer_item(msg_tag_t tag, word_t offset);
    word_t ctrlxfer( tcb_t *dst, msg_item_t item, word_t src_idx, word_t dst_idx, bool src_mr, bool dst_mr);
#if defined(CONFIG_DEBUG)
    void dump_ctrlxfer_state(bool extended);
#endif
#endif
 
    /* synchronization */
    void lock() { tcb_lock.lock(); }
    void unlock() { tcb_lock.unlock(); }

    /* thread notification */
    void notify(void (*func)());
    void notify(void (*func)(word_t), word_t arg1);
    void notify(void (*func)(word_t, word_t), word_t arg1, word_t arg2);
    void notify(void (*func)(tcb_t*, word_t), tcb_t * tcb, word_t arg)
	{ notify((void(*)(word_t, word_t))func, (word_t)tcb, arg); }

    /* thread manipulation */
    addr_t get_user_ip();
    addr_t get_user_sp();
    void set_user_ip(addr_t ip);
    void set_user_sp(addr_t sp);

    /* thread switching */
    void switch_to(tcb_t * current);

    /* space */
    space_t * get_space() { return space; }
    void set_space(space_t * space);

    /* Copy area */
    void adjust_for_copy_area (tcb_t * dst, addr_t * saddr, addr_t * daddr);
    void release_copy_area (void);
    addr_t copy_area_real_address (addr_t addr);

    /* processor */
    cpuid_t get_cpu() 
#if defined(CONFIG_SMP)
	{ return this->cpu; }
#else
	{ return 0; }
#endif
    void set_cpu(cpuid_t cpu);
    bool is_local_cpu();

    /* utcb access functions */
    utcb_t * get_utcb()
	const { return this->utcb; }
    void set_utcb(utcb_t *utcb)
	{ this->utcb = utcb; }
    
public:
    void set_pager(const threadid_t tid);
    void set_exception_handler(const threadid_t tid);
    void set_user_handle(const word_t handle);
    void set_user_flags(const word_t flags);

    threadid_t get_pager();

    threadid_t get_exception_handler();
    word_t get_user_handle();
    word_t get_user_flags();
    preempt_flags_t get_preempt_flags();
    void set_preempt_flags(preempt_flags_t flags);
    u8_t get_cop_flags();
    word_t * get_reg_stack_bottom (void);

    /* interrupt management */
    void set_irq_handler(const threadid_t tid);
    threadid_t get_irq_handler();
    bool is_interrupt_thread();

public:
    static tcb_t *allocate(const threadid_t dest);
    static void deallocate(const threadid_t dest);
    static void init_tcbs();
    static tcb_t *get_tcb(threadid_t tid);
    static bool is_tcb(addr_t addr);


    void arch_init_root_server(space_t * space, word_t ip, word_t sp);

private:
    void create_tcb(const threadid_t dest);
    void init(threadid_t dest, sktcb_type_e type);
    
    /* stack manipulation */
public:
    word_t * get_stack_top();
    void init_stack();
    void create_startup_stack(void (*func)());
    

private:
    friend void make_offsets(); /* generates OFS_TCB_ stuff */

public:

    typedef union {
	struct {
	    struct 
	    {
		/* IPC copy */
		word_t		mr[IPC_NUM_SAVED_MRS];
		word_t		br0;
		threadid_t	partner;
		threadid_t	vsender;
		word_t		state;
		word_t		error;
	    } saved_state[IPC_NESTING_LEVEL];

	    struct {
		word_t		copy_length;
		addr_t		copy_start_src;
		addr_t		copy_start_dst;
		addr_t		copy_fault;
	    } ipc_copy;
	};
	struct {
	    /* Exchange registers */
	    word_t		control;
	    word_t		sp;
	    word_t		ip;
	    word_t		flags;
	    threadid_t		pager;
	    word_t		user_handle;
	} exregs;
    } misc_tcb_t;
    
    void init_saved_state();
    threadid_t get_saved_partner (word_t level = 0);
    void set_saved_partner (threadid_t t, word_t level = 0);
    thread_state_t get_saved_state (word_t level = 0);
    void set_saved_state (thread_state_t s, word_t level = 0);
    
    /* do not delete this TCB_START_MARKER */

    // have relatively static values here
    threadid_t		myself_global;
    threadid_t		myself_local;

private:
    cpuid_t		cpu;
    utcb_t *		utcb;
    
    thread_state_t 	thread_state;
    threadid_t		partner;

public:
    resource_bits_t	resource_bits;
    word_t *		stack;
private:
    /* VU: pdir_cache should be architecture-specific!!! */
    word_t		pdir_cache;

public:
    queue_state_t	queue_state;

    /* queues and scheduling state */
    ringlist_t<tcb_t>	present_list;
    ringlist_t<tcb_t>	send_list;
    tcb_t *		send_head;
    sched_ktcb_t	sched_state;

    spinlock_t		tcb_lock;

#if defined(CONFIG_SMP)
    ringlist_t<tcb_t>	xcpu_list;
    cpuid_t		xcpu;
    word_t		xcpu_status;
    lockstate_t		lock_state;
#endif

private:
    /* pager etc */
    space_t *		space;

#if defined(CONFIG_X_CTRLXFER_MSG)
    ctrlxfer_mask_t	fault_ctrlxfer[4+arch_ktcb_t::fault_max];
#endif

public:
    bitmask_t<word_t>	flags;
    arch_ktcb_t		arch;

public:
    misc_tcb_t		misc;
    thread_resources_t	resources;

private:
    word_t		kernel_stack[0];
    /* do not delete this TCB_END_MARKER */

    /* class friends */
    friend void dump_tcb(tcb_t *, bool extended);
    friend void handle_ipc_error (void);
    friend class thread_resources_t;
    
#if defined(CONFIG_STATIC_TCBS)
    static  tcb_t *tcb_array[TOTAL_KTCBS];
#endif

};

/* union to allow allocation of tcb including stack */
typedef union _whole_tcb_t {
    u8_t pad[KTCB_SIZE];
} whole_tcb_t __attribute__((aligned(KTCB_SIZE)));
    

/**********************************************************************
 *
 *               Operations on thread ids and settings
 *
 **********************************************************************/

/**
 * Set the global thread ID in a TCB
 * @param tid	new thread ID
 */
INLINE void tcb_t::set_global_id(const threadid_t tid)
{
    myself_global = tid;
    ASSERT(get_utcb());
    get_utcb()->set_my_global_id(tid);
}

INLINE threadid_t tcb_t::get_global_id()
{
    return myself_global;
}

INLINE threadid_t tcb_t::get_local_id()
{
    return myself_local;
}


INLINE void tcb_t::set_irq_handler(const threadid_t tid)
{
    sched_state.set_scheduler(tid);
}

INLINE threadid_t tcb_t::get_irq_handler()
{
    return sched_state.get_scheduler();
}


/**********************************************************************
 *
 *                  Access functions
 *
 **********************************************************************/
__attribute__ ((const)) INLINE tcb_t * addr_to_tcb (addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}


#if defined(CONFIG_STATIC_TCBS)
INLINE bool tcb_t::is_tcb(addr_t addr)
{
    tcb_t * tcb = addr_to_tcb(addr);
    for (unsigned i = 0; i < TOTAL_KTCBS; i++)
	if (tcb_array[i] == tcb)
	    return true;
    return false;
}

INLINE tcb_t * tcb_t::get_tcb( threadid_t tid )
{
    return tcb_array[tid.get_threadno() & VALID_THREADNO_MASK];
}

#else

INLINE bool tcb_t::is_tcb(addr_t addr)
{
    return space_t::is_tcb_area(addr);
}
INLINE tcb_t * tcb_t::get_tcb( threadid_t tid )
{
    return (tcb_t *)((KTCB_AREA_START) + 
	    ((tid.get_threadno() & VALID_THREADNO_MASK) * KTCB_SIZE));
}

/**
 * allocate the tcb
 * The tcb pointed to by this will be allocated.
 */
INLINE tcb_t* tcb_t::allocate(threadid_t dest)
{
    tcb_t *tcb = get_tcb(dest);
    /**
     * tcb_t::allocate: allocate memory for TCB
     *
     * Allocate memory for the given TCB.  We do this by generating a
     * write to the TCB area.  If TCB area is not backed by writable
     * memory (i.e., already allocated) the pagefault handler will
     * allocate the memory and map it.
     */
    tcb->kernel_stack[0] = 0;
    
    return tcb;
}

INLINE void tcb_t::deallocate(threadid_t dest)
{ /* Nothing to do */ }
INLINE void tcb_t::init_tcbs()
{ /* Nothing to do */ }

#endif


INLINE void tcb_t::set_state(thread_state_t state)
{
    this->thread_state = state;
}

INLINE thread_state_t tcb_t::get_state()
{
    return this->thread_state;
}

INLINE void tcb_t::set_partner(threadid_t tid)
{
    this->partner = tid;
}

INLINE threadid_t tcb_t::get_partner()
{
    return this->partner;
}

INLINE tcb_t* tcb_t::get_partner_tcb()
{
    return get_tcb(partner);
}

INLINE void tcb_t::init_saved_state()
{
    for (int l = 0; l < IPC_NESTING_LEVEL; l++)
    {	
	set_saved_state (thread_state_t::aborted, l);
	set_saved_partner (threadid_t::nilthread(), l);
    }
}

INLINE threadid_t tcb_t::get_saved_partner (word_t level) 
{ 
    ASSERT(level < IPC_NESTING_LEVEL); 
    return misc.saved_state[level].partner; 
}

INLINE void tcb_t::set_saved_partner (threadid_t t, word_t level) 
{ 
    ASSERT(level < IPC_NESTING_LEVEL); 
    misc.saved_state[level].partner = t; 
}

INLINE thread_state_t tcb_t::get_saved_state (word_t level)
{ 
    ASSERT(level < IPC_NESTING_LEVEL); 
    return (thread_state_t) misc.saved_state[level].state; 
}

INLINE void tcb_t::set_saved_state (thread_state_t s, word_t level)
{ 
    ASSERT(level < IPC_NESTING_LEVEL); 
    misc.saved_state[level].state = s; 
}

/**
 * Get thread ID of a thread's pager
 * @return      thread ID of pager
 */
INLINE threadid_t tcb_t::get_pager()
{
    return get_utcb()->get_pager();
}

/**
 * Set new pager for a thread
 * @param tid   thread ID of new pager
 */
INLINE void tcb_t::set_pager(const threadid_t tid)
{
    get_utcb()->set_pager(tid);
}

/**
 * Get thread ID of a thread's exception handler
 * @return      thread ID of exception handler
 */
INLINE threadid_t tcb_t::get_exception_handler()
{
    return get_utcb()->get_exception_handler();
}

/**
 * Set new exception handler for a thread
 * @param tid   thread ID of new exception handler
 */
INLINE void tcb_t::set_exception_handler(const threadid_t tid)
{
    get_utcb()->set_exception_handler(tid);
}

/**
 * Get a thread's user-defined handle
 * @return      user-defined handle
 */
INLINE word_t tcb_t::get_user_handle()
{
    return get_utcb()->get_user_defined_handle();
}

/**
 * Set user-defined handle for a thread
 * @param handle        new value for user-defined handle
 */
INLINE void tcb_t::set_user_handle(const word_t handle)
{
    get_utcb()->set_user_defined_handle(handle);
}

/**
 * Set the actual sender ID of an IPC
 * @param tid   thread ID of actual sender
 */
INLINE void tcb_t::set_actual_sender (threadid_t tid)
{
    get_utcb()->set_virtual_sender(tid);
}

/**
 * Get the virtual sender of an IPC
 * @return      thread ID of virtual sender
 */
INLINE threadid_t tcb_t::get_virtual_sender (void)
{
    return get_utcb()->get_virtual_sender();
}

/**
 * Get the intended receiver of an IPC
 * @return      thread ID of intended receiver
 */
INLINE threadid_t tcb_t::get_intended_receiver (void)
{
    return get_utcb()->get_intended_receiver();
}

/**
 * Set the IPC error code
 * @param err   new IPC error code
 */
INLINE void tcb_t::set_error_code(word_t err)
{
    get_utcb()->set_error_code(err);
}

/**
 * Get the IPC error code
 * @return      IPC error code
 */
INLINE word_t tcb_t::get_error_code(void)
{
    return get_utcb()->get_error_code();
}

/**
 * Get a thread's preemption flags
 * @return      preemption flags
 */
INLINE preempt_flags_t tcb_t::get_preempt_flags (void)
{
    preempt_flags_t flags;
    flags.raw = get_utcb()->get_preempt_flags();
    return flags;
}

/**
 * Set a thread's preemption flags
 * @param flags new preemption flags
 */
INLINE void tcb_t::set_preempt_flags (preempt_flags_t flags)
{
    get_utcb()->set_preempt_flags(flags.raw);
}

/**
 * Get a thread's coprocessor flags
 * @return      coprocessor flags
 */
INLINE u8_t tcb_t::get_cop_flags (void)
{
    return get_utcb()->get_cop_flags();
}

/**
 * Get message tag
 * @returns message tag
 * The message tag will be read from message register 0
 */
INLINE msg_tag_t tcb_t::get_tag (void)
{
    msg_tag_t tag = get_mr(0);
    return tag;
}

/**
 * Set the message tag
 * @param tag   new message tag
 * The message tag will be stored in message register 0
 */
INLINE void tcb_t::set_tag (msg_tag_t tag)
{
    set_mr(0, tag.raw);
}

/**
 * Get a thread's send transfer timeout
 * @return      send transfer timeout
 */
INLINE time_t tcb_t::get_xfer_timeout_snd (void)
{
    return get_utcb()->get_xfer_timeout().get_snd();
}

/**
 * Get a thread's receive transfer timeout
 * @return      receive transfer timeout
 */
INLINE time_t tcb_t::get_xfer_timeout_rcv (void)
{
    return get_utcb()->get_xfer_timeout().get_rcv();
}


/**
 * enqueues the tcb into the send queue of tcb
 * @param tcb the thread control block to enqueue
 */
INLINE void tcb_t::enqueue_send(tcb_t * tcb, const bool head)
{
    //TRACEPOINT_TB(DEBUG, ("%t enqueue into send queue of %t", this, tcb));
    ASSERT( !queue_state.is_set(queue_state_t::send) );
    if (head)
	ENQUEUE_LIST_HEAD(tcb->send_head, this, send_list);
    else
	ENQUEUE_LIST_TAIL(tcb->send_head, this, send_list);
    queue_state.set(queue_state_t::send);
}

/**
 * dequeues the tcb from the send queue of tcb
 * @param tcb the thread control block to dequeue from
 */
INLINE void tcb_t::dequeue_send(tcb_t * tcb)
{
    ASSERT( queue_state.is_set(queue_state_t::send) );
    DEQUEUE_LIST(tcb->send_head, this, send_list);
    queue_state.clear(queue_state_t::send);
}

/**
 * enqueue a tcb into the present list
 * the present list primarily exists for debugging reasons, since task 
 * manipulations now happen on a per-thread basis */
#if defined(CONFIG_DEBUG)
extern tcb_t * global_present_list;
extern spinlock_t present_list_lock;
#endif

INLINE void tcb_t::enqueue_present()
{
#if defined(CONFIG_DEBUG)
    present_list_lock.lock();
    ENQUEUE_LIST_TAIL(global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE void tcb_t::dequeue_present()
{
#if defined(CONFIG_DEBUG)
    present_list_lock.lock();
    DEQUEUE_LIST(global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE arch_ktcb_t *tcb_t::get_arch()
{
    return &this->arch;
}

INLINE tcb_t * get_idle_tcb()
{
    extern tcb_t *__idle_tcb;
    return (tcb_t*)__idle_tcb;
}

INLINE tcb_t * get_dummy_tcb()
{
    extern tcb_t *__dummy_tcb;
    return (tcb_t*)__dummy_tcb;
}


/* 
 * include glue header file 
 */
#include INC_GLUE(tcb.h)


/**********************************************************************
 *
 *             global V4 thread management
 *
 **********************************************************************/

INLINE space_t * get_current_space()
{
    return get_current_tcb()->get_space();
}

#if !defined(CONFIG_SMP)
INLINE bool tcb_t::migrate_to_processor(cpuid_t processor)
{
    return processor == 0;
}
#endif

INLINE bool tcb_t::is_local_cpu()
{
    // on non-SMP always evaluates to true
    return (get_current_cpu() == get_cpu());
}

void handle_ipc_timeout (word_t state);


/**
 * creates a root server thread and a fresh space, if the 
 * creation fails the function does not return (assuming that root 
 * servers are functional necessary for the system)
 * 
 * @param dest_tid id of the thread to be created
 * @param scheduler_tid thread id of the scheduler
 * @param pager_tid thread id of the pager
 * @param utcb_area fpage describing the UTCB area
 * @param kip_area fpage describing the kernel interface page area
 * @return the newly created tcb
 */
tcb_t * create_root_server(threadid_t dest_tid, threadid_t scheduler_tid, 
			   threadid_t pager_tid, fpage_t utcb_area, fpage_t kip_area);

/**
 * initializes the root servers
 */
void init_root_servers();

/**
 * initializes the kernel threads
 */
void init_kernel_threads();


#endif /* !__API__V4__TCB_H__ */

