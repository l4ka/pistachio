/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/intctrl.cc
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
 * $Id: intctrl.cc,v 1.23 2006/05/24 09:41:47 stoess Exp $
 *                
 ********************************************************************/
#include INC_GLUE(intctrl.h)
#include INC_GLUE(hwspace.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_ARCH(cr.h)
#include INC_ARCH(pib.h)
#include INC_ARCH(ia64.h)
#include INC_ARCH(iosapic.h)
#include INC_API(types.h)

#include <acpi.h>

pib_t<phys_to_virt_uc (IA64_DEFAULT_PIB_ADDR)> pib;

word_t interrupt_vector[256];

intctrl_t intctrl;


static void irq_handler (word_t irq, ia64_exception_context_t * frame);
static void dummy_irq_handler (word_t irq, ia64_exception_context_t * frame);


#if defined (CONFIG_SMP)

class ia64_cpu_t
{
public:
    word_t		id;
    bool		avail;
    volatile bool	active;

public:
    void init (void)
	{ avail = false; }

    void set (u8_t id, u8_t eid, bool active = true);

    void set_active (bool active = true)
	{ this->active = active; }

    bool is_active (void) 
	{ return active; }

    bool is_avail (void) 
	{ return avail; }

    bool is_current_cpu (void)
	{ return id == (cr_get_lid ().id << 8) | (cr_get_lid ().eid); }

    u8_t get_id (void)
	{ return (id >> 8); }

    u8_t get_eid (void)
	{ return (id & 0xff); }

    u16_t get_id_eid (void)
	{ return id; }
};

INLINE void ia64_cpu_t::set (u8_t id, u8_t eid, bool avail)
{
    this->id = (id << 8) | eid;
    this->avail = avail;
    this->active = false;
}


ia64_cpu_t processors[CONFIG_SMP_MAX_CPUS];

void smp_startup_processor (cpuid_t cpu_id, word_t vector)
{
    ia64_cpu_t * cpu = &processors[cpu_id];

    if (! cpu->is_avail () || cpu->is_active ())
	return;

    TRACEF ("cpu %d (id = 0x%04x), vec: %x\n", cpu_id, cpu->id, vector);

    extern volatile word_t _ap_cpuid;
    _ap_cpuid = cpu_id;

    ia64_mf ();
    pib.ipi (cpu->id, vector);
}

void smp_send_ipi (cpuid_t cpu_id, word_t vector)
{
    ia64_cpu_t * cpu = &processors[cpu_id];
    pib.ipi (cpu->id, vector);
}

void smp_processor_online (cpuid_t cpu)
{
    processors[cpu].set_active ();
}

bool smp_is_processor_online (cpuid_t cpu)
{
    return processors[cpu].is_active ();
}

bool smp_is_processor_available (cpuid_t cpu)
{
    return processors[cpu].is_avail ();
}

bool smp_wait_for_processor (cpuid_t cpu)
{
    for (word_t i = 0; i < 1000000; i++)
	if (processors[cpu].is_active ())
	    return true;
    return false;
}

cpuid_t smp_get_cpuid (void)
{
    for (word_t i = 0; i < CONFIG_SMP_MAX_CPUS; i++)
	if (processors[i].is_current_cpu ())
	    return i;
    return ~0U;
}

#endif /* CONFIG_SMP */


