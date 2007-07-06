/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/pkr.h
 * Description:   Handling of protection key registers.
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
 * $Id: pkr.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__PKR_H__
#define __ARCH__IA64__PKR_H__


class pkr_t
{
public:
    enum access_rights_e {
	norights = 0,
	wo	= 1,
	ro	= 2,
	xo	= 4,
	rw	= 5,
	wx	= 6,
	rwx	= 7
    };

    union {
	struct {
	    word_t valid		: 1;
	    word_t wrx_disable		: 3;
	    word_t __rv1		: 4;
	    word_t key			: 24;
	    word_t __rv2		: 32;
	} x;
	word_t raw;
    };

    // Retrieval

    inline bool is_valid (void)
	{ return x.valid; }

    inline bool is_writable (void)
	{ return ! (x.wrx_disable & 1); }

    inline bool is_readable (void)
	{ return ! (x.wrx_disable & 2); }

    inline bool is_executable (void)
	{ return ! (x.wrx_disable & 4); }

    inline access_rights_e access_rights (void)
	{ return (access_rights_e) x.wrx_disable; }

    inline word_t key (void)
	{ return x.key; }

    // Modification

    inline void validate (void)
	{ x.valid = 1; }

    inline void invalidate (void)
	{ x.valid = 0; }

    inline void set (bool valid, access_rights_e rights, word_t key);

    // Register access

    inline void get (word_t num);
    inline void put (word_t num);
};


/**
 * pkr_t::pkr_set: Initialize the values of a protection key strcuture.
 * The valid bit is automatically set.
 */
INLINE void pkr_t::set (bool valid, access_rights_e rights, word_t key)
{
    x.valid = valid;
    x.wrx_disable = ~rights;
    x.key = key;
}

/**
 * pkr_t::pkr_get: Load pkr_t values from the indicated PKR.
 */
INLINE void pkr_t::get (word_t pkr_num)
{
    __asm__ __volatile__ (
	"	mov	%0 = pkr[%1]	\n"
	:
	"=r" (raw)
	:
	"r" (pkr_num));
}

/**
 * pkr_t::put: Store pkr_t values into the indicated PKR.
 */
INLINE void pkr_t::put (word_t pkr_num)
{
    __asm__ __volatile__ (
	"	mov	pkr[%1] = %0	\n"
	:
	:
	"r" (raw),
	"r" (pkr_num));
}

/**
 * get_pkr: Return value of indicated PKR.
 */
INLINE pkr_t get_pkr (word_t pkr_num)
{
    pkr_t ret;

    __asm__ __volatile__ (
	"	mov	%0 = pkr[%1]	\n"
	:
	"=r" (ret.raw)
	:
	"r" (pkr_num));

    return ret;
}


#endif /* !__ARCH__IA64__PKR_H__ */
