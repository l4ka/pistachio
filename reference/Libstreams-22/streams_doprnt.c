/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.1 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifdef SHLIB
#include "shlib.h"
#endif
/*
 * Copyright (c) 1987 NeXT, INC.
 *
 *	Streams version.
 */

#include "defs.h"
#include <ctype.h>
#include <stdarg.h>
 
#define MAXOCT	11	/* Maximum octal digits in a long */
#define MAXDIGS	10	/* number of decimal digits in unsigned long */
#define	MAXFPPREC 25	/* really only 17 are significant */


#define	FILL_MINUS	-1
#define	FILL_ZERO	0
#define	FILL_BLANK	1		/* default */

#define	SIGN_MINUS	-1		/* default */
#define	SIGN_SPACE	0
#define	SIGN_ALWAYS	1

#define	LEN_INT		0
#define	LEN_LONGINT	1
#define	LEN_SHORTINT	2
#define	LEN_LONGDOUBLE	3		/* ANSI C eventually */

void _doprnt();

static char *_p_dconv();
static char *_ffmt(va_list *app, char *buf, int prec, int alt);
static char *_efmt(va_list *app, char *buf, int ndigit, int fcode, int alt);
static char *_gfmt(va_list *app, char *buf, int prec, int fcode, int alt, int sign_type);
extern int isnan(double num);
extern int isinf(double num);
extern char *ecvt(double x, int ndigits, int *decimal, int *sign);
extern char *fcvt(double x, int ndigits, int *decimal, int *sign);

typedef struct _PrintfProcData {
    /* Records an entry in the printfProcTable */
    char		special;
    NXPrintfProc	*proc;
    void		*procData;
} PrintfProcData;

static PrintfProcData *printfProcTable = NULL;
static unsigned sizePrintfProcTable = 0;

void NXRegisterPrintfProc(char special, NXPrintfProc *proc, void *procData) {
    printfProcTable = realloc(printfProcTable, (sizePrintfProcTable+1) * sizeof(PrintfProcData));
    printfProcTable[sizePrintfProcTable].special = special;
    printfProcTable[sizePrintfProcTable].proc = proc;
    printfProcTable[sizePrintfProcTable].procData = procData;
    sizePrintfProcTable++;
}

static int isRegisteredPrintfChar(char fcode, NXPrintfProc **proc, void **procData) {
    unsigned	index = sizePrintfProcTable;
    while (index--) {
	if (printfProcTable[index].special == fcode) {
	    *proc = printfProcTable[index].proc;
	    *procData = printfProcTable[index].procData;
	    return 1;
	}
    }
    return 0;
}

