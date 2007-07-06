/*********************************************************************
 *                
 * Copyright (C) 1997, 1998, 2003, 2004,  University of New South Wales
 *                
 * File path:     kdb/platform/ofsparc64/z8530.cc
 * Description:   
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
 * $Id: z8530.cc,v 1.1 2004/06/04 05:21:22 philipd Exp $
 *                
 ********************************************************************/
#include "z8530.h"


#define ZSCCCLK	14745600	 /* clock rate 14.7456 MHz */

static const int zsccbrtc[] = {
    -1,
    BRTC (ZSCCCLK, 50),
    BRTC (ZSCCCLK, 75),
    BRTC (ZSCCCLK, 110),
    BRTC (ZSCCCLK, 134),
    BRTC (ZSCCCLK, 150),
    BRTC (ZSCCCLK, 200),
    BRTC (ZSCCCLK, 300),
    BRTC (ZSCCCLK, 600),
    BRTC (ZSCCCLK, 1200),
    BRTC (ZSCCCLK, 1800),
    BRTC (ZSCCCLK, 2400),
    BRTC (ZSCCCLK, 4800),
    BRTC (ZSCCCLK, 9600),
    BRTC (ZSCCCLK, 19200),
    BRTC (ZSCCCLK, 38400)};


static void
zsccputreg (addr_t base, unsigned char reg, unsigned char val)
{
    if (reg != zWR0) {
	__asm__ __volatile__(
			     "stba\t%0, [ %1 ] %2\t\n"
			     :: "r" (zWR0_REG | reg),
			        "r" ((char*)base + CMD),
			        "i" (ASI_PHYS_BYPASS_EC_E_L));
    }
    __asm__ __volatile__(
			 "stba\t%0, [ %1 ] %2\t\n"
			 :: "r" (val),
			    "r" ((char*)base + CMD),
			    "i" (ASI_PHYS_BYPASS_EC_E_L));
}

static unsigned char
zsccgetreg (addr_t base, unsigned char reg)
{
    unsigned char result;
    if (reg != zRR0) {
	__asm__ __volatile__(
			     "stba\t%0, [ %1 ] %2\t\n"
			     :: "r" (zWR0_REG | reg),
			        "r" ((char*)base + CMD),
			        "i" (ASI_PHYS_BYPASS_EC_E_L));
    }
    __asm__ __volatile__(
			 "lduba\t[ %1 ] %2, %0\t\n"
			 : "=r" (result)
			 : "r" ((char*)base + CMD),
			   "i" (ASI_PHYS_BYPASS_EC_E_L));

    return result;
}

static int
zsccinit (addr_t base)
{
    // Disable interrupts (everything else should be configured already)
    zsccputreg(base, zWR9, 0);

    return 0;
}


static void
zsccflush (addr_t base)
{
    /* wait for Tx fifo to drain */
    int timeout = 10;
    while (!(zsccgetreg (base, zRR0) & zRR0_TXEMPTY))
	if (--timeout == 0)
	    break;
}


static int
zsccprogram (addr_t base, int baudrate)
{
    zsccflush (base);

    baudrate &= 17;
    if (baudrate == 0)
	return 1;

    /*
     * See the zscc manual for details.
     */

    zsccputreg(base, zWR4, zWR4_1STOPBIT | zWR4_CLK);
    zsccputreg(base, zWR10, zWR10_NRZ);

    zsccputreg(base, zWR14, zWR14_NOP); /* stop BRG */
    zsccputreg(base, zWR11, zWR11_TRXCOUT| zWR11_TRXCBRG| zWR11_TCLKBRG | zWR11_RCLKBRG);
    zsccputreg(base, zWR12,  zsccbrtc [baudrate] & 0xff);
    zsccputreg(base, zWR13,  (zsccbrtc [baudrate] >> 8) & 0xff);

    zsccputreg(base, zWR14, zWR14_BRGENABLE | zWR14_BRGSRC );

    zsccputreg(base, zWR3, zWR3_RXENABLE | zWR3_RX8BITCHAR);
    zsccputreg(base, zWR5, zWR5_TXENABLE| zWR5_RTS | zWR5_TX8BITCHAR | zWR5_DTR);
    return 0;
}


int pzscc (int op, addr_t dat, int chan, int data)
{
    addr_t base = dat;

    switch (op) {
    case OP_INIT:
	return zsccinit (base);
    case OP_BAUD:
	return zsccprogram (base, data);
    case OP_TXRDY:
	return (zsccgetreg(base, zRR0) & zRR0_TXEMPTY);
    case OP_TX:
	__asm__ __volatile__(
			     "stba\t%0, [ %1 ] %2\t\n"
			     :: "r" (data),
				"r" ((char*)base + DATA),
				"i" (ASI_PHYS_BYPASS_EC_E_L));
	break;
    case OP_RXRDY:
	return (zsccgetreg(base, zRR0) & zRR0_RXAVAIL);
    case OP_RX:
	__asm__ __volatile__(
			     "lduba\t[ %1 ] %2, %0\t\n"
			     : "=r" (data)
			     : "r" ((char*)base + DATA),
			       "i" (ASI_PHYS_BYPASS_EC_E_L));
	return data;
    case OP_FLUSH:
	zsccflush (base);
	break;
    case OP_RXSTOP:
	/* rx flow control */
	zsccputreg (base, zWR5, zWR5_TXENABLE| zWR5_TX8BITCHAR | zWR5_DTR | 
		    (data ? 0 : zWR5_RTS));
	break;
    }
    return 0;
}

#warning philipd (04/06/04): base should be obtained from open firmware
#define FHC_BASE	0x1FFF8000000
#define Z8530_BASE	(FHC_BASE | 0x902000)

typedef struct ConfigEntry {
    addr_t devinfo;
    int chan;
    int (*handler)(int,addr_t,int,int);
    int rxqsize;
    int brate;
} ConfigEntry;

ConfigEntry     ConfigTable[] =
{
    {(addr_t)(Z8530_BASE+4), 0, pzscc, 256, 17},
    {(addr_t)(Z8530_BASE+0), 1, pzscc, 256, 17},
    {0}
};

void
z8530_putc(char c) {
    ConfigEntry *q = &ConfigTable[0];

    /* wait to transmit */
    while(!(*q->handler) (OP_TXRDY, (char*)q->devinfo, 0, 0));
	pzscc( OP_TX, (char*)q->devinfo, 0, c);
    if (c == '\n')
	z8530_putc('\r');
}

char
z8530_getc(bool block)
{
    ConfigEntry *q = &ConfigTable[0];

    /* Data ready? */
    if (!(*q->handler) (OP_RXRDY, (char*)q->devinfo, 0, 0))
    {
	if (!block)
	    return -1;
	/* wait to receive */
	while(!(*q->handler) (OP_RXRDY, (char*)q->devinfo, 0, 0));
    }

    return pzscc( OP_RX, (char*)q->devinfo, 0,0);
}

void
z8530_init ()
{
    int             i, brate;
    ConfigEntry    *q;

    for (i = 0; ConfigTable[i].devinfo; i++) {
	q = &ConfigTable[i];
	if (q->chan == 0)
	{
	    (*q->handler) (OP_INIT, (char*)q->devinfo, 0, 0);
	}
	//brate = q->brate;
	//(*q->handler) (OP_BAUD, (char*)q->devinfo, q->chan, brate);
    }
}

