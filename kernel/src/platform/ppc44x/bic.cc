/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     platform/ppc44x/bic.cc
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
 * $Id$
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/tracepoints.h>

#include <lib.h>
#include INC_GLUE(intctrl.h)
#include INC_ARCH(string.h)
#include INC_GLUE(space.h)
#include INC_PLAT(bic.h)
#include INC_PLAT(fdt.h)
#include INC_API(kernelinterface.h)

intctrl_t intctrl;

DECLARE_TRACEPOINT(SMP_IPI);

int bgic_t::get_pending_irq(word_t cpu)
{
    int group = -1, irq = -1;
    word_t hierarchy, mask = 0;
    
    hierarchy = core_non_crit[cpu];
    sync();

    // handle spurious interrupts...
    if (EXPECT_FALSE(hierarchy == 0))
	goto out;
    
    group = count_leading_zeros(hierarchy);

    if (EXPECT_FALSE(group >= BGP_MAX_GROUPS))
	goto out;
    
    mask = groups[group].noncrit_mask[cpu];
    sync();

    if (mask == 0)
	goto out;
    
    irq = group_to_irq(group) + count_leading_zeros(mask);

 out:
    return irq;
}

void bgic_t::dump()
{
    for (int i = 0; i < BGP_MAX_GROUPS; i++)
    {
	bgic_group_t *g = &groups[i];
	printf("%02x: [%p] S:%08x: T:[%08x %08x %08x %08x] M:[%08x %08x %08x %08x]\n",
	       i, g, g->status,
	       g->target_irq[0], g->target_irq[1], g->target_irq[2], g->target_irq[3],
	       g->noncrit_mask[0], g->noncrit_mask[1], g->noncrit_mask[2], g->noncrit_mask[3]);
    }
}

void SECTION (".init") intctrl_t::init_arch()
{
    fdt_t *fdt = get_dtree();

    fdt_header_t *hdr = fdt->find_subtree("/interrupt-controller");
    if (!hdr)
	panic("Couldn't find interrupt controller in FDT\n");

    fdt_property_t *prop = fdt->find_property_node(hdr, "compatible");

    if (!prop || strcmp(prop->get_string(), "ibm,bgic") != 0)
	panic("BGIC: Couldn't find compatible node in FDT\n");

    prop = fdt->find_property_node(hdr, "reg");
    if (!prop || prop->get_len() != 3 * sizeof(u32_t))
	panic("BGIC: Couldn't find valid 'reg' node in FDT (%p, %d)\n", 
	      prop, prop->get_len());

    phys_addr = prop->get_u64(0);
    mem_size = prop->get_word(2);

    prop = fdt->find_property_node(hdr, "interrupts");
    if (!prop || prop->get_len() != sizeof(u32_t))
	panic("BGIC: Couldn't find valid 'interrupts' node in FDT\n");
    
    num_irqs = prop->get_word(0);
    if (num_irqs > BGP_MAX_IRQS)
	panic("BGIC: reported number IRQs of %d exceeds specification\n", num_irqs);

    TRACE_INIT("BGIC: %x:%x, %d interrupts\n", 
	       (word_t)(phys_addr >> 32), (word_t)phys_addr, num_irqs);

    // needs to be provided by glue
    this->map();

    if (!ctrl)
	panic("BGIC: mapping IRQ controller failed\n");

    TRACE_INIT("BGIC: remapped at %p\n", ctrl);

    ctrl->mask_and_clear_all();

    // route all IRQs to CPU0
    memset(routing, 0, sizeof(routing));
}

void SECTION(".init") intctrl_t::init_cpu(int cpu)
{
    ASSERT(cpu < 4);

    /* map IPIs */
    set_irq_routing(get_ipi_irq(cpu, 0), cpu);
    enable(get_ipi_irq(cpu, 0));
}

void intctrl_t::handle_irq(word_t cpu)
{
    int irq = ctrl->get_pending_irq(cpu);
    if (irq < 0 || irq > (int)get_number_irqs())
    {
        printf("spurious interrupt\n");
        return;
    }

#ifdef CONFIG_SMP
    if (irq < 32)
    {
        ctrl->ack_irq(irq);
        handle_smp_ipi(irq);
        return;
    }
#endif

    mask(irq);
    ::handle_interrupt( irq );
}

void intctrl_t::map()
{
    ctrl = (bgic_t*)get_kernel_space()->
	map_device(phys_addr, mem_size, true, pgent_t::cache_inhibited);
}

void intctrl_t::start_new_cpu(word_t cpu)
{
    ASSERT(cpu < 4);
    
    extern word_t secondary_release_reloc;
    secondary_release_reloc = cpu;
}

void intctrl_t::send_ipi(word_t cpu)
{
    TRACEPOINT(SMP_IPI, "irq %d cpu %d\n", get_ipi_irq(cpu, 0), cpu);
    ctrl->raise_irq(get_ipi_irq(cpu, 0));
}