void
NXVPrintf(NXStream *stream, register const char *fmt, va_list ap)
{
    register char *begp, *endp;	/* begin and end ptrs in buf */
    register long int num;		/* current number being converted */
    char buf[400];			/* conversion done here */
    register char fcode;		/* current format char */
    int alt, sign, match;		/* '#', '+', ' ' flag stuff */
    int prec;			/* digits after . for flt conv */
    int length;			/* parameter length */
    int mask, nbits, hexcase, digit;/* octal, hex conv */
    int neg;
    int width, leftadj, fill;
    NXPrintfProc	*proc;
    void		*procData;

    _NXVerifyStream(stream);
    for (;;) {
	/* process format string first */
	while ((fcode = *fmt++)!='%') {
	    /* ordinary (non-%) character */
	    if (fcode=='\0')
		return;
	    NXPutc(stream, fcode);
	}
	/*
	 * Initialize default values for "flags"
	 */
	length = LEN_INT;
	leftadj = 0;
	fill = FILL_BLANK;
	alt = 0;
	sign = SIGN_MINUS;
	width = 0;
	prec = -1;

	match = 1;
	/*
	 * At exit from the following switch, we will
	 * emit the characters starting at "begp" and
	 * ending at "endp"-1, unless fcode is '\0'.
	 */
	fcode = *fmt++;
postprefix:	
	switch (fcode) {
	/* process characters and strings first */
	case 'c':
	    buf[0] = va_arg(ap, int);
	    endp = begp = &buf[0];
#ifdef notdef
	    /* it is deemed correct to emit null chars */
	    if (buf[0] != '\0')
#endif notdef
		endp++;
	    break;
	case 's':
	    begp = va_arg(ap,char *);
	    if (begp==0)
		begp = "(null pointer)";
	    for (endp = begp; *endp && prec-- != 0; endp++)
		continue;
	    break;
	case 'O':
	    if (match) goto prefix;
	    alt = 1;
	    fcode = 'o';
	    /* no break */
	case 'o':
	case 'X':
	case 'x':
	    hexcase = isupper(fcode) ? 'A' : 'a';
	    switch (length) {
	    default:
	    case LEN_INT:
		num = (unsigned)va_arg(ap,int);
		break;
	    case LEN_LONGINT:
		num = va_arg(ap,long);
		break;
	    case LEN_SHORTINT:
		num = (unsigned short)va_arg(ap,int);
		break;
	    }
	    if (fcode=='o') {
		mask = 0x7;
		nbits = 3;
	    } else {
		mask = 0xf;
		nbits = 4;
	    }
	    endp = begp = buf + MAXOCT + 3;
	    /* shift and mask for speed */
	    do {
		digit = (int)num & mask;
		if (digit < 10)
		    *--begp = digit + '0';
		else
		    *--begp = digit - 10 + hexcase;
	    } while (num = (unsigned)num >> nbits);
	    
	    if (alt) {
		if (fcode=='o') {
		    if (*begp != '0')
			*--begp = '0';
		} else if (!leftadj
			   && (fill == FILL_ZERO ||
			       fill == FILL_MINUS)) {
		    NXPutc(stream, '0');
		    NXPutc(stream, fcode);
		    if ((width-=2)<0) width=0;
		} else {
		    *--begp = fcode;
		    *--begp = '0';
		}
	    }
	    break;
	/* these are here only for backward compat */
	case 'D':
	case 'U':
	case 'I':
	    length = LEN_LONGINT;
	    fcode = tolower(fcode);
	    /* no break */
	case 'd':
	case 'i':
	case 'u':
	    switch (length) {
	    default:
	    case LEN_INT:
		if (fcode == 'u')
		    num = (unsigned)va_arg(ap,int);
		else
		    num = (long)va_arg(ap,int);
		break;
	    case LEN_LONGINT:
		num = va_arg(ap,long);
		break;
	    case LEN_SHORTINT:
		if (fcode == 'u')
		    num = (unsigned short)va_arg(ap,int);
		else
		    num = (short)va_arg(ap,int);
		break;
		break;
	    }
	    if (neg = (fcode != 'u' && num < 0))
		num = -num;
	    /* now convert to digits */
	    begp = _p_dconv(num, buf);
	    if (neg)
		*--begp = '-';
	    if (fill == FILL_ZERO)
		fill = FILL_MINUS;
	    endp = buf + MAXDIGS + 1;
	    break;
	case 'e':
	case 'E':
	    if (prec < 0)
		prec = 6;
	    endp = _efmt(&ap, buf, prec+1, fcode, alt);
	    begp = buf;
	    break;
	case 'g':
	case 'G':
	    if (prec < 0)
		prec = 6;
	    endp = _gfmt(&ap, buf, prec, fcode, alt, -1);
	    begp = buf;
	    break;
	case 'f':
	    if (prec < 0)
		prec = 6;
	    endp = _ffmt(&ap, buf, prec, alt);
	    begp = buf;
	    break;
	default: 
	    if (isRegisteredPrintfChar(fcode, &proc, &procData)) {
		proc(stream, va_arg(ap, void *), procData);
		begp = endp = "";
	    } else if (match) {
prefix:
		for (;;) {
		    switch (fcode) {
		    case '-':
			leftadj = 1;
			break;
		    case '+':
			sign = SIGN_ALWAYS;
			break;
		    case ' ':
			if (sign != SIGN_ALWAYS)
			    sign = SIGN_SPACE;
			break;
		    case '#':
			alt = 1;
			break;
		    default:
			match = 0;
			break;
		    }
		    if (!match) break;
		    fcode = *fmt++;
		}

		/*
		 * Left adjust with zero fill is bogus, but the spec
		 * let's you do it, so we do too!
		 */
		if (fcode == '0') {
		    fill = FILL_ZERO;
		    fcode = *fmt++;
		}

		/* Now comes a digit string which may be a '*' */
		if (fcode == '*') {
		    fcode = *fmt++;
		    width = va_arg(ap, int);
		    if (width < 0) {
			width = -width;
			leftadj = !leftadj;
		    }
		} else {
		    while (isdigit(fcode)) {
			width = width * 10 + (fcode - '0');
			fcode = *fmt++;
		    }
		}
		
		/* maybe a decimal point followed by more digits (or '*') */
		if (fcode=='.') {
		    fcode = *fmt++;
		    if (fcode == '*') {
			prec = va_arg(ap, int);
			fcode = *fmt++;
		    } else {
			prec = 0;
			while (isdigit(fcode)) {
			    prec = prec * 10 + (fcode - '0');
			    fcode = *fmt++;
			}
		    }
		}

		/*
		 * SunOS printf allows a '#' after the precision
		 * field, we will too for compatibility
		 */

		if (fcode == '#') {
		    alt = 1;
		    fcode = *fmt++;
		}
		
		/*
		 * At this point, "leftadj" is nonzero if value is to be
		 * left adjusted, "fill" is FILL_ZERO if there was a
		 * leading zero and FILL_BLANK otherwise, "width" and
		 * "prec" contain numbers corresponding to the digit
		 * strings before and after the decimal point,
		 * respectively, and "fmt" addresses the next
		 * character after the whole mess. If there was
		 * no decimal point, "prec" will be -1.
		 */
		switch (fcode) {
		case 'l':
		    length = LEN_LONGINT;
		    fcode = *fmt++;
		    break;
		case 'L':		/* ANSI C */
		    length = LEN_LONGDOUBLE;
		    fcode = *fmt++;
		    break;
		case 'h':		/* ANSI C */
		    length = LEN_SHORTINT;
		    fcode = *fmt++;
		    break;
		}
		goto postprefix;

	    } else {
		/* not a control character, 
		 * print it.
		 */
		buf[0] = fcode;
		endp = begp = buf;
		endp++;
		break;
	    }
	}
	if (fcode != '\0') {
/*
* This program sends string "s" to NXPutc. The character after
* the end of "begp" is given by "endp". This allows the size of the
* field to be computed; it is stored in "alen". "width" contains the
* user specified length. If width<alen, the width will be taken to
* be alen. "leftadj" is zero if the string is to be right-justified
* in the field, nonzero if it is to be left-justified. "fill" is
* FILL_ZERO if the string is to be padded with '0', FILL_BLANK if
* it is to be padded with ' ', and FILL_MINUS if an initial '-' should
* appear before any padding in right-justification (to avoid printing
* "-3" as "000-3" where "-0003" was intended).
*/
	    char cfill;
	    register int alen;
	    int npad;
	    
	    alen = endp - begp;
	    if (alen >= width) {
		/* emit the string itself */
		while (--alen >= 0)
			NXPutc(stream, *begp++);
	    } else {
		npad = width - alen;
		cfill = (fill == FILL_BLANK) ? ' ': '0';		
		
		/* emit any leading pad characters */
		if (!leftadj) {
			/* we may print a leading '-' before padding */
			if (*begp == '-' && fill == FILL_MINUS) {
				NXPutc(stream, '-');
				begp++;
				alen--;
			}
			while (--npad >= 0)
				NXPutc(stream, cfill);
		}
				
		/* emit the string itself */
		while (--alen >= 0)
			NXPutc(stream, *begp++);
			
		/* emit trailing pad characters */
		if (leftadj)
			while (--npad >= 0)
				NXPutc(stream, cfill);
	    }
	}
    }
}

