/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	src/platform/ofppc/opic.cc
 * Description:	The open pic driver.
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
 * $Id: opic.cc,v 1.26 2003/12/11 13:10:00 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include INC_ARCH(string.h)
#include INC_ARCH(bat.h)
#include INC_ARCH(pvr.h)

#include INC_PLAT(opic.h)
#include INC_PLAT(1275tree.h)

#include INC_GLUE(intctrl.h)
#include INC_GLUE(bat.h)

//#define TRACE_OPIC TRACEF
#define TRACE_OPIC(x...)

/*
 * Documentation:
 * It looks as if the OpenPIC standard has slipped into obscurity.  I have
 * found many references to AMD's "The Open Programmable Interrupt Controller
 * (PIC) Register Interface Specification Revision 1.2", issued October 1995.
 * I have also found references to IBM's "PowerPC Multiprocessor Interrupt
 * Controller (MPIC)", from January 1996.  Yet neither IBM's nor AMD's
 * websites offer their old OpenPIC resources.  And supposedly AMD
 * offered an email address openpic@amd.com to which one could submit
 * requests for documentation.  But no longer ...
 * So this implementation of the OpenPIC driver is based on information
 * extracted from NetBSD and PSIM.  And thus it is a combination of
 * knowledge about the OpenPIC and the MPIC.  My test bed is an Apple Pismo
 * PowerBook, which implements an IBM MPIC2 in Apple's KeyLargo chip.
 *
 * The Open Firmware device tree is queried for multiple pieces of information
 * related to the interrupt fabric.  First, one must locate the OpenPIC
 * device.  Second, one must figure out which devices connect to the
 * OpenPIC, and how they connect.
 *
 * Some Open Firmware documentation:
 *
 * 1. Open Firmware Recommended Practice: Interrupt Mapping Version 0.9, 
 *    Unapproved DRAFT.  Supposedly a version 1.0 of the document exists.
 *    http://playground.sun.com/1275/practice/imap/imap0_9d.pdf
 *
 * 2. PCI Bus Binding to: IEEE Std 1275-1994 Standard for Boot 
 *    (Initialization Configuration) Firmware, Revision 2.1, August 29 1998
 *    http://playground.sun.com/1275/bindings/ppc/release/ppc-2_1.ps
 *    http://bananajr6000.apple.com/1275/bindings/pci/pci2_1.pdf
 */

/*
 * #address-cells : the number of cells required to represent physical 
 * addresses.  (example: PCI uses 3 cells to encode lots of info)
 * #size-cells : the size of cells (in words) used to represent physical 
 * addresses.  (example: PCI uses 64-bit addresses)
 *
 * PCI host bridge nodes:
 * - "ranges" property : an array of tuples (child-phys, parent-phys, size).
 *   The first tuple corresponds to I/O space.  The second tuple relates to
 *   memory space.
 *
 * PCI client nodes:
 * - "reg" property : the first component is the Configuration Space address
 *   of the function's configuration registers, which is also the 
 *   function's "unit-number".  The second component is the size
 *   of the configuration space. 
 * - "interrupts" property: an integer that represents the PCI interrupt line
 *   used by the device.  INTA=1, INTB=2, INTC=3, INTD=4.
 *
 * CHRP Interrupt Controller Nodes:
 * - "name" property = "interrupt-controller"
 * - "device_type" property = "open-pic"
 * - "reg" property : a list of pairs, base physical address and size.
 *   The first pair is for the base controller.  Successive entries refer
 *   to additional controllers.
 * - "compatible" property = "chrp,open-pic"
 * - "interrupt-ranges" property : a list of pairs, the starting interrupt
 *   number and the number of interrupts handle by the controller.  This
 *   has a 1:1 correlation with the entries in the "reg" property.
 *
 * - when reading/writing ports, follow with eieio
 * - if the page attributes are caching-inhibited and guarded, the eieio
 *   can be loosened on store instructions.
 *
 */

