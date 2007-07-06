/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006,  Karlsruhe University
 *                
 * File path:     api/v4/exregs.cc
 * Description:   Iplementation of ExchangeRegisters() 
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
 * $Id: exregs.cc,v 1.12 2006/12/05 16:33:37 skoglund Exp $
 *                
 ********************************************************************/
#include INC_GLUE(syscalls.h)
#include INC_API(smp.h)
#include INC_API(schedule.h)

#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT (SYSCALL_EXCHANGE_REGISTERS);

void handle_ipc_error (void);
void thread_return (void);


#if defined(CONFIG_SMP)
static void perform_exregs (tcb_t * tcb, word_t * control, word_t * usp,
			    word_t * uip, word_t * uflags, threadid_t * pager,
			    word_t * uhandle);

/**
 * Handler invoked to pass the return values of an ExchangeRegisters()
 * back to the invoker of the syscall.
 */
static void do_xcpu_exregs_reply (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    
    //TRACEF ("current=%t, tcb=%t\n", get_current_tcb (), tcb);

    if (EXPECT_FALSE (! tcb->is_local_cpu ()))
    {
	// Forward request.
	xcpu_request (tcb->get_cpu (), do_xcpu_exregs_reply, tcb,
		      entry->param[0], entry->param[1], entry->param[2],
		      entry->param[3], entry->param[4], entry->param[5]);
	return;
    }

    if (EXPECT_FALSE
	(tcb->get_state () != thread_state_t::xcpu_waiting_exregs))
    {
	// Thread killed before exregs was completed.  Just ignore.
	return;
    }

    // Store exregs return values into TCB.
    threadid_t pager_tid;
    pager_tid.set_raw (entry->param[4]);

    tcb->misc.exregs.control =	entry->param[0];
    tcb->misc.exregs.sp =	entry->param[1];
    tcb->misc.exregs.ip	=	entry->param[2];
    tcb->misc.exregs.flags =	entry->param[3];
    tcb->misc.exregs.pager =	pager_tid;
    tcb->misc.exregs.user_handle = entry->param[5];

    // Reactivate thread.
    tcb->set_state (thread_state_t::running);
    get_current_scheduler ()->enqueue_ready (tcb);
}


/**
 * Handler invoked to perform ExchangeRegisers() on the remote CPU.
 */
static void do_xcpu_exregs (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    tcb_t * from = (tcb_t *) entry->param[0];

    //TRACEF ("current=%t, tcb=%t, from=%t\n", get_current_tcb (), tcb, from);

    if (EXPECT_FALSE (! tcb->is_local_cpu ()))
    {
	// Forward request.
	xcpu_request (tcb->get_cpu (), do_xcpu_exregs, tcb, entry->param[0],
		      entry->param[1], entry->param[2], entry->param[3],
		      entry->param[4], entry->param[5], entry->param[6]);
	return;
    }

    threadid_t pager_tid;
    pager_tid.set_raw (entry->param[5]);
    perform_exregs (tcb,
		    &entry->param[1],
		    &entry->param[2],
		    &entry->param[3],
		    &entry->param[4],
		    &pager_tid,
		    &entry->param[6]);

    // Pass return values back to invoker thread.
    xcpu_request (from->get_cpu (), do_xcpu_exregs_reply, from,
		  entry->param[1], entry->param[2], entry->param[3],
		  entry->param[4], pager_tid.get_raw (), entry->param[6]);
}


/**
 * Peform ExchangeRegisters() on a remore CPU.
 */
