/*********************************************************************
 *
 * Copyright (C) 2003-2004, University of New South Wales
 *
 * File path:    elf-loader/openfirmware/console.cc
 * Description:  Open Firmware (IEEE std 1275) console code.
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
 * $Id: console.cc,v 1.4 2004/01/21 23:49:02 philipd Exp $
 *
 ********************************************************************/

#include <openfirmware/console.h>
#include <openfirmware/openfirmware.h>

ofw_ihandle_t ofw_stdin;
ofw_ihandle_t ofw_stdout;

void
ofw_setup_console(void)
{
  /* Find 'chosen' package to find out what devices are stdin/out. */
  ofw_phandle_t chosen;
  unsigned int buffer;

  if((chosen = ofw_finddevice(ADDR2OFW_STRING("/chosen"))) == 
     INT2OFW_CELL(-1)) {

    ofw_panic(OFW_ERROR_NOCHOSEN);
  }

  /* Get stdout instance */
  if(ofw_getprop(chosen,
		 ADDR2OFW_STRING("stdout"),
		 ADDR2OFW_BUFFER(&buffer),
		 sizeof(int)) != sizeof(int)) {

    ofw_panic(OFW_ERROR_NOSTDOUT);
  } else {
    ofw_stdout = buffer;
  }

  /* Get stdin instance */
  if(ofw_getprop(chosen,
		  ADDR2OFW_STRING("stdin"), 
		  ADDR2OFW_BUFFER(&buffer),
		  sizeof(int)) != sizeof(int)) {

    ofw_error("ofw_setup_console:\tCan't open stdin!");
  } else {
    ofw_stdin = buffer;
  }
  
} // ofw_setup_console()

void
ofw_error(const char * msg)
{
  printf("elf-loader fatal error!\n%s\n", msg);

  ofw_enter(); // Trap to Open Firmware command interpreter.

} // ofw_error()

extern "C" void
putc(int c) {
  char ch = c;

  if(c == '\n') {
    putc('\r');
  }

  ofw_write(ofw_stdout, ADDR2OFW_BUFFER(&ch), INT2OFW_CELL(1));
  
} // putc()

extern "C" int
getc(void) {
  unsigned char ch = '\0';
  ofw_cell_t l;

  while(l = (ofw_read(ofw_stdin, ADDR2OFW_BUFFER(&ch), INT2OFW_CELL(1))) != 
	INT2OFW_CELL(1)) {
    if(l = INT2OFW_CELL(-1)) { 

      ofw_error("getc:\t Can't read from stdin!");

      return ch;
    }
  }

  return ch;

} // getc()
