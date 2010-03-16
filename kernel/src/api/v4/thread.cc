/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     api/v4/thread.cc
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

#include INC_API(config.h)
#include INC_API(tcb.h)
#include INC_API(thread.h)
#include INC_API(interrupt.h)
#include INC_API(schedule.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_API(syscalls.h)
#include INC_API(smp.h)
#include INC_GLUE(syscalls.h)

#if defined(CONFIG_X_EVT_LOGGING)
#include INC_GLUE(logging.h)
#endif

#include <generic/lib.h>
#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYSCALL_THREAD_CONTROL);
DECLARE_KMEM_GROUP(kmem_tcb);

whole_tcb_t __whole_dummy_tcb  __attribute__((aligned(sizeof(whole_tcb_t))));
const tcb_t *__dummy_tcb = (const tcb_t *) &__whole_dummy_tcb;	    

#if defined(CONFIG_STATIC_TCBS)
tcb_t *tcb_t::tcb_array[TOTAL_KTCBS];
addr_t static_tcb_array;

#warning fixme: remove iteration over tcbs in is_tcb()
/**
 * Static TCBS, a linear array
 */
tcb_t* tcb_t::allocate(threadid_t dest)
{
    word_t idx = dest.get_threadno();
    ASSERT(idx < TOTAL_KTCBS);
    if (tcb_array[idx] == get_dummy_tcb())
	tcb_array[idx] = (tcb_t*) kmem.alloc(kmem_tcb, KTCB_SIZE);
    return tcb_array[idx];
}

void tcb_t::deallocate(threadid_t dest)
{
    word_t idx = dest.get_threadno();
    ASSERT(idx < TOTAL_KTCBS);
    tcb_t *tcb = tcb_array[idx];
    tcb_array[idx] = get_dummy_tcb();
    kmem.free(kmem_tcb, (addr_t)tcb, KTCB_SIZE);
}
void tcb_t::init_tcbs()
{
    // initialize thread array
    for (word_t i = 0; i < TOTAL_KTCBS; i++)
	tcb_array[i] = get_dummy_tcb();
    
    static_tcb_array = (addr_t) &tcb_t::tcb_array[0];
}
#endif /* defined(CONFIG_STATIC_TCBS) */



void handle_ipc_error (void);

bool tcb_t::is_interrupt_thread ()
{ 
    return (get_global_id().is_interrupt() && 
	    get_global_id().get_threadno() < get_kip()->thread_info.get_system_base()); 
}

/**
 * Stub invoked after a startup message has been received from the
 * thread's pager.
 */
static void thread_startup()
{
    tcb_t * current = get_current_tcb();
    msg_tag_t tag = current->get_tag();

    // Poke received IP/SP into exception frame (or whatever is used
    // by the architecture).  No need to check for valid IP/SP.
    // Thread will simply fault if values are not valid.
    // To avoid a mess when thread is set up via ctrlxfer items, 
    // set IP/SP only if pager sends at least 2 untyped words
    
    if (current->get_tag().get_untyped() == 2)
    {
	current->set_user_ip((addr_t)current->get_mr(1));
	current->set_user_sp((addr_t)current->get_mr(2));
    }

    TRACE_SCHEDULE_DETAILS("startup %t: ip=%p  sp=%p\n", current,
			    current->get_user_ip(), current->get_user_sp());
    
    current->set_saved_state (thread_state_t::aborted);
    current->set_state (thread_state_t::running);
}


/**
 * Fake that thread is waiting for IPC startup message.
 *
 * @param tcb		thread to wait for startup message
 * @param pager		thread id to receive startup message from
 */
static void fake_wait_for_startup (tcb_t * tcb, threadid_t pager)
{
    // Fake that we are waiting to receive untyped words from our
    // pager. 
    tcb->set_state (thread_state_t::waiting_forever);

    tcb->set_partner (pager);
    
    acceptor_t acceptor;
    acceptor.clear();
#if defined(CONFIG_X_CTRLXFER_MSG)
    acceptor.x.ctrlxfer = 1;
    acceptor.set_rcv_window(fpage_t::complete_mem());
#endif
    tcb->set_br(0, acceptor.raw);

    // Make sure that unwind will work on waiting thread.
    tcb->set_saved_partner (threadid_t::nilthread ());

    // Make sure that IPC abort will restore user-level exception
    // frame instead of trying to return from IPC syscall.
    tcb->set_saved_state (thread_state_t::running);
}


void thread_return()
{
    /* normal return function - do nothing */
}

void tcb_t::init(threadid_t dest, sktcb_type_e type)
{
    ASSERT(this);
    
    /* clear utcb and space */
    utcb = NULL;
    space = NULL;

    tcb_lock.init();

    /* make sure, nobody messes around with the thread */
    set_state(thread_state_t::aborted);
    partner = threadid_t::nilthread();
    init_saved_state();


    /* set thread id */
    myself_global = dest;
    myself_local = ANYLOCALTHREAD;

    /* initialize thread resources */
    resources.init(this);

    /* queue initialization */
    queue_state.init();
    send_head = NULL;

#if defined(CONFIG_SMP)
    /* initially assign to this CPU */
    cpu = get_current_cpu();
    lock_state.init(true);
#endif

    /* initialize scheduling */
    sched_state.init(type);

    /* enqueue into present list, do not enqueue 
     * the idle thread since they are CPU local and break 
     * the pointer list */
    if (this != get_idle_tcb())
	enqueue_present();
    
    init_stack();
}


void tcb_t::create_kernel_thread(threadid_t dest, utcb_t * utcb, sktcb_type_e type)
{
    //TRACEF("dest=%t\n", TID(dest));
    init(dest, type);
    this->utcb = utcb;
}

void tcb_t::create_inactive(threadid_t dest, threadid_t scheduler, sktcb_type_e type)
{
    //TRACEF("tcb=%t, %t\n", this, TID(dest));
    init(dest, type);
    sched_state.set_scheduler(scheduler);
}

