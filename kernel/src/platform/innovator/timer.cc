/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/innovator/timer.cc
 * Description:   Periodic timer handling
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
 * $Id: timer.cc,v 1.3 2004/08/12 11:16:18 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)
#include INC_PLAT(timer.h)
#include INC_PLAT(reg.h)

timer_t timer;

extern "C" void
handle_timer_interrupt (word_t irq, arm_irq_context_t * context)
{
    get_current_scheduler ()->handle_timer_interrupt ();
}

void
timer_t::init_global (void)
{
    UNIMPLEMENTED ();
}

void
timer_t::init_cpu (void)
{
    /* register irq handler.*/
    get_interrupt_ctrl ()->register_interrupt_handler (TIMER1_IRQ,
						       handle_timer_interrupt);

    /* Use externel 12MHz clock.*/
    REG_OMAP_CKCTL &= ~(1 << 12);
    /* Allow timers to run while IDLE */
    REG_OMAP_IDLECT1 &= ~(1 << 9);
    /* Enable Timer clock */
    REG_OMAP_IDLECT2 |= (1 << 7);

    /* Using prescale of 16, that gives 1.33us per tick. Autoreload.*/
    REG_TIMER_CTRL = 0xE;

    /* set 7500 * 1.33 us = 10000us, which is TIMER_TICK_LENGTH.*/
    REG_TIMER_LOAD = 7500;

    /* enable irq.*/
    get_interrupt_ctrl ()->unmask (TIMER1_IRQ);

    /* start timer.*/
    REG_TIMER_CTRL |= 0x21;
}