static void remote_exregs (tcb_t * tcb, word_t * control,
			   word_t * usp, word_t * uip, word_t * uflags,
			   threadid_t * pager, word_t * uhandle)
{
    tcb_t * current = get_current_tcb ();

    //TRACEF ("current=%t  tcb=%t\n", current, tcb);

    // Pass exregs request to remote CPU.
    xcpu_request (tcb->get_cpu (), do_xcpu_exregs, tcb, (word_t) current,
		  *control, *usp, *uip, *uflags, pager->get_raw (),
		  *uhandle);

    // Now wait for operation to complete.
    get_current_tcb ()->set_state (thread_state_t::xcpu_waiting_exregs);
    get_current_tcb ()->switch_to_idle ();

    // Grab exregs return values from tcb.
    *control =	current->misc.exregs.control;
    *usp = 	current->misc.exregs.sp;
    *uip = 	current->misc.exregs.ip;
    *uflags = 	current->misc.exregs.flags;
    *pager =	current->misc.exregs.pager;
    *uhandle =	current->misc.exregs.user_handle;
}
#endif /* CONFIG_SMP */


/**
 * Do the actual ExhangeRegisters() syscall.  Separated into a
 * separate function so that it can be invoked on a remote CPU.
 *
 * @param tcb		destination tcb
 * @param control	control parameter (in/out)
 * @param usp		stack pointer (in/out)
 * @param uip		instrunction pointer (in/out)
 * @param uflags	flags (in/out)
 * @param pager		pager (in/out)
 * @param uhandler	user defined handle (in/out)
 */
static void perform_exregs (tcb_t * tcb, word_t * control, word_t * usp,
			    word_t * uip, word_t * uflags, threadid_t * pager,
			    word_t * uhandle)
{
    word_t ctrl = *control;

    // Load return values before they are clobbered.
    word_t old_usp = (word_t) tcb->get_user_sp();
    word_t old_uip = (word_t) tcb->get_user_ip();
    word_t old_uhandle = tcb->get_user_handle();
    word_t old_uflags = tcb->get_user_flags();
    threadid_t old_pager = tcb->get_pager();
    word_t old_control = 0;

    if (ctrl & EXREGS_CONTROL_SP)
	tcb->set_user_sp ((addr_t) *usp);

    if (ctrl & EXREGS_CONTROL_IP)
	tcb->set_user_ip ((addr_t) *uip);

    if (ctrl & EXREGS_CONTROL_PAGER)
	tcb->set_pager (*pager);

    if (ctrl & EXREGS_CONTROL_UHANDLE)
	tcb->set_user_handle (*uhandle);

    if (ctrl & EXREGS_CONTROL_FLAGS)
	tcb->set_user_flags (*uflags);

    // Check if thread was IPCing
    if (tcb->get_state().is_sending())
    {
	old_control |= EXREGS_CONTROL_SEND;
	if (ctrl & EXREGS_CONTROL_SEND)
	{
	    tcb->unwind (tcb_t::abort);
	    tcb->set_state(thread_state_t::running);
	    tcb->notify(handle_ipc_error);
	    get_current_scheduler()->enqueue_ready(tcb);
	}
    }
    else if (tcb->get_state().is_receiving())
    {
	old_control |= EXREGS_CONTROL_RECV;
	if (ctrl & EXREGS_CONTROL_RECV)
	{
	    tcb->unwind (tcb_t::abort);
	    tcb->set_state(thread_state_t::running);
	    tcb->notify(handle_ipc_error);
	    get_current_scheduler()->enqueue_ready(tcb);
	}
    }

    // Check if we should resume the thread.
    if (tcb->get_state().is_halted())
    {
	old_control |= EXREGS_CONTROL_HALT;

	// If thread is halted - resume it.
	if ((ctrl & EXREGS_CONTROL_HALTFLAG) && !(ctrl & EXREGS_CONTROL_HALT))
	{
	    tcb->set_state(thread_state_t::running);
#warning VU: do we have to preserve prios on ex-regs?
	    get_current_scheduler()->enqueue_ready(tcb);
	}
    } 

    // Check if we should halt the thread.
    else if ((ctrl & EXREGS_CONTROL_HALTFLAG) && (ctrl & EXREGS_CONTROL_HALT))
    {
	if (tcb->get_state().is_running())
	{
	    // Halt a running thread
	    tcb->set_state(thread_state_t::halted);
	    if (tcb == get_current_tcb())
		get_current_tcb()->switch_to_idle();
	}
#warning eSk: Halting a thread with ongoing kernel operations is not supported
	// Need to halt the thread after any ongoing IPC or other
	// kernel operations are finished.  We can not let the thread
	// return to user level.
    }

    // Load up return values.
    *control =	old_control;
    *usp =	old_usp;
    *uip =	old_uip;
    *uflags =	old_uflags;
    *pager =	old_pager;
    *uhandle =	old_uhandle;
}