bool tcb_t::activate(void (*startup_func)(), threadid_t pager)
{
    ASSERT(this);
    ASSERT(this->space);
    ASSERT(!this->is_activated());

    // UTCB location has already been checked during thread creation.
    // No need to check it again.  Just do an assert.
    ASSERT(check_utcb_location());

    //TRACEF("%t %x (%x)\n", TID(get_local_id()), get_utcb_location(), ((1 << get_kip()->utcb_info.alignment) - 1));

    /* allocate UTCB */
    this->utcb = get_space()->allocate_utcb(this);
    if (!this->utcb)
	return false;

    /* update global id in UTCB */
    set_global_id (this->get_global_id());

    /* initialize pager and exception handler */
    set_pager(pager);
    set_cpu(get_current_cpu());
    set_exception_handler(NILTHREAD);
    
#if defined(CONFIG_X_CTRLXFER_MSG)
    /* set default ctrlxfer items for faults */
    for(word_t fault = 0; fault < 4 + arch_ktcb_t::fault_max; fault++)
	fault_ctrlxfer[fault] = ctrlxfer_mask_t(0);
#endif

    /* initialize the startup stack */
    create_startup_stack(startup_func);
    return true;
}

/**
 * Check if supplied address is a valid UTCB location.  It is assumed
 * that the space of the thread is properly initialized.
 *
 * @param utcb_location		location of UTCB
 *
 * @return true if UCTB location is valid, false otherwise
 */
bool tcb_t::check_utcb_location (word_t utcb_location)
{
    return (get_kip()->utcb_info.is_valid_utcb_location(utcb_location) &&
		    get_space ()->get_utcb_page_area ().is_range_in_fpage
		    ((addr_t) utcb_location, (addr_t) (utcb_location + sizeof (utcb_t))));
}

/**
 * Check UTCB location of thread is valid.  It is assumed that the
 * space of the thread is properly initialized.
 *
 * @return true if UCTB location is valid, false otherwise
 */
bool tcb_t::check_utcb_location (void)
{
    return check_utcb_location (get_utcb_location ());
}


#if defined(CONFIG_SMP)
static void do_xcpu_delete_done(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    ASSERT(tcb);

    // still on same CPU? --> otherwise forward
    if (!tcb->is_local_cpu())
	UNIMPLEMENTED();

    if (tcb->get_state() != thread_state_t::xcpu_waiting_deltcb)
	UNIMPLEMENTED();

    tcb->xcpu_status = entry->param[0];
    tcb->set_state(thread_state_t::running);
    get_current_scheduler()->schedule(tcb);
}

static void idle_xcpu_delete(tcb_t *tcb, word_t src)
{
    tcb_t *src_tcb = (tcb_t *) src;

    tcb->delete_tcb();
    xcpu_request(src_tcb->get_cpu(), do_xcpu_delete_done, src_tcb, 0);
    
}

static void do_xcpu_delete(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    tcb_t * current = get_current_tcb();
    tcb_t * src = (tcb_t*)entry->param[0];
    word_t done = 1;

    // migrated meanwhile?
    if (tcb->is_local_cpu())
    {
        if (current == get_idle_tcb() || current == tcb)
	{
            // make sure we don't run on the deleted thread's ptab
            space_t::switch_to_kernel_space(get_current_cpu());
        }

        if (current == tcb){
	    get_idle_tcb()->notify(idle_xcpu_delete, tcb, (word_t) src);
	    get_current_scheduler()->schedule(get_idle_tcb(), sched_handoff);
	}

        tcb->delete_tcb();
        done = 0;

    }
    xcpu_request(src->get_cpu(), do_xcpu_delete_done, src, done);
    
    if (!get_current_scheduler()->get_accounted_tcb()) {
	get_current_scheduler()->set_accounted_tcb(current);
    }
}
#endif

void tcb_t::delete_tcb()
{
    ASSERT(this->exists());

#if defined(CONFIG_SMP) 
delete_tcb_retry:
    if ( !this->is_local_cpu() )
    {
	xcpu_request(this->get_cpu(), do_xcpu_delete, this, (word_t)get_current_tcb());
	get_current_tcb()->set_state(thread_state_t::xcpu_waiting_deltcb);
	get_current_scheduler()->schedule(get_idle_tcb(), sched_handoff);

	// wait for re-activation, if not successfull simply retry
	if (get_current_tcb()->xcpu_status)
	    goto delete_tcb_retry;
	return;
    }
#endif

    if ( is_activated() )
    {
	ASSERT(get_utcb());
	scheduler_t * sched = get_current_scheduler();

	// dequeue from ready queue
	sched->deschedule
	    (this);
	
	// unwind ongoing IPCs
	if (get_state().is_sending() || get_state().is_receiving())
	    unwind (tcb_t::abort);
	
	lock();
	
	// dequeue pending send requests
	while(send_head)
	{
	    send_head->dequeue_send(this);
	    // what do we do with these guys?
	}
	    
	unlock();
	    
	// free any used resources
	resources.free(this);
	
	// free UTCB
	this->utcb = NULL;

	// make sure that we don't get accounted anymore
	if (sched->get_accounted_tcb() == this)
	    sched->set_accounted_tcb(get_idle_tcb());
	
	// remove from requeue list
	sched_state.delete_tcb();
    }

    // clear ids
    this->myself_global = NILTHREAD;
    this->myself_local = ANYLOCALTHREAD;

    this->set_space(NULL);
    this->set_state(thread_state_t::aborted);
    dequeue_present();
}


/**
 * Caclulate the sender and receiver errorcodes when an IPC operation
 * has been aborted either by exchange_registers or by a timeout.
 *
 * @param reason		reason for abort (abort or timeout)
 * @param snd			sender thread
 * @param rcv			receiver thread
 * @param err_s			returned sender error code
 * @param err_r			returned receiver error code
 */
