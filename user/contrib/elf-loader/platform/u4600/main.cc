/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     elf-loader/src/platform/u4600/main.cc
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
 * $Id: main.cc,v 1.3 2004/05/14 05:15:32 cvansch Exp $
 *                
 ********************************************************************/

#include "elf-loader.h"
#include "z85230.h"

#define PHYS_OFFSET 0xffffffff80000000

extern L4_KernelConfigurationPage_t *kip;


#define CKSEG1          0xffffffffa0000000
#define CS1_BASE	0x1c800000
#define Z85230_BASE	(CS1_BASE | 0x30)
#define MPSC_BASE	Z85230_BASE

#define PHYS_TO_CKSEG1(n)    (CKSEG1 | (n))
#define PHYS_TO_K1(x) PHYS_TO_CKSEG1(x)

typedef struct zsccdev *zsccdp;

typedef struct ConfigEntry {
    void *devinfo;
    int chan;
    int (*handler)(int,char*,int,int);
    int rxqsize;
    int brate;
} ConfigEntry;

int pzscc (int op, char *dat, int chan, int data)
{
    volatile zsccdp dp = (zsccdp) dat;

    switch (op) {
//    case OP_INIT:
//	return zsccinit (dp);
//    case OP_BAUD:
//	return zsccprogram (dp, data);
    case OP_TXRDY:
	return (dp->ucmd & zRR0_TXEMPTY);
    case OP_TX:
	dp->udata = data; //wbflush ();
	break;
//    case OP_RXRDY:
//	return (dp->ucmd & zRR0_RXAVAIL);
//    case OP_RX:
//	return (dp->udata ); 
//    case OP_FLUSH:
//	zsccflush (dp);
//	break;
//    case OP_RXSTOP:
	/* rx flow control */
//	zsccputreg (dp, zWR5, zWR5_TXENABLE| zWR5_TX8BITCHAR | zWR5_DTR | 
//		    (data ? 0 : zWR5_RTS));
//	break;
    }
    return 0;
}

ConfigEntry     ConfigTable[] =
{
    /* p4000 has swapped mpsc ports */
    {(void *)PHYS_TO_K1(MPSC_BASE+2), 0, pzscc, 256, 17},
    {(void *)PHYS_TO_K1(MPSC_BASE+0), 1, pzscc, 256, 17},
    {0}
};


extern "C" void putc(char c)
{
    ConfigEntry    *q;
  
    /*  TRACE(T_PROCS);*/
    q = &ConfigTable[0];

    /* wait to transmit */
    while(!(*q->handler) (OP_TXRDY, (char*)q->devinfo, 0, 0));
	pzscc( OP_TX, (char*)q->devinfo, 0, c);
    if (c == '\n')
	putc('\r');
}

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
    void (*func)(unsigned long) = (void (*)(unsigned long)) (bootaddr | PHYS_OFFSET);

    /* XXX - Get this from boot loader */
    kip->MainMem.high = 64UL * 1024 * 1024;

    kip->MemoryInfo.n = 0;
    
    func(0);
}

int main(void)
{
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
