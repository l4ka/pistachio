/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/io.cc
 * Description:   Console/serial I/O for IA-64 platforms
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
 * $Id: io.cc,v 1.18 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/
#include INC_ARCH(ioport.h)
#include INC_ARCH(ski.h)
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_PLAT(system_table.h)
#include INC_PLAT(hcdp.h)
#include INC_GLUE(hwspace.h)
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/console.h>
#include <debug.h>

#if !defined(CONFIG_KDB_COMSPEED)
#define CONFIG_KDB_COMSPEED 115200
#endif

#if !defined(CONFIG_KDB_COMPORT)
#define CONFIG_KDB_COMPORT 0x3f8
#endif


#if !defined(CONFIG_KDB_CONS_SKI)
static void init_serial (void) SECTION (".init");
static void init_screen (void) SECTION (".init");
static void putc_serial (char) SECTION (".kdebug");
static char getc_serial (bool) SECTION (".kdebug");
static void putc_screen (char) SECTION (".kdebug");
static char getc_screen (bool) SECTION (".kdebug");

kdb_console_t kdb_consoles[] = {
    { "screen", &init_screen, &putc_screen, &getc_screen },
    { "serial", &init_serial, &putc_serial, &getc_serial },
    KDB_NULL_CONSOLE
};
    
#if defined(CONFIG_KDB_CONS_COM)
word_t kdb_current_console = 1;
#else
word_t kdb_current_console = 0;
#endif


/*
**
** Console I/O functions.
**
*/

#define DISPLAY		phys_to_virt_uc ((char *) 0xb8000)
#define NUM_LINES	(24)
#define NUM_COLS	(80)

static void init_screen (void)
{
    /*
     * Map screen memory uncacheable.
     */

    if (! dtrmap.is_mapped (DISPLAY))
    {
	translation_t tr (1, translation_t::uncacheable, 1, 1, 0,
			  translation_t::rwx, virt_to_phys (DISPLAY), 0);
	dtrmap.add_map (tr, DISPLAY, HUGE_PGSIZE, 0);
    }
}

static void putc_screen (char c)
{
    static unsigned cursor = 0;
    static unsigned color = 7;
    static unsigned new_color = 0;
    static unsigned esc = 0;
    static unsigned esc2 = 0;
    static const unsigned col[] = { 0, 4, 2, 14, 1, 5, 3, 15 };

    if (esc == 1)
    {
	if (c == '[')
	{
	    esc++;
	    return;
	}
    }
    else if (esc == 2)
    {
	switch (c)
	{
	case '0': case '1': case '2':
	case '3': case '4': case '7':
	    esc++;
	    esc2 = c;
	    return;
	}
    }
    else if (esc == 3)
    {
	switch (c)
	{
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	    if (esc2 == '3' || esc2 == '4')
	    {
		// New foreground or background color
		new_color = col[c - '0'];
		esc++;
		return;
	    }
	    break;
	case 'J':
	    if (esc2 == '2')
	    {
		// Clear screen
		for (int i = 0; i < NUM_COLS*NUM_LINES; i++)
		    ((u16_t *) DISPLAY)[i] = (color << 8) + ' ';
		cursor = 0;
		esc = 0;
		return;
	    }
	    break;
	case 'm':
	    switch (esc2)
	    {
	    case '0':
		// Normal text
		color = 7;
		esc = 0;
		return;
	    case '1':
		// Bright text
		color = 15;
		esc = 0;
		return;
	    case  '7':
		// Reversed
		color = (7 << 4);
		esc = 0;
		return;
	    }
	}
    }
    else if (esc == 4)
    {
	if (c == 'm' && esc2 == '3')
	{
	    // Foreground color
	    color = (color & 0xf0) | new_color;
	    esc = 0;
	    return;
	}
	else if (c == 'm' && esc2 == '4')
	{
	    // Background color
	    color = (color & 0x0f) | (new_color << 4);
	    esc = 0;
	    return;
	}
    }

    switch(c) {
    case '\e':
	esc = 1;
	return;
    case '\r':
        cursor -= (cursor % (NUM_COLS*2));
	break;
    case '\n':
	cursor += ((NUM_COLS*2) - (cursor % (NUM_COLS*2)));
	break;
    case '\t':
	cursor += (8 - (cursor % 8));
	break;
    case '\b':
	cursor -= 2;
	break;
    default:
	DISPLAY[cursor++] = c;
	DISPLAY[cursor++] = color;
    }

    esc = 0;

    if ((cursor / (NUM_COLS*2)) == NUM_LINES)
    {
	for (int i = NUM_COLS; i < NUM_COLS*NUM_LINES; i++)
	    ((u16_t *) DISPLAY)[i - NUM_COLS] = ((u16_t *) DISPLAY)[i];
	for (int i = 0; i < NUM_COLS; i++)
	    ((u16_t * )DISPLAY)[NUM_COLS * (NUM_LINES-1) + i] = 0;
	cursor -= (NUM_COLS*2);
    }
}

