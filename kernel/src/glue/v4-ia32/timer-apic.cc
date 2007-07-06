/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/timer-apic.cc
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
 * $Id: timer-apic.cc,v 1.5 2003/09/24 19:05:34 skoglund Exp $
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
IA32_EXC_NO_ERRORCODE(timer_interrupt, 0)
{
    local_apic.EOI();

    /* handle the timer */
    get_current_scheduler()->handle_timer_interrupt();
}


void timer_t::init_global()
{
    TRACE_INIT("init_global timer - trap gate %d\n", IDT_LAPIC_TIMER);
    idt.add_int_gate(IDT_LAPIC_TIMER, timer_interrupt);
}

void timer_t::init_cpu()
{ 
    // avoid competing for the RTC
    static spinlock_t timer_lock;
    timer_lock.lock();

    TRACE_INIT("calculating processor speed...\n");
    local_apic.timer_set_divisor(1);
    local_apic.timer_setup(IDT_LAPIC_TIMER, false);
    local_apic.timer_set(-1UL);

    /* calculate processor speed */
    wait_for_second_tick();

    u64_t cpu_cycles = ia32_rdtsc();
    u32_t bus_cycles = local_apic.timer_get();

    wait_for_second_tick();
    
    cpu_cycles = ia32_rdtsc() - cpu_cycles;
    bus_cycles -= local_apic.timer_get();

    proc_freq = cpu_cycles / 1000;
    bus_freq = bus_cycles / 1000;

    TRACE_INIT("CPU speed: %d MHz, bus speed: %d MHz\n", 
	       (word_t)(cpu_cycles / (1000000)), bus_cycles / (1000000));

    /* now set timer IRQ to periodic timer */
    local_apic.timer_setup(IDT_LAPIC_TIMER, true);
    local_apic.timer_set( bus_cycles / (1000000 / TIMER_TICK_LENGTH) );
    timer_lock.unlock();
}
