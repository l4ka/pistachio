/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,   University of New South Wales
 *                
 * File path:     kdb/platform/erpcn01/uart.cc
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
 * $Id: uart.cc,v 1.10 2004/03/17 22:36:05 cvansch Exp $
 *                
 ********************************************************************/

/* Talk to propane uart interface */

#include <debug.h>
#include INC_PLAT(serial.h)
#include INC_PLAT(gt64115.h)
#include INC_GLUE(schedule.h)
#include <kdb/console.h>
#include <debug.h>

#define ECHO 1

extern volatile unsigned int *propane_uart;

extern volatile unsigned int *propane;

/* Must be called first */
static int puart_change_settings(unsigned int settings)
{
	unsigned int config, baud;

	switch (settings & 0x00FF) {
	case BAUDRATE_1200_BPS: baud = 3472; break;
	case BAUDRATE_1800_BPS: baud = 2315; break;
	case BAUDRATE_2400_BPS: baud = 1736; break;
	case BAUDRATE_4800_BPS: baud = 868; break;
	case BAUDRATE_7200_BPS: baud = 579; break;
	case BAUDRATE_9600_BPS: baud = 434; break;
	case BAUDRATE_14400_BPS: baud = 289; break;
	case BAUDRATE_19200_BPS: baud = 217; break;
	case BAUDRATE_38400_BPS: baud = 109; break;
	case BAUDRATE_57600_BPS: baud = 72; break;
	case BAUDRATE_115200_BPS: baud = 36; break;
	case BAUDRATE_230400_BPS: baud = 18; break;
	case BAUDRATE_460800_BPS: baud = 9; break;
	case BAUDRATE_921600_BPS: baud = 5; break;	/* won't work */
	default:
		return -1;
	}
	config = baud << 8;
	switch ((settings>>8) & 0x0F) {
	case SERIAL_DATABITS_7: config |= 0x08; break;
	case SERIAL_DATABITS_8: config |= 0x00; break;
	default:
		return -1;
	}
	switch ((settings>>12) & 0x0F) {
	case SERIAL_PARITY_NONE:	config |= 0x00; break;
	case SERIAL_PARITY_EVEN:	config |= 0x03; break;
	case SERIAL_PARITY_ODD:		config |= 0x01; break;
	default:
		return -1;
	}
	switch ((settings>>16) & 0x0F) {
	case SERIAL_STOPBITS_10:	config |= 0x00; break;
	case SERIAL_STOPBITS_15:	config |= 0x04; break;
	case SERIAL_STOPBITS_20:	config |= 0x04; break;
	default:
		return -1;
	}
	config |= 1 << 21;	 /* Enable */
	propane_uart[0] = config;

	return 0;
}

extern "C" void puart_write_char(char c)
{
	while ((propane_uart[2] & 0x0F) >= 0xD); /* fifo count */
	propane_uart[1] = (unsigned char)c;
}

extern "C" int puart_read_char(int blocking)
{
    if (blocking) {
	while ((propane_uart[2] & 0x200));
    }
    if (!(propane_uart[2] & 0x200))
	return propane_uart[1];

    return -1;
}

void putc_serial(char chr)
{
    puart_write_char(chr);
    if (chr == '\n') puart_write_char('\r');
}

static char getc_serial(bool block)
{
    return puart_read_char(block);
}

extern int propane_init(void);

static void init_serial (void)
{
    if (propane_init())
	while(1);

    puart_change_settings(BAUDRATE_57600_BPS | SERIAL_DATABITS_8 |
			SERIAL_PARITY_NONE | SERIAL_STOPBITS_10);

    printf("\e[2J\e[H---- ERPCN01 ---- L4 Pistachio ----- \n");
    printf("(C) 2002, Carl van Schaik, University of New South Wales, OpenFuel\n\n");

    printf("Detected PROPANE system interface at 0x%016lx\n", propane);
    printf("  PROPANE UART at 0x%16lx\n", propane_uart);
}

word_t kdb_current_console = 0;

kdb_console_t kdb_consoles[] = {
    { "serial", &init_serial, &putc_serial, &getc_serial },
    KDB_NULL_CONSOLE
};


#ifdef CONFIG_KDB_BREAKIN
void SECTION(".kdebug") kdebug_check_breakin(void)
{
    if (puart_read_char(0) == 27)
	enter_kdebug("breakin");
}
#endif
