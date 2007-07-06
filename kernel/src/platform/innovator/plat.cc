/********************************************************************* *                
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/innovator/plat.cc
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
 * $Id: plat.cc,v 1.3 2004/08/13 10:52:46 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_CPU(io.h)
#include INC_PLAT(reg.h)
#include INC_ARCH(bootdesc.h)

/* 192k Internal SRAM region. Should be in good use. */
#define SRAM_START		0x20000000
#define SRAM_END		0x20030000

/* SDRAM region */
#define SDRAM_START		0x10000000
#define SDRAM_END		0x12000000

/* System Reserved Regions, shouldn't be used for any reason. */
#define RESERVED_1_START	0x02000000
#define RESERVED_1_END		0x04000000
#define RESERVED_2_START	0x06000000
#define RESERVED_2_END		0x08000000
#define RESERVED_3_START	0x0A000000
#define RESERVED_3_END		0x0C000000
#define RESERVED_4_START	0x0E000000
#define RESERVED_4_END		0x10000000
#define RESERVED_5_START	0x14000000
#define RESERVED_5_END		0x20000000
#define RESERVED_6_START	0x20030000
#define RESERVED_6_END		0x30000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map peripherals and control registers */
    space->add_mapping((addr_t)IODEVICE_VADDR, (addr_t)PHYS_CTL_REG_BASE,
		    pgent_t::size_1m, true, true, true);
}

/*
 * Platform memory descriptors
 */
struct arm_bootdesc SECTION(".init.data") platform_memory[] = {
	{ 0,		SDRAM_START,	memdesc_t::dedicated },
	{ SDRAM_START,	SDRAM_END,	memdesc_t::conventional },
	{ SDRAM_END,	0xFFFFFFFF,	memdesc_t::dedicated },
	{ RESERVED_1_START, RESERVED_1_END,	memdesc_t::reserved },
	{ RESERVED_2_START, RESERVED_2_END,	memdesc_t::reserved },
	{ RESERVED_3_START, RESERVED_3_END,	memdesc_t::reserved },
	{ RESERVED_4_START, RESERVED_4_END,	memdesc_t::reserved },
	{ RESERVED_5_START, RESERVED_5_END,	memdesc_t::reserved },
	{ RESERVED_6_START, RESERVED_6_END,	memdesc_t::reserved },
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
}
