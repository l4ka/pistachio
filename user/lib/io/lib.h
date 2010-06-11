/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      lib.h
 * Author:        Jan Stoess <stoess@kit.edu>
 * Description:   
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
 ********************************************************************/
#ifndef __USER__LIB__IO__LIB_H__
#define __USER__LIB__IO__LIB_H__


static unsigned strlen( const char *src )
{
    unsigned cnt = 0;

    while( src && src[cnt] )
	cnt++;
    return cnt;
}

static void strcpy( char *dst, const char *src )
{
    unsigned cnt = 0;
   
    if( !dst || !src )
	return;

    do {
	dst[cnt] = src[cnt];
    } while( src[cnt++] );
    
}
/*
 * For the remainder of this file:
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 */

/*
 * Compare strings.
 */
static int
strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

static int
strncmp(const char *s1, const char *s2, unsigned int n)
{
	if (n == 0)
		return (0);
	do {
		if (*s1 != *s2++)
			return (*(unsigned char *)s1 - *(unsigned char *)--s2);
		if (*s1++ == 0)
			break;
	} while (--n != 0);
	return (0);
}

/*
 * Find the first occurrence of find in s.
 */
static char *
strstr(const char *s, const char *find)
{
	char c, sc;
	int len;

	if ((c = *find++) != 0) {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (0);
			} while (sc != c);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

static char *
strchr(const char *p, int ch)
{
    for (;; ++p) {
	if (*p == ch) {
	    /* LINTED const cast-away */
	    return((char *)p);
	}
	if (!*p)
	    return((char *)0);
    }
    /* NOTREACHED */
}

#endif /* !__USER__LIB__IO__LIB_H__ */
