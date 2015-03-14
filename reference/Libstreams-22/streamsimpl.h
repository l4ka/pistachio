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
/*
 *	Stream implementation data structure definitions.
 *
 *	These definitions are not necessary if you only want to use
 *	the streams package. You will need these definitions if you 
 *	implement a stream.
 *
 */
 
#import "streams.h"
#import <objc/zone.h>

#ifndef STREAMS_IMPL_H
#define STREAMS_IMPL_H

#define NX_DEFAULTBUFSIZE	(16 * 1024)

/*
 *	Procedure declarations used in implementing streams.
 */

extern NXStream *NXStreamCreate(int mode, int createBuf);
extern NXStream *NXStreamCreateFromZone(int mode, int createBuf, NXZone *zone);
extern void NXStreamDestroy(NXStream *stream);
extern void NXChangeBuffer(NXStream *stream);
extern int NXFill(NXStream *stream);
    /* NXFill should only be called when the buffer is empty */
    
extern int NXDefaultWrite(NXStream *stream, const void *buf, int count);
extern int NXDefaultRead(NXStream *stream, void *buf, int count);

#endif


