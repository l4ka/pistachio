/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, 2008,  Karlsruhe University
 *                
 * File path:     lib.h
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
 * $Id: lib.h,v 1.7 2006/02/21 10:26:39 stoess Exp $
 *                
 ********************************************************************/

#include <l4/types.h>


#if defined(__cplusplus)
template<typename T> inline const T& min(const T& a, const T& b)
{
    if (b < a)
        return b;
    return a;
}

template<typename T> inline const T& max(const T& a, const T& b)
{
    if (a < b)
        return b;
    return a;
}
#endif /* defined(__cplusplus) */

/*
 * We declare functions weak to allow architecture-specific 
 * implementations to take precedence.
 */
extern "C" unsigned strlen( const char *src ) __attribute__((weak));
extern "C" void strcpy( char *dst, const char *src ) __attribute__((weak));
extern "C" int strcmp( const char *s1, const char *s2 ) __attribute__((weak));
extern "C" int strncmp( const char *s1, const char *s2, unsigned int n ) __attribute__((weak));
extern "C" char *strstr(const char *s, const char *find) __attribute__((weak));
extern "C" unsigned long strtoul(const char *cp, char **endp, int base) __attribute__((weak));
extern "C" char *strchr(const char *p, int ch) __attribute__((weak));
extern "C" void memcopy(L4_Word_t dst, L4_Word_t src, L4_Word_t len) __attribute__((weak));
extern "C" void memset(L4_Word_t dst, L4_Word8_t val, L4_Word_t len) __attribute__((weak));

extern inline void memcopy(void *dst, void *src, L4_Word_t len)
{
    memcopy( L4_Word_t(dst), L4_Word_t(src), len );
}

extern inline bool is_intersection( 
	L4_Word_t start1, L4_Word_t end1, 
	L4_Word_t start2, L4_Word_t end2 
	)
{
    return ((start1 >= start2) && (start1 <= end2))
	|| ((end1 >= start2) && (end1 <= end2))
	|| ((start1 <= start2) && (end1 >= end2));
}

#define MB(x)			(x*1024*1024)
#define ROUND_DOWN(x, size)	(x & ~(size-1))
#define ROUND_UP(x, size)	(ROUND_DOWN(x, size) == x ? x : ROUND_DOWN(x, size) + size)
