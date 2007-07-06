/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     lib.cc
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
 * $Id: lib.cc,v 1.7 2004/04/26 19:47:42 stoess Exp $
 *                
 ********************************************************************/
#include "lib.h"

extern "C" unsigned strlen( const char *src )
{
    unsigned cnt = 0;

    while( src && src[cnt] )
	cnt++;
    return cnt;
}

extern "C" void strcpy( char *dst, const char *src )
{
    unsigned cnt = 0;
   
    if( !dst || !src )
	return;

    do {
	dst[cnt] = src[cnt];
    } while( src[cnt++] );
    
}

/**
 * Copy a block of memory
 *
 * @param dst   destination address
 * @param src   source address
 * @param len   length of memory block in bytes
 *
 * This function copies a block of memory byte-by-byte from the source
 * address to the destination address. The function does not check for
 * overlapping of source and destination region.
 */
extern "C" void memcopy(L4_Word_t dst, L4_Word_t src, L4_Word_t len)
{
    L4_Word8_t* s = (L4_Word8_t*) src;
    L4_Word8_t* d = (L4_Word8_t*) dst;
    
    while (len--)
        *d++ = *s++;
}


/**
 * Fill memory with a constant byte
 *
 * @param dst   destination address
 * @param val   byte to write to memory block
 * @param len   length of memory block in bytes
 *
 * The memset() function fills the first len bytes of the memory
 * block pointed to by dst with the constant byte val.
 */
extern "C" void memset(L4_Word_t dst, L4_Word8_t val, L4_Word_t len)
{
    L4_Word8_t* d = (L4_Word8_t*) dst;

    while (len--)
        *d++ = val;
}



/* Some ctype like stuff to support strtoul */
#define isspace(c)      ((c == ' ') || (c == '\t'))
#define ULONG_MAX       (~0UL)
#define isdigit(c)      ((c) >= '0' && (c) <= '9')
#define islower(c)      (((c) >= 'a') && ((c) <= 'z'))
#define isupper(c)      (((c) >= 'A') && ((c) <= 'Z'))
#define isalpha(c)      (islower(c) || isupper(c))
/*
 * For the remainder of this file:
 *
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 */

/*
 * Compare strings.
 */
extern "C" int
strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == 0)
			return (0);
	return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

extern "C" int
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
extern "C" char *
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


/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
extern "C" unsigned long
strtoul(const char* nptr, char** endptr, int base)
{
	const char *s;
	unsigned long acc, cutoff;
	int c;
	int neg, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = ULONG_MAX / (unsigned long)base;
	cutlim = ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = (unsigned char) *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0)
			continue;
		if (acc > cutoff || acc == cutoff && c > cutlim) {
			any = -1;
			acc = ULONG_MAX;
		} else {
			any = 1;
			acc *= (unsigned long)base;
			acc += c;
		}
	}
	if (neg && any > 0)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}

extern "C" char *
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