/*
 * From http://playground.sun.com/1275/bindings/chrp/chrp1_8x.ps (changes
 * to CHRP PowerPC bindings, version 1.8).
 *
 * OpenPIC's interrupt sense:
 * 0 = low to high edge sensitive type
 * 1 = active low level sensitive type
 *
 * for the MPIC:
 *   polarity	sense	MPIC state
 *   --------	-----	----------
 *   0		0	negative edge
 *   0		1	active low
 *   1		0	positive edge
 *   1		1	active high
 */

class pci_addr_t {
public:
    union {
	struct {
	    u32_t relocatable	: 1;	// n-bit
	    u32_t prefetchable	: 1;	// p-bit
	    u32_t special	: 1;	// t-bit
	    u32_t zero		: 3;	// should be 000
	    u32_t space_code	: 2;
	    u32_t bus		: 8;
	    u32_t device	: 5;
	    u32_t function	: 3;
	    u32_t reg		: 8;
	} info;
	u32_t info_raw;
    };
    u32_t phys_hi;
    u32_t phys_lo;
};

class pci_reg_t {
public:
    pci_addr_t addr;
    u32_t size_hi;
    u32_t size_lo;
};

class macio_reg_t {
public:
    u32_t offset;
    u32_t size;
};

class opic_int_map_t {
public:
    word_t phandle;
    word_t source;
    word_t sense;
};

class opic_interrupt_t {
public:
    word_t source;
    word_t sense;
};

intctrl_t intctrl;


SECTION(".init") of1275_device_t *intctrl_t::find_opic()
{
    word_t len;
    char *data;
    of1275_device_t *dev_opic, *dev_bus, *dev_pci_client;
    pci_reg_t *assigned_addr;
    macio_reg_t *opic_reg;

    /* Look for the open-pic device. */
    dev_opic = get_of1275_tree()->find_device_type( "open-pic" );
    if( dev_opic == NULL ) {
	printf( "Error: unable to find an open-pic device.\n" );
	return NULL;
    }
    TRACE_INIT( "The open-pic device: %s\n", dev_opic->get_name() );

    /* Obtain the open-pic's parent handle. */
    dev_bus = get_of1275_tree()->get_parent( dev_opic );
    if( dev_bus == NULL ) {
	printf( "Error: unable to find the open-pic's bus.\n" );
	return NULL;
    }
    TRACE_INIT( "The opic-pic bus: %s\n", dev_bus->get_name() );

    /* Figure out whether the open-pic is attached to the pci bus,
     * or a mac-io.
     */
    if( !dev_bus->get_prop("device_type", &data, &len) ) {
	printf( "Error: unable to determine the device type of the open-pic's"
		" bus.\n" );
	return NULL;
    } 
    else if( !strcmp(data, "pci") )
	dev_pci_client = dev_opic;
    else if( !strcmp(data, "mac-io") )
	dev_pci_client = dev_bus;
    else {
	printf( "Error: expected the open-pic to be attached to mac-io or a\n"
		"pci bus.  But it is attached to a bus of type '%s'.\n",
		data );
	return NULL;
    }

    /* Look for the assigned-addresses property.
     */
    if( dev_pci_client->get_prop("assigned-addresses", (char **)&assigned_addr, &len) && (len == sizeof(pci_reg_t)) )
    {
	this->opic_paddr = assigned_addr[0].addr.phys_lo;
	this->opic_size = assigned_addr[0].size_lo;
    }

    /* Look for the "reg" property of the open-pic, assuming it is on the
     * pci bus.  This is a backup.  We should use the "assigned-addresses"
     * property.
     */
    else if( dev_pci_client->get_prop("reg", (char **)&assigned_addr, &len) 
	    && (len == 2*sizeof(pci_reg_t)) )
    {
	this->opic_paddr = assigned_addr[1].addr.phys_lo;
	this->opic_size = assigned_addr[1].size_lo;
    }

    else {
	printf( "Error: unable to determine the address range of the open-pic's"
		"bus.\n" );
	return NULL;
    }

    if( dev_pci_client == dev_bus )
    {
	/* We have the address for the mac-io device.  Get the offset and size
	 * of the open-pic.
	 */
	if( !dev_opic->get_prop("reg", (char **)&opic_reg, &len) ||
		(len != sizeof(macio_reg_t)) )
	{
	    printf( "Error: unable to find the 'reg' property of the"
		    " open-pic.\n" );
	    return NULL;
	}
	this->opic_paddr += opic_reg->offset;
	this->opic_size = opic_reg->size;
    }

    return dev_opic;
}


