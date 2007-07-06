/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/pleb/plat.cc
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
 * $Id: plat.cc,v 1.6 2004/08/13 10:52:45 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_PLAT(timer.h)
#include INC_ARCH(bootdesc.h)

// XXX FIXME
#define RAM_B1_START    0xC0000000
#define RAM_B1_END      0xC1000000
#define RAM_B2_START    0xC8000000
#define RAM_B2_END      0xC9000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map the console */
    space->add_mapping((addr_t)CONSOLE_VADDR, (addr_t)CONSOLE_PADDR,
		    pgent_t::size_4k, true, true, true);

    /* Map the timer, interrupt, reset controllers */
    space->add_mapping((addr_t)SA1100_OS_TIMER_BASE, (addr_t)SA1100_TIMER_PHYS,
		    pgent_t::size_1m, true, true, true);
}

/*
 * Platform memory descriptors
 */
struct arm_bootdesc SECTION(".init.data") platform_memory[] = {
	{ 0,		RAM_B1_START,	memdesc_t::dedicated },
	{ RAM_B1_START,	RAM_B1_END,	memdesc_t::conventional },
	{ RAM_B1_END,	RAM_B2_START,	memdesc_t::dedicated },
	{ RAM_B2_START, RAM_B2_END, 	memdesc_t::conventional },
	{ RAM_B2_END, 	0xFFFFFFFF,	memdesc_t::dedicated },
	{ 0, 0, 0 }
};

extern "C" struct arm_bootdesc* SECTION(".init") init_platform_mem(void)
{
    return (struct arm_bootdesc*)virt_to_phys(&platform_memory);
}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
    /* Map sa1100 zero bank */
    get_kernel_space()->add_mapping( (addr_t)ZERO_BANK_VADDR, (addr_t)0xE0000000,
		    pgent_t::size_1m, true, true );
}

extern "C" void init_cpu(void)
{
}
