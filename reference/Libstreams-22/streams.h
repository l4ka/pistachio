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
 *	File:	streams.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Header file definitions.
 */

#ifndef STREAMS_H
#define STREAMS_H

#import <stdarg.h>
#import <mach/port.h>

/* This structure may change size, and should not be included in client
   data structures.  Only use pointers to NXStreams.
 */
typedef struct _NXStream {
	unsigned int	magic_number;	/* to check stream validity */
	unsigned char  *buf_base;	/* data buffer */
	unsigned char  *buf_ptr;	/* current buffer pointer */
	int		buf_size;	/* size of buffer */
	int		buf_left;	/* # left till buffer operation */
	long		offset; 	/* position of beginning of buffer */
	int		flags;		/* info about stream */
	int		eof;
	const struct stream_functions
			*functions;	/* functions to implement stream */
	void		*info;		/* stream specific info */
} NXStream;


struct stream_functions {
	/*
	 * Called to read count bytes into buf.  If you arent doing any
	 * special buffer management, you can set this proc to be
	 * NXDefaultRead.  It should return how many bytes were read.
	 */
	int	(*read_bytes)(NXStream *s, void *buf, int count);

	/*
	 * Called to write count bytes from buf.  If you arent doing any
	 * special buffer management, you can set this proc to be
	 * NXDefaultWrite.  It should return how many bytes were written.
	 */
	int	(*write_bytes)(NXStream *s, const void *buf, int count);

	/*
	 * Called by the write routine to deal with a full output buffer.
	 * This routine should make space for more characters to be
	 * written.  NXDefaultWrite assumes this routine empties the
	 * buffer.  It should return how many bytes were written.
	 * If it returns -1, then a NX_illegalWrite exception will be raised.
	 */
	int	(*flush_buffer)(NXStream *s);

	/*
	 * Called by the read routine to deal with an empty input buffer.
	 * This routine should provide more characters to be read.
	 * NXDefaultRead assumes this routine adds new characters
	 * to the buffer.  It should return how many bytes were read.
	 * If it returns -1, then a NX_illegalRead exception will be raised.
	 */
	int	(*fill_buffer)(NXStream *s);

	/*
	 * Called to flip the mode of the stream between reading and writing.
	 * The current state can be found by masking the flags field with
	 * NX_READFLAG and NX_WRITEFLAG.  The caller will update the flags.
	 */
	void	(*change_buffer)(NXStream *s);

	/*
	 * Called to seek the stream to a certain position.  It must update
	 * any affected elements in the NXStream struct.
	 */
	void	(*seek)(NXStream *s, long offset);

	/*
	 * Called to free any resources used by the stream.  This proc
	 * should free the buffer if not allocated by NXStreamCreate.
	 */
	void	(*destroy)(NXStream *s);
};

/*
 *	Modes
 */

#define NX_READONLY		1	/* read on stream only */
#define NX_WRITEONLY		2	/* write on stream only */
#define NX_READWRITE		4	/* do read & write */

/*
 *	Seeking Modes - determines meaning of offset passed to NXSeek
 */

#define NX_FROMSTART		0	/* relative to start of file */
#define NX_FROMCURRENT		1	/* relative to current position */
#define NX_FROMEND		2	/* relative to end of file */

/*
 *	Private Flags and Private Routines
 */

#define NX_READFLAG	1		/* stream is for reading */
#define NX_WRITEFLAG	(1 << 1)	/* stream is for writing */
#define	NX_USER_OWNS_BUF	(1 << 2)	/* used by memory streams */
#define NX_EOS			(1 << 3)	/* end of stream detected */
#define NX_NOBUF		(1 << 4)	/* whether lib alloc'ed buf */
#define NX_CANWRITE		(1 << 5)
#define NX_CANREAD		(1 << 6)
#define NX_CANSEEK		(1 << 7)

/* private functions for NXGetc and NXPutc */
extern int _NXStreamFillBuffer(NXStream *s);
extern int _NXStreamFlushBuffer(NXStream *s, unsigned char c);
extern int _NXStreamChangeBuffer(NXStream *s, unsigned char ch);


/*
 *		Macro operations.
 */

#define NXPutc(s, c) \
    ( ((s)->flags & NX_WRITEFLAG) ? \
    ( --((s)->buf_left) >= 0 ? (*(s)->buf_ptr++ = (unsigned char)(c)) : \
	(_NXStreamFlushBuffer((s), (unsigned char)(c)), (unsigned char)0) ) : \
    _NXStreamChangeBuffer((s), (unsigned char)(c)) ) \

#define NXGetc(s)						\
    ( ((s)->flags & NX_READFLAG) ?				\
    ( --((s)->buf_left) >= 0 ? ((int)(*(s)->buf_ptr++)) :	\
	    _NXStreamFillBuffer((s)) ) :			\
    _NXStreamChangeBuffer((s), (unsigned char)(0)) )

#define NXAtEOS(s) (s->flags & NX_EOS)


/*
 *		Procedure declarations (exported).
 */

extern int NXFlush(NXStream *s);		
 /*
    flush the current contents of stream s.  Returns the number of chars
    written.
  */
  
extern void NXSeek(NXStream *s, long offset, int mode);		
 /*
    sets the current position of the stream.  Mode is either NX_FROMSTART,
    NX_FROMCURRENT, or NX_FROMEND.
  */
  
extern long NXTell(NXStream *s);		
 /*
    reports the current position of the stream.
  */
  
