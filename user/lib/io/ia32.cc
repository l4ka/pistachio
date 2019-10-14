/*********************************************************************
 *                
 * Copyright (C) 2001-2006, 2010,  Karlsruhe University
 *                
 * File path:     ia32.cc
 * Description:   putc() for x86-based PCs, serial and screen
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
 * $Id: ia32-putc.cc,v 1.13 2006/10/07 16:30:25 ud3 Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4/types.h>
#include "ia32.h"
//#include "lib.h" //HAX


extern "C" void putc (int c) __attribute__ ((weak, alias ("__l4_putc")));
extern "C" int __l4_getc (void);
extern "C" int getc (void) __attribute__ ((weak, alias ("__l4_getc")));
extern "C" void __l4_putc (int c);

#if defined(CONFIG_COMPORT)

//RS-232/Serial port interfaces
//http://wiki.osdev.org/Serial_Ports
#if CONFIG_COMPORT == 0
# define COMPORT 0x3f8
#elif CONFIG_COMPORT == 1
# define COMPORT 0x2f8
#elif CONFIG_COMPORT == 2
# define COMPORT 0x3e8
#elif CONFIG_COMPORT == 3
# define COMPORT 0x2e8
#else
#define COMPORT CONFIG_COMPORT
#endif

static void io_init( void )
{
    static bool io_initialized = false;

    if (io_initialized)
        return;

    io_initialized = true;

#define IER	(COMPORT+1)
#define EIR	(COMPORT+2)
#define LCR	(COMPORT+3)
#define MCR	(COMPORT+4)
#define LSR	(COMPORT+5)
#define MSR	(COMPORT+6)
#define DLLO	(COMPORT+0)
#define DLHI	(COMPORT+1)

        outb(LCR, 0x80);		/* select bank 1	*/
        for (volatile int i = 10000000; i--; );
        outb(DLLO, (((115200/CONFIG_COMSPEED) >> 0) & 0x00FF));
        outb(DLHI, (((115200/CONFIG_COMSPEED) >> 8) & 0x00FF));
        outb(LCR, 0x03);		/* set 8,N,1		*/
        outb(IER, 0x00);		/* disable interrupts	*/
        outb(EIR, 0x07);		/* enable FIFOs	*/
        outb(MCR, 0x0b);                /* force data terminal ready */
        outb(IER, 0x01);		/* enable RX interrupts	*/
        inb(IER);
        inb(EIR);
        inb(LCR);
        inb(MCR);
        inb(LSR);
        inb(MSR);
        

}



void __l4_putc(int c)
{
    io_init();
    
    while (!(inb(COMPORT+5) & 0x20));
    outb(COMPORT,c);
    while (!(inb(COMPORT+5) & 0x40));
    if (c == '\n')
	__l4_putc('\r');
}

int __l4_getc (void)
{
    io_init();

    while ((inb(COMPORT+5) & 0x01) == 0);
    return inb(COMPORT);
}

#else /* ! CONFIG_COMPORT */

#define DISPLAY ((char*)0xb8000)
#define COLOR 7
#define NUM_LINES 25

//Correspond with PS/2 keyboard interface
//http://wiki.osdev.org/%228042%22_PS/2_Controller
#define KBD_STATUS_REG		0x64	
#define KBD_CNTL_REG		0x64	
#define KBD_DATA_REG		0x60	

#define KBD_STAT_OBF 		0x01	/* Keyboard output buffer full */

#define kbd_read_input() inb(KBD_DATA_REG)
#define kbd_read_status() inb(KBD_STATUS_REG)

static unsigned char keyb_layout[128] =
	"\000\0331234567890-+\177\t"			/* 0x00 - 0x0f */
	"qwertyuiop[]\r\000as"				/* 0x10 - 0x1f */
	"dfghjkl;'`\000\\zxcv"				/* 0x20 - 0x2f */
	"bnm,./\000*\000 \000\201\202\203\204\205"	/* 0x30 - 0x3f */
	"\206\207\210\211\212\000\000789-456+1"		/* 0x40 - 0x4f */
	"230\177\000\000\213\214\000\000\000\000\000\000\000\000\000\000" /* 0x50 - 0x5f */
	"\r\000/";					/* 0x60 - 0x6f */


void __l4_putc(int c)
{
    unsigned int i;

    // Shared cursor pointer
    static unsigned __l4_putc_cursor = 160 * (NUM_LINES - 1);

    // Create thread-local copy. Using proper locking would be better.
    unsigned __cursor = __l4_putc_cursor;

    switch(c) {
    case '\r':
        break;
    case '\n':
        do
	{
	    DISPLAY[__cursor++] = ' ';
	    DISPLAY[__cursor++] = COLOR;
	}
        while (__cursor % 160 != 0);
        break;
    case '\t':
        do
	  {
	    DISPLAY[__cursor++] = ' ';
	    DISPLAY[__cursor++] = COLOR;
	}
        while (__cursor % 16 != 0);
        break;
    default:
        DISPLAY[__cursor++] = c;
        DISPLAY[__cursor++] = COLOR;
    }
    if (__cursor == (160 * NUM_LINES)) {
	for (i = (160 / sizeof (L4_Word_t));
	     i < (160 / sizeof (L4_Word_t)) * NUM_LINES;
	     i++)
	    ((L4_Word_t *) DISPLAY)[i - 160 / sizeof (L4_Word_t)]
	      = ((L4_Word_t *) DISPLAY)[i];
	for (i = 0; i < 160 / sizeof (L4_Word_t); i++)
	    ((L4_Word_t *) DISPLAY)[160 / sizeof (L4_Word_t)
				  * (NUM_LINES-1) + i] = 0;
	__cursor -= 160;
    }

    // Write back thread-local cursor value
    __l4_putc_cursor = __cursor;
}



/* No SHIFT key support!!! */

int __l4_getc()
{
    static unsigned char last_key = 0;
    char c;
    while(1) {
	unsigned char status = kbd_read_status();
	while (status & KBD_STAT_OBF) {
	    unsigned char scancode;
	    scancode = kbd_read_input();

	//if (scancode = 0x1d || scancode = \03/* Ctrl + C, maybe? */) printf("[kbd] : Ctrl+C pressed");

	    if (scancode & 0x80)
		last_key = 0;
	    else if (last_key != scancode)
	    {
//Why doesn't this printf() work, if the serial port's disabled?
#ifdef 	CONFIG_COMPORT
	printf("kbd: %d, %d, %c\n", scancode, last_key, keyb_layout[scancode]);//was disabled
#endif
		last_key = scancode;
		c = keyb_layout[scancode];
		if (c > 0) return c;
	    }
	}
    }
}

#endif