static void calculate_errorcodes (tcb_t::unwind_reason_e reason,
				  tcb_t * snd, tcb_t * rcv,
				  word_t * err_s, word_t * err_r)
{
    word_t offset = snd->misc.ipc_copy.copy_length;

    if (snd->get_space ()->is_copy_area (snd->misc.ipc_copy.copy_fault))
	offset += (word_t) snd->misc.ipc_copy.copy_fault -
	    (word_t) snd->misc.ipc_copy.copy_start_dst;
    else
	offset += (word_t) snd->misc.ipc_copy.copy_fault -
	    (word_t) snd->misc.ipc_copy.copy_start_src;

    if (reason == tcb_t::timeout)
    {
	time_t snd_to = snd->get_xfer_timeout_snd ();
	time_t rcv_to = rcv->get_xfer_timeout_rcv ();

	if (rcv_to < snd_to)
	{
	    *err_s = IPC_SND_ERROR (ERR_IPC_XFER_TIMEOUT_PARTNER (offset));
	    *err_r = IPC_RCV_ERROR (ERR_IPC_XFER_TIMEOUT_CURRENT (offset));
	}
	else
	{
	    *err_s = IPC_SND_ERROR (ERR_IPC_XFER_TIMEOUT_CURRENT (offset));
	    *err_r = IPC_RCV_ERROR (ERR_IPC_XFER_TIMEOUT_PARTNER (offset));
	}
    }
    else
    {
	*err_s = IPC_SND_ERROR (ERR_IPC_ABORTED (offset));
	*err_r = IPC_RCV_ERROR (ERR_IPC_ABORTED (offset));
    }
}


#if defined(CONFIG_SMP)

DECLARE_TRACEPOINT (IPC_XCPU_UNWIND);

/**
 * Handler invoked when our IPC partner has aborted/timed out the IPC.
 * Arguments are as follows:
 *
 * param[0]	thread id of partner
 * param[1]	expected thread state
 * param[2]	message tag to return
 * param[3]	error code to return
 */
static void do_xcpu_unwind_partner (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    threadid_t partner_id = threadid (entry->param[0]);
    thread_state_t expected_state (entry->param[1]);
    msg_tag_t tag = entry->param[2];
    word_t err = entry->param[3];

    TRACEPOINT (IPC_XCPU_UNWIND,
		"tcb=%t, partner=%t, state=%s, tag=%p, err=%p\n",
			TID (tcb->get_global_id ()), TID (partner_id),
			expected_state.string (), tag.raw, err);

    if (EXPECT_FALSE (! tcb->is_local_cpu ()))
    {
	// Forward request.
	xcpu_request (tcb->get_cpu (), do_xcpu_unwind_partner, tcb,
		      entry->param[0], entry->param[1],
		      entry->param[2], entry->param[3]);
	return;
    }

    if (EXPECT_FALSE (tcb->get_state () != expected_state) ||
	EXPECT_FALSE (tcb->get_partner () != partner_id))
    {
	// Request is outdated.
	return;
    }

    if (! tcb->get_saved_partner ().is_nilthread () &&
        ! tcb->get_saved_state ().is_running () )
    {
	// We have a nested IPC operation.  Perform another unwind.
	tcb->restore_state ();
	word_t e = (err >> 1) & 0x7;
	tcb->unwind ((e == 1 || e == 5 || e == 6) ?
		     tcb_t::timeout : tcb_t::abort);
    }
    else
    {
	tcb->set_error_code (err);
	tcb->set_tag (tag);
	tcb->set_state (thread_state_t::running);
    }

    // Reactivate thread.
    tcb->notify (handle_ipc_error);
    get_current_scheduler ()->schedule (tcb);
}

#endif /* CONFIG_SMP */


DECLARE_TRACEPOINT (IPC_UNWIND);

/**
 * Unwinds a thread from an ongoing IPC.
 *
 * @param reason		reason for unwind (abort or timeout)
 */
void tcb_t::unwind (unwind_reason_e reason)
{
    msg_tag_t tag = get_tag ();
    thread_state_t cstate;
    tcb_t * partner;

    TRACEPOINT (IPC_UNWIND,
		"Unwind: tcb=%t p=%t s=%s (saved: p=%t s=%s)\n",
			TID (get_global_id ()), TID (get_partner ()),
			get_state ().string (),	TID (get_saved_partner ()),
			get_saved_state ().string ());
    
    thread_state_t orig_cstate UNUSED = get_state ();
    thread_state_t orig_sstate UNUSED = get_saved_state ();
    threadid_t orig_cpartner = get_partner ();
    threadid_t orig_spartner = get_saved_partner ();

redo_unwind:

    cstate = get_state ();
    set_state (thread_state_t::running);
    partner = get_partner_tcb ();

    if (cstate.is_polling_or_waiting())
    {
	// IPC operation has not yet started.  I.e., partner is not
	// yet involved.

	sched_state.cancel_timeout();

	if (cstate.is_polling ())
	{
	    // The thread is enqueued in the send queue of the partner.
	    partner->lock ();
	    dequeue_send (partner);
	    partner->unlock ();
	}
	else
	{
	    // Thread is not yet receiving.  Cancel receive phase.
	    tag = tag.error_tag ();
	}

        if (! get_saved_partner ().is_nilthread () &&
            ! get_saved_state ().is_running () )
        {
	    // We're handling a nested IPC.
	    restore_state ();
	    goto redo_unwind;
	}

	// Set appropriate error code
	tag.set_error ();
	set_tag (tag);
	word_t err = (reason == timeout) ? ERR_IPC_TIMEOUT : ERR_IPC_CANCELED;
	set_error_code ((cstate.is_polling ()) ?
		       IPC_SND_ERROR (err) : IPC_RCV_ERROR (err));
	return;
    }
    else if (cstate == thread_state_t::waiting_tunneled_pf)
    {
	// We have tunneled a pagefault to our partner.  Inform the
	// partner that we're breaking off the IPC operation.

	// Get appropriate error codes
	word_t err_s, err_r;
	calculate_errorcodes (reason, this, partner, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner, partner,
			  get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_waiting,
			  tag.raw, err_r);
	}
	else
#endif
	{
	    // Reactivate partner directly
	    partner->set_error_code (err_r);
	    partner->set_tag (tag);
	    partner->set_state (thread_state_t::running);
	    partner->notify (handle_ipc_error);
	    get_current_scheduler ()->schedule (partner);
	}

	set_tag (tag);
	set_error_code (err_s);
	return;
    }

    else if (cstate == thread_state_t::locked_running_ipc_done)
    {
	// Nested pagefault (almost) completed.  No partner.
	restore_state ();
	goto redo_unwind;
    }

    else if (cstate == thread_state_t::locked_running)
    {
	// Thread is an active sender.  Abort both threads.

	word_t err_s, err_r;
	calculate_errorcodes (reason, this, partner, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_waiting,
			  tag.raw, err_r);
	}
	else
