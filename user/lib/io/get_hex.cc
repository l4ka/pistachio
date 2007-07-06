/*********************************************************************
 *                
 * Copyright (C) 2001, 2002,  Karlsruhe University
 *                
 * File path:     get_hex.c
 * Description:   generic get_hex() based on putc() and getc()
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
 * $Id: get_hex.cc,v 1.3 2003/09/24 19:06:28 skoglund Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <l4io.h>

L4_Word_t
get_hex(void)
{
  L4_Word_t val = 0;
  char c, t;
  int i = 0;

  while ((t = c = getc()) != '\r')
  {
      switch(c)
      {
      case '0' ... '9':
	  c -= '0';
	  break;
	  
      case 'a' ... 'f':
	  c -= 'a' - 'A';
      case 'A' ... 'F':
	  c = c - 'A' + 10;
	  break;
      default:
	  continue;
      };
      val <<= 4;
      val += c;
      
      /* let the user see what happened */
      putc(t);

      if (++i == 8)
	  break;
  };
  return val;
}
