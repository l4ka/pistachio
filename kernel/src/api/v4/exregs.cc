/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006-2008, 2010,  Karlsruhe University
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
#if defined(CONFIG_X_CTRLXFER_MSG)
EXTERN_TRACEPOINT(IPC_CTRLXFER_ITEM);
#endif

void handle_ipc_error (void);
void thread_return (void);

static bool perform_exregs (tcb_t *src, tcb_t * dst, exregs_ctrl_t * control, word_t * usp,
			    word_t * uip, word_t * uflags, threadid_t * pager,
			    word_t * uhandle);

#if defined(CONFIG_SMP)

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
    get_current_scheduler()->schedule(tcb);
}


/**
 * Handler invoked to perform ExchangeRegisers() on the remote CPU.
 */
static void do_xcpu_exregs (cpu_mb_entry_t * entry)
{
    tcb_t * dst = entry->tcb;
    tcb_t * from = (tcb_t *) entry->param[0];

    //TRACEF ("%t %t\n", get_current_tcb(), dst);

    if (EXPECT_FALSE (! dst->is_local_cpu ()))
    {
	// Forward request.
	xcpu_request (dst->get_cpu (), do_xcpu_exregs, dst, entry->param[0],
		      entry->param[1], entry->param[2], entry->param[3],
		      entry->param[4], entry->param[5], entry->param[6]);
	return;
    }

    threadid_t pager_tid;
    pager_tid.set_raw (entry->param[5]);
    exregs_ctrl_t ctrl(entry->param[1]);
    
    bool reschedule = perform_exregs (from, dst,
				      &ctrl,
				      &entry->param[2],
				      &entry->param[3],
				      &entry->param[4],
				      &pager_tid,
				      &entry->param[6]
				      );
    
    
    // Pass return values back to invoker thread.
    xcpu_request (from->get_cpu (), do_xcpu_exregs_reply, from,
		  ctrl.raw, entry->param[2], entry->param[3],
		  entry->param[4], pager_tid.get_raw (), entry->param[6]);
    
    if (reschedule)
	get_current_scheduler ()->schedule ();
}


/**
 * Peform ExchangeRegisters() on a remote CPU.
 */
static void remote_exregs (tcb_t *current, tcb_t * dst, word_t * control,
			   word_t * usp, word_t * uip, word_t * uflags,
			   threadid_t * pager, word_t * uhandle)
{
    //TRACEF ("current=%t tcb=%t\n", current, dst);
    
    // Pass exregs request to remote CPU.
    xcpu_request (dst->get_cpu (), do_xcpu_exregs, dst, (word_t) current,
		  *control, *usp, *uip, *uflags, pager->get_raw (),
		  *uhandle);

    // Now wait for operation to complete.
    current->set_state(thread_state_t::xcpu_waiting_exregs);
    get_current_scheduler()->schedule(get_idle_tcb(), sched_handoff);

    // Grab exregs return values from tcb.
    *control =	current->misc.exregs.control;
    *usp = 	current->misc.exregs.sp;
    *uip = 	current->misc.exregs.ip;
    *uflags = 	current->misc.exregs.flags;
    *pager =	current->misc.exregs.pager;
    *uhandle =	current->misc.exregs.user_handle;
    
    // Reinitialize state 
    current->init_saved_state();
}

#endif /* CONFIG_SMP */


/**
 * Do the actual ExhangeRegisters() syscall.  Separated into a
 * separate function so that it can be invoked on a remote CPU.
 *
 * @return if destination thread should be scheduled
 * @param tcb		destination tcb
 * @param control	control parameter (in/out)
 * @param usp		stack pointer (in/out)
 * @param uip		instrunction pointer (in/out)
 * @param uflags	flags (in/out)
 * @param pager		pager (in/out)
 * @param uhandler	user defined handle (in/out)
 */