void intctrl_t::init_arch (void)
{
    // Perform hardware independent initialization of interrupt
    // controller.

    for (word_t i = 0; i < sizeof (interrupt_vector) / sizeof (word_t); i++)
	register_handler (i, dummy_irq_handler);

    for (word_t i = 0; i < INTCTRL_REDIR_ENTRIES; i++)
	redir[i].init ();

    num_irqs = 0;


    // Perform hardware dependent initialization of interrupt
    // controller.

    acpi_rsdp_t * rsdp = acpi_rsdp_t::locate ();
    addr_t mapped_addr = NULL;

    if (rsdp == NULL)
    {
	printf ("Could not locate ACPI info\n");
	return;
    }

    TRACE_INIT ("ACPI information found at %p\n", rsdp);

    addr_t acpi_map = NULL;
    if (! dtrmap.is_mapped (rsdp))
    {
	translation_t tr (true, translation_t::write_back, true, true, 0,
			  translation_t::rwx, rsdp, 0);
	dtrmap.add_map (tr, rsdp, HUGE_PGSIZE, 0);
	acpi_map = rsdp;
    }

    acpi_rsdt_t * rsdt = rsdp->rsdt ();
    acpi_xsdt_t * xsdt = rsdp->xsdt ();

    if (acpi_map)
	dtrmap.free_map (acpi_map);

    if ((rsdt == NULL) && (xsdt == NULL))
	return;

    // Try locating madt in extended system desctriptor table

    acpi_madt_t * madt = NULL;
    if (xsdt != NULL)
    {
	if (! dtrmap.is_mapped (xsdt))
	{
	    translation_t tr (true, translation_t::write_back, true, true, 0,
			      translation_t::rwx, xsdt, 0);
	    dtrmap.add_map (tr, xsdt, HUGE_PGSIZE, 0);
	    mapped_addr = xsdt;
	}
	madt = (acpi_madt_t *) xsdt->find ("APIC", madt);
    }

    // Try locating madt in root system desctriptor table

    if ((madt == NULL) && (rsdt != NULL))
    {
	if (! dtrmap.is_mapped (rsdt))
	{
	    translation_t tr (true, translation_t::write_back, true, true, 0,
			      translation_t::rwx, rsdt, true);
	    dtrmap.add_map (tr, rsdt, HUGE_PGSIZE, 0);
	    mapped_addr = rsdt;
	}
	madt = (acpi_madt_t *) rsdt->find ("APIC", madt);
    }

    if (madt == NULL)
    {
	TRACE_INIT ("Could not find MADT\n");
	return;
    }

    if (madt->local_apic_addr != IA64_DEFAULT_PIB_ADDR)
    {
	printf ("Local APIC not at default location (%p != %p)\n",
		madt->local_apic_addr, IA64_DEFAULT_PIB_ADDR);
	return;
    }

#if defined(CONFIG_SMP)

    for (word_t i = 0; i < CONFIG_SMP_MAX_CPUS; i++)
	processors[i].init ();

    acpi_madt_lsapic_t * p;
    for (word_t i = 0; ((p = madt->lsapic (i)) != NULL); i++)
    {
	if (p->flags.enabled)
	{
	    if (p->apic_processor_id < CONFIG_SMP_MAX_CPUS)
		processors[p->apic_processor_id].set (p->id, p->eid);
	    else
		printf ("Processor id exceeds MAX_CPUS (%d)\n",
			p->apic_processor_id);
	}
    }

    int numcpus = 0;
    cr_lid_t lid = cr_get_lid ();
    for (word_t i = 0; i < CONFIG_SMP_MAX_CPUS; i++)
    {
	ia64_cpu_t * cpu = &processors[i];
	if (cpu->is_avail ())
	{
	    if (cpu->is_current_cpu ())
	    {
		cpu->set_active ();
		if (i != 0)
		{
		    // Make sure that CPU 0 is the boot cpu
		    ia64_cpu_t tmp = processors[i];
		    processors[i] = processors[0];
		    processors[0] = tmp;
		}
	    }
	    numcpus++;
	}
    }
       
    TRACE_INIT ("Found %d active CPUs, boot CPU is %02x:%02x\n",
		numcpus, processors[0].get_id (), processors[0].get_eid ());

#endif /* CONFIG_SMP */

    // Locate and initialize I/O APICs

    acpi_madt_iosapic_t * ioap;
    for (word_t i = 0; (ioap = madt->iosapic (i)) != NULL; i++)
    {
	iosapic_t * iosapic = phys_to_virt_uc ((iosapic_t *) ioap->address);
	if (! dtrmap.is_mapped (iosapic))
	{
	    translation_t tr (true, translation_t::uncacheable, true, true,
			      0, translation_t::rwx,
			      (addr_t) ioap->address, true);
	    dtrmap.add_map (tr, iosapic, HUGE_PGSIZE, 0);
	}

	TRACE_INIT ("Found I/O SAPIC @ %p: Id 0x%02x, Vectors %d-%d\n",
		    ioap->address, ioap->id, ioap->irq_base,
		    ioap->irq_base + iosapic->get_version ().max_redir);

	init_iosapic (iosapic, ioap->irq_base);
    }

    addr_t local_apic = 0;
    if (madt->local_apic_addr)
	local_apic = phys_to_virt_uc ((addr_t) (word_t) madt->local_apic_addr);

    // Free up temporary ACPI mapping

    if (mapped_addr)
	dtrmap.free_map (mapped_addr);

    // Map local apic as uncacheable memory

    if (local_apic && ! dtrmap.is_mapped (local_apic))
    {
	translation_t tr(true, translation_t::uncacheable, 
			 true, true, 0, translation_t::rw,
			 (addr_t) (word_t) madt->local_apic_addr, true);
	dtrmap.add_map (tr, (addr_t) local_apic, HUGE_PGSIZE, 0);
    }
}


