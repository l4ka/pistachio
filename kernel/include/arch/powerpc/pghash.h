/****************************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/pghash.h
 * Description:	PowerPC page hash abstractions.
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
 * $Id: pghash.h,v 1.8 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__PGHASH_H__
#define __ARCH__POWERPC__PGHASH_H__

#include INC_ARCH(bat.h)
#include INC_ARCH(page.h)
#include INC_ARCH(pgtab.h)
#include INC_ARCH(ibm750.h)

#define POWERPC_HTABORG_SHIFT	16

#if !defined(ASSEMBLY)

#include <generic/memregion.h>
#include INC_ARCH(pgtab.h)

class ppc_sdr1_t
{
public:
    union {
	struct {
    	    word_t htaborg : 16;
    	    word_t reserved : 7;
    	    word_t htabmask : 9;
	} x;
	u32_t raw;
    };

    void create( word_t base, word_t mask );
};

class ppc_segment_t 
{
public:
    union {
	struct {
	    word_t t : 1;		/* t = 0 selects this format */
	    word_t ks : 1;		/* supervisor-state protection key */
	    word_t kp : 1;		/* user-state protection key */
	    word_t n : 1;		/* no-execute protection bit */
	    word_t reserved : 4;
	    word_t vsid : 24;		/* virtual segment id */
	} x;
	u32_t raw;
    };
};

class ppc_translation_t
{
public:
    union {
	struct {
	    word_t v : 1;		/* v = 1 entry is valid */
	    word_t vsid : 24;		/* virtual segment id */
	    word_t h : 1;		/* hash function identifier */
	    word_t api : 6;		/* abbreviated page index */
	    word_t rpn : 20;		/* physical page number */
	    word_t reserved1 : 3;
	    word_t r : 1;		/* referenced bit */
	    word_t c : 1;		/* changed bit */
	    word_t wimg : 4;		/* write-through, caching-inhibited, 
					   memory coherence, gaurded */
	    word_t reserved2 : 1;
	    word_t pp : 2;		/* protection bits */
	} x;
	struct {
	    u32_t word0;
	    u32_t word1;
	} raw;
    };

    void create( word_t virt, word_t phys, word_t vsid, word_t wimg, word_t pp);
    void create( word_t virt, word_t entry, word_t vsid );

    static word_t virt_to_api( word_t virt )
	{ return (virt >> 22) & 0x3f; }
    static word_t api_to_virt( word_t api )
	{ return api << 22; }
};

#define HTAB_HASH_MASK	((1 << 19) - 1)
#define HTAB_PAGE_MASK	((1 << 16) - 1)
#define HTAB_PTEG_SIZE	8	// Number of PTE entries in a PTEG.
#define HTAB_PTEG_BITS	6	// Number of bits in a PTEG index (64 bytes
				// per PTEG).
#define HTAB_REVERSE_MASK	0x000003ff	// The 10-bits of an EA in 
						// the hash value.

class ppc_htab_t
{
public:
    void init( word_t phys_base, word_t virt_start, word_t size );
    void bat_map( void );
    void activate( ppc_segment_t segment_val );

    ppc_translation_t * locate_pte( word_t virt, word_t vsid, word_t slot, 
	    word_t is_second_hash );
    ppc_translation_t * find_insertion( word_t virt, word_t vsid, word_t *slot, 
	    word_t *is_second_hash );

    word_t primary_hash( word_t virt, word_t vsid );
    word_t secondary_hash( word_t hash );
    word_t reverse_hash( ppc_translation_t *pghash_pte );

    ppc_translation_t * get_pteg( word_t hash )
	{ return (ppc_translation_t *)((word_t)this->base | ((hash & this->hash_mask) << 6)); }

    word_t optimal_size( word_t tot_phys_mem )
	{ 
	    word_t size = tot_phys_mem / MB(8) * KB(64);
	    if( size < min_size() )
		size = min_size();
	    else {
		// Choose a size such that only a single bit is set.
		int bits = 0;
		while( size > 1 ) {
		    bits++;
		    size = size >> 1;
		}
		size = size << bits;
	    }
	    return size;
	}

    word_t min_size() { return BAT_SMALL_PAGE_SIZE; }

private:
    ppc_translation_t * base;
    word_t phys_base;
    word_t size;
    word_t hash_mask;
    word_t htab_mask;
};

/****************************************************************************
 *
 *                       inline functions
 *
 ****************************************************************************/

INLINE void ppc_set_sr( word_t sr, word_t val )
{
    asm volatile ( "mtsrin %0, %1" : : "r" (val), "r" (sr << 28) );
}