/* _p_dconv converts the unsigned long integer "value" to
 * printable decimal and places it in "buffer", right-justified.
 * The value returned is the address of the first non-zero character,
 * or the address of the last character if all are zero.
 * The result is NOT null terminated, and is MAXDIGS characters long,
 * starting at buffer[1] (to allow for insertion of a sign).
 *
 * This program assumes it is running on 32 bit int, 2's complement
 * machine with reasonable overflow treatment.
 */
static char *
_p_dconv(val, buf)
register unsigned long val;
char *buf;
{
	register char *bp;

	bp = buf + MAXDIGS + 1;

	do {
		*--bp = (val % 10) + '0';
		val /= 10;
	} while (val != 0);
	return(bp);
}


static char *_efmt(va_list *app, char *buf, int ndigit, int fcode, int alt)
{
	int sign, decpt;
	register char *p1, *p2;
	register i;
	char ebuf[MAXDIGS+1];
	double number;

	number = va_arg(*app, double);
	if (i = isnan(number)) {
		if (i > 0)
			strcpy(buf, "NaN");
		else
			strcpy(buf, "SNaN");
		return(&buf[strlen(buf)]);
	}
	if (i = isinf(number)) {
		if (i > 0)
			strcpy(buf, "+Infinity");
		else
			strcpy(buf, "-Infinity");
		return(&buf[strlen(buf)]);
	}
	/*
	 * Limit precision since IEEE doubles only have
	 * 17 or so digits of accuracy.  This also allows us
	 * to assume that ecvt/fcvt returned what we asked
	 * for (which isn't true beyond 350 or so digits,
	 * see ecvt.c).
	 */
	if (ndigit > MAXFPPREC)
		ndigit = MAXFPPREC;	/* anything more is garbage */
	p1 = ecvt(number, ndigit, &decpt, &sign);
	p2 = buf;
	if (*p1 != '0') {
		decpt--;
		if (sign)
			*p2++ = '-';
	}
	*p2++ = *p1++;
	*p2++ = '.';
	while (--ndigit > 0)
		*p2++ = *p1++;
	if (!alt && p2[-1]=='.')
		p2--;
	*p2++ = fcode;
	if (decpt < 0) {
		decpt = -decpt;
		*p2++ = '-';
	} else
		*p2++ = '+';
	p1 = _p_dconv(decpt, ebuf);
	/* always print at least 2 digits of exponent */
	if (&ebuf[sizeof(ebuf)] - p1 == 1)
		*p2++ = '0';
	while (p1 < &ebuf[sizeof(ebuf)])
		*p2++ = *p1++;
	*p2 = '\0';
	return(p2);
}

