/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/timer.cc
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
 * $Id: timer.cc,v 1.12 2004/04/05 06:19:28 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)

#include INC_ARCH(addrspace.h)
#include INC_ARCH(mips_cpu.h)

timer_t timer UNIT("cpulocal");

extern void putc(char chr);

extern "C" void handle_timer_interrupt(word_t irq, mips64_irq_context_t * frame)
{
    s64_t compare = (u32_t)get_timer()->compare;
    u32_t counter = (u32_t)read_32bit_cp0_register(CP0_COUNT);

    s64_t difference = (counter < compare) ? (s64_t)counter + (1ul<<32) - compare : counter - compare;

    while ((difference > 0)) {
	    compare += TIMER_PERIOD;
	    difference -= TIMER_PERIOD;
    }

    write_32bit_cp0_register (CP0_COMPARE, compare);
    get_timer()->compare = (u32_t)compare;

    get_current_scheduler()->handle_timer_interrupt();
}

void SECTION (".init")
timer_t::init_global(void)
{
    get_interrupt_ctrl()->register_interrupt_handler(7, handle_timer_interrupt);
}

void SECTION (".init")
timer_t::init_cpu(void)
{
    write_32bit_cp0_register(CP0_COUNT, 0);
    write_32bit_cp0_register(CP0_COMPARE, TIMER_PERIOD);
    compare = TIMER_PERIOD;

    get_interrupt_ctrl()->unmask(7);
}