INLINE void ppc_switch_addr_space( word_t id )
{
    word_t i;
    asm volatile ("isync");
    for( i = 0; i < 16; i++ ) {
	ppc_set_sr( i, id | i );
    }
    asm volatile ("isync");
}

INLINE void ppc_invalidate_tlb( void )
{
    word_t page;

    for( page = 0; page < POWERPC_TLBIE_MAX; page += POWERPC_TLBIE_INC )
	asm volatile( "tlbie %0" : : "r" (page) );
    asm volatile( "tlbsync" );
}

INLINE void ppc_invalidate_tlbe( addr_t addr )
{
    asm volatile( "tlbie %0" : : "r" (addr) );
}

INLINE word_t ppc_get_sr( word_t which )
{
    word_t val;
    asm volatile( "mfsrin %0, %1" : "=r" (val) : "r" (which << 28) );
    return val;
}

INLINE void ppc_sdr1_t::create( word_t base, word_t mask )
{
    this->x.htaborg = base >> POWERPC_HTABORG_SHIFT;
    this->x.htabmask = mask;
}

INLINE void ppc_translation_t::create( word_t virt, word_t phys, word_t vsid, word_t wimg, word_t pp )
{
    this->raw.word0 = this->raw.word1 = 0;
    this->x.vsid = vsid;
    this->x.api = virt_to_api( virt );
    this->x.rpn = phys >> POWERPC_PAGE_BITS;
    this->x.r = 1;
    this->x.c = 1;
    this->x.wimg = wimg;
    this->x.pp = pp;
    this->x.v = 1;
}

INLINE void ppc_translation_t::create( word_t virt, word_t entry, word_t vsid )
{
    this->raw.word0 = 0;
    this->raw.word1 = entry;
    this->x.vsid = vsid;
    this->x.api = virt_to_api( virt );
    this->x.v = 1;
}

INLINE word_t ppc_htab_t::primary_hash( word_t virt, word_t vsid )
{
    return (vsid & HTAB_HASH_MASK) ^ ((virt >> POWERPC_PAGE_BITS) & HTAB_PAGE_MASK);
}

INLINE word_t ppc_htab_t::secondary_hash( word_t hash )
{
    // TODO: see the PPC eqv instruction; it can compute the complete
    // secondary hash in one instruction.
    return ~hash & HTAB_HASH_MASK;
}

/**
 * ppc_htab_t::reverse_hash: computes the virtual address which the pte maps.
 * @param pghash_pte The location of the pte, in the page hash.
 * #return The virtual address which the pte maps.
 */
INLINE word_t ppc_htab_t::reverse_hash( ppc_translation_t *pghash_pte )
{
    // We bit manipulate with the whole pteg, but we are only interested 
    // in the portion related to the hash, bits 15-6.
    word_t virt, pteg;
 
    pteg = (word_t)pghash_pte;
    if( pghash_pte->x.h )
	pteg = ~pteg;	// Convert the 2nd hash to the 1st.

    // XOR the VSID with the PTEG location to get back part of the original
    // virtual address.
    virt = HTAB_REVERSE_MASK & (pghash_pte->x.vsid ^ (pteg >> HTAB_PTEG_BITS));
    // Convert to a page index.
    virt = virt << POWERPC_PAGE_BITS;
    // Add in the abbreviated page index.
    virt |= ppc_translation_t::api_to_virt( pghash_pte->x.api );
    // Add in the four msb of the virtual address, which are the four
    // lsb of the VSID.
    virt |= (pghash_pte->x.vsid & 0xf) << 28;
    return virt;
}

INLINE ppc_translation_t * ppc_htab_t::locate_pte( word_t virt, word_t vsid, 
	word_t slot, word_t is_second_hash )
{
    // Create the hash.
    word_t hash = this->primary_hash( virt, vsid );
    if( is_second_hash )
	hash = this->secondary_hash( hash );

    // Go directly to the pte in the pteg, based on the pteg_slot stored
    // in the pgent.
    ppc_translation_t *pte;
    pte = get_pteg(hash);
    pte = &pte[ slot ];

    // Verify that the pte matches the search criteria.
    word_t api = ppc_translation_t::virt_to_api( virt );
    if( (pte->x.v == 1) && (pte->x.vsid == vsid) && (pte->x.api == api) )
	return pte;
    return NULL;
}

#endif	/* !ASSEMBLY */

#endif	/* __ARCH__POWERPC__PGHASH_H__ */
