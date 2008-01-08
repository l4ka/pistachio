/****************************************************************************
 *
 * Copyright (C) 2002, 2006, Karlsruhe University
 *
 * File path:	glue/v4-amd64/space.cc
 * Description:	AMD64 space_t implementation.
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
 * $Id: space.cc,v 1.23 2006/11/18 10:15:54 stoess Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <generic/lib.h>
#include <linear_ptab.h>

#include INC_API(tcb.h)
#include INC_API(smp.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(mmu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(pgent.h)

#include INC_GLUE(cpu.h)
#include INC_GLUE(memory.h)
#include INC_GLUE(space.h)

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */




word_t space_t::readmem_phys(addr_t paddr)
{
    ASSERT( (word_t) paddr < (1ULL << 32));
        return * (word_t *) ( (word_t) paddr + REMAP_32BIT_START); 
}



#if defined(CONFIG_SMP)
void pgent_t::smp_sync(space_t * space, pgsize_e pgsize)
{
    ASSERT(pgsize >= size_sync);
    
    switch (pgsize)
    {
    case size_512g: 
	for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
	    if (cpu != space->data.reference_ptab && space->get_top_pdir(cpu))

	    {
		//TRACEF("smp sync pml4 %d / %x -> %d / %x\n",
		//     space->data.reference_ptab, space->pgent(idx()),
		//     cpu, space->pgent(idx(), cpu));
		*space->pgent(idx(), cpu) = *space->pgent(idx());
	    }
	break;
    case size_1g: 
	ASSERT(space->get_top_pdir()->get_kernel_pdp());
	if (!is_cpulocal(space, size_1g) && 
	    (this - idx() == space->get_top_pdir(space->data.reference_ptab)->get_kernel_pdp_pgent()))
	{
	    ASSERT(space->get_top_pdir(space->data.reference_ptab)->get_kernel_pdp());
	    
	    for (cpuid_t cpu = 0; cpu < cpu_t::count; cpu++)
		if (cpu != space->data.reference_ptab && space->get_top_pdir(cpu) &&
		    space->get_top_pdir(cpu)->get_kernel_pdp_pgent())
		{
		    //TRACEF("smp sync kernel pdp %x idx %d cpu %d cpulocal = %s\n", 
		    // this - idx(), idx(), cpu, (is_cpulocal(space, size_2m) ? "cpulocal" : "global"));
		    
		    *space->get_top_pdir(cpu)->get_kernel_pdp_pgent()->next(space, size_2m, idx()) =
			*space->get_top_pdir(space->data.reference_ptab)->get_kernel_pdp_pgent()->next(space, size_2m, idx());
		}
	    break;
	}
	default:
	    break;
    }
}

word_t pgent_t::smp_reference_bits(space_t * space, pgsize_e pgsize, addr_t vaddr)
{
    printf("L4 Kernel BUG: X64 shouldn't have non-global superpages");
    UNIMPLEMENTED();
}

#endif

/**
 * ACPI memory handling
 */
addr_t acpi_remap(addr_t addr)
{

    //TRACE_INIT("ACPI remap: %p -> %x\n", addr, (word_t) addr + REMAP_32BIT_START); 
    ASSERT((word_t) addr < (1ULL << 32));
    return (addr_t) ((word_t) addr + REMAP_32BIT_START); 
}

void acpi_unmap(addr_t addr)
{
    /* empty  */
}


#if defined(CONFIG_X86_COMPATIBILITY_MODE)
extern addr_t utcb_page;

word_t space_t::space_control(word_t ctrl)
{
    // Ignore parameter if 'c' bit is not set.
    if ((ctrl & (((word_t) 1) << 63)) == 0)
	return 0;

    if (!data.compatibility_mode)
    {
	data.compatibility_mode = true;

	/* Add 32-bit UTCB mapping, since the gs segment descriptor
	   is truncated in 32-bit mode.
	   Copied from init_kernel_mappings. */
	remap_area((addr_t) UTCB_MAPPING_32,
		   virt_to_phys(utcb_page),
		   pgent_t::size_4k, X86_PAGE_SIZE, true, false, false);

	/* Replace 64-bit KIP mapping with 32-bit KIP.
	   Copied from init. */
	if (is_initialized())
	{
	    add_mapping(get_kip_page_area().get_base(), virt_to_phys((addr_t) x32::get_kip()), pgent_t::size_4k, 
			false, false, false);
	}
    }

    // Set 'e' bit.
    return (((word_t) 1) << 63);
}

#else /* !defined(CONFIG_X86_COMPATIBILITY_MODE) */

word_t space_t::space_control(word_t ctrl)
{
    return 0;
}

#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */

