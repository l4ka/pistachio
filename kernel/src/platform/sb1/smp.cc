/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     platform/sb1/smp.cc
 * Description:   mips64 sibyte MP implementation
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
 * $Id: smp.cc,v 1.6 2006/03/01 14:10:32 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(types.h)
#include INC_API(tcb.h)
#include INC_GLUE(smp.h)
#include INC_PLAT(smp.h)
#include INC_PLAT(cache.h)
#include INC_GLUE(intctrl.h)

#if defined(CONFIG_SMP)

extern void cfe_init(word_t arg);
extern int cfe_start_cpu(int cpu, void (*fn)(void), long sp, long gp, long a1);
extern word_t plat_cpu_freq[CONFIG_SMP_MAX_CPUS], plat_bus_freq[CONFIG_SMP_MAX_CPUS];

bool processor_running[CONFIG_SMP_MAX_CPUS];

extern "C" void handle_ipi(word_t irq, mips64_irq_context_t * frame);
extern "C" void SECTION (".init") startup_cpu (cpuid_t cpuid);


volatile static word_t sim_fn = 0;

void sim_wait(void)
{
    int cpu = mips64_get_cpuid();

    while (sim_fn != cpu)
	cache_t::flush_cache_all();

    startup_cpu(cpu);
}

void SECTION(".init") init_platform(word_t arg)
{
    word_t syscfg = SBREADCSR(A_SCD_SYSTEM_CFG);
    word_t plldiv = G_SYS_PLL_DIV(syscfg);

    if (mips64_get_cpuid() != 0)    // error
	sim_wait();

    cfe_init(arg);

    plat_cpu_freq[0] = plldiv * 50000000ul;
    plat_bus_freq[0] = 100000000;
}

static void SECTION(".init") mips64_cpu_jump()
{
    int cpu = mips64_get_cpuid();
    startup_cpu(cpu);
}

void SECTION(".init") mips64_start_processor (cpuid_t cpu)
{
    if (cpu != 1)
	return;

    if (mips64_get_cpuid() != 0)    // error
	while (1);

    plat_cpu_freq[cpu] = plat_cpu_freq[0];
    plat_bus_freq[cpu] = plat_bus_freq[0];

    printf("Starting cpu %d\n", cpu);

    cfe_start_cpu(cpu, mips64_cpu_jump, 0xffffffff80800000, 0, 0);

    sim_fn = cpu;
    cache_t::flush_cache_all();
}

void SECTION(".init") mips64_processor_online (cpuid_t cpu)
{
    processor_running[cpu] = true;
    cache_t::flush_cache_all();
}

bool SECTION(".init") mips64_wait_for_processor (cpuid_t cpu)
{
    for (word_t i = 0; i < 1000000; i++)
    {
	cache_t::flush_cache_all();
	if (processor_running[cpu])
	    return true;
    }
    return false;
}

bool SECTION(".init") mips64_is_processor_available (cpuid_t cpu)
{
    word_t res;
    __asm__ __volatile__ (
	"la	$8,%[sys_rev]\n"
	"ld	$8,($8)\n"		// Get system revision
	"dsrl	$8,%[sys_part]\n"	// Shift part # to low bits
	"dsrl	$8,8\n"			// isolate CPU part of number
	"and	%[result],$8, 0x0F\n"	// number of CPUs
	:   [result] "=r" (res)
	:   [sys_rev] "i" (MIPS64_ADDR_K1(A_SCD_SYSTEM_REVISION)),
	    [sys_part] "i" (S_SYS_PART)
	: "$8"
    );

    if (cpu < res)
	return true;

    return false;
}

cpuid_t mips64_get_cpuid (void)
{
    cpuid_t res;
    __asm__ __volatile__ (
	"mfc0	$8," STR(CP0_PRID) "\n"	    // get CPU PRID register
	"srl	$8,$8,25\n"		    // determine cpu number
	"and	%[result],$8,0x07\n"	    // keep only low 3 bits
	:   [result] "=r" (res)
	: : "$8"
    );

    return res;
}

void SECTION(".init") mips64_init_ipi(cpuid_t cpu)
{
    /* Clear out our mailbox registers (both CPUs) */
    *IMR_POINTER(cpu, R_IMR_MAILBOX_CLR_CPU) = ~(0ul);

    /* Setup the Mailbox interrupts */
    *IMR_POINTER(cpu, R_IMR_INTERRUPT_MAP_BASE + 8*K_INT_MBOX_3) = 3;		/* Interrupt 3 */
    *IMR_POINTER(cpu, R_IMR_INTERRUPT_MASK) &= ~((word_t)1 << K_INT_MBOX_3);	/* Unmask interrupt */
    __asm__ __volatile__ ("sync" : : : "memory");

    get_interrupt_ctrl()->register_interrupt_handler(5, handle_ipi);
    get_interrupt_ctrl()->unmask(5);	/* Hardware Int 3 */
    __asm__ __volatile__ ("sync" : : : "memory");
}

#else

extern word_t plat_cpu_freq[1], plat_bus_freq[1];

void SECTION(".init") init_platform(word_t arg)
{
    word_t syscfg = SBREADCSR(A_SCD_SYSTEM_CFG);
    word_t plldiv = G_SYS_PLL_DIV(syscfg);

    plat_cpu_freq[0] = plldiv * 50000000ul;
    plat_bus_freq[0] = 100000000;
}

#endif /* CONFIG_SMP */
