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
 *	File:	streams.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	High-level streams routines.  These routines are exported to
 *	the user.
 */

#include "defs.h"
#include <stdio.h>

#ifdef SHLIB		/* since we use the "close" field of the stream */
#undef close		/* struct, we must undo this shilb macro */
#endif

/*
 *	NXPrintf: printf into a stream.
 */

void
NXPrintf(NXStream *stream, const char *format, ...)
{
    va_list ap;

    _NXVerifyStream(stream);
    va_start(ap, format);
    NXVPrintf(stream, format, ap);
    va_end(ap);
}

int NXScanf(NXStream *stream, const char *format, ...)
{
    va_list ap;
    int	ret;

    _NXVerifyStream(stream);
    va_start(ap, format);
    ret = NXVScanf(stream, format, ap);
    va_end(ap);
    return ret;
}

/*
 *	NXClose:
 *
 *	close the specified stream.
 */

void NXClose(register NXStream *s)
{
    _NXVerifyStream(s);
    if (s->flags & NX_WRITEFLAG)
	(void)NXFlush(s);
    s->functions->destroy(s);
    NXStreamDestroy(s);
}


long NXTell(register NXStream *s)
{
    _NXVerifyStream(s);
    if (s->flags & NX_READFLAG) {
	return s->offset + (s->buf_ptr - s->buf_base);
    } else if (s->flags & NX_WRITEFLAG) {
	return s->offset + s->buf_size - s->buf_left;
    } else {
	fprintf(stderr, "Stream is neither readable nor writable in NXTell\n");
	return -1;
    }
}


void NXSeek(register NXStream *s, long offset, int ptrname)
{
    long curPos;

    _NXVerifyStream(s);
    curPos = NXTell(s);
    if (curPos > s->eof)
	s->eof = curPos;
    switch (ptrname) {
        case NX_FROMSTART:
	    break;
	case NX_FROMCURRENT:
	    offset += NXTell(s);
	    break;
	case NX_FROMEND:
	    offset += s->eof;
	    break;
	default:
	    NX_RAISE( NX_illegalSeek, s, 0 );
    }
    if (   offset < 0
//	|| !(s->flags&NX_CANSEEK)	*** This breaks too much code ***
	|| (   (s->flags & NX_READFLAG)
	    && !(s->flags & NX_CANWRITE) 
	    && (offset > s->eof)) )
	NX_RAISE( NX_illegalSeek, s, 0 );
    if (s->flags & NX_EOS && offset < s->eof)
	s->flags &= ~NX_EOS;
    else if (offset >= s->eof)
	s->flags |= NX_EOS;
    s->functions->seek(s, offset);
}


void NXUngetc(register NXStream *s)
{
    _NXVerifyStream(s);
    if (NXTell(s) > 0) {
	if (s->buf_ptr == s->buf_base)
	    fprintf(stderr, "Assertion failed: NXUngetc: last character read unknown\n");
	else if (!(s->flags & NX_EOS)) {
	    s->buf_left++;
	    s->buf_ptr--;
	}
    }
}