#endif
	{
	    // The receiver should be locked waiting.  It might also
	    // happen that we just completed a pagefault RPC without
	    // being scheduled yet.  In this case, our partner will
	    // not be communicating with us (and we should not abort
	    // any of his ongoing IPC operations).
	    if (partner->get_state () == thread_state_t::locked_waiting &&
		partner->get_partner () == get_global_id ())
	    {
		partner->set_error_code (err_r);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler ()->schedule (partner);
	    }
	}

        if (! get_saved_partner ().is_nilthread () &&
            ! get_saved_state ().is_running () )
        {
            restore_state ();
	    goto redo_unwind;
	}

	set_tag (tag);
	set_error_code (err_s);
	return;
    }

    else if (cstate == thread_state_t::locked_running_nested)
    {
	// Thread is handling tunneled pagefault, but IPC operation
	// has not yet started.  Abort both current thread and waiting
	// sender.

	word_t err_s, err_r;
	calculate_errorcodes (reason, partner, this, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::waiting_tunneled_pf,
			  tag.raw, err_s);
	}
	else
#endif
	{
	    // The sender should be waiting for tunneled page fault
	    if (partner->get_state() == thread_state_t::waiting_tunneled_pf)
	    {
		partner->set_error_code (err_s);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler()->schedule(partner);
	    }
	}

	set_tag (tag);
	set_error_code (err_r);
	return;
    }

    else if (cstate == thread_state_t::locked_waiting)
    {
	// Thread is an active receiver.  Abort both threads.

	word_t err_s, err_r;
	calculate_errorcodes (reason, partner, this, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_running,
			  tag.raw, err_s);
	}
	else
#endif
	{
	    // The sender should be locked running.
	    if (partner->get_state() == thread_state_t::locked_running)
	    {
		partner->set_error_code (err_s);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler()->schedule(partner);
	    }
	}

        if (! get_saved_partner ().is_nilthread () &&
            ! get_saved_state ().is_running () )
        {
	    restore_state ();
	    goto redo_unwind;
	}

	set_tag (tag);
	set_error_code (err_r);
	return;
    }

    else if (cstate.is_xcpu_waiting ())
    {
	// Waiting for xcpu TCB deletion or exregs completion.  Just
	// ignore.
	return;
    }

    else if (cstate.is_running ())
    {
	// May happen, e.g., when doing a redo_unwind on a synthesized
	// IPC (e.g., pagefault) that was aborted.
	return;
    }

    WARNING ("Unresolved unwind: tcb=%t p=%t s=%s (saved: p=%t s=%s)\n",
	     TID (get_global_id ()),
	     TID (orig_cpartner), orig_cstate.string (),
	     TID (orig_spartner), orig_sstate.string ());
    return;
}


bool tcb_t::migrate_to_space(space_t * space)
{
    ASSERT(space);
    ASSERT(get_space());
    space_t *old_space = get_space();
    
    lock();

    if (this->is_activated ())
    {
	// allocate utcb in destination space
	utcb_t * new_utcb = space->allocate_utcb (this);
	if (! new_utcb)
	    return false;

	// make sure nobody messes around with the tcb
	unwind (tcb_t::abort);

	memcpy (new_utcb, this->utcb, sizeof (utcb_t));
	this->utcb = new_utcb;
    }

    // remove from old space
    if (old_space->remove_tcb(this, get_current_cpu()))
    {
	old_space->free();
        space_t::free_space(old_space);
    }

    // now change the space
    this->set_space(space);
    space->add_tcb(this, get_current_cpu());

    unlock();

    return true;
}

#if defined(CONFIG_SMP)
/**
 * releases all resources held by the thread
 */
static void xcpu_release_thread(tcb_t * tcb)
{
    scheduler_t * sched = get_current_scheduler();

    // thread is on the current CPU
    sched->deschedule(tcb);

    // make sure that we don't get accounted anymore
    if (sched->get_accounted_tcb() == tcb)
	sched->set_accounted_tcb(get_idle_tcb());

    // remove from wakeup queue, we use the absolute timeout
    // as identifier whether a thread was in the wakeup list or not
    if (!tcb->queue_state.is_set(queue_state_t::wakeup))
	tcb->sched_state.set_timeout(0, false);
    else
	tcb->sched_state.cancel_timeout();

    tcb->resources.purge(tcb);

    // when migrating IRQ threads disable interrupts
    if (tcb->is_interrupt_thread())
	migrate_interrupt_start (tcb);
}


static void xcpu_put_thread(tcb_t * tcb, word_t processor)
{
    TRACE_SCHEDULE_DETAILS("migrating %t from %d -> %d (%s)", 
		    tcb, tcb->get_cpu(), processor, tcb->get_state().string());

    // dequeue all threads from requeue list and continue holding lock
    get_current_scheduler()->move_tcb(tcb, processor);
    
}

/**
 * remote handler for tcb_t::migrate_to_processor
 */
static void do_xcpu_set_thread(cpu_mb_entry_t * entry)
{
    if (!entry->tcb)
    {
	enter_kdebug("do_xcpu_set_thread");
	return;
    }

    entry->tcb->migrate_to_processor(entry->param[0]);
}

bool tcb_t::migrate_to_processor(cpuid_t processor)
{
    ASSERT(this);
    // check if the thread is already on that processor
    if (processor == this->get_cpu())
	return true;

    tcb_t * current = get_current_tcb();

    TRACE_SCHEDULE_DETAILS("%s %t %d", __func__, this, processor);

    if (EXPECT_FALSE( this->get_cpu() != get_current_cpu() ))
    {
	// thread not on current CPU (or migrated away meanwhile)
	xcpu_request(this->get_cpu(), do_xcpu_set_thread, this, processor);
    }
    else if ( this->get_cpu() != processor )
    {
	// thread is on local CPU and should be migrated
	xcpu_release_thread(this);

	// if we migrate ourself we use the idle thread to perform the notification
	if (current == this)
	{
	    get_idle_tcb()->notify(xcpu_put_thread, this, processor);
	    space_t::switch_to_kernel_space(get_current_cpu());
	    get_current_scheduler()->schedule(get_idle_tcb(), sched_handoff);
	}
	else 
	{
	    xcpu_put_thread(this, processor);

	    // schedule if we've been running on the thread's timeslice
	    if (!get_current_scheduler()->get_accounted_tcb()) 
		get_current_scheduler()->schedule();
	}
    }

    return true;
}
#endif /* CONFIG_SMP */

