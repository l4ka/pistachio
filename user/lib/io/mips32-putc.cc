/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     mips32-putc.cc
 * Description:   Serial output for MIPS32/Malta
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
 * $Id: mips32-putc.cc,v 1.1 2006/02/23 21:07:58 ud3 Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <l4/mips32/kdebug.h>
#include "mips32-uart16550.h"
#include "mips32-pio.h"

extern "C" void __l4_putc (int c);
extern "C" void putc (int c) __attribute__ ((weak, alias ("__l4_putc")));

static int _serial_tx_ready( unsigned int port ) {
  return inb(port + USART_LSR) & LSR_THR_Empty;
}

static void _serial_tx_byte( unsigned int port, char c ) {
  outb(port + USART_THR, c);
}


extern "C" void __l4_putc( int c ) {
	while( !_serial_tx_ready( USART_0_BASE ) );
	_serial_tx_byte( USART_0_BASE, c );
	if( c == '\n' )
		__l4_putc('\r');
}

