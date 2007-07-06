/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/timer.cc
 * Description:   Timer interrupt handler
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
 * $Id: timer.cc,v 1.3 2003/09/24 19:05:35 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(itc_timer.h)
#include INC_ARCH(cr.h)
#include INC_API(schedule.h)
#include INC_GLUE(intctrl.h)

itc_ptimer_t itc_ptimer UNIT ("cpulocal");

extern "C" void handle_timer_interrupt (word_t vector,
					ia64_exception_context_t * frame)
{
    word_t itm = cr_get_itm ();

    do {
	itm += get_itc_ptimer ()->get_rate ();
	cr_set_itm (itm);
    } while (ar_get_itc () > itm);

    get_current_scheduler()->handle_timer_interrupt();
}

void itc_ptimer_t::init_global (word_t vector)
{
    get_interrupt_ctrl ()->register_handler (vector, handle_timer_interrupt);
}