#if defined(CONFIG_X_PAGER_EXREGS)
FEATURESTRING ("pagerexregs");
#endif

static inline bool has_exregs_perms(tcb_t * dest, threadid_t dest_tid)
{
    space_t * space = get_current_space();
    // correct tid?
    if (dest->myself_global != dest_tid)
	return false;
    if (dest->get_space() == space)
	return true;

#if defined(CONFIG_X_PAGER_EXREGS)
    // all threads in pager address space can ex-regs
    threadid_t pager_tid = dest->get_pager(); // make copy
    tcb_t * pager = dest->get_space()->get_tcb(pager_tid);

    if ( pager->myself_global == pager_tid &&
	 pager->get_space() == space )
	return true;
#endif

    return false;
}


SYS_EXCHANGE_REGISTERS (threadid_t dest_tid, word_t control, 
			word_t usp, word_t uip, word_t uflags,
			word_t uhandle, threadid_t pager_tid,
			bool is_local)
{
    TRACEPOINT_TB (SYSCALL_EXCHANGE_REGISTERS,
		   ("sys_exchange_registers (dest=%t, control=%x)",
		    TID(dest_tid), control),
		   printf ("SYS_EXCHANGE_REGISTERS: dest=%t [%s], "
			   "control=0x%x [%c%c%c%c%c%c%c%c%c], "
			   "usp=%p, uip=%p, uflags=%p, pager=%t, "
			   "uhandle=%x\n", 
			   TID (dest_tid),
			   is_local ? "local" : "global",
			   control,
			   control & EXREGS_CONTROL_HALTFLAG? 'h' : '~',
			   control & EXREGS_CONTROL_PAGER   ? 'p' : '~',
			   control & EXREGS_CONTROL_UHANDLE ? 'u' : '~',
			   control & EXREGS_CONTROL_FLAGS   ? 'f' : '~',
			   control & EXREGS_CONTROL_IP      ? 'i' : '~',
			   control & EXREGS_CONTROL_SP      ? 's' : '~',
			   control & EXREGS_CONTROL_SEND    ? 'S' : '~',
			   control & EXREGS_CONTROL_RECV    ? 'R' : '~',
			   control & EXREGS_CONTROL_HALT    ? 'H' : '~',
			   usp, uip, uflags,
			   TID (pager_tid), uhandle));

    // Upon entry a local dest_tid will be converted into a global
    // thread ID before kernel entry.  If user somehow tricked kernel
    // entry with a local ID this will be handled in the test case
    // below.

    space_t * space = get_current_space();
    tcb_t * tcb = space->get_tcb(dest_tid);

    // Only allow exregs on:
    //  - active threads
    //  - in the same address space
    //  - with a valid thread ID.
    if ((! tcb->is_activated ()) || (! has_exregs_perms(tcb, dest_tid)) )
    {
	get_current_tcb ()->set_error_code (EINVALID_THREAD);
	return_exchange_registers (threadid_t::nilthread (), 0, 0, 0, 0,
				   threadid_t::nilthread (), 0);
    }

#if defined(CONFIG_SMP)
    if (! tcb->is_local_cpu ())
    {
	// Destination thread on remote CPU.  Must perform operation
	// remotely.
	remote_exregs (tcb, &control, &usp, &uip, &uflags,
		       &pager_tid, &uhandle);
    }
    else
#endif
    {
	// Destination thread on same CPU.  Perform operation immediately.
	perform_exregs (tcb, &control, &usp, &uip, &uflags,
			&pager_tid, &uhandle);
    }

    return_exchange_registers
	(is_local ? tcb->get_global_id () : tcb->get_local_id (),
	 control, usp, uip, uflags, pager_tid, uhandle);
}

