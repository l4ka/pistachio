/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/timer.cc
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

#if !defined(CONFIG_X86_TSC)
#warning JD: ticks are not SMP safe
u64_t ticks SECTION(".user.counter");
#endif

extern "C" void timer_interrupt(void);
#if defined(CONFIG_IS_64BIT)
X86_EXCNO_ERRORCODE(timer_interrupt, IRQLINE)
#else
X86_EXCNO_ERRORCODE(timer_interrupt, IRQLINE)
#endif
{
    /* acknowledge irq on PIC */
    get_interrupt_ctrl()->ack(IRQLINE);

    /* reset intr line on RTC */
    rtc_t<0x70> rtc;
    rtc.read(0x0c);

    /* count interrupts */
#if !defined(CONFIG_X86_TSC)
    ticks++;
#endif

    /* handle the timer */
    get_current_scheduler()->handle_timer_interrupt();
}



void SECTION (".init") timer_t::init_global()
{
    /* TODO: Should be irq_manager.register(hwirq, 8, &timer_interrupt); */
    idt.add_gate(0x20+IRQLINE, idt_t::interrupt, timer_interrupt);

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


void SECTION (".init") timer_t::init_cpu(cpuid_t cpu)
{
#if defined(CONFIG_CPU_X86_SIMICS)
    u64_t cpu_cycles;

    TRACE_INIT("Simics CPU hardcoding processor speed ...\n");
    /* Set frequencies statically on SIMICS, waiting would take ages... */
    cpu_cycles = CONFIG_CPU_X86_SIMICS_SPEED * 1000 * 1000;
    proc_freq = cpu_cycles  / 1000;
    bus_freq = 0;

    TRACE_INIT("\tCPU speed: %d MHz\n", (word_t)(cpu_cycles / (1000000)));
    return;

#elif defined(CONFIG_X86_TSC)
    u64_t cpu_cycles;

    TRACE_INIT("\tCalculating processor speed ...\n");
    /* calculate processor speed */
    wait_for_second_tick();

    cpu_cycles = x86_rdtsc();
    wait_for_second_tick();

    cpu_cycles = x86_rdtsc() - cpu_cycles;

    proc_freq = cpu_cycles / 1000;
    bus_freq = 0;

    TRACE_INIT("\tCPU speed: %d MHz\n", (word_t)(cpu_cycles / (1000000)));
    return;

#elif defined(CONFIG_CPU_X86_I486)
    /* We just estimate the current cpu speed, this is needed in
     * absence of any TSC.
     * We simply assume that it's really something like an i486
     */
    TRACE_INIT("\tEstimating processor speed ...\n");

    rtc_t<0x70> rtc;
    u64_t ticks = 0;
    word_t rounds = 0;

    wait_for_second_tick ();

    // wait that update bit is off
    while (rtc.read(0x0a) & 0x80);

    // read second value
    word_t secstart = rtc.read(0);

    while (secstart == rtc.read(0)) {
	__asm__ __volatile__ (
	    "       xor      %eax,%eax      \n"
	    "       movl     $20000,%ecx    \n"
	    "1:     add      %eax,%eax      \n"
	    /* We don't use loop here by intention !!! */
	    "       dec      %ecx           \n"
	    "       jnz      1b             \n"
	    );
	rounds++;
    }

    proc_freq = (rounds * 10000) / (99);
    bus_freq = 0;

    TRACE_INIT("\tRounds: %d CPU speed: %d kHz\n",
               rounds, (word_t)((rounds * 10000) / (99)));
    return;

#else /* !defined(CONFIG_CPU_X86_I486) */

    proc_freq = 0;
    bus_freq = 0;
    return;

#endif /* !defined(CONFIG_CPU_X86_SIMICS) */
}

