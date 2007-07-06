/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,  University of New South Wales
 *                
 * File path:     kdb/platform/sb1/io.cc
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
 * $Id: io.cc,v 1.7 2004/03/17 22:36:04 cvansch Exp $
 *                
 ********************************************************************/

#include INC_PLAT(sb1250_uart.h)

#include <kdb/console.h>
#include <debug.h>

#define CKSEG1      0xffffffffa0000000
#define PHYS_TO_CKSEG1(n)     (CKSEG1 | (n))

#ifndef BAUD_RATE
#define	BAUD_RATE		115200
#endif

#define	CLK_DIVISOR		DUART_BAUD_RATE(BAUD_RATE)
#define	DATA_BITS		DUART_BITS_PER_CHAR_8
#define	PARITY			DUART_PARITY_MODE_NONE
#define	STOP_BITS		DUART_STOP_BIT_LEN_1

static void putc_serial (char);
static char getc_serial (bool);
static void init_serial (void);

kdb_console_t kdb_consoles[] = {
    { "serial", &init_serial, &putc_serial, &getc_serial },
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

static void 
duart_out(unsigned long reg, unsigned long val)
{
    *((volatile unsigned long *)PHYS_TO_CKSEG1(DUART_REG_PHYS(0,reg))) = val;
}

static unsigned long 
duart_in(unsigned long reg) 
{
    return *((volatile unsigned long *)PHYS_TO_CKSEG1(DUART_REG_PHYS(0,reg)));
}

static void init_serial (void)
{
    /* Initialise duart */
    /* FIXME -- check/test this code */

#if 1
    (*(volatile unsigned long *)PHYS_TO_CKSEG1(0x10020098)) = 0x0;
    __asm__("sync\n");
    (*(volatile unsigned long *)PHYS_TO_CKSEG1(0x10020098)) = 0x2;
    __asm__("sync\n");
    (*(volatile unsigned long *)PHYS_TO_CKSEG1(0x10020078)) = ~(0ULL);
    __asm__("sync\n");
    (*(volatile unsigned long *)PHYS_TO_CKSEG1(0x10020098)) = 0x3;
    __asm__("sync\n");
    
    duart_out(DUART_MODE_REG_1, DATA_BITS | PARITY);
    duart_out(DUART_MODE_REG_2, STOP_BITS);
    duart_out(DUART_CLK_SEL, CLK_DIVISOR);

    duart_out(DUART_CMD, DUART_RX_EN | DUART_TX_EN);
#endif    
}

static void putc_serial(char chr)
{
    while ((duart_in(DUART_STATUS) & DUART_TX_RDY) == 0);
    duart_out(DUART_TX_HOLD, chr);

    if (chr == '\n')
	    putc_serial('\r');

}

static char getc_serial(bool block)
{
    if ((duart_in(DUART_STATUS) & DUART_RX_RDY) == 0)
    {
	if (!block)
	    return -1;
	while ((duart_in(DUART_STATUS) & DUART_RX_RDY) == 0) ;
    }
    return duart_in(DUART_RX_HOLD);
}


#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)
{
    if ((duart_in(DUART_STATUS) & DUART_RX_RDY) != 0)
	if ((char)duart_in(DUART_RX_HOLD) == 27)
	    enter_kdebug("breakin");
}
#endif
