/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:    api/v4/threadstate.h 
 * Description:  thread state
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
 * $Id: threadstate.h,v 1.19 2004/06/02 18:03:29 stoess Exp $
 *
 *********************************************************************/
#ifndef __API__V4__THREADSTATE_H__
#define __API__V4__THREADSTATE_H__

/**
 * thread state definitions
 *
 * VU: they should go into the architecture specific part to allow
 *     special optimized encoding
 */
#define RUNNABLE_STATE(id)	((id << 1) | 0)
#define BLOCKED_STATE(id)	((id << 1) | 1)

/**
 * thread_state_t: current thread state
 */
class thread_state_t 
{
public:
    enum thread_state_e
    {
	running			= RUNNABLE_STATE(1),
	waiting_forever		= BLOCKED_STATE(~0UL),
	waiting_timeout		= BLOCKED_STATE(2),
	waiting_tunneled_pf	= BLOCKED_STATE(10),
	locked_waiting		= BLOCKED_STATE(3),
	locked_running		= RUNNABLE_STATE(4),
	locked_running_ipc_done	= RUNNABLE_STATE(9),
	locked_running_nested	= RUNNABLE_STATE(11),
	polling			= BLOCKED_STATE(5),
	halted			= BLOCKED_STATE(6),
	aborted			= BLOCKED_STATE(7),
	xcpu_waiting_deltcb    	= BLOCKED_STATE(8),
	xcpu_waiting_exregs    	= BLOCKED_STATE(12),
    };

    /* constructors */
    thread_state_t() {}; 
    thread_state_t(thread_state_e state) { this->state = state; }
    thread_state_t(word_t state) { this->state = (thread_state_e) state; }

    /* state manipulation */
    bool is_runnable();
    bool is_sending()
	{ return state == polling || state == locked_running; }
    bool is_receiving()
	{ return state == waiting_forever || state == waiting_timeout ||
	      state == locked_waiting; }
    bool is_halted()
	{ return state == halted; }
    bool is_aborted()
	{ return state == aborted; }
    bool is_running()
	{ return state == running; }
    bool is_waiting()
	{ return state == waiting_forever || state == waiting_timeout; }
    bool is_waiting_forever()
	{ return state == waiting_forever; }
    bool is_waiting_with_timeout()
	{ return state == waiting_timeout; }
    bool is_polling()
	{ return state == polling; }
    bool is_locked_running()
	{ return state == locked_running; }
    bool is_locked_waiting()
	{ return state == locked_waiting; }
    bool is_xcpu_waiting()
	{ return state == xcpu_waiting_deltcb ||
	      state == xcpu_waiting_exregs; }

    
    /* operators */
    bool operator == (const thread_state_t thread_state)
	{
	    return this->state == thread_state.state;
	}

    bool operator == (const thread_state_e state)
	{
	    return this->state == state;
	}

    bool operator != (const thread_state_t thread_state)
	{
	    return this->state != thread_state.state;
	}

    bool operator != (const thread_state_e state)
	{
	    return this->state != state;
	}

    operator word_t()
	{
	    return (word_t)this->state;
	}

    /* debugging */
    const char * string (void)
	{
	    switch (state) {
	    case running:			return "RUNNING ";
	    case waiting_forever:		return "WAIT_FE ";
	    case waiting_timeout:		return "WAIT_TO ";
	    case waiting_tunneled_pf:		return "WAIT_TP ";
	    case locked_waiting:		return "LOCK_WT ";
	    case locked_running:		return "LOCK_RU ";
	    case locked_running_ipc_done: 	return "LOCK_RD ";
	    case locked_running_nested:		return "LOCK_RN ";
	    case polling:			return "POLLING ";
	    case halted:			return "HALTED  ";
	    case aborted:			return "ABORTED ";
	    case xcpu_waiting_deltcb:		return "XCPU_DT ";
	    case xcpu_waiting_exregs:		return "XCPU_EX ";
	    default:				return "UNKNOWN ";
	    }
	}

private:
    thread_state_e state;
};

INLINE bool thread_state_t::is_runnable()
{
    /* invers logic - lowestmost bit no set means runnable */
    return !((word_t)this->state & 1);
}

#endif /* __API__V4__THREADSTATE_H__ */