SECTION(".init") void intctrl_t::bat_map()
{
    if( this->opic_paddr == 0 )
	return;

    // TODO: remove glue deps
    this->opic_vaddr = DEVICE_AREA_START;

    ppc_bat_t opic_bat;
    opic_bat.raw.upper = opic_bat.raw.lower = 0;
    opic_bat.x.bepi = this->opic_vaddr >> BAT_BEPI;
    opic_bat.x.bl = BAT_BL_256K;
    opic_bat.x.vs = 1;
    opic_bat.x.brpn = this->opic_paddr >> BAT_BRPN;
    opic_bat.x.w = 0;
    opic_bat.x.i = 1;	/* caching inhibited	*/
    opic_bat.x.m = 1;	/* memory coherent	*/
    opic_bat.x.g = 1;	/* gaurded access	*/
    opic_bat.x.pp = BAT_PP_READ_WRITE;
    ppc_set_opic_dbat( l, opic_bat.raw.lower );
    ppc_set_opic_dbat( u, opic_bat.raw.upper );
    isync();

    TRACE_OPIC( "mapped open-pic to %p (paddr %p)\n", 
	        this->opic_vaddr, this->opic_paddr);
}

SECTION(".init") void intctrl_t::init_intctrl()
{
    opic_feature0_t feature0;
    u32_t freq;
    u32_t num_sources;

    if( this->opic_paddr == 0 )
	return;

    feature0 = this->get_feature0();
    num_sources = feature0.x.last_source;
    this->num_cpus = feature0.x.last_cpu;
    if( !powerpc_version_t::read().is_psim() )
    {
    	num_sources++;
	this->num_cpus++;
    }
    this->last_vector = num_sources + intctrl_t::source_start_vec;

    TRACE_INIT( "Open-Pic version %d, supports %d cpu's and %d "
	        "interrupt sources\n",
	        feature0.x.version, this->num_cpus, num_sources );

    freq = this->get_timer_freq();
    TRACE_INIT( "Open-Pic timer freq %d.%06d MHz\n",
	        freq / 1000000, freq % 1000000 );

    this->disable_8259_pass_through();
}

SECTION(".init") void intctrl_t::init_arch()
{
    ASSERT( DEVICE_AREA_BAT_SIZE == BAT_256K_PAGE_SIZE );
    ASSERT( (this->opic_paddr % BAT_256K_PAGE_SIZE) == 0 );
    ASSERT( (this->opic_vaddr % BAT_256K_PAGE_SIZE) == 0 );

    this->opic_vaddr = 0;
    this->opic_paddr = 0;
    this->opic_size = 0;

    of1275_device_t *dev_opic = this->find_opic();
    if( dev_opic == NULL )
    {
	printf( ">> Running without an interrupt controller! <<\n" );
	return;
    }
    TRACE_INIT( "Found an open-pic at 0x%x, size 0x%x.\n", 
	        this->opic_paddr, this->opic_size );

    this->bat_map();
    this->init_intctrl();

    this->init_timers();
    this->init_spurious();
    this->init_all_ipi();
    this->scan_interrupt_tree( dev_opic );
}

