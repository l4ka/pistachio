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
 *	streamsextra.h
 *
 *	Private memory stream like implemenation
 *
 */

#import "streams.h"

 /*
  * NXOpenSmallMemory: 
  *
  * This is a cheaper version of NXOpenMemory(NULL, 0, someMode). This is
  * especially effiencient for small temporary buffers. 
  *
  * mode is one of NX_WRITEONLY or NX_READWRITE. NXSaveToFile, and
  * NXGetMemoryBuffer work compatibly with these streams. To close this
  * stream its best to use NXClose, rather that NXCloseMemory. If you use
  * NXCloseMemory, the only compatible option is NX_FREEBUFFER, because there
  * is no really to know how to free the buffer. 
  *
  */

extern NXStream *NXOpenSmallMemory(int mode);

 /*
  * NXGetStreamOnSectionForBestArchitecture:
  *
  * opens a read-only memory stream with the contents of the named section
  * within the file.  If only one architecture appears in the file, then that
  * architecture's stream is returned.  If more than one architecture appears
  * in the file, but not the native architecture, then the last architecture
  * in the file is returned.  When NXClose is called on a memory stream, the
  * internal buffer is not freed.  Use NXGetMemoryBuffer to get the internal
  * buffer and use vm_deallocate to free it.
  */
extern NXStream *NXGetStreamOnSectionForBestArchitecture(const char *fileName, const char *segmentName, const char *sectionName);

