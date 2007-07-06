/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/generic/acpi.cc
 * Description:   Kernel deubgger ACPI acccess
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
 * $Id: acpi.cc,v 1.4 2003/09/24 19:05:11 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/input.h>

#include <acpi.h>

#include INC_GLUE(hwspace.h)


/**
 * Root System Descriptor Pointer.
 */
static acpi_rsdp_t * rsdp = NULL;

/**
 * Root System Description Table.
 */
static acpi_rsdt_t * rsdt = NULL;

/**
 * Extended System Description Table.
 */
static acpi_xsdt_t * xsdt = NULL;


void SECTION (SEC_KDEBUG) dump_apic (acpi_madt_t * madt);

static void SECTION (SEC_KDEBUG)
dump_acpi_header (acpi_thead_t & h, char * prefix = "")
{
    printf ("%sOEM:    [\"%.6s\", tid: \"%.8s\", rev: %d]\n"
	    "%sVendor: [id: 0x%x, rev: 0x%x]\n",
	    prefix, phys_to_virt (&h.oem_id), phys_to_virt (&h.oem_tid),
	    h.oem_rev,
	    prefix, h.creator_id, h.creator_rev);
}


/*
 * Command group for ACPI commands.
 */

DECLARE_CMD_GROUP (acpi);

DECLARE_CMD (cmd_acpi, arch, 'a', "acpi", "ACPI access");

CMD (cmd_acpi, cg)
{
    if (rsdp == NULL)
    {
	rsdp = acpi_rsdp_t::locate ();
	if (rsdp == NULL)
	{
	    printf ("Could not locate ACPI info.\n");
	    return CMD_NOQUIT;
	}

	rsdt = rsdp->rsdt ();
	xsdt = rsdp->xsdt ();
    }

    return acpi.interact (cg, "acpi");
}


DECLARE_CMD (cmd_acpi_dump, acpi, 'd', "dump",
	     "dump system description table");

CMD (cmd_acpi_dump, cg)
{
    printf ("ACPI rev: %d, OEM: \"%.6s\"\n", rsdp->rev, rsdp->oemid);
    if (rsdt)
    {
	printf ("RSDT @ %p\n", rsdt);
	dump_acpi_header (rsdt->header, "  ");
	printf ("\n");

	word_t num_entries = (xsdt->header.len - sizeof (acpi_thead_t)) /
	    sizeof (xsdt->ptrs[0]);

	for (word_t i = 0; i < num_entries; i++)
	{
	    acpi_thead_t * t = (acpi_thead_t *) ((word_t) xsdt->ptrs[i]);
	    printf ("  %.4s @ %p\n", t->sig, t);
	    dump_acpi_header (*t, "    ");

	    if (t->sig[0] == 'A' && t->sig[1] == 'P' &&
		t->sig[2] == 'I' && t->sig[3] == 'C')
		dump_apic ((acpi_madt_t *) t);
	}
    }

    if (xsdt)
    {
	printf ("XSDT @ %p\n", xsdt);
	dump_acpi_header (xsdt->header, "  ");
	printf ("\n");

	word_t num_entries = (xsdt->header.len - sizeof (acpi_thead_t)) /
	    sizeof (xsdt->ptrs[0]);

	for (word_t i = 0; i < num_entries; i++)
	{
	    acpi_thead_t * t = (acpi_thead_t *) ((word_t) xsdt->ptrs[i]);
	    printf ("  %.4s @ %p\n", t->sig, t);
	    dump_acpi_header (*t, "    ");

	    if (t->sig[0] == 'A' && t->sig[1] == 'P' &&
		t->sig[2] == 'I' && t->sig[3] == 'C')
		dump_apic ((acpi_madt_t *) t);
	}
    }

    return CMD_NOQUIT;
}

