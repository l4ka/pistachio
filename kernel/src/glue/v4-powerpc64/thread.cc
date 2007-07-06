/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/thread.cc
 * Description:	Misc thread stuff.
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
 * $Id: thread.cc,v 1.4 2004/06/04 06:38:41 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(msr.h)
#include INC_ARCH(frame.h)
#include INC_API(tcb.h)
//#include INC_GLUE(tracepoints.h)

//#define TRACE_THREAD(x...)	TRACEF(x)
#define TRACE_THREAD(x...)

extern "C" void powerpc64_initial_to_user( void );

/**
 * Setup TCB to execute a function when switched to
 * @param func pointer to function
 *
 * The old stack state of the TCB does not matter.
 */
void tcb_t::create_startup_stack( void (*func)() )
{
    /* Re-init the stack */
    this->init_stack();

    this->notify(powerpc64_initial_to_user);

    this->notify(func);

    powerpc64_irq_context_t *context =
	    (powerpc64_irq_context_t *)get_stack_top() - 1;

    /* Set user mode */
    context->srr1 = MSR_USER_MODE;
    context->r13 = this->get_utcb_location();

    //TRACEF( "done %p\n", this );
}