#define NXRead(s, buf, count)	\
		((*s->functions->read_bytes)((s), (buf), (count)))
 /*
    int NXRead(NXStream *s, void *buf, int count) 
	
    read count bytes starting at pointer buf from stream s.  Returns the
    number of bytes read.
  */
  
#define NXWrite(s, buf, count)		\
		((*s->functions->write_bytes)((s), (buf), (count)))
 /*
    int NXWrite(NXStream *s, const void *buf, int count) 
	
    write count bytes starting at pointer buf to stream s.  Returns the
    number of bytes written.
  */
  
extern void NXPrintf(NXStream *s, const char *format, ...);
extern void NXVPrintf(NXStream *s, const char *format, va_list argList);
 /*
    writes args according to format string to stream s.  The first routine
    takes its variable arguments on the stack, the second takes a va_list
    as defined in <stdarg.h>.
  */
  
extern int NXScanf(NXStream *s, const char *format, ...);
extern int NXVScanf(NXStream *s, const char *format, va_list argList);
 /*
    reads args from stream s according to format string.  The first routine
    takes its variable arguments on the stack, the second takes a va_list
    as defined in <varargs.h>.
  */

extern void NXUngetc(NXStream *s);		
 /*
    backs up one character previously read by getc.  Only a single character
    can be backed over between reads.
  */
  
extern void NXClose(NXStream *s);		
 /*
    closes stream s.
  */
  
extern NXStream *NXOpenFile(int fd, int mode);
 /*
    opens a file stream on file descriptor fd with access mode. mode
    can be NX_READONLY, NX_WRITEONLY or NX_READWRITE.
    The fd should have the same access rights as the mode passed in.
    When NXClose is called on a file stream, the fd is not closed.
  */

extern NXStream *NXOpenPort(mach_port_t port, int mode);
 /*
    opens a stream on a MACH port with access mode. If mode is NX_READONLY,
    messages will be received on port.  If mode is NX_WRITEONLY,
    messages will be sent to port.  mode can not be NX_READWRITE.
    These streams are not positionable, thus you cannot call NXSeek
    with a port stream.  
  */
	
extern NXStream *NXOpenMemory(const char *addr, int size, int mode);
 /*
    Opens a stream on memory with access mode. mode
    can be NX_READONLY, NX_WRITEONLY or NX_READWRITE
    When NXClose is called on a memory stream, the internal buffer is
    not freed.  Use NXGetMemoryBuffer to get the internal buffer and
    use vm_deallocate to free it.
  */

extern NXStream *NXMapFile(const char *name, int mode);
 /*
    opens a stream on memory with access mode, initializing the
    memory with the contents of a file. mode
    can be NX_READONLY, NX_WRITEONLY or NX_READWRITE
    When NXClose is called on a memory stream, the internal buffer is
    not freed.  Use NXGetMemoryBuffer to get the internal buffer and
    use vm_deallocate to free it. Use NXSaveToFile to write the 
    contents of the memory stream to a file.
  */

extern NXStream *NXGetStreamOnSection(const char *fileName, const char *segmentName, const char *sectionName);
 /*
    opens a read-only memory stream with the contents of the named section
    within the file.  When NXClose is called on a memory stream, the internal
    buffer is not freed.  Use NXGetMemoryBuffer to get the internal buffer and
    use vm_deallocate to free it.
  */

extern int NXSaveToFile(NXStream *s, const char *name);
 /*
    Write the contents of a memory stream to a file .
  */

extern void NXGetMemoryBuffer(NXStream *s, char **addr, int *len, int *maxlen);
 /*
    return the memory buffer, the current length, and the max length
    of the buffer. Use the maxlen value when you vm_deallocate.
  */

extern void NXCloseMemory(NXStream *s, int option);
 /*
    Closes a memory stream, with options NX_FREEBUFFER which frees the
    internal buffer, NX_TRUNCATEBUFFER which truncates to buffer to
    the size of the data, and NX_SAVEBUFFER which does nothing to the
    buffer. The stream is then closed and freed.
  */

#define NX_FREEBUFFER		0	/* constants for NXCloseMemory */
#define NX_TRUNCATEBUFFER	1
#define NX_SAVEBUFFER		2


typedef void NXPrintfProc(NXStream *stream, void *item, void *procData);
 /*
    A proc that is registered to format and output item on the given stream.
  */

extern void NXRegisterPrintfProc(char formatChar, NXPrintfProc *proc,
							void *procData);
 /*
    Registers a NXPrintProc for a format character used in the format
    string for NXPrintf or NXVPrintf.  If formatChar is used in a
    format string, proc will be called with the corresponding argument
    passed to NXPrintf.  The format characters in the string "vVwWyYzZ"
    are available for non-NeXT applications to use; the rest are reserved
    for future use by NeXT.  procData is for client data that will be
    blindly passed along to the proc.
  */

#define NX_STREAMERRBASE 2000	/* 1000 error codes for us start here */

/* Errors that stream routines raise.  When these exceptions are raised,
   the first data element is always the NXStream * being operated on,
   or zero if thats not applicable.  The second is any error code
   returned from the operating system.
 */
typedef enum _NXStreamErrors {
    NX_illegalWrite = NX_STREAMERRBASE,
    NX_illegalRead,
    NX_illegalSeek,
    NX_illegalStream,
    NX_streamVMError
} NXStreamErrors;

#endif
