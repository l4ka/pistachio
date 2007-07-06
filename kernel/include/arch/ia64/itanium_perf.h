/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/itanium_perf.h
 * Description:   Performance monitor functionality for Itanium
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
 * $Id: itanium_perf.h,v 1.7 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__ITANIUM__PERF_H__
#define __ARCH__IA64__ITANIUM__PERF_H__

#include INC_ARCH(perf.h)


/**
 * Generic performance monitor configuration register for the Itanium.
 */
class pmc_itanium_t
{
    union {
	struct {
	    word_t plm		: 4;
	    word_t ev		: 1;
	    word_t oi		: 1;
	    word_t pm		: 1;
	    word_t __ig1	: 1;
	    word_t es		: 8;	// Only 7 bits on Itanium 1
	    word_t umask	: 4;
	    word_t threshold	: 3;
	    word_t enabled	: 1;	// Ignored for Itanium 1
	    word_t ism		: 2;
	    word_t zeros	: 2;	// Ignored for Itanium 1
	    word_t __ig3	: 36;
	};
	word_t raw;
    };

public:

    enum ism_e {
	mon_all		= 0,
	mon_ia32	= 1,
	mon_ia64	= 2,
	mon_disabled	= 3,
    };

    // Construction

    pmc_itanium_t (void) {}
    pmc_itanium_t (word_t w) { raw = w; }

    // Conversion
    
    operator word_t (void) { return raw; }
    operator pmc_t (void) { pmc_t pmc = raw; return pmc; }
};


/**
 * Wrapper class for accessing the Itanium instruction address range
 * check register.
 */
class pmc_instr_range_t
{
public:
    bool is_all_tagged (void)
	{ return get_pmc (13) & 1; }

    void tag_all (void)
	{ set_pmc (13, 1); }

    void tag_range (void)
	{ set_pmc (13, 0); }
};




#endif /* !__ARCH__IA64__ITANIUM__PERF_H__ */