SECTION(".init") void intctrl_t::init_cpu( word_t cpu )
{
    if( this->opic_paddr == 0 )
	return;

    // Clear any pending interrupts.  Otherwise L4 will throw a fit
    // if we accept any outstanding interrupts.
    u32_t cnt = 0;
    while( (cnt < OPIC_NUM_VECTORS) && 
	    (this->get_irq_ack(cpu).x.vector != intctrl_t::spurious_vec) ) {
	this->clear_eoi(cpu);
	cnt++;
    }
 
    // Enable interrupts lower than priority 0.
    this->set_current_task_priority( 0, cpu );
}

#if defined(CONFIG_SMP)
SECTION(".init") void intctrl_t::start_new_cpu( word_t cpu )
{
    if( this->opic_paddr == 0 )
	return;

    this->init_cpu( cpu );
    this->send_ipi0( get_current_cpu(), 1 << cpu );
}
#endif

SECTION(".init") void intctrl_t::disable_8259_pass_through()
{
    opic_global_config0_t val = this->get_global_config0();
    val.x.disable_8259 = 1;
    this->set_global_config0( val );
}

SECTION(".init") void intctrl_t::init_timers()
{
    opic_vector_priority_t info;
    word_t timer;

    for( timer = 0; timer < OPIC_NUM_TIMERS; timer++ ) {
	/* Initialize as disabled. */
	info.raw = 0;
	info.x.vector = intctrl_t::timer0_vec + timer;
	TRACE_OPIC( "timer %d, vector %d\n", timer, info.x.vector );
	info.x.mask = 1;  // Disable.
	info.x.priority = intctrl_t::priority_timer;
	this->set_timer_vector_priority( timer, info );
	this->set_timer_cpu( timer, OPIC_CPU_DISABLE );
    }
}

SECTION(".init") void intctrl_t::init_source( int source, int sense )
{
    opic_vector_priority_t info;

    info.raw = 0;
    info.x.vector = intctrl_t::source_start_vec + source;
    info.x.mask = 1;	// Disable.
    info.x.priority = intctrl_t::priority_std_source;
    info.x.level = sense;
    info.x.positive = !sense;

    if( info.x.vector >= source_end_vec )
    {
	printf( "Error: unable to accomodate interrupt source %d.\n", source );
	return;
    }

    this->mask_source( source );
    this->set_source_vector_priority( source, info );
    this->set_source_cpu( source, get_current_cpu() );
}

SECTION(".init") void intctrl_t::init_all_ipi()
{
    opic_vector_priority_t ipi;

    // Enable ipi 0
    ipi.raw = 0;
    ipi.x.vector = intctrl_t::ipi0_vec;
    ipi.x.mask = 0; // Enable.
    ipi.x.priority = intctrl_t::priority_std_ipi;
    TRACE_OPIC( "ipi0 vector %d\n", ipi.x.vector );
    this->write_vector_priority( OPIC_IPI0_PRIORITY_REG, ipi );

    // Disable the other ipi vectors.
    ipi.raw = 0;
    ipi.x.vector = intctrl_t::spurious_vec;
    ipi.x.mask = 1;  // Disable!
    ipi.x.priority = priority_spurious;
    this->write_vector_priority( OPIC_IPI1_PRIORITY_REG, ipi );
    this->write_vector_priority( OPIC_IPI2_PRIORITY_REG, ipi );
    this->write_vector_priority( OPIC_IPI3_PRIORITY_REG, ipi );
}

SECTION(".init") void intctrl_t::init_spurious()
{
    opic_vector_priority_t info;

    info.raw = 0;
    info.x.vector = intctrl_t::spurious_vec;
    info.x.mask = 0;
    info.x.priority = intctrl_t::priority_spurious;
    TRACE_OPIC( "spurious vector %d\n", info.x.vector );
    this->write_vector_priority( OPIC_SPURIOUS_REG, info );
}

/*****************************************************************************/

void intctrl_t::write_vector_priority( word_t reg, opic_vector_priority_t val )
{
    opic_vector_priority_t t;

    t = this->get_vector_priority( reg );
    t.x.mask = 1;
    while( this->get_vector_priority(reg).x.activity ) ;
    this->out32le( reg, val.raw );
}

