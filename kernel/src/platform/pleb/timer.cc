/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/pleb/timer.cc
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
 * $Id: timer.cc,v 1.6 2005/01/12 02:51:17 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_API(processor.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)
#include INC_PLAT(timer.h)

timer_t timer;

extern "C" void handle_timer_interrupt(word_t irq, arm_irq_context_t *context)
{
    get_current_scheduler()->handle_timer_interrupt();

    /* inaccurate */
    SA1100_OS_TIMER_OSSR = (1 << 0);
    SA1100_OS_TIMER_OSCR = 0;
    SA1100_OS_TIMER_OSMR0 = TIMER_PERIOD;
}

SECTION(".init") void timer_t::init_global(void)
{
    UNIMPLEMENTED();
}

SECTION(".init") void timer_t::init_cpu(void)
{
    word_t ccf = SA1000_POWER_PPCR & 0x01f;
    word_t freq;

    /* Enable clock switching */
    __asm__  __volatile__ (
	"   mcr	    p15, 0, r0, c15, c1, 2  \n"
	::: "r0"
    );

    // from SA1100-refman table 8-1
    switch (ccf) {
    case 0:	freq = 59000; break;
    case 1:	freq = 73700; break;
    case 2:	freq = 88500; break;
    case 3:	freq = 103200; break;
    case 4:	freq = 118000; break;
    case 5:	freq = 132700; break;
    case 6:	freq = 147500; break;
    case 7:	freq = 162200; break;
    case 8:	freq = 176900; break;
    case 9:	freq = 191700; break;
    case 10:	freq = 206400; break;
    case 11:	freq = 221200; break;
    default :	freq = 0; break;
    }
    /* initialize V4 processor info */
    init_processor (0, freq ? freq/2 : 0 /* bus */, freq);

    get_interrupt_ctrl()->register_interrupt_handler(SA1100_IRQ_OS_TIMER_0, 
            handle_timer_interrupt);

    SA1100_OS_TIMER_OSCR = 0;
    SA1100_OS_TIMER_OSMR0 = TIMER_PERIOD;
    SA1100_OS_TIMER_OIER = 0x00000001UL;

    get_interrupt_ctrl()->unmask(SA1100_IRQ_OS_TIMER_0);
}
