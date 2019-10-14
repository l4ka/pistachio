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

#include "defs.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define	SPC	01
#define	STP	02

#define	SHORT	0
#define	REGULAR	1
#define	LONG	2
#define	INT	0
#define	FLOAT	1

static int _instr();
static int _innum();
static char *_getccl();

/* moved this table to the stack to make this code reentrant, and not
   contribute to the data segment.  -trey
 */
#ifdef IN_DATA_SEGMENT
static char _sctab[256] = {
	0,0,0,0,0,0,0,0,
	0,SPC,SPC,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	SPC,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};
#endif

#define SCTAB_SIZE	256

int
NXVScanf(NXStream *stream, register const char *fmt, va_list arg)
{
	register int ch;
	int nmatch, len, ch1;
	void *ptr;
	int fileended, size;
	char _sctab[SCTAB_SIZE];

	_NXVerifyStream(stream);
	bzero( _sctab, SCTAB_SIZE );
	_sctab[9] = _sctab[10] = _sctab[32] = SPC;
	nmatch = 0;
	fileended = 0;
	for (;;) switch (ch = *fmt++) {
	case '\0': 
		return (nmatch);
	case '%': 
		if ((ch = *fmt++) == '%')
			goto def;
		ptr = 0;
		if (ch != '*')
			ptr = va_arg(arg, void *);
		else
			ch = *fmt++;
		len = 0;
		size = REGULAR;
		while (isdigit(ch)) {
			len = len*10 + ch - '0';
			ch = *fmt++;
		}
		if (len == 0)
			len = 30000;
		if (ch=='l') {
			size = LONG;
			ch = *fmt++;
		} else if (ch=='h') {
			size = SHORT;
			ch = *fmt++;
		} else if (ch=='[')
			fmt = _getccl(fmt, _sctab);
		if (isupper(ch)) {
			ch = tolower(ch);
			size = LONG;
		}
		if (ch == '\0')
			return(-1);
		if (_innum(ptr, ch, len, size, stream, &fileended, _sctab) && ptr)
			nmatch++;
		if (fileended)
			return(nmatch? nmatch: -1);
		break;

	case ' ':
	case '\n':
	case '\t': 
		while ((ch1 = NXGetc(stream))==' ' || ch1=='\t' || ch1=='\n')
			;
		if (ch1 != EOF)
			NXUngetc(stream);
		break;

	default: 
	def:
		ch1 = NXGetc(stream);
		if (ch1 != ch) {
			if (ch1==EOF)
				return(-1);
			NXUngetc(stream);
			return(nmatch);
		}
	}
}

