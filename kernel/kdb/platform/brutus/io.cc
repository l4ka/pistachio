/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,  Karlsruhe University
 *                
 * File path:     kdb/platform/brutus/io.cc
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
 * $Id: io.cc,v 1.9 2004/03/17 19:13:24 skoglund Exp $
 *                
 ********************************************************************/

#include "../sa1100/uart.h"

#define BRD CONFIG_KDB_COMSPEED
#define UART_VBASE 0xFF

void init_console (void)
{
    u32_t enbl = 0;

    enbl |= L4_UART_RXE;
    enbl |= L4_UART_TXE;

    L4_UART_UTCR3 = 0;			/* Diable UART */
    L4_UART_UTSR0 = 0xf;		/* Clear pending interrupts */

    L4_UART_UTCR0 = L4_UART_DSS;	/* No parity, 1 stop bit, 8 bit */
    L4_UART_UTCR1 = BRD >> 8;		/* Set baudrate */
    L4_UART_UTCR2 = BRD & 0xff;
    L4_UART_UTCR3 = enbl;		/* Enable UART */

}


void putc(char chr)
{
    volatile u32_t tmp;

    /*
     * Wait till the transmit FIFO has a free slot.
     */
    do {
	tmp = L4_UART_UTSR1;
    } while ( !(tmp & L4_UART_TNF) );
    
    /*
     * Add the character to the transmit FIFO.
     */
    L4_UART_UTDR = (u32_t) chr;
}

char getc(bool block)
{
    volatile u32_t tmp;

    /*
     * Wait till the receive FIFO has something in it.
     */
    do {
	tmp = L4_UART_UTSR1;
    } while ( !(tmp & L4_UART_RNE) );

    /*
     * Read a character from the receive FIFO.
     */
    return (char) L4_UART_UTDR;
}