/**
 * Handler invoked when IPC errors (aborts or timeouts) occur.  Any
 * IPC resources are released and control is transferred to
 * user-level.  If the aborted IPC happens to be a pagefault IPC
 * (caused by user-level memory access) we need to restore the thread
 * prior to the pagefault and return to user-level.
 */
EXTERN_TRACEPOINT(IPC_ERROR);

void handle_ipc_error (void)
{
    tcb_t * current = get_current_tcb ();

    TRACEPOINT (IPC_ERROR, "handle ipc error %s %d\n", 
                current->get_saved_state().string(), (word_t) current->get_error_code());
                
    current->release_copy_area ();
    current->flags -= tcb_t::has_xfer_timeout;

    // We're going to skip the last part of the switch_to() function
    // invoked when switching from the current thread.  Make sure that
    // we still manage to load up the appropriate resources when
    // switching to the thread.

    if (EXPECT_FALSE (current->resource_bits))
	current->resources.load (current);


    if (current->get_saved_state ().is_running ())
    {
	// Thread was doing a pagefault IPC.  Restore thread state
	// prior to IPC operation and return directly to user-level.
	current->restore_state();
        current->set_partner(threadid_t::nilthread()); // sanity
	current->return_from_user_interruption ();
    }
    else
    {
        TRACEPOINT (IPC_ERROR, "ipc error ret from ipc\n");
	current->set_saved_state (thread_state_t::aborted); // sanity
        current->set_saved_partner (threadid_t::nilthread());
	current->return_from_ipc ();
    }

    /* NOTREACHED */
}


/**
 * Handler invoked when an IPC timeout has occured.  The handler
 * aborts any ongoing IPC operations (including partners) and returns
 * to user-level with an IPC error code.
 *
 * @param state		thread state when timeout occured
 */
void handle_ipc_timeout (word_t state)
{
    tcb_t * current = get_current_tcb ();
    //TRACEF("%t (%s, %s)\n", current, current->state.string(), current->get_saved_state().string());

    // Restore thread state when timeout occured
    current->set_state ((thread_state_t) state);

    current->unwind (tcb_t::timeout);
    current->set_state (thread_state_t::running);
    handle_ipc_error ();
}


void tcb_t::save_state ()
{ 
    TRACE_SCHEDULE_DETAILS("save_state %t p %t s %s e %x mr <%x:%x:%x>", this,
			   TID(get_partner()), get_state().string(),
			   get_error_code(), get_mr(0), get_mr(1), get_mr(2));
			   
			 
    ASSERT (get_saved_partner (IPC_NESTING_LEVEL-1) == threadid_t::nilthread ());
    ASSERT (get_saved_state (IPC_NESTING_LEVEL-1) == thread_state_t::aborted);

    for (int l = 1; l < IPC_NESTING_LEVEL; l++)
    {
	for (int i = 0; i < IPC_NUM_SAVED_MRS; i++)
	    misc.saved_state[l].mr[i] = misc.saved_state[l-1].mr[i];
	misc.saved_state[l].br0 = misc.saved_state[l-1].br0;
	misc.saved_state[l].error = misc.saved_state[l-1].error;
	misc.saved_state[l].partner = misc.saved_state[l-1].partner;
	misc.saved_state[l].vsender = misc.saved_state[l-1].vsender;
	misc.saved_state[l].state = misc.saved_state[l-1].state;

    }
    
    for (int i = 0; i < IPC_NUM_SAVED_MRS; i++)
	misc.saved_state[0].mr[i] = get_mr(i);
    
    misc.saved_state[0].br0 = get_br(0);
    misc.saved_state[0].error = get_error_code();
    misc.saved_state[0].partner = get_partner();
    misc.saved_state[0].vsender = get_virtual_sender();
    misc.saved_state[0].state = get_state();

}

void tcb_t::restore_state ()
{
    TRACE_SCHEDULE_DETAILS("restore_state %t sp %t ss %s se %x mr <%x:%x:%x>", this,
			   TID(get_saved_partner()), get_saved_state().string(),
			   misc.saved_state[0].error,
			   misc.saved_state[0].mr[0],
			   misc.saved_state[0].mr[1],
			   misc.saved_state[0].mr[2]);


    //x86_x32_dump_frame_tb(get_user_frame(this));

    for (int i = 0; i < IPC_NUM_SAVED_MRS; i++)
	set_mr (i, misc.saved_state[0].mr[i]);
    set_br (0, misc.saved_state[0].br0);
    set_partner (get_saved_partner ());
    set_state(get_saved_state ());
    set_error_code (misc.saved_state[0].error);
    set_actual_sender(misc.saved_state[0].vsender);
    
    for (int l = 1; l < IPC_NESTING_LEVEL; l++)
    {
	for (int i = 0; i < IPC_NUM_SAVED_MRS; i++)
	    misc.saved_state[l-1].mr[i] = misc.saved_state[l].mr[i];
	misc.saved_state[l-1].br0 = misc.saved_state[l].br0;
	misc.saved_state[l-1].error = misc.saved_state[l].error;
	misc.saved_state[l-1].partner = misc.saved_state[l].partner;
	misc.saved_state[l-1].state = misc.saved_state[l].state;
	misc.saved_state[l-1].vsender = misc.saved_state[l].vsender;
    }
    
    set_saved_partner (threadid_t::nilthread(), IPC_NESTING_LEVEL-1);
    set_saved_state (thread_state_t::aborted, IPC_NESTING_LEVEL-1);
    
#if defined(CONFIG_X_CTRLXFER_MSG)
    flags -= tcb_t::kernel_ctrlxfer_msg;
#endif
    
}

