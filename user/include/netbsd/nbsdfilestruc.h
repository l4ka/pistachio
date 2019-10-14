/*	$NetBSD: stdio.h,v 1.80.2.1 2014/06/03 13:55:44 msaitoh Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)stdio.h	8.5 (Berkeley) 4/29/95
 */

//http://cvsweb.netbsd.org/bsdweb.cgi/~checkout~/src/include/stdio.h?rev=1.80.2.1&content-type=text/plain

typedef	struct __sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	unsigned short _flags;	/* flags, below; this FILE is free if 0 */
	short	_file;		/* fileno, if Unix descriptor, else -1 */
	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;	/* cookie passed to io functions */
	int	(*_close)(void *);
	int	(*_read) (void *, char *, int);
	__off_t	(*_seek) (void *, __off_t, int);
	int	(*_write)(void *, const char *, int);

	/* file extension */
	struct	__sbuf _ext;

	/* separate buffer for long sequences of ungetc() */
	unsigned char *_up;	/* saved _p when _p is doing ungetc data */
	int	_ur;		/* saved _r when _r is counting ungetc data */

	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];	/* guarantee a getc() buffer */

	/* Formerly used by fgetln/fgetwln; kept for binary compatibility */
	struct	__sbuf _lb__unused;

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
	__off_t	_offset;	/* current lseek offset */
} FILE;