/*****************************************************************************/

SECTION(".kdebug") void intctrl_t::enable_timer( word_t timer )
{
    ASSERT( timer < OPIC_NUM_TIMERS );
    this->set_timer_freq( 4667 );
    this->set_timer_cpu( timer, 1 );
    this->restart_timer( timer );
}

void intctrl_t::handle_irq( word_t irq )
{
    opic_irq_ack_t ack;

    // Retrieve the first pending interrupt.
    ack = this->get_irq_ack( get_current_cpu() );
    //TRACEF( "interrupt vector %d\n", ack.x.vector );

    if( ack.x.vector == intctrl_t::spurious_vec )
	return;
#if defined(CONFIG_SMP)
    if( ack.x.vector == intctrl_t::ipi0_vec ) {
	this->ack( ack.x.vector );
	::handle_smp_ipi( intctrl_t::ipi0_vec );
	return;
    }
#endif

    this->mask_and_ack( ack.x.vector );
    ::handle_interrupt( ack.x.vector );
}

/*****************************************************************************/

/**
 * Walks all device nodes of the Open Firmware device tree and looks for
 * interrupt definitions.
 */
SECTION(".init") void intctrl_t::scan_interrupt_tree( 
	of1275_device_t *dev_opic )
{
    of1275_device_t *dev;

    dev = get_of1275_tree()->first();
    if( !dev )
	return;

    while( dev->is_valid() )
    {
	this->scan_interrupt_map( dev_opic, dev );
	dev = dev->next();
    }
}

/**
 * Checks whether the device uses the open-pic as its interrupt-parent.
 * If so, then it looks for interrupt resources in the "interrupts"
 * property, and in the "interrupt-map" property.
 */
SECTION(".init") void intctrl_t::scan_interrupt_map( 
	of1275_device_t *dev_opic, of1275_device_t *node )
{
    word_t interrupt_parent;
    word_t *map_buf;
    word_t address_cells, interrupt_cells, record_len;
    word_t len, j;
    opic_int_map_t *opic_map;
    opic_interrupt_t *opic_int;

    // Ensure that this device's interrupt-parent points to our open-pic.
    if( !node->get_prop("interrupt-parent", &interrupt_parent) )
	return;
    if( interrupt_parent != dev_opic->get_handle() )
	return;
    TRACE_OPIC( "interrupt client: %s\n", node->get_name() );

    // Extract the device's interrupts property.
    if( node->get_prop("interrupts", (char **)&map_buf, &len) && (len > 0) )
    {
	len /= sizeof(word_t);
	for( j = 0; j < len; j += 2 ) 
	{
	    opic_int = (opic_interrupt_t *)&map_buf[j];
	    if( opic_int->source < OPIC_MAX_SOURCES )
		this->init_source( opic_int->source, opic_int->sense );
	    TRACE_OPIC( "source %d, sense %d\n", 
		    opic_int->source, opic_int->sense );
	}
    }

    // Get the number of address cells.
    if( !node->get_prop("#address-cells", &address_cells) )
	return;

    // Get the number of interrupt cells.
    if( !node->get_prop("#interrupt-cells", &interrupt_cells) )
	return;

    // Get the interrupt map.
    if( !node->get_prop("interrupt-map", (char **)&map_buf, &len) || (len <= 0))
	return;

    // Walk the interrupt map.
    record_len = address_cells + interrupt_cells + 
	sizeof(opic_int_map_t)/sizeof(word_t);
    len /= sizeof(word_t);
    for( j = 0; j < len; j += record_len ) 
    {
	opic_map = (opic_int_map_t *)
	    &map_buf[ j+address_cells+interrupt_cells ];
	if( opic_map->source < OPIC_MAX_SOURCES )
	    this->init_source( opic_map->source, opic_map->sense );
	TRACE_OPIC( "source %d, sense %d\n",
		opic_map->source, opic_map->sense );
    }

}

