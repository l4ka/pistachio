/*********************************************************************
 *                
 * Copyright (C) 2003-2005,  Karlsruhe University
 *                
 * File path:     ia32.cc
 * Description:   IA-32 specific implementation fragments of kickstart
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
 * $Id: ia32.cc,v 1.20 2005/03/10 15:52:09 ud3 Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>

#include "kickstart.h"
#include "mbi.h"                // MultiBoot Info structure
#include "kipmgr.h"             // KIP management
#include "lib.h"


/*
 * Loader formats supported for IA32.
 */
loader_format_t loader_formats[] = {
    { "multiboot compliant loader", mbi_probe, mbi_init },
    NULL_LOADER
};


void fail(int ec)
{
    printf("PANIC: FAIL in line %d\n", ec);
    while(1);
}


void flush_cache()
{
    __asm__ __volatile__ ("wbinvd");
}



/*
 * Start kernel at its entry point. No preconditions
 */
void launch_kernel(L4_Word_t entry)
{
    __asm__ __volatile__ ("jmp *%0" : : "r"(entry));
}


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

extern inline L4_Word8_t inb(const L4_Word16_t port)
{
    L4_Word8_t val;

    __asm__ __volatile__ ("inb  %w1, %0" : "=a"(val) : "dN"(port));

    return val;
}

extern inline void outb(const L4_Word16_t port, const L4_Word8_t val)
{
    __asm__ __volatile__ ("outb %0, %w1" : : "a"(val), "dN"(port));
}

static void init_serial(void)
{
#define IER     (COMPORT+1)
#define EIR     (COMPORT+2)
#define LCR     (COMPORT+3)
#define MCR     (COMPORT+4)
#define LSR     (COMPORT+5)
#define MSR     (COMPORT+6)
#define DLLO    (COMPORT+0)
#define DLHI    (COMPORT+1)

    outb(LCR, 0x80);          /* select bank 1        */
    for (volatile int i = 10000000; i--; );
    outb(DLLO, (((115200/CONFIG_COMSPEED) >> 0) & 0x00FF));
    outb(DLHI, (((115200/CONFIG_COMSPEED) >> 8) & 0x00FF));
    outb(LCR, 0x03);          /* set 8,N,1            */
    outb(IER, 0x00);          /* disable interrupts   */
    outb(EIR, 0x07);          /* enable FIFOs */
    inb(IER);
    inb(EIR);
    inb(LCR);
    inb(MCR);
    inb(LSR);
    inb(MSR);
}

#define DISPLAY   ((char*)0xb8000)
#define COLOR     15
#define NUM_LINES 25

void __vga_putc(int c)
{
    unsigned int i;
    static unsigned __cursor = 160 * (NUM_LINES - 1);

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
	
extern "C" void putc(int c)
{
    static bool do_init = true;

    if( do_init )
    {
	do_init = false;
	init_serial();
    }

    extern void __l4_putc(int c);
    __l4_putc(c);
    __vga_putc(c);
}

#endif	/* CONFIG_COMPORT */