void tcb_t::send_pagefault_ipc (addr_t addr, addr_t ip,
				space_t::access_e access)
{
    save_state ();

    /* generate pagefault message */
    msg_tag_t tag;
    tag.set(0, 2, IPC_MR0_PAGEFAULT | 
	    ((access == space_t::read)      ? (1 << 2) : 0) |
	    ((access == space_t::write)     ? (1 << 1) : 0) |
	    ((access == space_t::execute)   ? (1 << 0) : 0) |
	    ((access == space_t::readwrite) ? (1 << 2)+(1 << 1) : 0));

    /* create acceptor for whole address space */
    acceptor_t acceptor = 0;
    acceptor.set_rcv_window(fpage_t::complete_mem());

#if defined(CONFIG_X_CTRLXFER_MSG)
    acceptor.x.ctrlxfer = 1;
    tag.x.typed += append_ctrlxfer_item(tag, 3);
#endif

    set_tag(tag);
    set_mr(1, (word_t)addr);
    set_mr(2, (word_t)ip);
    set_br(0, acceptor.raw);


    tag = do_ipc(get_pager(), get_pager(), timeout_t::never());
    if (tag.is_error())
    {
	printf("result tag = %p, ip = %p, addr = %p, errcode = %p\n", 
	       tag.raw, ip, addr, get_error_code());
	enter_kdebug("pagefault IPC error");
    }

    restore_state ();
}

bool tcb_t::send_preemption_ipc()
{
    u64_t time = get_current_scheduler()->get_current_time();
    threadid_t to;
    acceptor_t acceptor;
    msg_tag_t tag;

#warning preemption IPC with timeout never -- should be zero
    save_state ();

    tag = msg_tag_t::preemption_tag();
	
    /* generate preemption message */
    to = sched_state.get_scheduler();
    
    set_mr(1, (word_t) time);
    set_mr(2, (word_t)((time >> (BITS_WORD-1)) >> 1)); // Avoid gcc warn

    acceptor.raw = get_br(0);
#if defined(CONFIG_X_CTRLXFER_MSG)
    acceptor.x.ctrlxfer = 1;
    acceptor.set_rcv_window(fpage_t::complete_mem());
    tag.x.typed += append_ctrlxfer_item(tag, 3);
#endif

    set_tag(tag);
    set_br(0, acceptor.raw);
    
    tag = do_ipc(to, sched_state.get_scheduler(), timeout_t::never());

    restore_state ();

    if (tag.is_error())
    {
	enter_kdebug("preemption IPC error");
    }
    return tag.is_error();
}

#if defined(CONFIG_X_CTRLXFER_MSG)

word_t tcb_t::ctrlxfer(tcb_t *dst, msg_item_t item, word_t src_idx, word_t dst_idx, bool src_mr, bool dst_mr)
{
    word_t num_regs = 0;
    word_t ctrlxfer_item_id;
    msg_item_t ctrlxfer_item;
    ctrlxfer_mask_t ctrlxfer_mask;

    if (flags.is_set(tcb_t::kernel_ctrlxfer_msg))
    {
	/*
	 * on kernel we have inserted a single dummy ctrlxfer item with the
	 * fault id encoded to reduce the number of saved MRs space; we get the
	 * "real" items by inspecting the fault bitmasks
	 */
	ctrlxfer_mask = get_fault_ctrlxfer_items(item.get_ctrlxfer_id());
	ctrlxfer_item_id = lsb(ctrlxfer_mask);	
	ctrlxfer_item = ctrlxfer_item_t::fault_item((ctrlxfer_item_t::id_e) ctrlxfer_item_id);
	TRACE_CTRLXFER_DETAILS( "ctrlxfer kernel msg fault %d mask %x", 
				item.get_ctrlxfer_id(), (word_t) ctrlxfer_mask);
	
    }
    else
    {
	ctrlxfer_item_id = 1;
	ctrlxfer_mask += ctrlxfer_item_id;
	ctrlxfer_item = item;
    }

    do 
    {
	word_t id = ctrlxfer_item.get_ctrlxfer_id();
        word_t num = 0;
        word_t mask = ctrlxfer_item.get_ctrlxfer_mask();

        ctrlxfer_item_t::mask_hwregs(id, mask);
        TRACE_CTRLXFER_DETAILS( "ctrlxfer id %d %s mask %x ", id, ctrlxfer_item_t::get_idname(id), mask);
            
        if (src_mr)
        {
            if (dst_mr)
            {
                dst->set_mr(dst_idx++, ctrlxfer_item.raw);

                for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
                {
                    TRACE_CTRLXFER_DETAILS( "\t (m%06d->m%06d) -> %08x", src_idx+1, dst_idx, get_mr(src_idx+1));
                    dst->set_mr(dst_idx++, get_mr(src_idx++ +1));
                }
            }
            else
            {
                // skip ctrlxfer item
                src_idx++;
                /* transfer from src mrs to dst frame */
                num += (dst->arch.*(arch_ktcb_t::set_ctrlxfer_regs[id]))(id, mask, this, src_idx);
            }
        }
        else 
        { 
            if (dst_mr)
            {
                dst->set_mr(dst_idx++, ctrlxfer_item.raw);
                /* transfer from src frame to dst mrs */
                num += (arch.*arch_ktcb_t::get_ctrlxfer_regs[id])(id, mask, dst, dst_idx);
            }
            else 
            {
                TRACEF("Ignore frame2frame ctrlxfer");
            }
        }
                
        num_regs += 1 + num;
        ctrlxfer_mask -= ctrlxfer_item_id;
        ctrlxfer_item_id = lsb(ctrlxfer_mask);	
        ctrlxfer_item = ctrlxfer_item_t::fault_item((ctrlxfer_item_t::id_e) ctrlxfer_item_id);
        
    } while (ctrlxfer_mask);

    flags -= tcb_t::kernel_ctrlxfer_msg;

    return num_regs;
}

#endif /* defined(CONFIG_X_CTRLXFER_MSG) */


/**********************************************************************
 *
 *             global V4 thread management
 *
 **********************************************************************/

tcb_t SECTION(".init") * 
create_root_server(threadid_t dest_tid, threadid_t scheduler_tid, 
		   threadid_t pager_tid, fpage_t utcb_area, 
		   fpage_t kip_area, word_t utcb_location,
		   word_t ip, word_t sp)
{
    ASSERT(dest_tid.is_global());
    ASSERT(scheduler_tid.is_global());
    ASSERT(pager_tid.is_global() || pager_tid.is_nilthread());
    ASSERT(!utcb_area.is_nil_fpage() && !(kip_area.is_nil_fpage()));
    
    tcb_t * tcb = tcb_t::allocate(dest_tid);
    space_t * space = space_t::allocate_space();
    
    /* VU: we always assume these calls succeed for the root servers
     * if not - we are in deep shit anyway */
    ASSERT(space);
    ASSERT(tcb);
    
    tcb->arch_init_root_server(space, ip, sp);

    tcb->create_inactive(dest_tid, scheduler_tid, sktcb_root);
    space->init(utcb_area, kip_area);
    
    /* set the space */
    tcb->set_space(space);
    space->add_tcb(tcb, get_current_cpu());
    
    tcb->set_utcb_location (utcb_location);

    /* activate the guy */
    if (!tcb->activate(&thread_return, pager_tid))
	panic("failed to activate root server\n");

    /* set instruction and stack pointer */
    tcb->set_user_ip((addr_t) ip);
    tcb->set_user_sp((addr_t) sp);

    /* and off we go... */
    tcb->set_state(thread_state_t::running);
    
    get_current_scheduler()->schedule(tcb, sched_current);
    return tcb;
}