void dump_apic (acpi_madt_t * madt)
{
    printf ("    Local APIC @ %p\n", madt->local_apic_addr);
    for (word_t i = 0; i < (madt->header.len - sizeof (acpi_madt_t));)
    {
	acpi_madt_hdr_t * h = (acpi_madt_hdr_t *) &madt->data[i];
	switch (h->type)
	{
	case 0:
	{
	    // Local APIC
	    acpi_madt_lapic_t * lapic = (acpi_madt_lapic_t *) h;
	    printf ("      Local APIC  ");
	    printf ("[Id: 0x%x, CPU Id: 0x%x, %s]\n",
		    lapic->id, lapic->apic_processor_id,
		    lapic->flags.enabled ? "enabled" : "disabled");
	    break;
	}    
	case 1:
	{
	    // I/O Apic
	    acpi_madt_ioapic_t * ioapic = (acpi_madt_ioapic_t *) h;
	    printf ("      I/O APIC  ");
	    printf ("[Id: 0x%x, IRQ base: %d, Addr: %p]\n",
		    ioapic->id, ioapic->irq_base, ioapic->address);
	    break;
	}
	case 2:
	{
	    // Interrupt Source Override
	    acpi_madt_irq_t * irq = (acpi_madt_irq_t *) h;
	    acpi_madt_irq_t::polarity_t p = irq->get_polarity ();
	    acpi_madt_irq_t::trigger_mode_t t = irq->get_trigger_mode ();
	    printf ("      Interrupt Override  ");
	    printf ("[%s, Bus IRQ: %d, Glob IRQ: %d, Pol: %s, Trigger: %s]\n",
		    irq->src_bus == 0 ? "ISA" : "unknown bus",
		    irq->src_irq, irq->dest,
		    p == acpi_madt_irq_t::conform_polarity ? "conform" :
		    p == acpi_madt_irq_t::active_high ? "active high" :
		    p == acpi_madt_irq_t::active_low ? "active low" : "?",
		    t == acpi_madt_irq_t::conform_trigger ? "conform" :
		    t == acpi_madt_irq_t::edge ? "edge" :
		    t == acpi_madt_irq_t::level ? "level" : "?");
	    break;
	}
	case 3:
	{
	    // NMI Source
	    acpi_madt_nmi_t * nmi = (acpi_madt_nmi_t *) h;
	    acpi_madt_nmi_t::polarity_t p = nmi->get_polarity ();
	    acpi_madt_nmi_t::trigger_mode_t t = nmi->get_trigger_mode ();
	    printf ("      NMI Source  ");
	    printf ("[Glob IRQ: %d, Pol: %s, Trigger: %s]\n",
		    nmi->irq,
		    p == acpi_madt_nmi_t::conform_polarity ? "conform" :
		    p == acpi_madt_nmi_t::active_high ? "active high" :
		    p == acpi_madt_nmi_t::active_low ? "active low" : "?",
		    t == acpi_madt_nmi_t::conform_trigger ? "conform" :
		    t == acpi_madt_nmi_t::edge ? "edge" :
		    t == acpi_madt_nmi_t::level ? "level" : "?");
	    break;
	}
	case 4:
	{
	    // Local APIC NMI
	    printf ("      Local APIC NMI\n");
	    break;
	}
	case 5:
	{
	    // Local APIC Address Override
	    printf ("      Local APIC Address Override\n");
	    break;
	}
	case 6:
	{
	    // I/O SAPIC
	    acpi_madt_iosapic_t * iosapic = (acpi_madt_iosapic_t *) h;
	    printf ("      I/O SAPIC  ");
	    printf ("[Id: 0x%x, IRQ base: %d, Addr: %p]\n", iosapic->id,
		    iosapic->irq_base, iosapic->address);
	    break;
	}
	case 7:
	{
	    // Local SAPIC
	    acpi_madt_lsapic_t * lsapic = (acpi_madt_lsapic_t *) h;
	    printf ("      Local SAPIC  ");
	    printf ("[Id: 0x%x:0x%x, CPU Id: 0x%x, %s]\n",
		    lsapic->id, lsapic->eid, lsapic->apic_processor_id,
		    lsapic->flags.enabled ? "enabled" : "disabled");
	    break;
	}
	case 8:
	{
	    // Platform Interrupt Source
	    printf ("      Platform Interrupt Source\n");
	    break;
	}
	}

	i += h->len;
    }
}
