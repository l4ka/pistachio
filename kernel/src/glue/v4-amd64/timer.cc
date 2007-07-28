/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2005, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/timer.cc
 * Description:   Implements RTC timer
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
 * $Id: timer.cc,v 1.7 2006/10/19 22:57:39 ud3 Exp $
 *                
 ********************************************************************/

#include INC_GLUE(idt.h)
#include INC_ARCH(trapgate.h)

#include INC_PLAT(rtc.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)

#include INC_API(schedule.h)

#define IRQLINE 8

/* global instance of timer object */
timer_t timer;


extern "C" void timer_interrupt(void);
AMD64_EXC_NO_ERRORCODE(timer_interrupt, IRQLINE)
{
    
    /* acknowledge irq on PIC */
    get_interrupt_ctrl()->ack(IRQLINE);

    /* reset intr line on RTC */
    rtc_t<0x70> rtc;
    rtc.read(0x0c);

    /* handle the timer */
    get_current_scheduler()->handle_timer_interrupt();
    
}


void timer_t::init_global()
{
    /* TODO: Should be irq_manager.register(hwirq, 8, &timer_interrupt); */
    idt.add_int_gate(0x20+IRQLINE, timer_interrupt);
    
    {
	rtc_t<0x70> rtc;
	
	/* wait for update-in-progress to finish */
	while(rtc.read(0x0a) & 0x80);
	
	/* set rtc to 512, rate = 2Hz*2^(15-(x:3..0))*/
	rtc.write(0x0A, (rtc.read(0x0a) & 0xf0) | 0x07);
	/* enable interrupts
	   Periodic Interrupt Enable = 0x40	*/
	rtc.write(0x0b, rtc.read(0x0b) | 0x40);
	
	/* read(0x0c) clears all interrupts in RTC */
	rtc.read(0x0c);
	
	/* TODO: Should this become irq_manager.unmask(hwirq, 8) ? */
	get_interrupt_ctrl()->unmask(IRQLINE);
    }
}

void timer_t::init_cpu()
{
    u64_t cpu_cycles;

#if !defined(CONFIG_CPU_AMD64_SIMICS)
    TRACE_INIT("Calculating processor speed...\n");
    /* calculate processor speed */
    wait_for_second_tick();

    cpu_cycles = x86_rdtsc();
    wait_for_second_tick();

    cpu_cycles = x86_rdtsc() - cpu_cycles;

    proc_freq = cpu_cycles  / 1000;
    bus_freq = 0;

#else
    TRACE_INIT("SIMICS CPU hardcoding processor speed...\n");
    /* Set frequencies statically on SIMICS, waiting would take ages... */

    cpu_cycles = CONFIG_CPU_AMD64_SIMICS_SPEED * 1000 * 1000;
    proc_freq = cpu_cycles  / 1000;
    bus_freq = 0;


#endif
    TRACE_INIT("CPU speed: %d MHz\n", (word_t)(cpu_cycles / (1000000)));
}

