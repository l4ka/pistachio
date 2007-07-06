/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/types.h
 * Description:   types for Compatibility Mode
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
 * $Id: types.h,v 1.2 2006/10/20 16:18:38 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_AMD64__IA32__TYPES_H__
#define __GLUE__V4_AMD64__IA32__TYPES_H__

#include INC_API(types.h)

#undef TIME_BITS_WORD
#define TIME_BITS_WORD 32

namespace ia32 {

	typedef u32_t word_t;
	typedef word_t addr_t;

#undef __API__V4__TYPES_H__
#include INC_API(types.h)

}

INLINE ia32::time_t time_32(time_t t)
{
    ia32::time_t r;
    r.set_raw(t.raw);
    return r;
}

INLINE time_t time(ia32::time_t t)
{
    time_t r;
    r.set_raw(t.raw);
    return r;
}

INLINE ia32::timeout_t timeout_32(timeout_t t)
{
    ia32::timeout_t r;
    r.set_raw(t.raw);
    return r;
}

INLINE timeout_t timeout(ia32::timeout_t t)
{
    timeout_t r;
    r.set_raw(t.raw);
    return r;
}

#undef TIME_BITS_WORD

#endif /* !__GLUE__V4_AMD64__IA32__TYPES_H__ */
