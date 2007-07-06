/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/ofio.cc
 * Description:	IBM Open Firmware RTAS and serial IO.
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
 * $Id: ofio.cc,v 1.13 2006/11/17 17:16:07 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include <kdb/console.h>
#include INC_PLAT(prom.h)
#include INC_ARCH(rtas.h)
#include INC_ARCH(string.h)
#include INC_ARCH(1275tree.h)	// the device tree
#include INC_ARCH(segment.h)
#include INC_GLUE(space.h)
#include INC_GLUE(pgent_inline.h)
#include INC_ARCH(cache.h)


#define NULL_NAME	"NULL"
#define RTAS_NAME	"rtas"
#define SERIAL_NAME	"serial"

static void switch_console( const char *name )
{
    for( int i = 0; kdb_consoles[i].name; i++ )
	if( !strcmp(kdb_consoles[i].name, name) )
	{
	    kdb_current_console = i;
	    return;
	}
}

/****************************************************************************
 *
 *    NULL console support
 *
 ****************************************************************************/

static void putc_null( char c )
{
}

static char getc_null( bool block )
{
    return 0;
}

#if defined(CONFIG_KDB_CONS_RTAS)

/****************************************************************************
 *
 *    IBM RTAS (Front display) console support
 *
 ****************************************************************************/

static u32_t putc_token = 0;

SECTION( ".init" )
void init_rtas_console()
{
    if (!get_rtas()->get_token( "display-character", &putc_token ))
    {
	printf( "Failed to open rtas console\r\n" );
	return;
    }
    switch_console(RTAS_NAME);
}

static void putc_rtas( char c )
{
    if (putc_token)
    {
	if (c == '\n')
	    get_rtas()->rtas_call( putc_token, 1, 1, NULL, '\r' );

	get_rtas()->rtas_call( putc_token, 1, 1, NULL, c );
    }
}

static char getc_rtas( bool block )
{
    return 0;
}

#endif /* CONFIG_KDB_CONS_RTAS */

/****************************************************************************
 *
 *    Serial console support
 *
 ****************************************************************************/

struct serial_ns16550 {
    /* this struct must be packed */
    unsigned char rbr;  /* 0 */
    unsigned char ier;  /* 1 */
    unsigned char fcr;  /* 2 */
    unsigned char lcr;  /* 3 */
    unsigned char mcr;  /* 4 */
    unsigned char lsr;  /* 5 */
    unsigned char msr;  /* 6 */
    unsigned char scr;  /* 7 */
};

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier
#define dlab lcr

#define LSR_DR		0x01	/* Data ready */
#define LSR_OE		0x02	/* Overrun */
#define LSR_PE		0x04	/* Parity error */
#define LSR_FE		0x08	/* Framing error */
#define LSR_BI		0x10	/* Break */
#define LSR_THRE	0x20	/* Xmit holding register empty */
#define LSR_TEMT	0x40	/* Xmitter empty */
#define LSR_ERR		0x80	/* Error */

static volatile struct serial_ns16550 *serial_regs = 0;