SYS_THREAD_CONTROL (threadid_t dest_tid, threadid_t space_tid,
		    threadid_t scheduler_tid, threadid_t pager_tid, 
		    word_t utcb_location)
{
    TRACEPOINT (SYSCALL_THREAD_CONTROL, 
		"SYS_THREAD_CONTROL: dest=%t, space=%t, "
			"scheduler=%t, pager=%t, utcb=%x\n", 
			TID (dest_tid), TID (space_tid), 
			TID (scheduler_tid), TID (pager_tid), 
			utcb_location);
    tcb_t * current = get_current_tcb();
    
    // Check privilege
    if (EXPECT_FALSE (! is_privileged_space(get_current_space())))
    {
	current->set_error_code (ENO_PRIVILEGE);
	return_thread_control(0);
    }

    // Check for valid thread id
    if (EXPECT_FALSE (! dest_tid.is_global()))
    {
	current->set_error_code (EINVALID_SPACE);
	return_thread_control(0);
    }

    tcb_t * dest_tcb = tcb_t::get_tcb(dest_tid);

    /* interrupt thread id ? */
    if (dest_tid.get_threadno() < get_kip()->thread_info.get_system_base())
    {
	if (EXPECT_TRUE (thread_control_interrupt (dest_tid, pager_tid)))
	    return_thread_control (1);
	current->set_error_code (EINVALID_THREAD);
    	return_thread_control (0);
    }

    /* do not allow the user to mess with kernel threads */
    if (EXPECT_FALSE (dest_tid.get_threadno() <
		      get_kip()->thread_info.get_user_base()))
    {
	current->set_error_code (EINVALID_THREAD);
    	return_thread_control (0);
    }

    if (space_tid.is_nilthread())
    {
	if (dest_tcb == current)
	{
	    // do not allow deletion of ourself
	    current->set_error_code (EINVALID_THREAD);
	    return_thread_control(0);
	}
	else if (dest_tcb->exists())
	{
	    space_t * space = dest_tcb->get_space();
	    cpuid_t cpu = dest_tcb->get_cpu();
		    
	    dest_tcb->delete_tcb();

	    if (space->remove_tcb(dest_tcb, cpu))
	    {
		// was the last thread
		space->free();
                space_t::free_space(space);
	    }
	    tcb_t::deallocate(dest_tid);


	    // schedule if we've been running on this thread's timeslice
	    if (!get_current_scheduler()->get_accounted_tcb()) 
		get_current_scheduler()->schedule();
	}

	return_thread_control(1);
    }
    else
    {
	/* 
	 * thread creation/modification 
	 * VU: since we are going to manipulate the thread allocate the tcb 
	 * before checking it. If it already exists this is a no-op, otherwise
	 * it saves unmapping the dummy tcb directly afterwards
	 */
	dest_tcb = tcb_t::allocate(dest_tid);
	// get the tcb of the space 
	tcb_t * space_tcb = tcb_t::get_tcb(space_tid);

	if (dest_tcb->exists())
	{
	    //TRACEF("thread modification (%t)\n", dest_tcb);
	    if (utcb_location != ~0UL)
	    {
		// do not allow modification of UTCB locations of 
		// already activated threads
		if (dest_tcb->is_activated() ||
		    ! dest_tcb->check_utcb_location (utcb_location))
		{
		    current->set_error_code (EUTCB_AREA);
		    return_thread_control (0);
		}
		dest_tcb->set_utcb_location(utcb_location);
	    }

	    // the hardest part first - space modifications
	    if (EXPECT_FALSE (space_tcb->get_global_id() != space_tid))
	    {
		current->set_error_code (EINVALID_SPACE);
		return_thread_control (0);
	    }

	    space_t * space = space_tcb->get_space();
	    if (dest_tcb->get_space() != space)
	    {
		// space migration
		if (dest_tcb->is_activated () &&
		    EXPECT_FALSE (! (space->is_initialized () &&
				     space_tcb->check_utcb_location
				     (dest_tcb->get_utcb_location ()))))
		{
		    current->set_error_code (EUTCB_AREA);
		    return_thread_control (0);
		}
		if (EXPECT_FALSE (! dest_tcb->migrate_to_space(space)))
		{
		    current->set_error_code (ENO_MEM);
		    return_thread_control (0);
		}
	    }

	    if (!pager_tid.is_nilthread())
	    {
		/* if the thread was inactive, setting the pager will
		 * activate the thread */
		if (!dest_tcb->is_activated())
		{
		    if (! dest_tcb->check_utcb_location ())
		    {
			current->set_error_code (EUTCB_AREA);
			return_thread_control (0);
		    }
		    if (! dest_tcb->activate(&thread_startup, pager_tid))
		    {
			current->set_error_code (ENO_MEM);
			return_thread_control (0);
		    }
		    fake_wait_for_startup (dest_tcb, pager_tid);
		}
		else
		    dest_tcb->set_pager(pager_tid);
	    }

	    if (!scheduler_tid.is_nilthread())
		dest_tcb->sched_state.set_scheduler(scheduler_tid);

	    // change global id
	    if (dest_tcb->get_global_id() != dest_tid)
		dest_tcb->set_global_id(dest_tid);

	    return_thread_control(1);

	}
	else
	{
	    /* on creation of a new space scheduler must not be nilthread */
	    if (EXPECT_FALSE (scheduler_tid.is_nilthread()))
	    {
		current->set_error_code (EINVALID_THREAD);
		return_thread_control (0);
	    }

	    /* if the thread is not created in a fresh space
	     * make sure the space id is valid */
	    if (EXPECT_FALSE ((dest_tid != space_tid) &&
			      (space_tcb->get_global_id() != space_tid)))
	    {
		current->set_error_code (EINVALID_SPACE);
		return_thread_control (0);
	    }

	    // For actively created threads, make sure that space is
	    // initialized and UTCB location is valid.
	    if (! pager_tid.is_nilthread ())
	    {
		// Check for initialized space
		if (dest_tid == space_tid ||
		    (! space_tcb->get_space ()->is_initialized ()))
		{
		    current->set_error_code (EINVALID_SPACE);
		    return_thread_control (0);
		}

		// Check for valid UTCB location
		if (utcb_location == ~0UL ||
		    (! space_tcb->check_utcb_location (utcb_location)))
		{
		    current->set_error_code (EUTCB_AREA);
		    return_thread_control (0);
		}
	    }

	    /* ok, we can create the thread */
	    dest_tcb->create_inactive(dest_tid, scheduler_tid, sktcb_user);

	    /* set UTCB location, the multiple checks are necessary to
	     * be compliant with the manual which says the operation
	     * succeeds or fails _completely_, do this after
	     * create_inactive -- it overwrites the local_tid */
	    if (utcb_location != ~0UL)
		dest_tcb->set_utcb_location(utcb_location);

	    space_t * space;
	    if (dest_tid != space_tid)
		space = space_tcb->get_space();
	    else
		space = space_t::allocate_space();

	    /* VU: at that point we must have a space.
	     * do we have to handle a failing allocate_space? */
	    ASSERT(space); 

	    // set the space for the tcb
	    dest_tcb->set_space (space);
	    space->add_tcb (dest_tcb, get_current_cpu());

	    // if pager is not nil the thread directly goes into an IPC
	    if (! pager_tid.is_nilthread() )
	    {
		if (!dest_tcb->activate(thread_startup, pager_tid))
		{
		    // clean up tcb
		    UNIMPLEMENTED();
		}
		fake_wait_for_startup (dest_tcb, pager_tid);
	    }
	    return_thread_control(1);
	}
    }

    /* NOTREACHED */
    spin_forever ();
}


