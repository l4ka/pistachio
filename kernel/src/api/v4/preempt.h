/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     api/v4/preempt.h
 * Description:   Preemption flags
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
 * $Id: preempt.h,v 1.2 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__PREEMPT_H__
#define __API__V4__PREEMPT_H__

class preempt_flags_t
{
public:
    union {
	u8_t raw;
	struct {
	    BITFIELD4 (u8_t,
		       : 5,
		       signaled	: 1,
		       delayed	: 1,
		       pending	: 1);
	} flags;
    };

    bool is_signaled()	{ return flags.signaled; }
    bool is_delayed()	{ return flags.delayed; }
    bool is_pending()	{ return flags.pending; }
    preempt_flags_t set_pending() 
	{ 
	    flags.pending = true; 
	    return (*this); 
	}
    preempt_flags_t clear_pending()
	{
	    flags.pending = false;
	    return (*this);
	}
} __attribute__((packed));

#endif /* !__API__V4__PREEMPT_H__ */