static bool perform_exregs (tcb_t *src, tcb_t * dst, exregs_ctrl_t * control, word_t * usp,
			    word_t * uip, word_t * uflags, threadid_t * pager,
			    word_t * uhandle)
{
    //TRACEF("perform_exregs %x\n", dest);
    exregs_ctrl_t ctrl = *control;

    // Load return values before they are clobbered.
    word_t old_usp = (word_t) dst->get_user_sp();
    word_t old_uip = (word_t) dst->get_user_ip();
    word_t old_uhandle = dst->get_user_handle();
    word_t old_uflags = dst->get_user_flags();
    threadid_t old_pager = dst->get_pager();
    exregs_ctrl_t old_control = 0;
    
    bool reschedule = false;

    UNUSED word_t src_idx = 1;
    
#if defined(CONFIG_X_CTRLXFER_MSG) 
    word_t items = 0;
    msg_item_t src_item;
    acceptor_t acceptor = dst->get_br(0);

    if (ctrl.is_set(exregs_ctrl_t::ctrlxfer_conf_flag))
    {
	do 
	{
	    src_item.raw = src->get_mr(src_idx++);
	    
	    if (!src_item.is_ctrlxfer_item())
		break;
	    
	    TRACEPOINT(IPC_CTRLXFER_ITEM, "ctrlxfer item: conf %t->%t fault=%d, id_mask=%x",
		       src, dst, src_item.get_ctrlxfer_id(), src_item.get_ctrlxfer_mask());
	    
	    dst->set_fault_ctrlxfer_items( src_item.get_ctrlxfer_id(), 
					   ctrlxfer_mask_t(src_item.get_ctrlxfer_mask()));	
		    
	} while (src_item.more_ctrlxfer_items());
	
    }
    if (ctrl.is_set(exregs_ctrl_t::ctrlxfer_read_flag))
    {
	do 
	{
	    src_item.raw = src->get_mr(src_idx);
	    
	    if (!src_item.is_ctrlxfer_item() || !acceptor.accept_ctrlxfer())
		break;
	    
    	    TRACEPOINT(IPC_CTRLXFER_ITEM,
		       "ctrlxfer item: read %t->%t id=%d, mask=%x (m->%c)",
		       src, dst, 
		       src_item.get_ctrlxfer_id(), src_item.get_ctrlxfer_mask(),
		       acceptor.accept_ctrlxfer() ? 'f' : 'm');
	    
	    if( (items = dst->ctrlxfer(src, src_item, 0, src_idx, false, true)) == 0)
		break;

	    src_idx += items;
	    
	} while (src_item.more_ctrlxfer_items());

    }
    if (ctrl.is_set(exregs_ctrl_t::ctrlxfer_write_flag))
    {
	do 
	{
	    src_item.raw = src->get_mr(src_idx);
	    
	    if (!src_item.is_ctrlxfer_item() || !acceptor.accept_ctrlxfer())
		break;
	    
    	    TRACEPOINT(IPC_CTRLXFER_ITEM,
		       "ctrlxfer item: write %t->%t id=%d, mask=%x (m->%c)",
		       src, dst, 
		       src_item.get_ctrlxfer_id(), src_item.get_ctrlxfer_mask(),
		       acceptor.accept_ctrlxfer() ? 'f' : 'm');
	    
	    if( (items = src->ctrlxfer(dst, src_item, src_idx, 0, true, false)) == 0)
		break;

	    src_idx += items; 

	} while (src_item.more_ctrlxfer_items());
		
    }

#endif
    
    if (ctrl.is_set(exregs_ctrl_t::sp_flag))
	dst->set_user_sp ((addr_t) *usp);
    
    if (ctrl.is_set(exregs_ctrl_t::ip_flag))
	dst->set_user_ip ((addr_t) *uip);
    
    if (ctrl.is_set(exregs_ctrl_t::flags_flag))
	dst->set_user_flags (*uflags);

    if (ctrl.is_set(exregs_ctrl_t::pager_flag))
	dst->set_pager (*pager);

    if (ctrl.is_set(exregs_ctrl_t::uhandle_flag))
	dst->set_user_handle (*uhandle);

    
    // Check if thread was IPCing
    if (dst->get_state().is_sending())
    {
	old_control.set(exregs_ctrl_t::send_flag);
	if (ctrl.is_set(exregs_ctrl_t::send_flag))
	{
	    dst->unwind (tcb_t::abort);
	    dst->set_state(thread_state_t::running);
	    dst->notify(handle_ipc_error);
	    get_current_scheduler()->schedule(dst, sched_current);
	    reschedule = true;
	}
    }
    else if (dst->get_state().is_receiving())
    {
	old_control.set(exregs_ctrl_t::recv_flag);
	if (ctrl.is_set(exregs_ctrl_t::recv_flag))
	{
	    dst->unwind (tcb_t::abort);
	    dst->set_state(thread_state_t::running);
	    dst->notify(handle_ipc_error);
	    get_current_scheduler()->schedule(dst, sched_current);
	    reschedule = true;
	}
    }

    // Check if we should resume the thread.
    if (dst->get_state().is_halted())
    {
	old_control.set(exregs_ctrl_t::halt_flag);

	// If thread is halted - resume it.
	if ((ctrl.is_set(exregs_ctrl_t::haltflag_flag)) && !(ctrl.is_set(exregs_ctrl_t::halt_flag)))
	{
	    dst->set_state(thread_state_t::running);
	    get_current_scheduler()->schedule(dst, sched_current);
	    reschedule = true;
	}
    } 

    // Check if we should halt the thread.
    else if ((ctrl.is_set(exregs_ctrl_t::haltflag_flag)) && (ctrl.is_set(exregs_ctrl_t::halt_flag)))
    {
	if (dst->get_state().is_running())
	{
	    // Halt a running thread
	    dst->set_state(thread_state_t::halted);
	    reschedule = true;
	}
	else
	{
	    printf("Halting a thread with ongoing kernel operations is not supported");
	    enter_kdebug("UNIMPLEMENTED");
	}
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
    
    return reschedule;
    
}



#if defined(CONFIG_X_PAGER_EXREGS)
FEATURESTRING ("pagerexregs");
#endif
#if defined(CONFIG_X_CTRLXFER_MSG)
FEATURESTRING ("ctrlxfer");
#endif

static inline bool has_exregs_perms(tcb_t * dst, threadid_t dst_tid)
{
    space_t * space = get_current_space();
    // correct tid?
    if (dst->myself_global != dst_tid)
	return false;
    if (dst->get_space() == space)
	return true;

#if defined(CONFIG_X_PAGER_EXREGS)
    // all threads in pager address space can ex-regs
    threadid_t pager_tid = dst->get_pager(); // make copy
    tcb_t * pager = tcb_t::get_tcb(pager_tid);

    if ( pager->myself_global == pager_tid &&
	 pager->get_space() == space )
	return true;
#endif
#if defined(CONFIG_X_CTRLXFER_MSG)
    if ( is_privileged_space(space))
	return true;
#endif
    return false;
}


SYS_EXCHANGE_REGISTERS (threadid_t dst_tid, word_t control, 
			word_t usp, word_t uip, word_t uflags,
			word_t uhandle, threadid_t pager_tid,
			bool is_local)
{

    tcb_t *current = get_current_tcb();
    exregs_ctrl_t ctrl(control);
    
    TRACEPOINT (SYSCALL_EXCHANGE_REGISTERS,
		"SYS_EXCHANGE_REGISTERS: current %t, dst=%t [%s], control=0x%x [%s]"
		", usp=%p, uip=%p, uflags=%p, pager=%t, uhandle=%x\n", 
		current, TID(dst_tid), is_local ? "local" : "global",
		ctrl.raw, ctrl.string(), usp, uip, uflags, TID(pager_tid), uhandle);

    // Upon entry a local dst_tid will be converted into a global
    // thread ID before kernel entry.  If user somehow tricked kernel
    // entry with a local ID this will be handled in the test case
    // below.
    tcb_t * dst = tcb_t::get_tcb(dst_tid);

    // Only allow exregs on:
    //  - active threads
    //  - in the same address space
    //  - with a valid thread ID.

    if ((! dst->is_activated ()) || (! has_exregs_perms(dst, dst_tid)) )
    {
    
	current->set_error_code (EINVALID_THREAD);
	return_exchange_registers (threadid_t::nilthread (), 0, 0, 0, 0,
				   threadid_t::nilthread (), 0);
    }
 
#if defined(CONFIG_SMP)
    if (! dst->is_local_cpu ())
    {
	// Destination thread on remote CPU.  Must perform operation
	// remotely.
	remote_exregs (current, dst, &ctrl.raw, &usp, &uip, &uflags,
		       &pager_tid, &uhandle);
    }
    else
#endif
    {
	// Dstination thread on same CPU.  Perform operation immediately.
	if (perform_exregs (current, dst, &ctrl, &usp, &uip, &uflags,
			    &pager_tid, &uhandle))
	    get_current_scheduler()->schedule();
	    
    }

    return_exchange_registers
	(is_local ? dst->get_global_id () : dst->get_local_id (),
	 ctrl.raw, usp, uip, uflags, pager_tid, uhandle);

}

