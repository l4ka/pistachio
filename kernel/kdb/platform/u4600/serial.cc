/*********************************************************************
 *                
 * Copyright (C) 1997, 1998, 2003-2004,  University of New South Wales
 *                
 * File path:     kdb/platform/u4600/serial.cc
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
 * $Id: serial.cc,v 1.9 2004/03/17 22:36:04 cvansch Exp $
 *                
 ********************************************************************/
#include "z85230.h"
#include INC_ARCH(addrspace.h)
#include INC_GLUE(schedule.h)

#include <kdb/console.h>
#include <debug.h>

extern int  pzscc(int,char*,int,int);
int devinit();

//static
void putc_serial (char);
static char getc_serial (bool);
static void init_serial (void);
# if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void);
#endif

kdb_console_t kdb_consoles[] = {
    { "serial", &init_serial, &putc_serial, &getc_serial },
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

#define CS1_BASE	0x1c800000
#define Z85230_BASE	(CS1_BASE | 0x30)
#define MPSC_BASE	Z85230_BASE

#define PHYS_TO_CKSEG1(n)    (CKSEG1 | (n))
#define PHYS_TO_K1(x) PHYS_TO_CKSEG1(x)

typedef struct ConfigEntry {
    addr_t devinfo;
    int chan;
    int (*handler)(int,char*,int,int);
    int rxqsize;
    int brate;
} ConfigEntry;

ConfigEntry     ConfigTable[] =
{
    /* p4000 has swapped mpsc ports */
    {(addr_t)PHYS_TO_K1(MPSC_BASE+2), 0, pzscc, 256, 17},
    {(addr_t)PHYS_TO_K1(MPSC_BASE+0), 1, pzscc, 256, 17},
    {0}
};

//static
void putc_serial(char c)
{
    ConfigEntry *q = &ConfigTable[0];

    /* wait to transmit */
    while(!(*q->handler) (OP_TXRDY, (char*)q->devinfo, 0, 0));
	pzscc( OP_TX, (char*)q->devinfo, 0, c);
    if (c == '\n')
	putc_serial('\r');
}

static char getc_serial(bool block)
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

static void init_serial ()
{
/*
    int             i, brate;
    ConfigEntry    *q;

    asm("break;\n\t");

    for (i = 0; ConfigTable[i].devinfo; i++) {
	q = &ConfigTable[i];
	if (q->chan == 0)
	{
	    (*q->handler) (OP_INIT, (char*)q->devinfo, 0, 0);
	}
	brate = q->brate;
	(*q->handler) (OP_BAUD, (char*)q->devinfo, q->chan, brate);
    }
    return (0);*/
}

#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)

{
    ConfigEntry    *q;
    q = &ConfigTable[0];

    if ((*q->handler) (OP_RXRDY, (char*)q->devinfo, 0, 0))
	if (pzscc( OP_RX, (char*)q->devinfo, 0,0) == 27)
	    enter_kdebug("breakin");
}
#endif
