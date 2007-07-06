/****************************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/string.h
 * Description:	String and memory manipulation functions.  Why are these
 * 		necessary?  Other platforms don't seem to need them.
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: string.h,v 1.5 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__STRING_H__
#define __ARCH__POWERPC__STRING_H__

extern "C" void *memcpy( void *dst, const void *src, word_t size );

extern void hex( word_t num, char str[] );

extern void zero_block( word_t *dst, word_t size );
extern void memcpy_cache_flush( word_t *dst, const word_t *src, word_t size );
extern void *memcpy_aligned( word_t *dst, const word_t *src, word_t size );

extern void sstrncat( char *dst, const char *src, word_t n );
extern void sstrncpy( char *dst, const char *src, word_t n );
extern word_t strlen( const char *src );
extern int strcmp( const char *s1, const char *s2 );
extern int strncmp( const char *s1, const char *s2, word_t len );

#endif	/* __ARCH__POWERPC__STRING_H__ */
