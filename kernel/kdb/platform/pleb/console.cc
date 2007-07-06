/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     kdb/arch/arm/console.cc
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
 * $Id: console.cc,v 1.6 2004/12/09 01:21:41 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_GLUE(config.h)
#include INC_PLAT(console.h)
#include <kdb/console.h>

/****************************************************************************
 *
 *    Serial console support
 *
 ****************************************************************************/

struct serial_uart3 {
    /* this struct must be packed */
    u32_t   UTCR0;
    u32_t   UTCR1;
    u32_t   UTCR2;
    u32_t   UTCR3;
    u32_t   res1;
    u32_t   UTDR;
    u32_t   res2;
    u32_t   UTSR0;
    u32_t   UTSR1;
};


// XXX FIXME - define

static volatile struct serial_uart3 *serial_regs = NULL;

static void putc_serial( char c )
{
    if ( serial_regs )
    {
	while (( serial_regs->UTSR1 & 0x04 ) == 0 );

	serial_regs->UTDR = c;
	if ( c == '\n' )
	    putc_serial( '\r' );
    }
}

static char getc_serial( bool block )
{
    if ( serial_regs )
    {
	char c;
	if (( serial_regs->UTSR1 & 0x02) == 0 )
	{
	    if (!block)
		return -1;
	    while (( serial_regs->UTSR1 & 0x02) == 0 );
	}
	c = serial_regs->UTDR;
	if (c == '\n') c = '\r';
	return c;
    }
    return -1;
}

#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)

{
    if (( serial_regs->UTSR1 & 0x02) != 0 )
    {
	if (serial_regs->UTDR == 27)
	    enter_kdebug("breakin");
    }
}
#endif

static void init(void) 
{
    serial_regs = (struct serial_uart3 *)CONSOLE_VADDR;
}

word_t kdb_current_console = 0;

kdb_console_t kdb_consoles[] = {
        { "serial", &init, &putc_serial, &getc_serial},
        KDB_NULL_CONSOLE
};
