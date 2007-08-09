/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/timer-apic.cc
 * Description:   implementation of apic timer
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
 * $Id: timer-apic.cc,v 1.6 2006/10/19 22:57:39 ud3 Exp $
 *                
 ********************************************************************/
#include INC_ARCH(trapgate.h)

#include INC_GLUE(idt.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)

#include INC_PLAT(rtc.h)

#include INC_API(schedule.h)


/* global instance of timer object */
timer_t timer UNIT("cpulocal");
static local_apic_t<APIC_MAPPINGS> local_apic;

extern "C" void timer_interrupt(void);
AMD64_EXC_NO_ERRORCODE(timer_interrupt, 0)
{
    local_apic.EOI();

    /* handle the timer */
    get_current_scheduler()->handle_timer_interrupt();
}


void timer_t::init_global()
{
    TRACE_INIT("Init_global timer - trap gate %d\n", IDT_LAPIC_TIMER);
    idt.add_int_gate(IDT_LAPIC_TIMER, timer_interrupt);
}

void timer_t::init_cpu()
{ 
    intctrl_t *intctrl = get_interrupt_ctrl();
    bool pmtimer = intctrl->has_pmtimer();
    
    // avoid competing for the RTC
    static spinlock_t timer_lock;

#if !defined(CONFIG_CPU_AMD64_SIMICS)
    TRACE_INIT("calculating processor speed...\n");
    local_apic.timer_set_divisor(1);
    local_apic.timer_setup(IDT_LAPIC_TIMER, false);
    local_apic.timer_set(-1UL);

   
    word_t delay = 1000;
    
    /* calculate processor speed */
    if (pmtimer)
    {
	delay = 100;
	intctrl->pmtimer_wait(delay);	
    }
    else
    {
	timer_lock.lock();
	wait_for_second_tick();
    }

    u64_t cpu_cycles = x86_rdtsc();
    u32_t bus_cycles = local_apic.timer_get();
    
    if (pmtimer)
	intctrl->pmtimer_wait(delay);	
    else
    {
	wait_for_second_tick();
	timer_lock.unlock();
    }

    
    cpu_cycles = x86_rdtsc() - cpu_cycles;
    bus_cycles -= local_apic.timer_get();

    word_t local_apic_cpu_mhz = cpu_cycles / (delay * 1000);
    word_t local_apic_bus_mhz = bus_cycles / (delay * 1000);

    proc_freq = cpu_cycles / delay;
    bus_freq = bus_cycles / delay;
#else 
    /*
     * Set frequencies statically on SIMICS, waiting would take ages...
     */
    
    cpu_cycles = CONFIG_CPU_AMD64_SIMICS_SPEED * 1000 * 1000;
    proc_freq = cpu_cycles  / 1000;
    bus_freq = 0;

#endif 


    TRACE_INIT("CPU speed: %d MHz, bus speed: %d MHz\n", 
	       (word_t)(local_apic_cpu_mhz), local_apic_bus_mhz);

    /* now set timer IRQ to periodic timer */
    local_apic.timer_setup(IDT_LAPIC_TIMER, true);
    local_apic.timer_set( bus_cycles / (1000 * delay / TIMER_TICK_LENGTH) );

}
