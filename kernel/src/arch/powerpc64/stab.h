/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/stab.h
 * Description:	PowerPC64 segment table hash abstractions.
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
 * $Id: stab.h,v 1.3 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__STAB_H__
#define __ARCH__POWERPC64__STAB_H__

#if !defined(ASSEMBLY)

#include INC_GLUE(hwspace.h)

#define STAB_HASH_MASK		0x1f
#define STAB_STEG_BITS		7

class ppc64_asr_t
{
public:
    union {
	struct {
	    word_t staborg : 52;	/* Physical address of segment table */
	    word_t reserved : 11;
	    word_t valid : 1;		/* Power3 ASR valid bit */
	} x;
	u64_t raw;
    };
};


class ppc64_ste_t 
{
public:
    union {
	struct {
	    word_t esid : 36;		/* Effective segment ID */
	    word_t reserved1 : 20;
	    word_t v : 1;		/* Entry valid */
	    word_t t : 1;		/* T = 0 selects this format */
	    word_t ks : 1;		/* Supervisor protection key */
	    word_t kp : 1;		/* User protection key */
	    word_t n : 1;		/* No-execute bit */
	    word_t ste_class : 1;	/* Class */
	    word_t reserved2 : 2;
	    word_t vsid : 52;		/* Virtual Segment ID */
	    word_t reserved3 : 12;
	} x;
	struct {
	    u64_t word0;
	    u64_t word1;
	} raw;
    };

    void set_entry( word_t esid, word_t ks, word_t kp, word_t n, word_t vsid );
    void invalidate( bool sync );
};


class ppc64_stab_t
{
public:
    void init();
    void free();

    ppc64_ste_t * lookup_ste( word_t virt );
    ppc64_ste_t * find_insertion( word_t vsid, word_t esid );

    word_t reverse_hash( ppc64_ste_t *seghash_ste );

    ppc64_ste_t * get_steg( word_t hash )
	{ return (ppc64_ste_t*)(get_stab() | ((hash & STAB_HASH_MASK) << STAB_STEG_BITS)); }

    word_t get_asr() { return base.raw; }
    word_t get_stab() { return phys_to_virt(base.x.staborg << POWERPC64_PAGE_BITS); }

private:
    word_t primary_hash( word_t esid )
    {
	return (esid & STAB_HASH_MASK);
    }
    
    word_t secondary_hash( word_t esid )
    {
	return ((~esid) & STAB_HASH_MASK);
    }

    ppc64_asr_t base;
};


INLINE void ppc64_ste_t::set_entry( word_t esid, word_t ks, word_t kp, word_t n, word_t vsid )
{
    this->raw.word1 = 0;
    this->x.vsid = vsid;
    /* Order VSID updte */
    __asm__ __volatile__ ("eieio" : : : "memory");
    this->raw.word0 = 0;
    this->x.esid = esid;
    this->x.v = 1;
    this->x.ks = ks;
    this->x.kp = kp;
    this->x.n = n;
    /* Order update     */
    __asm__ __volatile__ ("sync" : : : "memory");
}

INLINE void ppc64_ste_t::invalidate( bool sync )
{
    this->x.v = 0;

    if ( sync )
    {
	/* Order update     */
	__asm__ __volatile__ ("sync" : : : "memory");
    }
}

INLINE ppc64_ste_t *ppc64_stab_t::find_insertion( word_t vsid, word_t esid )
{
    word_t group, entry, random;
    ppc64_ste_t *ste = get_steg( primary_hash( esid ) );

    for( group = 0; group < 2; group ++ )
    {
	for( entry = 0; entry < 8; entry ++, ste++ )
	{
	    if (ste->x.v == 0)	    /* Invalid entry */
	    {
		return ste;	    /* Return the entry */
	    }
	}
	
	ste = get_steg( secondary_hash( esid ) );
    }

    /* No free entry found, need to evict one */

    do {
	__asm__ __volatile__ ("mftbl    %0;" : "=r" (random));


	if (random & 0x8)
	    ste = get_steg( primary_hash( esid ) );
	else
	    ste = get_steg( secondary_hash( esid ) );

	ste = &ste[random & 0x7];
    } while (((ste->x.esid >> 20) >= 0xfff0) && (!(random & 0x8))); /* Don't evict kernel entries */

    /* Force previous translations to complete. DRENG */
    __asm__ __volatile__ ("isync" : : : "memory" );

    ste->invalidate( true );

    return ste;
}


#endif	/* !ASSEMBLY */

#endif /* __ARCH__POWERPC64__STAB_H__ */
