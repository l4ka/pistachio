/*********************************************************************
 *                
 * Copyright (C) 2001-2004, 2007  Karlsruhe University
 *                
 * File path:     print.cc
 * Description:   fully featured printf
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
 * $Id: print.cc,v 1.15 2004/09/06 16:20:21 ud3 Exp $
 *                
 ********************************************************************/
#include <l4/types.h>
#include <stdarg.h>
#include <l4io.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

/*
 * Make __l4_ prefixes for regular output functions, and create weak
 * symbols that aliases these functions. 
 */

extern "C" int __l4_printf (const char *fmt, ...);
extern "C" int printf (const char *fmt, ...)
    __attribute__ ((weak, alias ("__l4_printf")));
extern "C" int __l4_snprintf (char *str, L4_Size_t size, const char *fmt, ...);
extern "C" int snprintf (char *str, L4_Size_t size, const char *fmt, ...)
    __attribute__ ((weak, alias ("__l4_snprintf")));
extern "C" int __l4_vsnprintf (char *str, L4_Size_t size, const char *fmt, va_list);
extern "C" int vsnprintf (char *str, L4_Size_t size, const char *fmt, va_list)
    __attribute__ ((weak, alias ("__l4_vsnprintf")));
extern "C" int __l4_puts (const char * str);
extern "C" int puts (const char * str)
    __attribute__ ((weak, alias ("__l4_puts")));
extern "C" int __l4_putchar (int c);
extern "C" int putchar (int c)
    __attribute__ ((weak, alias ("__l4_putchar")));



static int strlen (const char * s);
static void print_string (const char * s);

int __l4_puts (const char * str)
{
    print_string (str);
    putc ('\n');
    return strlen (str);
}

int __l4_putchar (int c)
{
    putc (c);
    return c;
}


/*
 * Function strlen (s)
 *
 *    Return the length of string `s', not including the terminating
 *    `\0' character.
 *
 */
static int strlen (const char * s)
{	
    int n;

    for ( n = 0; *s++ != '\0'; n++ )
	;

    return n;
}


/*
 * Function print_string (s)
 *
 *    Print string `s' to terminal (or some kind of output).
 *
 */
static void print_string (const char * s)
{
    while ( *s )
	putc(*s++);
}


/*
 * Function vsnprintf (str, size, fmt, ap)
 *
 *    Formated output conversion according to format string `fmt' and
 *    argument list `ap'.  The formated string is stored in `out'.  If
 *    the formated string is longer tha `outsize' characters, the
 *    string is truncated.
 *
 *    The format conversion works as in printf(3), except for the
 *    conversions; e, E, g, and G which are not supported.
 *
 */
int __l4_vsnprintf(char *str, L4_Size_t size, const char *fmt, va_list ap)
{
    char convert[64];
    int f_left, f_sign, f_space, f_zero, f_alternate;
    int f_long, f_short, f_ldouble;
    int width, precision;
    int i, c, base, sign, signch, numdigits, numpr = 0, someflag;
    unsigned long uval, uval2;
    const char *digits;
    char *string, *p = str;
    L4_ThreadId_t tid;

#define PUTCH(ch) do {			\
	if ( size-- <= 0 )		\
	    goto Done;			\
	*p++ = (ch);			\
    } while (0)

#define PUTSTR(st) do {			\
	const char *s = (st);		\
	while ( *s != '\0' ) {		\
	    if ( size-- <= 0 )	\
		goto Done;		\
	    *p++ = *s++;		\
	}				\
    } while (0)

#define PUTDEC(num) do {					\
	uval = num;						\
	numdigits = 0;						\
	do {							\
	    convert[ numdigits++ ] = digits[ uval % 10 ];	\
	    uval /= 10;						\
	} while ( uval > 0 );					\
	while ( numdigits > 0 )					\
	    PUTCH( convert[--numdigits] );			\
    } while (0)

#define LEFTPAD(num) do {				\
	int cnt = (num);				\
	if ( ! f_left && cnt > 0 )			\
	    while ( cnt-- > 0 )				\
		PUTCH( f_zero ? '0' : ' ' );		\
    } while (0)