void intctrl_t::init_iosapic (iosapic_t * iosapic, word_t irq_base)
{
    word_t maxirq = iosapic->get_version ().max_redir;

    // Initialize all I/O SAPIC interrupts.  By default all interrupts
    // are steered to CPU 0xFF:0xFF.  The kernel re-routes them later
    // via set_cpu().

    for (word_t i = 0; i <= maxirq; i++)
    {
	iosapic_redir_entry_t * r = &redir[irq_base + i];
	r->set (iosapic, i);

	if (irq_base + i < 16)
	    r->entry.set (INTERRUPT_VECTOR_OFFSET + irq_base + i,
			  iosapic_redir_t::high_active,
			  iosapic_redir_t::edge,
			  true, 0xffff);
	else
	    r->entry.set (INTERRUPT_VECTOR_OFFSET + irq_base + i,
			  iosapic_redir_t::low_active,
			  iosapic_redir_t::level,
			  true, 0xffff);

	register_handler (INTERRUPT_VECTOR_OFFSET + irq_base + i, irq_handler);

	// Make sure that interrupt is masked and not routed to any CPU.

	synch (irq_base + i);
	if (! r->entry.is_edge_triggered ())
	    eoi (irq_base + i);
    }

    // Possibly increase number of available IRQs.  Any holes in the
    // IRQ list are handled correctly by not having the IRQ entries
    // marked as valid.

    if (irq_base + maxirq + 1 > num_irqs)
	num_irqs = irq_base + maxirq + 1;
}


void intctrl_t::set_cpu (word_t irq, word_t cpu)
{
#if defined(CONFIG_SMP)
    word_t id_eid = processors[cpu].get_id_eid ();
#else
    word_t id_eid = (cr_get_lid ().id << 8) | (cr_get_lid ().eid);
#endif

    if (redir[irq].entry.dest_id_eid != id_eid)
    {
	redir[irq].entry.set_cpu (id_eid);
	synch (irq);
	unmask (irq);
    }
}

word_t intctrl_t::get_number_irqs (void)
{
    return num_irqs;
}

bool intctrl_t::is_irq_available (word_t irq)
{
    return redir[irq].is_valid ();
}

void intctrl_t::mask (word_t irq)
{
    ASSERT (redir[irq].is_valid ());

    if (redir[irq].entry.is_edge_triggered ())
    {
	// Do not mask on the I/O SAPIC to avoid blocking all
	// interrupts.
	redir[irq].entry.mask_irq ();
    }
    else
    {
	// Can safely mask interrupts on the I/O SAPIC.
	redir[irq].entry.mask_irq ();
	synch (irq, true);
    }
}

bool intctrl_t::unmask (word_t irq)
{
    ASSERT (redir[irq].is_valid ());

    if (redir[irq].entry.is_edge_triggered ())
    {
	// Check if any interrupts have arrived while beeing masked.
	if (redir[irq].pending)
	{
	    redir[irq].pending = false;
	    return true;
	}
	redir[irq].entry.unmask_irq ();
    }
    else
    {
	redir[irq].entry.unmask_irq ();
	synch (irq, true);
    }

    return false;
}

void intctrl_t::enable (word_t irq)
{
    redir[irq].entry.unmask_irq ();
    synch (irq, true);
}

void intctrl_t::disable (word_t irq)
{
    redir[irq].pending = false;
    redir[irq].entry.mask_irq ();
    synch (irq, true);
}

INLINE void intctrl_t::handle_irq (word_t irq)
{
    if (redir[irq].entry.is_edge_triggered () &&
	redir[irq].entry.mask)
    {
	// Just mark interrupt as pending so that we can receive it
	// when unmasking the interrupt.
	redir[irq].pending = true;
	mask (irq);
	return;
    }

    mask (irq);

    if (! redir[irq].entry.is_edge_triggered ())
	eoi (irq);

    handle_interrupt (irq);
}

static void irq_handler (word_t vec, ia64_exception_context_t * frame)
{
    get_interrupt_ctrl ()->handle_irq (vec - INTERRUPT_VECTOR_OFFSET);
}

static void dummy_irq_handler (word_t irq, ia64_exception_context_t * frame)
{
}