#define KBD_STATUS_REG		0x64	
#define KBD_CNTL_REG		0x64	
#define KBD_DATA_REG		0x60	

#define KBD_STAT_OBF 		0x01	/* Keyboard output buffer full */

#define kbd_read_input() in_u8 (KBD_DATA_REG)
#define kbd_read_status() in_u8 (KBD_STATUS_REG)

static unsigned char keyb_layout[2][128] =
{
    "\000\0331234567890-=\010\t"			/* 0x00 - 0x0f */
    "qwertyuiop[]\r\000as"				/* 0x10 - 0x1f */
    "dfghjkl;'`\000\\zxcv"				/* 0x20 - 0x2f */
    "bnm,./\000*\000 \000\201\202\203\204\205"		/* 0x30 - 0x3f */
    "\206\207\210\211\212\000\000789-456+1"		/* 0x40 - 0x4f */
    "230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000"
    "\r\000/"						/* 0x60 - 0x6f */
    ,
    "\000\033!@#$%^&*()_+\010\t"			/* 0x00 - 0x0f */
    "QWERTYUIOP{}\r\000AS"				/* 0x10 - 0x1f */
    "DFGHJKL:\"`\000\\ZXCV"				/* 0x20 - 0x2f */
    "BNM<>?\000*\000 \000\201\202\203\204\205"		/* 0x30 - 0x3f */
    "\206\207\210\211\212\000\000789-456+1"		/* 0x40 - 0x4f */
    "230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000"
    "\r\000/"						/* 0x60 - 0x6f */
};

static char getc_screen (bool block)
{
    static u8_t last_key = 0;
    static u8_t shift = 0;
    char c;

    for (;;)
    {
	unsigned char status = kbd_read_status ();
	while (status & KBD_STAT_OBF)
	{
	    u8_t scancode;
	    scancode = kbd_read_input ();

	    // Check for SHIFT-keys
	    if (((scancode & 0x7F) == 42) || ((scancode & 0x7F) == 54))
	    {
		shift = !(scancode & 0x80);
		continue;
	    }

	    // Ignore all other RELEASED-codes
	    if (scancode & 0x80)
		last_key = 0;
	    else if (last_key != scancode)
	    {
		last_key = scancode;
		c = keyb_layout[shift][scancode];
		if (c > 0)
		    return c;
	    }
	}
    }
}



/*
**
** Serial port I/O functions.
**
*/

/* Comport register locations */
#define DATA		0
#define IER		1
#define EIR		2
#define LCR		3
#define MCR		4
#define LSR		5
#define MSR		6
#define DLLO		0
#define DLHI		1

/* LCR related constants */
#define BITS_5          0
#define BITS_6          1
#define BITS_7          2
#define BITS_8          3
#define STOP_ONE	(0 << 2)
#define STOP_TWO	(1 << 2)
#define PARITY_NONE	(0 << 3)
#define PARITY_ODD	(1 << 3)
#define PARITY_EVEN	(3 << 3)
#define PARITY_MARK	(6 << 3)
#define PARITY_SPACE	(7 << 3)
#define BREAK_CONTROL	(1 << 6)
#define DLR_ON          (1 << 7)

/* ISR related constants */
#define RxRDY		(1 << 0)
#define TBE		(1 << 5)

/* Defaults, usually overridden from EFI HCDP table */
static u64_t com_base = CONFIG_KDB_COMPORT;
static u32_t com_baud = CONFIG_KDB_COMSPEED;
static u8_t  com_mmio = 0;

