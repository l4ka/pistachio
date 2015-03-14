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
	File:	ipc_funcs.c	
	Author:	Trey Matteson
	
	Copyright (c) 1988 NeXT, Inc. as an unpublished work.
	All rights reserved.
 
 	Streams implemented as IPC operations.
*/

#include "defs.h"
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/message.h>

/* ??? tune these */
#define BUFSIZE	 (4*1024 - 8)		

static int ipc_flush(register NXStream *s);
static int ipc_fill(register NXStream *s);
static void ipc_change(register NXStream *s);
static void ipc_seek(register NXStream *s, register long offset);
static void ipc_close(register NXStream *s);

static const struct stream_functions ipc_functions = {
    NXDefaultRead,
    NXDefaultWrite,
    ipc_flush,
    ipc_fill,
    ipc_change,
    ipc_seek,
    ipc_close,
};

typedef struct {
    mach_msg_header_t header;
#ifdef notyet
    msg_type_long_t type;
    unsigned char data[BUFSIZE];
#endif 
} InlineMsg;


/*
 *	ipc_flush:  flush a stream buffer.
 */
static int ipc_flush(register NXStream *s)
{
    InlineMsg *msg;
    kern_return_t ret;
    int flushSize;

    flushSize = s->buf_size - s->buf_left;
    if (flushSize) {
	msg = (InlineMsg *)s->info;
#ifdef notyet
	msg->header.msgh_size = flushSize + sizeof(mach_msg_header_t) 
		+ sizeof(msg_type_long_t);
#else
	msg->header.msgh_size = flushSize + sizeof(mach_msg_header_t) ;
#endif
#ifdef notyet
	msg->type.msg_type_long_number = flushSize;
#endif
	ret = mach_msg_send( (mach_msg_header_t *)msg);
	if(ret == MACH_MSG_SUCCESS) {
	    s->buf_ptr = s->buf_base;
	    s->buf_left = s->buf_size;
	    s->offset += flushSize;
	    return flushSize;
	} else
	    return -1;
    } else
	return 0;
}


/*
 *	ipc_fill:  fill a stream buffer, return how much
 *	we filled.
 */
static int ipc_fill(register NXStream *s)
{
    InlineMsg *msg;
    kern_return_t ret;

    msg = (InlineMsg *)s->info;
    msg->header.msgh_size = sizeof(InlineMsg);
    ret = mach_msg_receive((mach_msg_header_t *)msg);

    if (ret != KERN_SUCCESS)
	return -1;
    else
#ifdef notyet
	return msg->type.msg_type_long_number;
#else
	return -1;
#endif
}


static void ipc_change(register NXStream *s)
{
    /* NOP for IPC streams */
}


static void ipc_seek(register NXStream *s, register long offset)
{
    /* NOP for IPC streams */
}


/*
 *	ipc_close:	shut down an ipc stream.
 *
 *	Send a zero length packet to signify end.
 */
static void ipc_close(register NXStream *s)
{
    InlineMsg *msg;

    msg = (InlineMsg *)s->info;
#ifdef notyet
    msg->header.msgh_size = sizeof(mach_msg_header_t) + sizeof(msg_type_long_t);
#else
    msg->header.msgh_size = sizeof(mach_msg_header_t);
#endif
#ifdef notyet
    msg->type.msg_type_long_number = 0;
#endif
    (void)mach_msg_send( (mach_msg_header_t *)msg);
}


/*
 *	NXOpenPort:
 *
 *	open a stream using a ipc descriptor.
 */
NXStream *
NXOpenPort(mach_port_t port, int mode)
{
#ifdef notyet
    NXStream *s;
    InlineMsg *msg;

/*??? set the flags */
    if (mode == NX_READWRITE)
	return NULL;
    s = NXStreamCreate(mode, FALSE);
    s->functions = &ipc_functions;
    msg = (InlineMsg *)malloc(sizeof(InlineMsg));
    s->info = (char *)msg;
    if (mode == NX_READONLY) {
	msg->header.msg_local_port = port;
	s->buf_left = 0;
    } else {
	msg->header.msg_simple = TRUE;
	msg->header.msg_type = MSG_TYPE_NORMAL;
	msg->header.msg_local_port = PORT_NULL;
	msg->header.msg_remote_port = port;
	msg->type.msg_type_header.msg_type_inline = TRUE;
	msg->type.msg_type_header.msg_type_longform = TRUE;
	msg->type.msg_type_header.msg_type_deallocate = FALSE;
	msg->type.msg_type_long_name = MSG_TYPE_CHAR;
	msg->type.msg_type_long_size = 8;
	s->buf_left = BUFSIZE;
    }
    s->buf_base = s->buf_ptr = msg->data;
    s->buf_size = BUFSIZE;
    s->flags &= ~NX_CANSEEK;
    return s;
#else
	return NULL;
#endif
}

