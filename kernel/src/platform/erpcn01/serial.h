/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     platform/erpcn01/serial.h
 * Description:   Propane serial port defines
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
 * $Id: serial.h,v 1.4 2003/09/24 19:04:55 skoglund Exp $
 *                
 ********************************************************************/
/*
 * Carl van Schaik - Openfuel Pty Ltd.
 */


/* Serial api definitions */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#define BAUDRATE_NOT_DEFINED  0
#define BAUDRATE_075_BPS      1
#define BAUDRATE_110_BPS      2
#define BAUDRATE_150_BPS      3
#define BAUDRATE_300_BPS      4
#define BAUDRATE_600_BPS      5
#define BAUDRATE_1200_BPS     6
#define BAUDRATE_1800_BPS     7
#define BAUDRATE_2400_BPS     8
#define BAUDRATE_4800_BPS     9
#define BAUDRATE_7200_BPS     10
#define BAUDRATE_9600_BPS     11
#define BAUDRATE_14400_BPS    12
#define BAUDRATE_19200_BPS    13
#define BAUDRATE_38400_BPS    14
#define BAUDRATE_57600_BPS    15
#define BAUDRATE_115200_BPS   16
#define BAUDRATE_230400_BPS   17
#define BAUDRATE_460800_BPS   18
#define BAUDRATE_921600_BPS   19

#define SERIAL_DATABITS_5           1
#define SERIAL_DATABITS_6           2
#define SERIAL_DATABITS_7           3
#define SERIAL_DATABITS_8           4

#define SERIAL_PARITY_NONE          1
#define SERIAL_PARITY_ODD           2
#define SERIAL_PARITY_EVEN          3
#define SERIAL_PARITY_MARK          4
#define SERIAL_PARITY_SPACE         5

#define SERIAL_STOPBITS_10          1
#define SERIAL_STOPBITS_15          2
#define SERIAL_STOPBITS_20          3

/* UART structure */
typedef struct {
	unsigned int	settings;	/* Masks : 
									0x000000FF - Baud,
									0x00000F00 - Databits,
									0x0000F000 - Parity,
									0x000F0000 - Stopbits
								*/
	int (*change_settings)(unsigned int);
	int (*read_char)(int);		/* input - '1' - blocking, '0' nonblocking */
	void (*write_char)(char);

	void (*puts)(char*);
	int (*gets)(char*, int);
	int (*data_ready)();
} serial_uart;

extern serial_uart *uart;

extern serial_uart fpga_uart;
extern serial_uart puart;

/* Defined in the libraries */
int mprintf(char *fmt, ...);

#endif
