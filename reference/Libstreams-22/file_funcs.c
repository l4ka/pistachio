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
 *	File:	file_funcs.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Streams implemented as file operations.
 */

#include "defs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mach/mach_init.h>
#include <errno.h>

//extern int write( int fd, void *buf, int size );
//extern int read( int fd, void *buf, int size );

static int file_flush(register NXStream *s);
static int file_fill(register NXStream *s);
static void file_change(register NXStream *s);
static void file_seek(register NXStream *s, register long offset);
static void file_close(register NXStream *s);

static const struct stream_functions file_functions = {
    NXDefaultRead,
    NXDefaultWrite,
    file_flush,
    file_fill,
    file_change,
    file_seek,
    file_close
};

struct file_info {
    int fd;		/* file descriptor */
};

/*
 *	file_flush:  flush a stream buffer.
 */

static int file_flush(register NXStream *s)
{
    register struct file_info *fi;

    if (s->buf_size == s->buf_left)
	return 0;
    fi = (struct file_info *) s->info;
    return write(fi->fd, s->buf_base, s->buf_size - s->buf_left);
}

/*
 *	file_fill:  fill a stream buffer, return how much
 *	we filled.
 */

static int file_fill(register NXStream *s)
{
    register struct file_info *fi;

    fi = (struct file_info *) s->info;
    return read(fi->fd, s->buf_base, s->buf_size);
}

static void file_change(register NXStream *s)
{
    register int    curPos = NXTell(s);
    register struct file_info *fi;

    fi = (struct file_info *) s->info;
    if (s->flags & NX_READFLAG) {
	s->offset = curPos;
	s->buf_ptr = s->buf_base;
	s->buf_left = s->buf_size;
	if ((s->flags & NX_CANSEEK) && lseek(fi->fd, curPos, L_SET) < 0)
	    NX_RAISE(NX_illegalSeek, s, (void *)errno);
    } else {
	if (s->buf_size != s->buf_left)
	    (void)NXFlush(s);
	s->buf_left = 0;
    }
}


static void file_seek(register NXStream *s, register long offset)
{
    register struct file_info *fi;
    register long curPos = NXTell(s);

    if (curPos == offset)
	return;

    /*
     * Don't seek on pipes or sockets.
     */
    if (!(s->flags & NX_CANSEEK))
	return;

    fi = (struct file_info *) s->info;
    if (s->flags & NX_WRITEFLAG) {
	if (s->buf_size != s->buf_left)
	    (void)NXFlush(s);
	if( lseek(fi->fd, offset, 0) < 0)
	    NX_RAISE(NX_illegalSeek, s, (void *)errno);
	s->offset = offset;
	s->buf_ptr = s->buf_base;
	s->buf_left = s->buf_size;
    } else {
	int lastRead;
	long pageptr, pageoffset;

	lastRead = (s->buf_ptr - s->buf_base) + s->buf_left;
	if (offset >= s->offset && offset <= (s->offset + lastRead)) {
	    int delta = offset - s->offset;

	    s->buf_ptr = s->buf_base + delta;
	    s->buf_left = lastRead - delta;
	} else {
	    pageoffset = offset % vm_page_size;
	    pageptr = offset - pageoffset;
	    if( lseek(fi->fd, pageptr, 0) < 0)
		NX_RAISE(NX_illegalSeek, s, (void *)errno);
	    s->buf_left = s->functions->fill_buffer(s);
	    s->offset = pageptr;
	    s->buf_ptr = s->buf_base + pageoffset;
	    s->buf_left -= pageoffset;
	}
    }
}


static void file_close(register NXStream *s)
{
}

/*
 *	NXOpenFile:
 *
 *	open a stream using a file descriptor.
 */

NXStream *NXOpenFile(int fd, int mode)
{
    NXStream       *s;
    struct file_info *fi;
    struct stat     statInfo;

    if (fstat(fd, &statInfo) == -1)
	return NULL;
    s = NXStreamCreate(mode, TRUE);
    s->functions = &file_functions;
    fi = (struct file_info *) malloc(sizeof(struct file_info));
    fi->fd = fd;
    s->info = (char *)fi;
    s->eof = statInfo.st_size;
    
    /*
     * Stream is only seekable if it's not a socket or a pipe.
     */
    if (   (statInfo.st_mode&S_IFMT) != S_IFSOCK
	&& (statInfo.st_mode&S_IFMT) != S_IFIFO)
	s->flags |= NX_CANSEEK;
    return s;
}
