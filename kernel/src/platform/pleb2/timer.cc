/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     platform/pleb2/timer.cc
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
 * $Id: timer.cc,v 1.3 2005/01/12 02:49:02 cvansch Exp $
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
    XSCALE_OS_TIMER_TSR = 0x01;  /* Clear timer */
    XSCALE_OS_TIMER_TCR = 0x00000000;

    get_current_scheduler()->handle_timer_interrupt();
}

SECTION(".init") void timer_t::init_global(void)
{
    UNIMPLEMENTED();
}

SECTION(".init") void timer_t::init_cpu(void)
{
    /* Turn on TURBO mode */
    __asm__  __volatile__ (
	"   mov	    r0,	    #1		    \n"
	"   mcr	    p14, 0, r0, c6, c0, 0   \n"
	::: "r0"
    );
    get_interrupt_ctrl()->register_interrupt_handler(XSCALE_IRQ_OS_TIMER, 
            handle_timer_interrupt);

    XSCALE_OS_TIMER_TSR = 0x0f;
    XSCALE_OS_TIMER_MR0 = TIMER_RATE / (1000000/TIMER_TICK_LENGTH);
    XSCALE_OS_TIMER_TCR = 0x00000000;
    XSCALE_OS_TIMER_IER = 0x01;	    /* Enable timer channel 0 */

    get_interrupt_ctrl()->unmask(XSCALE_IRQ_OS_TIMER);

    {
	word_t cccr = XSCALE_CLOCKS_CCCR;
	word_t x, y, z;
	word_t bus, cpu;

	switch (cccr & 0x1f)
	{
	case 0x1:   x = 27; break;
	case 0x3:   x = 36; break;
	case 0x5:   x = 45; break;
	default:    x = 0;  break;
	}
	bus = x * 3686400 / 1000;
	printf("Mem Speed = %dkHz\n", x * 3686400 / 1000);

	switch ((cccr >> 5) & 0x3)
	{
	case 0x1:   y = 1; break;
	case 0x2:   y = 2; break;
	case 0x3:   y = 4; break;
	default:    y = 0;  break;
	}
	printf("Run Speed = %dkHz\n", x * 3686400 * y / 1000);

	switch ((cccr >> 7) & 0x7)
	{
	case 0x2:   z = 10; break;
	case 0x3:   z = 15; break;
	case 0x4:   z = 20; break;
	case 0x6:   z = 30; break;
	default:    z = 0;  break;
	}
	cpu = x * 36864 * y * z / 100;
	printf("Turbo Speed = %dkHz\n", x * 36864 * y * z / 100);

	/* initialize V4 processor info */
	init_processor (0,  bus, cpu);
    }
}
