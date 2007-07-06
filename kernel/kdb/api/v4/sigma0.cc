/*********************************************************************
 *                
 * Copyright (C) 2002, 2006,  Karlsruhe University
 *                
 * File path:     kdb/api/v4/sigma0.cc
 * Description:   Sigma0 interaction
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
 * $Id: sigma0.cc,v 1.3 2006/12/05 15:23:15 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_API(kernelinterface.h)
#include INC_API(thread.h)
#include INC_API(schedule.h)
#include INC_API(space.h)
#include INC_API(ipc.h)
#include INC_API(tcb.h)


/*
 * Sigma0 extended protocol definitions.
 */
#define SIGMA0_EXTPROT_ID	(-1001)

enum sigma0_request_e {
    s0_verbose =	1,
    s0_dumpmem =	2,
};


static void sigma0_send (sigma0_request_e type, word_t arg = 0);


DECLARE_CMD_GROUP (s0_interact);


/**
 * Sigma0 interaction.
 */
DECLARE_CMD (cmd_sigma0, root, '0', "sigma0", "sigma0 interaction");

CMD (cmd_sigma0, cg)
{
    return s0_interact.interact (cg, "sigma0");
}


/**
 * Change sigma0 verboseness.
 */
DECLARE_CMD (cmd_s0_verbose, s0_interact, 'v', "verbose",
	     "change sigma0 verboseness");

CMD (cmd_s0_verbose, cg)
{
    sigma0_send (s0_verbose, get_dec ("Verbose level", 1));
    return CMD_QUIT;
}


/**
 * Dump sigma0 memory pools.
 */
DECLARE_CMD (cmd_s0_dumpmem, s0_interact, 'm', "dumpmem",
	     "dump sigma0 memory pools");

CMD (cmd_s0_dumpmem, cg)
{
    sigma0_send (s0_dumpmem);
    return CMD_QUIT;
}


static void sigma0_ipc (word_t type, word_t arg)
{
    tcb_t * current = get_current_tcb ();

    // Create message.
    msg_tag_t tag;
    tag.set (0, 2, (word_t) SIGMA0_EXTPROT_ID << 4);
    current->set_mr (0, tag.raw);
    current->set_mr (1, type);
    current->set_mr (2, arg);

    // Send to sigma0.
    threadid_t s0id;
    s0id.set_global_id (get_kip ()->thread_info.get_user_base (), 1);
    tag = current->do_ipc (s0id, NILTHREAD, timeout_t::never());

    // Abort kernel thread execution.
    current->set_space (NULL);
    current->set_state (thread_state_t::aborted);
    get_current_scheduler ()->dequeue_ready (current);
    get_current_scheduler ()->set_priority (current, 0);
    current->switch_to_idle ();
}


/**
 * Send a two word IPC message from a kernel thread to sigma0 using an
 * extended sigma0 protocol.
 *
 * @param type		type of message to send
 * @param arg		argument of message
 */
static void sigma0_send (sigma0_request_e type, word_t arg)
{
    threadid_t ktid;
    ktid.set_global_id (get_kip ()->thread_info.get_system_base (), 1);

    // Make kernel thread invoke IPC sending stub.
    tcb_t * tcb = get_kernel_space ()->get_tcb (ktid);
    tcb->init_stack ();
    tcb->notify (sigma0_ipc, (word_t) type, arg);

    // Make kernel thread run on highest prio.
    get_current_scheduler ()->set_priority (tcb, MAX_PRIO);
    tcb->set_space (get_kernel_space ());
    tcb->set_state (thread_state_t::running);
    get_current_scheduler ()->enqueue_ready (tcb);
}
