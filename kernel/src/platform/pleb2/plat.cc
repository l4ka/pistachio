/********************************************************************* *                
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/pleb2/plat.cc
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
 * $Id: plat.cc,v 1.2 2005/01/12 02:49:02 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_PLAT(timer.h)
#include INC_PLAT(intctrl.h)
#include INC_CPU(cpu.h)
#include INC_ARCH(bootdesc.h)

// XXX FIXME
#define RAM_START    0xa0000000
#define RAM_END      0xa2000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map in the control registers */

    space->add_mapping((addr_t)(IODEVICE_VADDR + CONSOLE_VOFFSET),
		    (addr_t)(XSCALE_DEV_PHYS + CONSOLE_POFFSET),
		    pgent_t::size_4k, true, true, true);

    space->add_mapping((addr_t)(IODEVICE_VADDR + INTERRUPT_VOFFSET),
		    (addr_t)(XSCALE_DEV_PHYS + INTERRUPT_POFFSET),
		    pgent_t::size_4k, true, true, true);

    space->add_mapping((addr_t)(IODEVICE_VADDR + TIMER_VOFFSET),
		    (addr_t)(XSCALE_DEV_PHYS + TIMER_POFFSET),
		    pgent_t::size_4k, true, true, true);

    space->add_mapping((addr_t)(IODEVICE_VADDR + CLOCKS_VOFFSET),
		    (addr_t)(XSCALE_DEV_PHYS + CLOCKS_POFFSET),
		    pgent_t::size_4k, true, true, true);
}

/*
 * Platform memory descriptors
 */
struct arm_bootdesc SECTION(".init.data") platform_memory[] = {
	{ RAM_START,	RAM_END,	memdesc_t::conventional },
	/* PCMCIA / CF Slot 0 */
	{ 0x20000000, 	0x30000000,	memdesc_t::dedicated },	
	/* PCMCIA / CF Slot 1 */
	{ 0x30000000, 	0x40000000,	memdesc_t::dedicated },	
	/* IO Devices - Peripherals/LCD/Mem ctrl */
	{ 0x40000000, 	0x4c000000,	memdesc_t::dedicated },	
	/* Extras XXX ?? */
//	{ 0xCC000000, 	0xCC000100,	memdesc_t::dedicated },	
	{ 0, 0, 0 }
};

extern "C" struct arm_bootdesc* SECTION(".init") init_platform_mem(void)
{
    return (struct arm_bootdesc*)virt_to_phys(&platform_memory);
}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
}

extern "C" void SECTION(".init") init_cpu(void)
{
    arm_cache::cache_invalidate_d();
    arm_cache::tlb_flush();
}
