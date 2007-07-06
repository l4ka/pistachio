/****************************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:	platform/csb337/console.cc
 * Description:	Cogent CSB337 Console
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
 * $Id: console.cc,v 1.1 2004/08/12 12:00:34 cvansch Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include INC_GLUE(config.h)
#include INC_PLAT(console.h)
#include <kdb/console.h>

#define SERIAL_NAME	"serial"

/****************************************************************************
 *
 *    Serial console support
 *    Atmel AT91RM9200
 *
 ****************************************************************************/

struct serial_at91rm9200 {
    word_t us_cr;	/* 0x00 Control Register			*/
    word_t us_mr;	/* 0x04 Mode Register				*/
    word_t us_ier;	/* 0x08 Interrupt Enable Register		*/
    word_t us_idr;	/* 0x0c Interrupt Disable Register		*/
    word_t us_imr;	/* 0x10 Interrupt Mask Register			*/
    word_t us_csr;	/* 0x14 Channel Status Register			*/
    word_t us_rhr;	/* 0x18 Receiver Holding Register		*/
    word_t us_thr;	/* 0x1c Transmitter Holding Register		*/
    word_t us_brgr;	/* 0x20 Baud Rate Generator Register		*/
    word_t us_rtor;	/* 0x24 Receiver Time-out Register		*/
    word_t us_rrgr;	/* 0x28 Transmitter Timeguard Register		*/
    word_t res1[5];
    word_t us_fidi;	/* 0x40 FI DI Ratio Register			*/
    word_t us_ner;	/* 0x44 Number of Errors Register		*/
    word_t res2;
    word_t us_if;	/* 0x4c IrDA Filter Register			*/
};

#define	CSR_TXRDY	(1 << 1)
#define CSR_RXRDY	(1 << 0)

static volatile struct serial_at91rm9200 *serial_regs = 0;

static void putc_serial( char c )
{
    if ( serial_regs )
    {
	while ((serial_regs->us_csr & CSR_TXRDY) == 0 );

	serial_regs->us_thr = c;
	if ( c == '\n' )
	    putc_serial( '\r' );
    }
}

static char getc_serial( bool block )
{
    if ( serial_regs )
    {
	if ((serial_regs->us_csr & CSR_RXRDY) == 0 )
	{
	    if (!block)
		return (signed char)-1;

	    while ((serial_regs->us_csr & CSR_RXRDY) == 0 );
	}
	return (serial_regs->us_rhr & 0xff);
    }
    return 0;
}


#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)

{
    if (( serial_regs->us_csr & CSR_RXRDY) != 0 )
    {
	if ((serial_regs->us_rhr & 0xff) == 27)
	    enter_kdebug("breakin");
    }
}
#endif

static void init(void) 
{
    serial_regs = (struct serial_at91rm9200*)(CONSOLE_VADDR);
}

/****************************************************************************
 *
 *    Console registration
 *
 ****************************************************************************/

kdb_console_t kdb_consoles[] = {
    { SERIAL_NAME, init, putc_serial, getc_serial},
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

