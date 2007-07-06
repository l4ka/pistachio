/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     api/v4/types.h
 * Description:   General type declarations for V4 API
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
 * $Id: types.h,v 1.26 2006/10/18 11:57:20 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__TYPES_H__
#define __API__V4__TYPES_H__

#include INC_API(config.h)

#if !defined(TIME_BITS_WORD)
#if defined(CONFIG_IS_64BIT)
#define TIME_BITS_WORD 64
#elif defined(CONFIG_IS_32BIT)
#define TIME_BITS_WORD 32
#endif
#endif /* !defined(TIME_BITS_WORD) */

/* time */
class time_t 
{
public:
    u64_t get_microseconds();
    
    static time_t never()
    {
	time_t ret;
	ret.raw = 0;
	return ret;
    }
    
    static time_t zero()
    {
	time_t ret;
	ret.time.mantissa = 0;
	ret.time.exponent = 1;
	ret.time.type = 0;
	return ret;
    }
    
    static time_t period(u16_t mantissa, u16_t exponent)
    {
	time_t ret;
	ret.time.mantissa = mantissa;
	ret.time.exponent = exponent;
	ret.time.type = 0;
	return ret;
    }
    
    static time_t point(u16_t mantissa, u16_t exponent)
    {
	time_t ret;
	ret.time.mantissa = mantissa;
	ret.time.exponent = exponent;
	ret.time.type = 1;
	return ret;
    }
    
    void set_raw(u16_t raw) { this->raw = raw; }

    bool is_never() { return raw == 0; }
    bool is_zero() { return zero().raw == raw; }
    bool is_period() { return time.type == 0; }
    bool is_point() { return time.type == 1; }

    bool operator< (time_t & r);

    union {
	u16_t raw;
	struct {
	    BITFIELD3(u16_t,
		mantissa	: 10,
		exponent	: 5,
		type		: 1);
	} __attribute__((packed)) time;
    } __attribute__((packed)); 
} __attribute__((packed));

INLINE u64_t time_t::get_microseconds()
{
    return (1 << time.exponent) * time.mantissa;
}


class timeout_t 
{
public:
    static timeout_t never() 
	{return (timeout_t){{raw: 0}};}

    inline time_t get_rcv() { return x.rcv_timeout; }
    inline time_t get_snd() { return x.snd_timeout; }
    inline void set_raw(word_t raw) { this->raw = raw; }
    inline bool is_never() { return this->raw == never().raw; }
public:
    union {
	struct {
#if TIME_BITS_WORD == 64
	    BITFIELD4( time_t,
		rcv_timeout,
		snd_timeout,
		_rv0,
		_rv1
	    );
#elif TIME_BITS_WORD == 32
	    BITFIELD2( time_t,
		rcv_timeout,
		snd_timeout
	    );
#endif
	} __attribute__((packed)) x;
	word_t raw;
    };
};


typedef u16_t cpuid_t;

#endif /* __API__V4__TYPES_H__ */
