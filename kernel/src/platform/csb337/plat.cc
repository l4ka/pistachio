/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/csb337/plat.cc
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
 * $Id: plat.cc,v 1.3 2004/08/21 13:31:33 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_PLAT(timer.h)
#include INC_ARCH(bootdesc.h)

// Cogent CSB337 - Atmel AT91RM9200
#define RAM_SDRAM_START	    0x20000000
#define RAM_SDRAM_END	    0x22000000

#define RAM_SRAM_START	    0x00200000	/* Internal SRAM */
#define RAM_SRAM_END	    0x00204000
#define RAM_ROM_START	    0x00100000	/* Internal ROM */
#define RAM_ROM_END	    0x00120000
#define RAM_USB_START	    0x00300000	/* Internal USB Host Port */
#define RAM_USB_END	    0x00400000
#define RAM_FLASH1_START    0x10000000	/* Strata Flash */
#define RAM_FLASH1_END      0x11000000
#define RAM_LCD1_START	    0x30000000	/* LCD Device Registers */
#define RAM_LCD1_END	    0x30000200
#define RAM_LCD2_START	    0x30040000	/* LCD Device 80k Buffer */
#define RAM_LCD2_END	    0x30070000
#define RAM_EXP0_START	    0x40000000	/* Expansion Slot 0 */
#define RAM_EXP0_END	    0x50000000
#define RAM_CF0_START	    0x60000000	/* Compact Flash 0 */
#define RAM_CF0_END	    0x70000000
#define RAM_CF1_START	    0x70000000	/* Compact Flash 1 */
#define RAM_CF1_END	    0x80000000
#define RAM_EXP1_START	    0x80000000	/* Expansion Slot 1 */
#define RAM_EXP1_END	    0x90000000
#define RAM_PERIPH_START    0xFFFFF000	/* Expansion Slot 1 */
#define RAM_PERIPH_END	    0xFFFFFFFF

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map the AT91RM9200 system peripherals */
    space->add_mapping((addr_t)SYS_VADDR, (addr_t)SYS_PADDR,
		    pgent_t::size_4k, true, true, true);
}

/*
 * Platform memory descriptors
 */
struct arm_bootdesc SECTION(".init.data") platform_memory[] = {
	{ RAM_SDRAM_START,  RAM_SDRAM_END,	memdesc_t::conventional},
	{ RAM_SRAM_START,   RAM_SRAM_END,	memdesc_t::dedicated },
	{ RAM_ROM_START,    RAM_ROM_END,	memdesc_t::dedicated },
	{ RAM_USB_START,    RAM_USB_END,	memdesc_t::dedicated },
	{ RAM_FLASH1_START, RAM_FLASH1_END,	memdesc_t::dedicated },
	{ RAM_LCD1_START,   RAM_LCD1_END,	memdesc_t::dedicated },
	{ RAM_LCD2_START,   RAM_LCD2_END,	memdesc_t::dedicated },
	{ RAM_EXP0_START,   RAM_EXP0_END,	memdesc_t::dedicated },
	{ RAM_CF0_START,    RAM_CF0_END,	memdesc_t::dedicated },
	{ RAM_CF1_START,    RAM_CF1_END,	memdesc_t::dedicated },
	{ RAM_EXP1_START,   RAM_EXP1_END,	memdesc_t::dedicated },
	{ RAM_PERIPH_START, RAM_PERIPH_END,	memdesc_t::dedicated },
	{ 0, 0, 0 }
};

extern "C" struct arm_bootdesc* SECTION(".init") init_platform_mem(void)
{
    return (struct arm_bootdesc*)virt_to_phys(&platform_memory);
}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
}

extern "C" void init_cpu(void)
{
    arm_cache::cache_invalidate_d();
    arm_cache::tlb_flush();

    /* Program the Address Remap Register to map Internal SRAM */
    *((volatile word_t *)(SYS_PADDR + 0xf00)) = 1;

    *((volatile word_t *)(SYS_PADDR + 0xf78)) = 0x1100318a;
}