SECTION( ".init" )
void init_serial_console()
{
    of1275_device_t *aliases;
    of1275_device_t *serial_dev;
    of1275_device_t *isa_dev;
    of1275_device_t *pci_dev;
    of1275_isa_reg_property *isa_reg;
    of1275_pci_ranges *pci_range;
    
    char *path, *type;
    u32_t len, *cellptr, adrcells, sizcells;

    serial_regs = NULL;
 
    aliases = get_of1275_tree()->find( "/aliases" );
    if (!aliases) goto error;

    if (!aliases->get_prop( "serial", &path, &len ))
	goto error;

    if (!(serial_dev = get_of1275_tree()->find( path )))
	goto error;

    if (!serial_dev->get_prop( "reg", (char**)&isa_reg, &len ))
	goto error;

    if (!(isa_dev = get_of1275_tree()->get_parent( serial_dev )))
	goto error;

    if (!isa_dev->get_prop( "device_type", (char**)&type, &len ))
	goto error;

    if (strncmp(type, "isa", 3)) {
	printf("\ninvalid parent"); goto error;
    }

    if (!( pci_dev = get_of1275_tree()->get_parent( isa_dev ) ))
	goto error;

    if (!pci_dev->get_prop( "device_type", (char**)&type, &len ))
	goto error;

    if (strncmp(type, "pci", 3)) {
	printf("\ninvalid parent"); goto error;
    }

    if (!pci_dev->get_prop( "ranges", (char**)&pci_range, &len ))
	goto error;

    if (!pci_dev->get_prop( "#address-cells", (char **)&cellptr, &len ))
	goto error;

    adrcells = *cellptr;

    if (!pci_dev->get_prop( "#size-cells", (char **)&cellptr, &len ))
	goto error;

    sizcells = *cellptr;

    if (adrcells != 3) {
	printf("invalid address-cells"); goto error;
    }
    if ((sizcells < 1) || (sizcells > 2)) {
	printf("invalid size-cells"); goto error;
    }

    if (sizcells == 1)
	serial_regs = (struct serial_ns16550*)((word_t)(pci_range->pci32.phys) + isa_reg->address);
    else
	serial_regs = (struct serial_ns16550*)(((((word_t)pci_range->pci64.phys_hi) << 32)
		      | (pci_range->pci64.phys_lo)) + isa_reg->address);

    {
	pgent_t pg;

	/* XXX - we should lookup mapping first */
//#ifdef CONFIG_POWERPC64_LARGE_PAGES
//	pgent_t::pgsize_e size = pgent_t::size_16m;
//#else
	pgent_t::pgsize_e size = pgent_t::size_4k;
//#endif
	/* Create a page table entry, noexecute, nocache */
	pg.set_entry( get_kernel_space(), size, (addr_t)serial_regs,
		      6, pgent_t::cache_inhibit, true );
    
	serial_regs = (struct serial_ns16550*)((word_t)serial_regs | DEVICE_AREA_START);

	/* Insert the kernel mapping, bolted */
	get_pghash()->insert_mapping( get_kernel_space(),
			(addr_t)serial_regs, &pg, size, true );
    }

    serial_regs->lcr = 0x00; eieio();
    serial_regs->ier = 0xFF; eieio();
    serial_regs->ier = 0x00; eieio();
    serial_regs->lcr = 0x80; eieio();	/* Access baud rate */
    serial_regs->dll = 12;   eieio();	/* 1 = 115200,  2 = 57600, 3 = 38400, 12 = 9600 baud */
    serial_regs->dlm = 0;    eieio();	/* dll >> 8 which should be zero for fast rates; */
    serial_regs->lcr = 0x03; eieio();	/* 8 data, 1 stop, no parity */
    serial_regs->mcr = 0x03; eieio();	/* RTS/DTR */
    serial_regs->fcr = 0x07; eieio();	/* Clear & enable FIFOs */

    switch_console(SERIAL_NAME);

    printf( "Opened serial port at %p\n", serial_regs );
    return;

error:
    printf( "\nSerial Init Error" );
    return;
}

static void putc_serial( char c )
{
    if ( serial_regs )
    {
	while (( serial_regs->lsr & LSR_THRE ) == 0 );

	serial_regs->thr = c; eieio();
	if ( c == '\n' )
	    putc_serial( '\r' );
    }
}

static char getc_serial( bool block )
{
    if ( serial_regs )
    {
	if (( serial_regs->lsr & LSR_DR ) == 0 )
	{
	    if (!block)
		return (signed char)-1;

	    while (( serial_regs->lsr & LSR_DR ) == 0 );
	}
	return serial_regs->rbr;
    }
    return 0;
}


#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)

{
    if (( serial_regs->lsr & LSR_DR ) != 0 )
    {
	if (serial_regs->rbr == 27)
	    enter_kdebug("breakin");
    }
}
#endif

/****************************************************************************
 *
 *    Console registration
 *
 ****************************************************************************/

kdb_console_t kdb_consoles[] = {
    { NULL_NAME, NULL, putc_null, getc_null },
#if defined(CONFIG_KDB_CONS_RTAS)
    { RTAS_NAME, NULL, putc_rtas, getc_rtas },
#endif
    { SERIAL_NAME, NULL, putc_serial, getc_serial },
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

