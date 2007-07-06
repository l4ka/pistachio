/*********************************************************************
 *                
 * Copyright (C) 2001, 2002, 2003,  Karlsruhe University
 *                
 * File path:     mini-print.cc
 * Description:   tiny version of printf
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
 * $Id: mini-print.cc,v 1.5 2003/09/24 19:06:28 skoglund Exp $
 *                
 ********************************************************************/
#include <l4io.h>
#include <l4/types.h>
#include <stdarg.h>


void print_hex(L4_Word_t val);
void print_string(char* s);
void print_dec(L4_Word_t val);


extern "C" int puts (const char * str)
{
    return printf ("%s\n", str);
}

extern "C" int putchar (int c)
{
    printf ("%c", c);
    return c;
}

static const char hexchars[] = "0123456789ABCDEF";

void print_hex(L4_Word_t val)
{
    signed int i;	/* becomes negative */
    for (i=sizeof(L4_Word_t)*8-4; i >= 0; i -= 4)
	putc(hexchars[(val >> i) & 0xF]);
}

void print_string(char* s)
{
    while (*s)
	putc(*s++);
}


void print_dec(L4_Word_t val)
{
    char buff[16];
    L4_Word_t i = 14;

    buff[15] = '\0';
    do
    {
	buff[i] = (val % 10) + '0';
	val = val / 10;
	i--;
    } while(val);

  
    print_string(&buff[i+1]);

}




int printf(const char* format, ...)
{
    L4_Word_t ret = 1;
    L4_Word_t i = 0;
    va_list ap;
    
    va_start (ap, format);

//#define arg(x) (((L4_Word_t *) &format)[(x)+1])
#define arg(x) va_arg (ap, L4_Word_t)

    /* sanity check */
    if (format == NULL)
	return 0;

    while (*format)
    {
	switch (*(format))
	{
	case '%':
	next_fmt:
	    switch (*(++format))
	    {
	    case 'l': case '-':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		goto next_fmt;
		
	    case 'c':
		putc (arg (i));
		break;
	    case 'd':
		print_dec (arg (i));
		break;
	    case 'p':
	    case 'x':
		print_hex ((L4_Word_t) arg (i));
		break;
	    case 's':
		print_string ((char*) arg (i));
		break;
	    default:
		print_string ("?");
		break;
	    }
	    i++;
	    break;

	default:
	    putc (*format);
	    if (*format == '\n')
		putc('\r');
	    break;
	}

	format++;
    }

    va_end (ap);

    return ret;
}

