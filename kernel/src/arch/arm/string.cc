/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:      arch/arm/string.cc
 * Description:    ARM optimised string operations
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
 * $Id: string.cc,v 1.1 2004/12/09 01:31:22 cvansch Exp $
 *
 ********************************************************************/

#include <debug.h>

extern "C" void * memcpy (void * dst, const void * src, unsigned int len)
{
    if (EXPECT_FALSE( ((word_t)dst & 3) || ((word_t)src & 3) || (len & 3)))
    {
	u8_t *d = (u8_t *) dst;
	u8_t *s = (u8_t *) src;

	while (len-- > 0)
	    *d++ = *s++;
    } else {
	u32_t *d = (u32_t *) dst;
	u32_t *s = (u32_t *) src;
	len = len / 4;

	while (len-- > 0)
	    *d++ = *s++;
    }
    return dst;
}

extern "C" void * memset (void * dst, unsigned int c, unsigned int len)
{
    u8_t *s = (u8_t *) dst;

    while (len-- > 0)
	*s++ = c;

    return dst;
}

