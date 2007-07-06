/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     kdb/platform/malta/console.cc
 * Description:   Serial console for the Malta platform
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
 * $Id: console.cc,v 1.1 2006/02/23 21:07:44 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>

#include <kdb/console.h>

#include "uart16550.h"
#include "pio.h"


//static
void putc_serial (char);
static char getc_serial (bool);
static void init_serial (void);

kdb_console_t kdb_consoles[] = {
    { "serial", &init_serial, &putc_serial, &getc_serial },
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

// SEND

static int _serial_tx_ready( unsigned int port ) {
    return inb(port + USART_LSR) & LSR_THR_Empty;
}

static void _serial_tx_byte( unsigned int port, char c ) {
    outb(port + USART_THR, c);
}


void putc_serial(char c) { // XXX
    while( !_serial_tx_ready( USART_0_BASE ) );
    _serial_tx_byte( USART_0_BASE, c );
    if( c == '\n' ) {
        putc_serial('\r');
    }
	
}


// RECEIVE

static int _serial_rx_ready( unsigned int port ) {
    return inb(port + USART_LSR) & LSR_Data_Ready;
}

static char _serial_rx_byte( unsigned int port ) {
    return inb(port + USART_RBR);
}


static char getc_serial(bool block) { // XXX
    if( block ) {
        while( !_serial_rx_ready( USART_0_BASE ) );
    }
    else if( !_serial_rx_ready( USART_0_BASE ) ) {
        return 0; // XXX
    }
    return( _serial_rx_byte( USART_0_BASE ) );
}

// INIT

static void init_serial() {

#define COMPORT USART_0_BASE
#define out_u8 outb
#define in_u8 inb

#define RATE 115200

#define IER	(COMPORT+1)
#define EIR	(COMPORT+2)
#define LCR	(COMPORT+3)
#define MCR	(COMPORT+4)
#define LSR	(COMPORT+5)
#define MSR	(COMPORT+6)
#define DLLO	(COMPORT+0)
#define DLHI	(COMPORT+1)

    __asm__ __volatile__ (" .word 0x24000000 ");

    out_u8(LCR, 0x80);		/* select bank 1	*/
    for (volatile int i = 10000000; i--; );
    out_u8(DLLO, (((115200/RATE) >> 0) & 0x00FF));
    out_u8(DLHI, (((115200/RATE) >> 8) & 0x00FF));
    out_u8(LCR, 0x03);		/* set 8,N,1		*/
    out_u8(IER, 0x00);		/* disable interrupts	*/
    out_u8(EIR, 0x07);		/* enable FIFOs	*/
    out_u8(IER, 0x01);		/* enable RX interrupts	*/
    in_u8(IER);
    in_u8(EIR);
    in_u8(LCR);
    in_u8(MCR);
    in_u8(LSR);
    in_u8(MSR);
}
