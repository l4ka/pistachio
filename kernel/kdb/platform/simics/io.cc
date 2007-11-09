/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/platform/simics/io.cc
 * Description:   Simics specific I/O functions
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
 * $Id: io.cc,v 1.4 2006/10/19 22:57:38 ud3 Exp $
 *                
 ********************************************************************/
#include INC_ARCH(ioport.h)

#include <kdb/kdb.h>
#include <kdb/init.h>
#include <kdb/cmd.h>
#include <kdb/console.h>
#include <init.h>
#include <debug.h>

#if !defined(CONFIG_KDB_COMPORT)
#define CONFIG_KDB_COMPORT 0x3f8
#endif

#if !defined(CONFIG_KDB_COMSPEED)
#define CONFIG_KDB_COMSPEED 115200
#endif

#define KERNEL_VIDEO		(KERNEL_OFFSET + 0xb8000)

#define SEC_SIMICS_IO		".kdebug"


/* Section assignements */
static void putc_screen (char) SECTION (SEC_SIMICS_IO);
static char getc_screen (bool) SECTION (SEC_SIMICS_IO);
static void init_screen (void) SECTION (SEC_SIMICS_IO);

static void putc_serial (char) SECTION (SEC_SIMICS_IO);
static char getc_serial (bool) SECTION (SEC_SIMICS_IO);
static void init_serial (void) SECTION (SEC_SIMICS_IO);

void init_console (void) SECTION (SEC_INIT);
# if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void) SECTION (SEC_SIMICS_IO);
#endif

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
** Screen I/O functions.
**
*/

# define DISPLAY ((char *) KERNEL_VIDEO)
# define NUM_LINES (24)
# define NUM_COLS  (80)

static void init_screen()
{
    /* Clear Screen */
    
    putc_screen('\e');
    putc_screen('[');
    putc_screen('2');
    putc_screen('J');
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
		for (int i = 0; i < 80*NUM_LINES; i++)
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
        cursor -= (cursor % (NUM_COLS * 2));
	break;
    case '\n':
	cursor += ((NUM_COLS * 2) - (cursor % (NUM_COLS * 2)));
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

    if ((cursor / (NUM_COLS * 2)) == NUM_LINES)
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

#define kbd_read_input() in_u8(KBD_DATA_REG)
#define kbd_read_status() in_u8(KBD_STATUS_REG)

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
    while(1) {
	unsigned char status = kbd_read_status();
	while (status & KBD_STAT_OBF) {
	    u8_t scancode;
	    scancode = kbd_read_input();
	    /* check for SHIFT-keys */
	    if (((scancode & 0x7F) == 42) || ((scancode & 0x7F) == 54))
	    {
		shift = !(scancode & 0x80);
		continue;
	    }
	    /* ignore all other RELEASED-codes */
	    if (scancode & 0x80)
		last_key = 0;
	    else if (last_key != scancode)
	    {
		last_key = scancode;
		c = keyb_layout[shift][scancode];
		if (c > 0) return c;
	    }
	}
    }
}


#define COMPORT         CONFIG_KDB_COMPORT
#define RATE            CONFIG_KDB_COMSPEED

static void init_serial (void)
{
#define IER     (COMPORT+1)
#define EIR     (COMPORT+2)
#define LCR     (COMPORT+3)
#define MCR     (COMPORT+4)
#define LSR     (COMPORT+5)
#define MSR     (COMPORT+6)
#define DLL     (COMPORT+0)

#if 1
    /* Adopt to SIMICS' serial port NS16650 */
    out_u8(LCR, 0x80);          /* select bank 1        */
    for (volatile int i = 100000; i--; );
    out_u16(DLL, (u16_t) (115200 / RATE));
    out_u8(LCR, 0x03);          /* set 8,N,1            */
    out_u8(IER, 0x00);          /* disable interrupts   */
    out_u8(EIR, 0x07);          /* enable FIFOs */
    in_u8(IER);
    in_u8(EIR);
    in_u8(LCR);
    in_u8(MCR);
    in_u8(LSR);
    in_u8(MSR);
#else
    out_u8(LCR, 0x80);          /* select bank 1        */
    for (volatile int i = 10000000; i--; );
    out_u16(DLL, (u16_t) (115200 / RATE));
    out_u8(LCR, 0x03);          /* set 8,N,1            */
    out_u8(IER, 0x00);          /* disable interrupts   */
    out_u8(EIR, 0x07);          /* enable FIFOs */
    out_u8(IER, 0x01);          /* enable RX interrupts */
    in_u8(IER);
    in_u8(EIR);
    in_u8(LCR);
    in_u8(MCR);
    in_u8(LSR);
    in_u8(MSR);
#endif    
}


static void putc_serial (const char c)
{
    while ((in_u8(LSR) & (1 << 5)) == 0);
    out_u8(COMPORT,c);
    if (c == '\n')
        putc_serial('\r');

}

static char getc_serial (bool block)
{
    while ((in_u8(COMPORT+5) & 0x01) == 0);
    return in_u8(COMPORT);
}


#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)
{
    if ((in_u8(COMPORT+5) & 0x01))
        if (in_u8(COMPORT) == 0x1b)
            enter_kdebug("breakin");
}
#endif