static utcb_t kernel_utcb;

void SECTION(".init") init_kernel_threads()
{
    // Initialize the user base.  Currently simply leave some space.
    get_kip ()->thread_info.set_user_base
	(get_kip ()->thread_info.get_system_base () + 32);

    // Create a dummy kernel thread.
    threadid_t ktid;
    ktid.set_global_id (get_kip ()->thread_info.get_system_base (), 1);
    tcb_t * tcb = tcb_t::get_tcb (ktid);
    tcb->create_kernel_thread (ktid, &kernel_utcb, sktcb_lo);
    tcb->set_state (thread_state_t::aborted);
}


/**
 * initializes the root servers
 * Uses the configuration of the kernel interface page and sets up the
 * corresponding servers. At least sigma0 must be configured, otherwise the
 * function panics. The thread ids of sigma0, sigma1, and the root servers
 * are taken from the kip. Currently, UTCB and KIP area are compile-time 
 * constants.
 */
void SECTION(".init") init_root_servers()
{
    TRACE_INIT ("Initializing root servers\n");
    //ENABLE_TRACEPOINT(SCHEDULE_PM_IPC, ~0, ~0);
    //ENABLE_TRACE_SCHEDULE_DETAILS( ~0, ~0);
    
    word_t ubase = get_kip()->thread_info.get_user_base();
    tcb_t * tcb;

    fpage_t utcb_area = fpage_t::nilpage(), kip_area = fpage_t::nilpage();
    word_t size_utcb = 0;

    /* calculate size of UTCB area for root servers */
    
    while((1U << size_utcb) < max( get_kip()->utcb_info.get_minimal_size(), 
				   get_kip()->utcb_info.get_utcb_size() * ROOT_MAX_THREADS ))
	size_utcb++;

    utcb_area.set(ROOT_UTCB_START, size_utcb , 0, 0, 0);
    kip_area.set(ROOT_KIP_START, get_kip()->kip_area_info.get_size_log2(),
		 0, 0, 0);

    TRACE_INIT ("root-servers: utcb_area: %p (%dKB), kip_area: %p (%dKB)\n", 
		utcb_area.raw, utcb_area.get_size() / 1024, 
		kip_area.raw, kip_area.get_size() / 1024);

    // stop if system has no sigma0
    if (get_kip()->sigma0.mem_region.is_empty())
	panic ("Sigma0's memory region is empty, "
	       "system will not be functional.  Halting.\n");

    threadid_t sigma0, sigma1, root_server;
    sigma0.set_global_id(ubase, ROOT_VERSION);
    sigma1.set_global_id(ubase+1, ROOT_VERSION);
    root_server.set_global_id(ubase+2, ROOT_VERSION);
    
    TRACE_INIT ("Creating sigma0 (%t)\n", TID(sigma0));
    tcb = create_root_server(
	sigma0,			// tid and space
	root_server,		// scheduler
	NILTHREAD,		// pager
	utcb_area,
	kip_area,
	ROOT_UTCB_START,
	get_kip()->sigma0.ip,
	get_kip()->sigma0.sp);

    sigma0_space = tcb->get_space();
    
    /* start sigma1 */
    if (!get_kip()->sigma1.mem_region.is_empty())
    {
	TRACE_INIT ("Creating sigma1 (%t)\n", TID(sigma1));
	tcb = create_root_server(
	    sigma1,		// tid and space
	    root_server,	// scheduler
	    sigma0,		// pager
	    utcb_area,
	    kip_area,
	    ROOT_UTCB_START,
	    get_kip()->sigma1.ip,
	    get_kip()->sigma1.sp);

	sigma1_space = tcb->get_space();
    }

    /* start root task */
    if (!get_kip()->root_server.mem_region.is_empty())
    {
	TRACE_INIT ("Creating root server (%t)\n", TID(root_server));
	tcb = create_root_server(
	    root_server,	// tid and space
	    root_server,	// scheduler
	    sigma0,		// pager
	    utcb_area,
	    kip_area,
	    ROOT_UTCB_START,
	    get_kip()->root_server.ip,
	    get_kip()->root_server.sp);
	roottask_space = tcb->get_space();
    }
}
