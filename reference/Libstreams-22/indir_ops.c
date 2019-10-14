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
 *	File:	indir_ops.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Stream indirect operations.
 */

#include "defs.h"

/*
 *	Flush the specified buffer.
 */
int NXFlush(register NXStream *stream)
{
    int flushSize;
    int retval;

    _NXVerifyStream(stream);
    flushSize = stream->buf_size - stream->buf_left;
    retval = stream->functions->flush_buffer(stream);
    if (retval < 0)
	NX_RAISE(NX_illegalWrite, stream, 0);
    if (!(stream->flags & NX_NOBUF)) {
	stream->buf_ptr = stream->buf_base;
	stream->buf_left = stream->buf_size;
	stream->offset += flushSize;
    }
    return retval;
}

/*
 *	Fill the specified buffer. Returns number of characters read.
 */

int NXFill(register NXStream *stream)
{
    volatile int    actual = 0;		/* initted for clean -Wall */
    volatile int    lastRead;

    _NXVerifyStream(stream);
    lastRead = (stream->buf_ptr - stream->buf_base) + stream->buf_left;
    NX_DURING {
	actual = stream->functions->fill_buffer(stream);
	if (actual > 0) {
	    stream->buf_ptr = stream->buf_base;
	    stream->offset += lastRead;
	    stream->buf_left = actual;
	} else {
	    stream->buf_ptr = stream->buf_base + lastRead;
	    stream->buf_left = 0;
	}
    } NX_HANDLER {
	stream->buf_ptr = stream->buf_base + lastRead;
	stream->buf_left = 0;
	NX_RERAISE();
    } NX_ENDHANDLER
    if (actual < 0)
	NX_RAISE(NX_illegalRead, stream, 0);
    return actual;
}


void NXChangeBuffer(register NXStream *s)
{
    int reading = (s->flags & NX_READFLAG);
    long curPos;

    _NXVerifyStream(s);
    if (reading ? !(s->flags & NX_CANWRITE) : !(s->flags & NX_CANREAD) )
	NX_RAISE( NX_illegalStream, s, 0 );
    s->functions->change_buffer(s);
    if (reading) {
	s->flags &= ~NX_READFLAG;
	s->flags |= NX_WRITEFLAG;
    } else {
	curPos = s->offset + (s->buf_ptr - s->buf_base);
	if (curPos > s->eof)
	    s->eof = curPos;
	s->flags &= ~NX_WRITEFLAG;
	s->flags |= NX_READFLAG;
    }
}

