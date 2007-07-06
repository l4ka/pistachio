/*********************************************************************
 *                
 * Copyright (C) 2001-2003,  Karlsruhe University
 *                
 * File path:     amd64-putc.cc
 * Description:   putc() for amd64-based PCs, serial and screen
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
 * $Id: amd64-putc.cc,v 1.4 2006/10/19 22:57:40 ud3 Exp $ 
 *                
 ********************************************************************/
#include <config.h>
#include <l4/types.h>

#include "amd64-port.h"

extern "C" void __l4_putc (int c);
extern "C" void putc (int c) __attribute__ ((weak, alias ("__l4_putc")));

#if defined(CONFIG_COMPORT)
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

void __l4_putc(int c)
{
    while (!(inb(COMPORT+5) & 0x60));
    outb(COMPORT,c);
    if (c == '\n')
	putc('\r');
}
#else /* ! CONFIG_COMPORT */

#define DISPLAY ((char*)0xb8000)
#define COLOR 7
#define NUM_LINES 25
unsigned __cursor;

void __l4_putc(int c)
{
    unsigned int i;
    static int initialized = 0;

    if (! initialized)
      {
	__cursor = NUM_LINES * 160 - 2;
	while (__cursor >= 0)
	  if (DISPLAY[__cursor] != ' ' && DISPLAY[__cursor] != 0)
	    break;
	  else
	    __cursor -= 2;

	__cursor += (160 - (__cursor % 160));
	initialized = 1;
      }

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
}

#endif
