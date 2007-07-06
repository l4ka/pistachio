/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/src/platform/vr41xx/main.cc
 * Description:   Main file for elf loader 
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
 * $Id: main.cc,v 1.1 2003/11/17 06:09:13 cvansch Exp $
 *                
 ********************************************************************/

#include "elf-loader.h"

#define PHYS_OFFSET 0xffffffff80000000

extern L4_KernelConfigurationPage_t *kip;

#define CKSEG1          0xffffffffa0000000

#define SIU_BASE	0x0c000010      /* serial i/o unit */

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

static volatile struct serial_ns16550 *serial_regs = (struct serial_ns16550 *)(CKSEG1 | SIU_BASE);

void init_serial_console()
{
//    serial_regs = (struct serial_ns16550 *)(CKSEG1 | SIU_BASE);

#if 0
    serial_regs->lcr = 0x00;
    serial_regs->ier = 0xFF;
    serial_regs->ier = 0x00;
    serial_regs->lcr = 0x80;	/* Access baud rate */
    serial_regs->dll = 12;  	/* 1 = 115200,  2 = 57600, 3 = 38400, 12 = 9600 baud */
    serial_regs->dlm = 0;   	/* dll >> 8 which should be zero for fast rates; */
    serial_regs->lcr = 0x03;	/* 8 data, 1 stop, no parity */
    serial_regs->mcr = 0x03;	/* RTS/DTR */
    serial_regs->fcr = 0x07;	/* Clear & enable FIFOs */
#endif
}

extern "C" void putc(char c)
{
    if ( serial_regs )
    {
	while (( serial_regs->lsr & LSR_THRE ) == 0 );

	serial_regs->thr = c;
	if ( c == '\n' )
	    putc( '\r' );
    }
}

extern "C" int printf(const char * format, ...);
 
extern "C" void memset (char * p, char c, int size)
{
    for (;size--;)
	*(p++)=c;
}

extern "C" __attribute__ ((weak)) void *
memcpy (void * dst, const void * src, unsigned int len)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned char *s = (unsigned char *) src;

    while (len-- > 0)
	*d++ = *s++;

    return dst;
}


void start_kernel(L4_Word_t bootaddr)
{
    printf("Starting kernel...\n\n");
    void (*func)(unsigned long) = (void (*)(unsigned long)) (bootaddr | PHYS_OFFSET);

    /* XXX - Get this from boot loader */
    kip->MainMem.high = 8UL * 1024 * 1024;
    
    func(0);
}

int main(void)
{
    init_serial_console();

    unsigned int temp;
    /* Disable caches */
    __asm__ __volatile__ (
	"mfc0    %0,$16;    \n\t"
	"li      $31,-8;    \n\t"
	"and     %0,%0,$31;  \n\t"
	"ori     %0,%0,0x2; \n\t"
	"mtc0    %0,$16;    \n\t"
	: "=r" (temp) : : "$31"
    );

    L4_Word_t entry;
    
    if(load_modules(&entry, PHYS_OFFSET)) {
	asm ("break\n\t");
    }
    
    start_kernel(entry);
    
    asm ("break\n\t");
}
