/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     screen.cc
 * Description:   Console Output / Input
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
 * $Id: screen.cc,v 1.1 2004/03/01 19:04:32 stoess Exp $
 *                
 ********************************************************************/

#include "globals.h"
/*
**
** Console I/O functions.
**
*/

# define DISPLAY (( char *) (0xb8000))
# define NUM_LINES 24

static inline L4_Word8_t inb(L4_Word_t port)
{
    L4_Word8_t tmp;

    if (port < 0x100) /* GCC can optimize this if constant */
	__asm__ __volatile__ ("inb %w1, %0" :"=al"(tmp) :"dN"(port));
    else
	__asm__ __volatile__ ("inb %%dx, %0" :"=al"(tmp) :"d"(port));

    return tmp;
}

static inline void outb(L4_Word_t port, L4_Word8_t val)
{
    if (port < 0x100) /* GCC can optimize this if constant */
	__asm__ __volatile__ ("outb %1, %w0" : :"dN"(port), "al"(val));
    else
	__asm__ __volatile__ ("outb %1, %%dx" : :"d"(port), "al"(val));
}



void putc(int c)
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
		    ((L4_Word16_t *) DISPLAY)[i] = (color << 8) + ' ';
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
        cursor -= (cursor % 160);
	break;
    case '\n':
	cursor += (160 - (cursor % 160));
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

    if ((cursor / 160) == NUM_LINES)
    {
	for (int i = 40; i < 40*NUM_LINES; i++)
	    ((L4_Word_t *) DISPLAY)[i - 40] = ((L4_Word_t *) DISPLAY)[i];
	for (int i = 0; i < 40; i++)
	    ((L4_Word_t * )DISPLAY)[40 * (NUM_LINES-1) + i] = 0;
	cursor -= 160;
    }
}

#define KBD_STATUS_REG		0x64	
#define KBD_CNTL_REG		0x64	
#define KBD_DATA_REG		0x60	

#define KBD_STAT_OBF 		0x01	/* Keyboard output buffer full */

#define kbd_read_input() inb(KBD_DATA_REG)
#define kbd_read_status() inb(KBD_STATUS_REG)

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

char getc (void)
{
    static L4_Word8_t last_key = 0;
    static L4_Word8_t shift = 0;
    char c;
    while(1) {
	unsigned char status = kbd_read_status();
	while (status & KBD_STAT_OBF) {
	    L4_Word8_t scancode;
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
