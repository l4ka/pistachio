/*********************************************************************
 *                
 * Copyright (C) 2002, 2007,  Karlsruhe University
 *                
 * File path:     kdb/generic/sprintf.cc
 * Description:   disassembler support code
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
 * $Id: sprintf.cc,v 1.9 2003/09/24 19:05:11 skoglund Exp $
 *                
 ********************************************************************/

#include <stdarg.h>
#include <debug.h>
#include <kdb/kdb.h>

int do_printf(const char* format_p, va_list args);




/****************************************************************************
 *
 * HACKY - born for ia32 - have to see how it works for others
 *
 ****************************************************************************/

/* convert nibble to lowercase hex char */
#define hexchars(x) (((x) < 10) ? ('0' + (x)) : ('a' + ((x) - 10)))


static SECTION(SEC_KDEBUG) void putc(char** obuf, char c)
{
    (*(*obuf)++) = c;
}
#define putc(x) putc(obuf, x)


static int SECTION(SEC_KDEBUG) print_hex(char** obuf, word_t val, int width)
{
    int i, n = 0;
    if (width == 0) 
    {
	while( (val >> (4*width)) &&
	       (width < (int) (2*sizeof(val))) ) width++;
	if (width == 0) width = 1;
    }
    for ( i = 4*(width-1); i >= 0; i -= 4, n++ )
	putc(hexchars((val >> i) & 0xF));
    return n;
}
#define print_hex(x...) print_hex(obuf, x)


static int SECTION(SEC_KDEBUG) print_string(char** obuf, char* s, int width)
{
    int n = 0;
    while (*s) { putc(*s++); n++; }
    while (n < width) { putc(' '); n++; }
    return n;
}
#define print_string(x...) print_string(obuf, x)


static int SECTION(SEC_KDEBUG) print_dec(char** obuf, word_t val, int width)
{
    word_t divisor;
    int digits;

    /* estimate number of spaces and digits */
    for (divisor = 1, digits = 1; val/divisor >= 10; divisor *= 10, digits++);

    /* print spaces */
    for ( ; digits < width; digits++ )
	putc(' ');
    
    /* print digits */
    do {
	putc(((val/divisor) % 10) + '0');
    } while (divisor /= 10);

    /* report number of digits printed */
    return digits;
}
#define print_dec(x...) print_dec(obuf, x)


static int SECTION(SEC_KDEBUG) do_sprintf(char** obuf, const char* format_p, va_list args)
{
    const char* format = format_p;
    int n = 0;
    int i = 0;
    int width = 8;
    
#define arg(x) va_arg(args, x)

    /* sanity check */
    if (format == NULL)
	return 0;

    while (*format)
    {
	switch (*(format))
	{
	case '%':
	    width = 0;
	reentry:
	    switch (*(++format))
	    {
		/* modifiers */
	    case '0'...'9':
		width = width*10 + (*format)-'0';
		goto reentry;
		break;
	    case 'l':
		goto reentry;
		break;
	    case 'c':
		putc(arg(int));
		n++;
		break;
	    case 'd':
	    {
		long val = arg(long);
		if (val < 0)
		{
		    putc('-');
		    val = -val;
		}
		n += print_dec(val, width);
		break;
	    }
	    case 'u':
		n += print_dec(arg(long), width);
		break;
	    case 'p':
		width = sizeof (word_t) * 2;
	    case 'x':
		n += print_hex(arg(long), width);
		break;
	    case 's':
	    {
		char* s = arg(char*);
		if (s)
		    n += print_string(s, width);
		else
		    n += print_string((char *) "(null)", width);
	    }
	    break;
	    case 'T':
	    {
#if 1
		print_hex(arg(word_t), 2*sizeof(word_t));
#else
		l4_threadid_t tid = arg(l4_threadid_t);
		if (l4_is_invalid_id(tid))
		{
		    n += print_string("INVALID "); break;
		}
		if (l4_is_nil_id(tid))
		{
		    n += print_string("NIL_ID  "); break;
		}
		if (tid.raw < 17)
		{
		    n += print_string("IRQ_");
		    if (tid.raw < 11) { putc(' '); n++; }
		    n += print_dec(tid.raw-1); if (tid.raw < 11) putc(' ');
		    n += print_string("  ");
		}
		print_hex(tid.raw);
	    }
	    break;
	    case 't':
	    {
		l4_threadid_t tid = arg(l4_threadid_t);
		if (l4_is_invalid_id(tid))
		{
		    n += print_string("INVALID"); break;
		}
		if (l4_is_nil_id(tid))
		{
		    n += print_string("NIL_ID"); break;
		}
		if (tid.raw < 17)
		{
		    n += print_string("IRQ_");
		    n += print_dec(tid.raw-1);
		}
		else
		{
# if defined(CONFIG_VERSION_X0)
		    print_hex((u8_t) tid.x0id.task);
		    putc('.');
		    print_hex((u8_t) tid.x0id.thread);
		    n += 5;
# else
		    n += print_hex(tid.raw);
# endif
		}
#endif
	    }
	    break;
	    case '%':
		putc('%');
		n++;
		format++;
		continue;
	    default:
		n += print_string((char *) "?", 0);
		break;
	    };
	    i++;
	    break;
	default:
	    putc(*format);
	    n++;
	    if(*format == '\n')
		putc('\r');
	    break;
	}
	format++;
    }
    return n;
}

extern "C" int SECTION(SEC_KDEBUG) sprintf(char* obuf, const char* format, ...)
{
    va_list args;

    int printed;
    char* buf = obuf;

    va_start(args, format);
    printed = do_sprintf(&buf, format, args);
    va_end(args);
    return printed;
};

extern "C" int SECTION(SEC_KDEBUG) fprintf(char* f, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = do_printf(format, args);
    va_end(args);
    return i;
};



