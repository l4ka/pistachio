/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/timer.cc
 * Description:   
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
 * $Id: timer.cc,v 1.3 2004/07/01 04:09:33 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE_API_ARCH(timer.h)
#include INC_CPU(asi.h)

timer_t timer UNIT("cpulocal");

static word_t timer_interval;

INLINE void
set_tick_cmpr(word_t tick_cmpr)
{
    asm volatile (
	"1: wr	%0, 0, %%tick_cmpr\n\t"
	  :: "r" (tick_cmpr)
    );
}

INLINE word_t
get_tick(void)
{
    word_t tick;
    asm volatile (
	"rdpr	%%tick, %0\n\t"
	: "=r" (tick)
    );
    return tick;
}

INLINE void
set_tick(word_t tick)
{
    asm volatile (
	"wrpr	%0, %%tick\n\t"
	:: "r" (tick)
    );
}

void SECTION(".init")
timer_t::init_global(void)
{
    extern word_t plat_cpu_freq;
    
    timer_interval = TIMER_TICK_LENGTH * plat_cpu_freq / 1000000;

    TRACE_INIT("Timebase frequency %dHz\n", plat_cpu_freq);
}

void SECTION(".init")
timer_t::init_cpu(void)
{
    set_tick_cmpr(timer_interval);
    
    last_tick = 0;

    set_tick(0);
}

extern "C" void timer_interrupt(void)
{
    word_t new_tick;
    timer_t* timer = get_timer();

    do {
	timer->last_tick += timer_interval;
	set_tick_cmpr(timer->last_tick);
	new_tick = get_tick();
    } while(timer->last_tick < new_tick);

    asm volatile ("wr %g0, 1, %clear_softint");
    
    get_current_scheduler()->handle_timer_interrupt();
}