#define RIGHTPAD(num) do {				\
	int cnt = (num);				\
	if ( f_left && cnt > 0 )			\
	    while ( cnt-- > 0 )				\
		PUTCH( ' ' );				\
    } while (0)

    /*
     * Make room for null-termination.
     */
    size--;

    /*
     * Sanity check on fmt string.
     */
    if ( fmt == NULL ) {
	PUTSTR( "(null fmt string)" );
	goto Done;
    }

    while ( size > 0 ) {
	/*
	 * Copy characters until we encounter '%'.
	 */
	while ( (c = *fmt++) != '%' ) {
	    if ( size-- <= 0 || c == '\0' )
		goto Done;
	    *p++ = c;
	}

	f_left = f_sign = f_space = f_zero = f_alternate = 0;
	someflag = 1;

	/*
	 * Parse flags.
	 */
	while ( someflag ) {
	    switch ( *fmt ) {
	    case '-': f_left = 1;	fmt++; break;
	    case '+': f_sign = 1;	fmt++; break;
	    case ' ': f_space = 1;	fmt++; break;
	    case '0': f_zero = 1;	fmt++; break;
	    case '#': f_alternate = 1;	fmt++; break;
	    default:  someflag = 0;            break;
	    }
	}

	/*
	 * Parse field width.
	 */
	if ( (c = *fmt) == '*' ) {
	    width = va_arg( ap, int );
	    fmt++;
	} else if ( c >= '0' && c <= '9' ) {
	    width = 0;
	    while ( (c = *fmt++) >= '0' && c <= '9' ) {
		width *= 10;
		width += c - '0';
	    }
	    fmt--;
	} else {
	    width = -1;
	}

	/*
	 * Parse precision.
	 */
	if ( *fmt == '.' ) {
	    if ( (c = *++fmt) == '*' ) {
		precision = va_arg( ap, int );
		fmt++;
	    } else if ( c >= '0' && c <= '9' ) {
		precision = 0;
		while ( (c = *fmt++) >= '0' && c <= '9' ) {
		    precision *= 10;
		    precision += c - '0';
		}
		fmt--;
	    } else {
		precision = -1;
	    }
	} else {
	    precision = -1;
	}

	f_long = f_short = f_ldouble = 0;

	/*
	 * Parse length modifier.
	 */
	switch ( *fmt ) {
	case 'h': f_short = 1;		fmt++; break;
	case 'l': f_long = 1;		fmt++; break;
	case 'L': f_ldouble = 1;	fmt++; break;
	}

	sign = 1;

	/*
	 * Parse format conversion.
	 */
	switch ( c = *fmt++ ) {

	case 'b':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 2;
	    digits = "01";
	    goto Print_unsigned;

	case 'o':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 8;
	    digits = "012345678";
	    goto Print_unsigned;

	case 'p':
	    precision = width = sizeof (long) * 2;
	    f_alternate = 1;
	    if (sizeof (void *) == sizeof (long))
		f_long = 1;
	case 'x':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 16;
	    digits = "0123456789abcdef";
	    goto Print_unsigned;

	case 'X':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 16;
	    digits = "0123456789ABCDEF";
	    goto Print_unsigned;

	case 'd':
	case 'i':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 10;
	    digits = "0123456789";
	    goto Print_signed;

	case 'u':
	    uval = f_long ? va_arg( ap, long ) : va_arg( ap, int );
	    base = 10;
	    digits = "0123456789";

	Print_unsigned:
	    sign = 0;

	Print_signed:
	    signch = 0;
	    uval2 = uval;

	    /*
	     * Check for sign character.
	     */
	    if ( sign ) {
		if ( f_sign && (long) uval >= 0 ) {
		    signch = '+';
		} else if ( f_space && (long) uval >= 0 ) {
		    signch = ' ';
		} else if ( (long) uval < 0 ) {
		    signch = '-';
		    uval = -( (long) uval );
		}
	    }

	    /*
	     * Create reversed number string.
	     */
	    if (sizeof (long) >= 8 && base == 16)
	    {
		numdigits = 0;
		do {
		    convert[numdigits++] =  digits[uval & 0xf];
		    uval /= 16;
		} while ( uval > 0 );
	    }
	    else
	    {
		numdigits = 0;
		do {
		    convert[numdigits++] =  digits[(unsigned int) uval % base];
		    uval /= base;
		} while ( uval > 0 );
	    }

	    /*
	     * Calculate the actual size of the printed number.
	     */
	    numpr = numdigits > precision ? numdigits : precision;
	    if ( signch )
		numpr++;
	    if ( f_alternate /*&& uval2 != 0*/ ) {
		if ( base == 8 )
		    numpr++;
		else if ( base == 16 || base == 2 )
		    numpr += 2;
	    }

	    /*
	     * Insert left padding.
	     */
	    if ( ! f_left && width > numpr ) {
		if ( f_zero ) {
		    numpr = width;
		} else {
		    for ( i = width - numpr; i > 0; i-- )
			PUTCH(' ');
		}
	    }

	    /*
	     * Insert sign character.
	     */
	    if ( signch ) {
		PUTCH( signch );
		numpr--;
	    }

	    /*
	     * Insert number prefix.
	     */
	    if ( f_alternate /*&& uval2 != 0*/ ) {
		if ( base == 2 ) {
		    numpr--;
		    PUTCH('%');
		} else if ( base == 8 ) {
		    numpr--;
		    PUTCH('0');
		} else if ( base == 16 ) {
		    numpr -= 2;
		    PUTSTR("0x");
		}
	    }

	    /*
	     * Insert zero padding.
	     */
	    for ( i = numpr - numdigits; i > 0; i-- )
		PUTCH('0');

	    /*
	     * Insert number.
	     */
	    while ( numdigits > 0 )
		PUTCH( convert[--numdigits] );
	    RIGHTPAD( width - numpr - (signch ? 1 : 0) );
	    break;

#if !defined(L4_ARCH_ALPHA)
	case 'f': 
	{
	    double fval = va_arg( ap, double );
	    if ( precision == -1 )
		precision = 6;

	    /*
	     * Check for sign character.
	     */
	    if ( f_sign && fval >= 0.0 ) {
		signch = '+';
	    } else if ( f_space && fval >= 0.0 ) {
		signch = ' ';
	    } else if ( fval < 0.0 ) {
		signch = '-';
		fval = -fval;
	    } else {
		signch = 0;
	    }

	    /*
	     * Get the integer part of the number.  If the floating
	     * point value is greater than the maximum value of an
	     * unsigned long, the result is undefined.
	     */
	    uval = (unsigned long) fval;
	    numdigits = 0;
	    do {
		convert[ numdigits++ ] =  '0' + uval % 10;
		uval /= 10;
	    } while ( uval > 0 );

	    /*
	     * Calculate the actual size of the printed number.
	     */
	    numpr = numdigits + (signch ? 1 : 0);
	    if ( precision > 0 )
		numpr += 1 + precision;

	    LEFTPAD( width - numpr );

	    /*
	     * Insert sign character.
	     */
	    if ( signch )
		PUTCH( signch );

	    /*
	     * Insert integer number.
	     */
	    while ( numdigits > 0 )
		PUTCH( convert[--numdigits] );

	    /*
	     * Insert precision.
	     */
	    if ( precision > 0 ) {
		/*
		 * Truncate number to fractional part only.
		 */
		while ( fval >= 1.0 )
		    fval -= (double) (unsigned long) fval;

		PUTCH('.');

		/*
		 * Insert precision digits.
		 */
		while ( precision-- > 0 ) {
		    fval *= 10.0;
		    uval = (unsigned long) fval;
		    PUTCH( '0' + uval );
		    fval -= (double) (unsigned long) fval;
		}
	    }

	    RIGHTPAD( width - numpr );
	    break;
	}
#endif
	case 't':
	    tid = va_arg( ap, L4_ThreadId_t );
	    if ( L4_IsNilThread(tid) ) {
		PUTSTR("NIL_ID");
		break;
	    }
	    digits = "0123456789";
	    if (L4_IsGlobalId(tid))
	    {
		PUTSTR("GlobThread=");
		PUTDEC(tid.global.X.thread_no);
		PUTSTR(",ver=");
		PUTDEC(tid.global.X.version);
	    } 
	    else if (L4_IsLocalId(tid))
	    {
		PUTSTR("LocThread=");
		PUTDEC(tid.local.X.local_id);
	    }
	    else {
		PUTSTR("? ");
		PUTDEC(tid.raw);
	    }
	    break;


	case 's':
	    string = va_arg( ap, char * );

	    /*
	     * Sanity check.
	     */
	    if ( string == NULL ) {
		PUTSTR( "(null)" );
		break;
	    }

	    if ( width > 0 ) {
		/*
		 * Calculate printed size.
		 */
		numpr = strlen( string );
		if ( precision >= 0 && precision < numpr )
		    numpr = precision;

		LEFTPAD( width - numpr );
	    }

	    /*
	     * Insert string.
	     */
	    if ( precision >= 0 ) {
 		while ( precision-- > 0 && (c = *string++) != '\0' )
		    PUTCH( c );
	    } else {
 		while ( (c = *string++) != '\0' )
		    PUTCH( c );
	    }

	    RIGHTPAD( width - numpr );
	    break;

	case 'c':
	    PUTCH( va_arg( ap, int ) );
	    break;

	case '%':
	    PUTCH('%');
	    break;

	case 'n':
	    *(va_arg( ap, int * )) = p - str;
	    break;

	default:
	    PUTCH('%');
	    PUTCH(c);
	    break;
	}
    }
 Done:

    /*
     * Null terminate string.
     */
    *p = '\0';

    /*
     * Return size of printed string.
     */
    return p - str;
}


/*
 * Function snprintf (str, size, fmt, ...)
 *
 *    Print formated string to a string like snprintf(3).
 *
 */
int __l4_snprintf(char *str, L4_Size_t size, const char *fmt, ...)
{
    va_list ap;
    int r;

    /*
     * Safety check
     */
    if ( fmt == NULL )
	return 0;

    /*
     * Print into buffer.
     */
    va_start(ap, fmt);
    r = vsnprintf(str, size, fmt, ap);
    va_end(ap);

    return r;
}

/*
 * Function printf (fmt, ...)
 *
 *    Print formated string to terminal like printf(3).
 *
 */
int __l4_printf(const char *fmt, ...)
{
    char outbuf[256];
    va_list ap;
    int r;

    /*
     * Safety check
     */
    if ( fmt == NULL )
	return 0;

    /*
     * Print into buffer.
     */
    va_start(ap, fmt);
    r = vsnprintf(outbuf, sizeof(outbuf), fmt, ap);
    va_end(ap);

    /*
     * Output to terminal.
     */
    if ( r > 0 )
	print_string(outbuf);

    return r;
}