static char *_ffmt(va_list *app, char *buf, int prec, int alt)
{
	int sign, decpt;
	register char *p1, *p2;
	register i;
	double number;

	number = va_arg(*app, double);
	if (i = isnan(number)) {
		if (i > 0)
			strcpy(buf, "NaN");
		else
			strcpy(buf, "SNaN");
		return(&buf[strlen(buf)]);
	}
	if (i = isinf(number)) {
		if (i > 0)
			strcpy(buf, "+Infinity");
		else
			strcpy(buf, "-Infinity");
		return(&buf[strlen(buf)]);
	}
	/*
	 * Limit precision since IEEE doubles only have
	 * 17 or so digits of accuracy.  This also allows us
	 * to assume that ecvt/fcvt returned what we asked
	 * for (which isn't true beyond 350 or so digits,
	 * see ecvt.c).
	 */
	if (prec > MAXFPPREC)
		prec = MAXFPPREC;
	p1 = fcvt(number, prec, &decpt, &sign);
	p2 = buf;
	/* don't print a - sign if all we print is zeros */
	if (*p1 != '0' && sign && -decpt < prec)
		*p2++ = '-';
	if (decpt <= 0) {
		*p2++ = '0';
		*p2++ = '.';
		while (decpt < 0 && prec > 0) {
			*p2++ = '0';
			decpt++;
			prec--;
		}
	} else {
		while (decpt > 0) {
			*p2++ = *p1++;
			decpt--;
		}
		*p2++ = '.';
	}
	while (prec > 0) {
		*p2++ = *p1++;
		prec--;
	}
	if (!alt && p2[-1] == '.')
		p2--;
	*p2 = '\0';
	return(p2);
}


static char *btod(unsigned long val, char *buf)
{
        register char *bp;

        bp = buf + MAXDIGS + 1;

        do {
                *--bp = (val % 10) + '0';
                val /= 10;
        } while (val != 0);
        return(bp);
}