void find_serial (void)
{
    hcdp_table_t *hcdp_table;
    hcdp_dev_t *hcdp_dev;

    hcdp_table = (hcdp_table_t *)efi_config_table.find_table(HCDP_TABLE_GUID);
    if (hcdp_table == NULL)
	return;

    hcdp_table = phys_to_virt(hcdp_table);
    hcdp_dev = hcdp_table->find(HCDP_DEV_CONSOLE);
    if (hcdp_dev == NULL)
	return;

    switch (hcdp_dev->base_addr.id)
    {
	case ACPI_MEM_SPACE:
	    com_mmio = 1;
	    break;
	case ACPI_IO_SPACE:
	    com_mmio = 0;
	    break;
	default:
	    return; /* fall back to defaults */
    }

    com_base = hcdp_dev->base_addr.address ();
    if (hcdp_dev->baud)
	com_baud = hcdp_dev->baud;
}

static void com_out_u8 (u8_t port, u8_t data)
{
    if (com_mmio)
	*(u8_t *)(com_base + port) = data;
    else
	out_u8(com_base + port, data);
}

static u8_t com_in_u8 (u8_t port)
{
    if (com_mmio)
	return *(u8_t *)(com_base + port);
    else
	return in_u8(com_base + port);
}

static void init_serial (void)
{
    static bool initialized = false;
    u16_t dll;

    if (initialized)
	return;
    initialized = true;

    find_serial();

    if (com_mmio)
    {
	com_base = phys_to_virt_uc(com_base);
	if (!dtrmap.is_mapped ((void *)com_base))
	{
	    translation_t tr (1, translation_t::uncacheable, 1, 1, 0,
			      translation_t::rwx,
			      virt_to_phys ((void *)com_base), 0);
	    dtrmap.add_map (tr, (void *)com_base, HUGE_PGSIZE, 0);
	}
    }

    /* 64-bit division otherwise we'd need __udivsi3 */
    dll = 115200UL / com_baud;

    com_out_u8 (LCR, DLR_ON);
    com_out_u8 (DLLO, dll & 0xff);
    com_out_u8 (DLHI, dll >> 8);
    com_out_u8 (LCR, BITS_8 | PARITY_NONE | STOP_ONE); /* 8N1 */
}

void putc_serial (char c)
{
    while (! (com_in_u8 (LSR) & TBE))
	;

    com_out_u8 (DATA, c);

    if (c == '\n')
  	putc_serial ('\r');
}

char getc_serial (bool block)
{
    if (block)
    {
	while (! (com_in_u8 (LSR) & RxRDY))
	    spin (70);
	return com_in_u8 (DATA);
    }
    else
    {
	if (com_in_u8 (LSR) & RxRDY)
	    return com_in_u8 (DATA);
	else
	    return -1;
    }
}

#if defined(CONFIG_KDB_BREAKIN)
void SECTION (".kdebug") kdebug_check_breakin (void)
{
    while (com_in_u8 (LSR) & RxRDY)
	if (com_in_u8 (DATA) == 0x1b)
	    enter_kdebug ("breakin");
}
#endif

#else /* CONFIG_KDB_SKI */

static void init_ski_console (void) SECTION (".init");
static void putc_ski (char) SECTION (".kdebug");
static char getc_ski (bool) SECTION (".kdebug");

kdb_console_t kdb_consoles[] = {
    { "SKI", &init_ski_console, &putc_ski, &getc_ski },
    KDB_NULL_CONSOLE
};
    
word_t kdb_current_console = 0;

void init_ski_console (void)
{
//    ia64_ski_ssc (0, 0, 0, 0, IA64_SKI_SSC_CONSOLE_INIT);
}

char getc_ski (bool block)
{
    char c;
    do {
	c = (char)ia64_ski_ssc (0, 0, 0, 0, IA64_SKI_SSC_GETCHAR);
    } while (c == '\0');
    return c;
}

void putc_ski (char c)
{
    ia64_ski_ssc ((word_t)c, 0, 0, 0, IA64_SKI_SSC_PUTCHAR);
    if (c == '\n')
	putc_ski ('\r');
}

#if defined(CONFIG_KDB_BREAKIN)
void SECTION (".kdebug") kdebug_check_breakin (void)
{
    if ( ia64_ski_ssc (0, 0, 0, 0, IA64_SKI_SSC_GETCHAR) == 0x1b)
	enter_kdebug("breakin");
}
#endif

#endif