static int
_innum(ptr, type, len, size, stream, eofptr, _sctab)
void *ptr;
int *eofptr;
NXStream *stream;
char *_sctab;
int type, len, size;
{
	register char *np;
	char numbuf[64];
	register c, base;
	int expseen, scale, negflg, c1, ndigit;
	long lcval;

	if (type=='c' || type=='s' || type=='[')
		return(_instr(ptr? (char *)ptr: (char *)NULL, type, len, stream, eofptr, _sctab));
	lcval = 0;
	ndigit = 0;
	scale = INT;
	if (type=='e'||type=='f')
		scale = FLOAT;
	base = 10;
	if (type=='o')
		base = 8;
	else if (type=='x')
		base = 16;
	np = numbuf;
	expseen = 0;
	negflg = 0;
	while ((c = NXGetc(stream))==' ' || c=='\t' || c=='\n');
	if (c=='-') {
		negflg++;
		*np++ = c;
		c = NXGetc(stream);
		len--;
	} else if (c=='+') {
		len--;
		c = NXGetc(stream);
	}
	for ( ; --len>=0; *np++ = c, c = NXGetc(stream)) {
		if (   isdigit(c)
		    || (base==16 && (('a'<=c && c<='f') || ('A'<=c && c<='F'))))
		{
			ndigit++;
			if (base==8)
				lcval <<=3;
			else if (base==10)
				lcval = ((lcval<<2) + lcval)<<1;
			else
				lcval <<= 4;
			c1 = c;
			if (isdigit(c))
				c -= '0';
			else if ('a'<=c && c<='f')
				c -= 'a'-10;
			else
				c -= 'A'-10;
			lcval += c;
			c = c1;
			continue;
		} else if (c=='.') {
			if (base!=10 || scale==INT)
				break;
			ndigit++;
			continue;
		} else if ((c=='e'||c=='E') && expseen==0) {
			if (base!=10 || scale==INT || ndigit==0)
				break;
			expseen++;
			*np++ = c;
			c = NXGetc(stream);
			if (c!='+'&&c!='-'&&('0'>c||c>'9'))
				break;
		} else
			break;
	}
	if (negflg)
		lcval = -lcval;
	if (c != EOF) {
		NXUngetc(stream);
		*eofptr = 0;
	} else
		*eofptr = 1;
 	if (ptr==NULL || np==numbuf || (negflg && np==numbuf+1) )/* gene dykes*/
		return(0);
	*np++ = 0;
	switch((scale<<4) | size) {

	case (FLOAT<<4) | SHORT:
	case (FLOAT<<4) | REGULAR:
		*(float *)ptr = atof(numbuf);
		break;

	case (FLOAT<<4) | LONG:
		*(double *)ptr = atof(numbuf);
		break;

	case (INT<<4) | SHORT:
		*(short *)ptr = lcval;
		break;

	case (INT<<4) | REGULAR:
		*(int *)ptr = lcval;
		break;

	case (INT<<4) | LONG:
		*(long *)ptr = lcval;
		break;
	}
	return(1);
}

static int
_instr(ptr, type, len, stream, eofptr, _sctab)
register char *ptr;
register NXStream *stream;
int *eofptr;
char *_sctab;
int type, len;
{
	register ch;
	register char *optr;
	int ignstp;

	*eofptr = 0;
	optr = ptr;
	if (type=='c' && len==30000)
		len = 1;
	ignstp = 0;
	if (type=='s')
		ignstp = SPC;
	while ((ch = NXGetc(stream)) != EOF && _sctab[ch] & ignstp)
		;
	ignstp = SPC;
	if (type=='c')
		ignstp = 0;
	else if (type=='[')
		ignstp = STP;
	while (ch!=EOF && (_sctab[ch]&ignstp)==0) {
		if (ptr)
			*ptr++ = ch;
		if (--len <= 0)
			break;
		ch = NXGetc(stream);
	}
	if (ch != EOF) {
		if (len > 0)
			NXUngetc(stream);
		*eofptr = 0;
	} else
		*eofptr = 1;
	if (ptr && ptr!=optr) {
		if (type!='c')
			*ptr++ = '\0';
		return(1);
	}
	return(0);
}

static char *
_getccl(s, _sctab)
register unsigned char *s;
char *_sctab;
{
	register c, t;

	t = 0;
	if (*s == '^') {
		t++;
		s++;
	}
	for (c = 0; c < SCTAB_SIZE; c++)
		if (t)
			_sctab[c] &= ~STP;
		else
			_sctab[c] |= STP;
	if ((c = *s) == ']' || c == '-') {	/* first char is special */
		if (t)
			_sctab[c] |= STP;
		else
			_sctab[c] &= ~STP;
		s++;
	}
	while ((c = *s++) != ']') {
		if (c==0)
			return((char *)--s);
		else if (c == '-' && *s != ']' && s[-2] < *s) {
			for (c = s[-2] + 1; c < *s; c++)
				if (t)
					_sctab[c] |= STP;
				else
					_sctab[c] &= ~STP;
		} else if (t)
			_sctab[c] |= STP;
		else
			_sctab[c] &= ~STP;
	}
	return((char *)s);
}
