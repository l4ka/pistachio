/*********************************************************************
 *
 * Copyright (C) 2003-2004,  University of New South Wales
 *
 * File path:      contrib/elf-loader/platform/innovator/io.h
 * Description:    Interface to basic IO
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
 * $Id: io.h,v 1.2 2004/05/31 05:42:36 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ELF_LOADER__PLATFORM__INNOVATOR__IO_H__
#define __ELF_LOADER__PLATFORM__INNOVATOR__IO_H__

#define ETH_IOBASE   0x08000300
#define FLASH_IOBASE 0x00000000

#define FPGA_DIP_SWITCH_REGISTER (volatile unsigned char *)0x800000e
#define DONT_USE_SERIAL_VALUE    0x8


#define SER 0
#define PAR 1
#define USB 2

#define __swap_16(x)       \
  ((((x) & 0xff00) >> 8) | \
   (((x) & 0x00ff) << 8))

#define __swap_32(x)            \
  ((((x) & 0xff000000) >> 24) | \
   (((x) & 0x00ff0000) >>  8) | \
   (((x) & 0x0000ff00) <<  8) | \
   (((x) & 0x000000ff) << 24))

extern void io_putchar(unsigned char c);
extern unsigned char io_getchar_con(void); 
extern unsigned char io_getchar_ser(void);
extern unsigned char io_getchar_par(void); //see below.
extern void print_byte_ser(unsigned char);
extern unsigned char io_getc(int timeout_msec);

extern void io_init(void);

#endif /* __ELF_LOADER__PLATFORM__INNOVATOR__IO_H__ */
