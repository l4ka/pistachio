/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/string.cc
 * Description:	String manipulation functions, and memory copy functions.
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
 * $Id: string.cc,v 1.7 2003/09/24 19:05:30 skoglund Exp $
 *
 ***************************************************************************/

#include INC_ARCH(string.h)
#include INC_ARCH(cache.h)

/****************************************************************************/

void hex( word_t num, char str[] )
{
	word_t i;
	static char hex_chars[] = "0123456789abcdef";

	for( i = 0; i < sizeof(word_t)*2; i++ )
		str[sizeof(word_t)*2-1-i] = hex_chars[ (num >> (i*4)) & 0xf ];
	str[sizeof(word_t)*2] = '\0';
}

/****************************************************************************/

void zero_block( word_t *dst, word_t size )
    /*  Zeros a block of memory.  The block must be cache line aligned.
     *  And size must be a multiple of the cache line size.
     */
{
    word_t i;

    for( i = 0; i < size; i += POWERPC_CACHE_LINE_SIZE )
	ppc_cache_zero_block( (word_t)dst + i );
}

void memcpy_cache_flush( word_t *dst, const word_t *src, word_t size )
	/*  dst and src must be aligned by sizeof(word_t).  size must be
	 *  a multiple of sizeof(word_t).
	 */
{
	word_t cnt;
	int line_words = POWERPC_CACHE_LINE_SIZE / sizeof(word_t);

	for( cnt = 0; cnt < size/sizeof(word_t); cnt++ ) {
		dst[cnt] = src[cnt];
		if( cnt && ((cnt % line_words) == 0) )
			/*  We just started a new cache line, so flush the
			 *  prior cache line.
			 */
			cache_partial_code_sync( (word_t)&dst[cnt-1] );
	}
	cache_partial_code_sync( (word_t)&dst[cnt-1] );
	cache_complete_code_sync();
}


void *memcpy_aligned( word_t *dst, const word_t *src, word_t size )
	/* dst and src must be aligned by sizeof(word_t).  size must
	 * be a multiple of sizeof(word_t).
	 */
{
	word_t cnt;

	for( cnt = 0; cnt < size/sizeof(word_t); cnt++ )
		dst[cnt] = src[cnt];
	return dst;
}

void *memcpy( void *dst, const void *src, word_t size )
{
	word_t cnt;

	if( !((word_t)dst % sizeof(word_t)) && 
			!((word_t)src % sizeof(word_t)) &&
			!(size % sizeof(word_t)) )
		return memcpy_aligned( (word_t *)dst, (word_t *)src, size );

	for( cnt = 0; cnt < size; cnt++ )
		((u8_t *)dst)[cnt] = ((u8_t *)src)[cnt];
	return dst;
}

void sstrncat( char *dst, const char *src, word_t n )
{
	word_t dst_i = strlen(dst);
	word_t src_i;

	src_i = 0;
	while( (dst_i < n) && src[src_i] )
		dst[ dst_i++ ] = src[ src_i++ ];

	if( dst_i >= n )
		dst_i = n-1;
	dst[ dst_i ] = '\0';
}

void sstrncpy( char *dst, const char *src, word_t n )
{
	word_t cnt;

	cnt = 0;
	while( (cnt < n) && src[cnt] ) {
		dst[cnt] = src[cnt];
		cnt++;
	}

	if( cnt >= n )
		cnt = n-1;
	dst[ cnt ] = '\0';
}

word_t strlen( const char *src )
{
	word_t cnt = 0;

	while( src[cnt] )
		cnt++;
	return cnt;
}

int strcmp( const char *s1, const char *s2 )
{
	while( 1 ) {
		if( !*s1 && !*s2 )
			return 0;
		if( (!*s1 && *s2) || (*s1 < *s2) )
			return -1;
		if( (*s1 && !*s2) || (*s1 > *s2) )
			return 1;
		s1++;
		s2++;
	}
}

int strncmp( const char *s1, const char *s2, word_t len )
{
	while( len > 0 ) {
		if( !*s1 && !*s2 )
			return 0;
		if( (!*s1 && *s2) || (*s1 < *s2) )
			return -1;
		if( (*s1 && !*s2) || (*s1 > *s2) )
			return 1;
		s1++;
		s2++;
		len--;
	}
	return 0;
}

