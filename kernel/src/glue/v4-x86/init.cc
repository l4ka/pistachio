/*********************************************************************
 *
 * Copyright (C) 2002-2007,  Karlsruhe University
 *
 * File path:     glue/v4-x86/init.cc
 * Description:   ia32-specific initialization
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

#include <debug.h>
#include <kmemory.h>
#include <mapping.h>
#include <ctors.h>
#include <init.h>

#include INC_ARCHX(x86,apic.h)

#include INC_GLUE(space.h)

#include INC_API(schedule.h)


// from either glue/v4-ia32/ or glue/v4-x86/
void SECTION(".init.cpu") check_cpu_features();
cpuid_t SECTION(".init.cpu") init_cpu();


#if defined(CONFIG_SMP)
/**************************************************************************
 *
 * SMP functions.
 *
 *************************************************************************/
extern "C" void _start_ap(void);
extern void setup_smp_boot_gdt();

spinlock_t smp_boot_lock;
/* commence to sync TSC */
spinlock_t smp_commence_lock;


//INLINE
u8_t get_apic_id (void)
{
    local_apic_t<APIC_MAPPINGS> apic;
    return apic.id();
}


//static
void smp_ap_commence (void)
{
    smp_boot_lock.unlock();

    /* finally we sync the time-stamp counters */
    while( smp_commence_lock.is_locked() );

    x86_settsc(0);
}


//static
void smp_bp_commence (void)
{
    // wait for last processor to call in
    smp_boot_lock.lock();

    // now release all at once
    smp_commence_lock.unlock();

    x86_settsc(0);
}


/**
 * startup_processor
 */
extern "C" void SECTION(SEC_INIT) startup_processor (void)
{
    TRACE_INIT("AP processor is alive\n");
    x86_mmu_t::set_active_pagetable((u32_t)get_kernel_space()->get_pagetable());
    TRACE_INIT("AP switched to kernel ptab\n");

    // first thing -- check CPU features
    check_cpu_features();

    /* perform processor local initialization */
    cpuid_t cpuid = init_cpu();

    get_current_scheduler()->init (false);
    get_idle_tcb()->notify (smp_ap_commence);
    get_current_scheduler()->start (cpuid);

    spin_forever(cpuid);
}
#endif /* defined(CONFIG_SMP) */



void SECTION(SEC_INIT) clear_bss (void)
{
    extern u8_t _start_bss[];
    extern u8_t _end_bss[];
    for (u8_t* p = _start_bss; p < _end_bss; p++)
	*p = 0;
}