static char *_gfmt_common(double number, char *buf, int prec, int fcode, int alt, int sign_type)
{
	int is_neg, decpt;
	register char *p1, *p2;
	register i;
	char ebuf[MAXDIGS+1];

	if (i = isnan(number)) {
		if (i > 0)
			strcpy(buf, "NaN");
		else
			strcpy(buf, "SNaN");
		return(&buf[strlen(buf)]);
	}
	if (i = isinf(number)) {
		if (i > 0)
			strcpy(buf, "+Infinity");
		else
			strcpy(buf, "-Infinity");
		return(&buf[strlen(buf)]);
	}
	/*
	 * Limit precision since IEEE doubles only have
	 * 17 or so digits of accuracy.  This also allows us
	 * to assume that ecvt/fcvt returned what we asked
	 * for (which isn't true beyond 350 or so digits,
	 * see ecvt.c).
	 */
	if (prec > MAXFPPREC)
		prec = MAXFPPREC;	/* anything more is garbage */
	p1 = ecvt(number, prec+1, &decpt, &is_neg);
	p2 = buf;
	/*
	 * ANSI C standard says, "... style e will be used only if
	 * the exponent resulting from the conversion is less than -4
	 * or greater than or equal to the precision.  NOTE: this isn't
	 * the same as the old "minimal length string" UNIX definition.)
	 */
	if (decpt > prec || decpt < -3) {
		/* use E-style */
		/* don't put - signs on zero's */
		if (is_neg && *p1 != '0')
			*p2++ = '-';
		else if (sign_type == SIGN_ALWAYS)
			*p2++ = '+';
		else if (sign_type == SIGN_SPACE)
			*p2++ = ' ';
		if (*p1 != '0')
			decpt--;
		*p2++ = *p1++;
		*p2++ = '.';

		/* The ANSI C standard says for g or G format, "the double
		 * argument is converted in style f or e (or in style E
		 * in the case of G conversion specifier), with the
		 * precision specifying the the number of significant digits"
		 * The "number of significant digits" is not the same
		 * as "the number of digits to appear after the decimal-point"
		 * as in the e, E, and f conversions.  So since this is g
		 * format, which is being printed as E-style the and the
		 * leading digit has been placed in the buffer above before
		 * the decimal-point, we need to put in prec - 1 digits more
		 * in the buffer to get "the number of significant digits"
		 * requested.
		 */
		while (prec - 1 > 0) {
			*p2++ = *p1++;
			prec--;
		}
		if (! alt) {
			while (p2[-1] == '0')
				p2--;
			if (p2[-1] == '.')
				p2--;
		}
		*p2++ = isupper(fcode) ? 'E' : 'e';
		if (decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		p1 = btod(decpt, ebuf);
		/* always print at least 2 digits of exponent */
		if (&ebuf[sizeof(ebuf)] - p1 == 1)
			*p2++ = '0';
		while (p1 < &ebuf[sizeof(ebuf)])
			*p2++ = *p1++;
	} else {
		/* F-style */
		/*
		 * Reconvert to get correct rounding
		 * Unfortunately, quite expensive....
		 */
		p1 = fcvt(number, prec, &decpt, &is_neg);
		/* don't put - signs on zero's */
		if (*p1 != '0' && is_neg && -decpt < prec)
			*p2++ = '-';
		else if (sign_type == SIGN_ALWAYS)
			*p2++ = '+';
		else if (sign_type == SIGN_SPACE)
			*p2++ = ' ';
		if (decpt <= 0) {
			*p2++ = '0';
			*p2++ = '.';
			while (decpt < 0) {
				*p2++ = '0';
				decpt++;
				prec--;
			}
		} else {
			while (decpt > 0) {
				*p2++ = *p1++;
				decpt--;
			}
			*p2++ = '.';
		}
		while (prec > 0 && *p1) {
			*p2++ = *p1++;
			prec--;
		}
		if (! alt) {
			while (p2[-1] == '0')
				p2--;
			if (p2[-1] == '.')
				p2--;
		}
	}
	*p2 = '\0';
	return(p2);
}

static char *_gfmt(va_list *app, char *buf, int prec, int fcode, int alt, int sign_type)
{
        double	number;

        number = va_arg(*app, double);

        return _gfmt_common(number, buf, prec, fcode, alt, sign_type);
}


