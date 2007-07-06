/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     kdb/arch/platform/innovator/console.cc
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
 * $Id: console.cc,v 1.2 2004/06/04 06:43:13 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_GLUE(config.h)
#include <kdb/console.h>
#include INC_PLAT(reg.h)
#include INC_PLAT(console.h)
#include INC_CPU(io.h)

/****************************************************************************
 *
 *    Serial console support
 *
 ****************************************************************************/
static word_t UART_IOBASE = NULL;

static char
inreg_ser (word_t reg)
{
    char val;
    val = *((volatile char *) (UART_IOBASE + (reg * SER_UART_REGDELTA)));
    return val;
}

static void
outreg_ser (word_t reg, char val)
{
    *((volatile char *) (UART_IOBASE + (reg * SER_UART_REGDELTA))) = val;
}

static bool
is_ser_char ()
{
    if (inreg_ser (SER_LSR) & SER_LSR_DR)
	return true;
    else
	return false;
}

static void
putc_serial (char c)
{
    if (UART_IOBASE)
    {
	if ('\n' == c)
	    putc_serial ('\r');

	while (0 == (inreg_ser (SER_LSR) & SER_LSR_THRE))
	{
	}
	outreg_ser (SER_THR, c);
    }
}

static char
getc_serial (bool block)
{
    if (UART_IOBASE)
    {
	if (is_ser_char ())
	    return inreg_ser (SER_RBR);
	else
	{
	    if (!block)
		return 0xff;
	    while (!is_ser_char ());
	}
	return inreg_ser (SER_RBR);
    }
    return 0xff;
}



#if defined(CONFIG_KDB_BREAKIN)
void
kdebug_check_breakin (void)
{
    if (is_ser_char ())
    {
	if (inreg_ser (SER_RBR) == 27)
	    enter_kdebug ("breakin");
    }
}
#endif

static void
init (void)
{
    UART_IOBASE = io_to_virt (CONSOLE_PADDR);

//    printf("DPLL is %p, CKCTL is %p\n", \
//      *((volatile word_t*) io_to_virt(0xfffecf00)), \
//      *((volatile word_t*) io_to_virt(0xfffece00)));
}

word_t kdb_current_console = 0;

kdb_console_t kdb_consoles[] = {
    {"serial", &init, &putc_serial, &getc_serial},
    KDB_NULL_CONSOLE
};
