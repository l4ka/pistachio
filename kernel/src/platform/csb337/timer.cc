/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/csb337/timer.cc
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
 * $Id: timer.cc,v 1.2 2004/08/21 13:31:33 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)
#include INC_PLAT(timer.h)

timer_t timer;
extern "C" void intctrl_t_handle_irq(word_t , arm_irq_context_t *);

struct timer_at91rm9200 {
    word_t  st_cr;	/* Control Register */
    word_t  st_pimr;	/* Period Interval Mode Register */
    word_t  st_wdmr;	/* Watchdog Mode Register */
    word_t  st_rtmr;	/* Real-time Mode Register */
    word_t  st_sr;	/* Status Register */
    word_t  st_ier;	/* Interrupt Enable Register */
    word_t  st_idr;	/* Interrupt Disable Register */
    word_t  st_imr;	/* Interrupt Mask Register */
    word_t  st_rtar;	/* Real-time Alarm Register */
    word_t  st_crtr;	/* Current Real-time Register */
};

static volatile struct timer_at91rm9200* st =
	    (struct timer_at91rm9200*)ST_VADDR;

extern "C" void handle_timer_interrupt(word_t irq, arm_irq_context_t *context)
{
    word_t status = st->st_sr;

    if (status & ST_SR_PITS)
    {
	get_current_scheduler()->handle_timer_interrupt();

	if ((status & st->st_imr) & (~ST_SR_PITS))
	    intctrl_t_handle_irq(irq, context);
	else
	    get_interrupt_ctrl()->ack(irq);
    } else
    {
	intctrl_t_handle_irq(irq, context);
    }
}

void timer_t::init_global(void)
{
    UNIMPLEMENTED();
}

void timer_t::init_cpu(void)
{
    get_interrupt_ctrl()->register_interrupt_handler(AIC_IRQ_SYS, 
            handle_timer_interrupt);

    st->st_pimr = TIMER_PERIOD;
    st->st_idr = ~(0ul);	/* Disable all system interrupts */
    st->st_ier = ST_SR_PITS;	/* Enable timer interrupts */

    get_interrupt_ctrl()->unmask(AIC_IRQ_SYS);
}
