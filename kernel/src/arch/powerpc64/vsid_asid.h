/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     arch/powerpc64/vsid_asid.h
 * Description:   PowerPC64 specific reverse lookup ASID management
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
 * $Id: vsid_asid.h,v 1.5 2004/06/04 02:14:26 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__POWERPC64__VSID_ASID_H__
#define __ARCH__POWERPC64__VSID_ASID_H__

#define ASID_INVALID	    (~0ul)

#define ASID_BITS	    (POWERPC64_VIRTUAL_BITS - POWERPC64_USER_BITS)
#define ASID_MAX	    (1ul << ASID_BITS)

#define VSID_REVERSE_SHIFT  (POWERPC64_USER_BITS - POWERPC64_SEGMENT_BITS)

class space_t;

class vce_t
{
public:
    inline bool is_valid() { return ( asid != ASID_INVALID ); };

    word_t  asid;
    space_t *space;
};


class vsid_asid_cache_t
{
public:
    void init( space_t *kernel_space );
    word_t alloc( space_t *space );
    void release( word_t asid );

    space_t *lookup( word_t vsid );
private:
    vce_t cache[ ASID_MAX ];
    s64_t first_free;
};


class vsid_asid_t
{
public:
    inline void init( void ) { vsid_asid = ASID_INVALID; };

    word_t get( space_t *space );
    void free( void );

private:
    word_t vsid_asid;
};


INLINE vsid_asid_cache_t *get_vsid_asid_cache(void)
{
    extern vsid_asid_cache_t vsid_asid_cache;
    return &vsid_asid_cache;
}

INLINE void vsid_asid_cache_t::init( space_t *kernel_space )
{
    for ( word_t i = 0; i < ( ASID_MAX ); i++ )
    {
	cache[i].asid = ASID_INVALID;
	cache[i].space = NULL;
    }
    cache[0].asid = 0;
    cache[0].space = kernel_space;

    first_free = 1;
}

INLINE space_t *vsid_asid_cache_t::lookup( word_t vsid )
{
    return cache[ (vsid >> VSID_REVERSE_SHIFT) & (ASID_MAX-1) ].space;
}

INLINE word_t vsid_asid_t::get( space_t *space )
{
    if ( vsid_asid == ASID_INVALID )
	vsid_asid = get_vsid_asid_cache()->alloc( space );

    return vsid_asid;
}

#endif /* __ARCH__POWERPC64__VSID_ASID_H__ */
