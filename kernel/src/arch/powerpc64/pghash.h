/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/pghash.h
 * Description:	PowerPC64 page hash abstractions.
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
 * $Id: pghash.h,v 1.8 2005/01/18 13:24:01 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__PGHASH_H__
#define __ARCH__POWERPC64__PGHASH_H__

#include INC_ARCH(page.h)

#if (CONFIG_CPU_POWERPC64_POWER4 || CONFIG_CPU_POWERPC64_POWER4p)
#include INC_ARCH(power4.h)
#else
#endif

#include <debug.h>	// for UNIMPL

#define POWERPC64_HTABORG_SHIFT	18

#define PTE_API_SHIFT	23

#if !defined(ASSEMBLY)

#include <generic/memregion.h>

class ppc64_sdr1_t
{
public:
    union {
	struct {
    	    word_t htaborg : 46;
    	    word_t reserved : 13;
    	    word_t htabsize : 5;
	} x;
	u64_t raw;
    };

    void create( word_t base, word_t size);
};

class ppc64_pte_t
{
public:
    union {
	struct {
	    /* Word 0 */
	    word_t vsid : 52;		/* virtual segment id */
	    word_t api : 5;		/* abbreviated page index */
	    word_t reserved1 : 2;	/* Software use */
	    word_t bolted : 1;		/* Hash entry is "bolted" */
	    word_t lock : 1;		/* Lock of pSeries SMP */
	    word_t l : 1;		/* Virtual page is large (L=1) or 4 KB (L=0) */
	    word_t h : 1;		/* hash function identifier */
	    word_t v : 1;		/* v = 1 entry is valid */
	    /* Word 1 */
	    word_t pp0 : 1;		/* Page protection bit 0 */
	    word_t ts : 1;		/* Tag set bit */
	    word_t rpn : 50;		/* physical page number */
	    word_t reserved2 : 2;
	    word_t ac : 1;		/* Address compare */ 
	    word_t r : 1;		/* referenced bit */
	    word_t c : 1;		/* changed bit */
	    word_t wimg : 4;		/* write-through, caching-inhibited, 
					   memory coherence, gaurded */
	    word_t n : 1;		/* No-execute */
	    word_t pp : 2;		/* protection bits */
	} x;
	struct {
	    u64_t word0;
	    u64_t word1;
	} raw;
    };

//    void create( word_t virt, word_t phys, word_t vsid, word_t wimg, word_t noexecute, word_t pp);
    void create_4k( word_t virt, word_t entry, word_t vsid, word_t second );
    void create( word_t virt, word_t entry, word_t vsid, word_t second, word_t large );
    void create( word_t virt, word_t entry, word_t vsid, word_t second, word_t large, word_t bolted );

    static word_t virt_to_api( word_t virt )
	{ return (virt >> PTE_API_SHIFT) & 0x1f; }
    static word_t api_to_virt( word_t api )
	{ return api << PTE_API_SHIFT; }
};

#define HTAB_VSID_HASH_MASK	((1ul << 39) - 1)
#define HTAB_EA_PAGE_MASK	((1ul << 16) - 1)
#define HTAB_EA_LARGE_MASK	((1ul << 4) - 1)
#define HTAB_PTEG_SIZE		8	// Number of PTE entries in a PTEG.
#define HTAB_PTEG_BITS		7	// Number of bits in a PTEG index (128 bytes
					// per PTEG).
#define HTAB_PTEGs_BITS		11
#define HTAB_REVERSE_MASK	0x000007ff	// The 11-bits of an EA in 
						// the hash value.
#define PTE_PAGE_INDEX_MASK	((1ul << 11)-1)
#define PTE_PAGE_INDEX_SHIFT	7

class ppc64_htab_t
{
public:
    void init( word_t phys_base, word_t virt_start, word_t size );
    void activate();

    ppc64_pte_t * locate_pte( word_t virt, word_t vsid, word_t slot, 
	    bool is_second_hash, bool large );
    ppc64_pte_t * find_insertion( word_t virt, word_t vsid, word_t *slot, 
	    word_t *is_second_hash, bool large );

    word_t reverse_hash( ppc64_pte_t *pghash_pte );

    ppc64_pte_t * get_pteg( word_t hash )
	{ return (ppc64_pte_t *)((word_t)this->base |
			((hash & this->hash_mask) << HTAB_PTEG_BITS)); }

    word_t optimal_size( word_t tot_phys_mem )
	{ 
	    word_t size = ((tot_phys_mem >> (POWERPC64_PAGE_BITS + 1)) << HTAB_PTEG_BITS);
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

    word_t min_size() { return (1ul << (HTAB_PTEGs_BITS + HTAB_PTEG_BITS)); }

private:
    word_t primary_hash( word_t virt, word_t vsid, bool large );
    word_t secondary_hash( word_t hash );


    ppc64_pte_t * base;
    word_t phys_base;
    word_t size;
    word_t hash_mask;
    word_t htab_size;
};

/****************************************************************************
 *
 *                       inline functions
 *
 ****************************************************************************/

/*
 * Invalidate the TLB. Context syncronizing must
 * be called before calling us.
 */
INLINE void ppc64_invalidate_tlb( void )
{
    word_t ea = 0;

#if CONFIG_PLAT_OFPOWER4 || CONFIG_CPU_POWERPC64_PPC970
    for (word_t i = 0; i < ( CONFIG_POWERPC64_TLB_SIZE/CONFIG_POWERPC64_TLB_WAYS ); i++)
    {
	ea = i << POWERPC64_PAGE_BITS;
	asm volatile( "tlbiel	%0;" :: "r" (ea) : "memory" );
    }
    asm volatile( "ptesync" : : : "memory" );
#elif CONFIG_PLAT_OFPOWER3
    asm volatile( "ptesync" : : : "memory" );
    for (word_t i = 0; i < ( CONFIG_POWERPC64_TLB_SIZE ); i++)
    {
	ea = i << POWERPC64_PAGE_BITS;
	asm volatile( "tlbie	%0,0;" :: "r" (ea) : "memory" );
    }
    asm volatile( "eieio; tlbsync; ptesync" : : : "memory" );
#endif
}

/*
 * Invalidate a TLB entry. Context syncronizing must
 * be called before calling us.
 */
INLINE void ppc64_invalidate_tlbe( addr_t addr, bool large )
{
    asm volatile( "ptesync" : : : "memory" );

    if (large)
	asm volatile( "tlbie    %0,1" : : "r"(addr) : "memory" );
    else
	asm volatile( "tlbie    %0,0" : : "r"(addr) : "memory" );

    asm volatile( "eieio; tlbsync; ptesync" : : : "memory" );
}

INLINE void ppc64_sdr1_t::create( word_t base, word_t size)
{
    this->raw = 0;
    this->x.htaborg = base >> POWERPC64_HTABORG_SHIFT;
    this->x.htabsize = size;
}

/*
INLINE void ppc64_pte_t::create( word_t virt, word_t phys, word_t vsid, word_t wimg, word_t noexecute, word_t pp )
{
    this->raw.word0 = this->raw.word1 = 0;
    this->x.vsid = vsid;
    this->x.api = virt_to_api( virt );
    this->x.rpn = phys >> POWERPC64_PAGE_BITS;
    this->x.r = 1;
    this->x.c = 1;
    this->x.wimg = wimg;
    this->x.n = noexecute;
    this->x.pp = pp;
    this->x.v = 1;
}
*/

INLINE void ppc64_pte_t::create_4k( word_t virt, word_t entry,
		word_t vsid, word_t second )
{
    this->raw.word1 = entry;
    asm volatile( "eieio;" ::: "memory" );
    this->raw.word0 = 0;
    this->x.vsid = vsid;
    this->x.api = virt_to_api( virt );
    this->x.h = second;
    this->x.v = 1;
    asm volatile( "sync;" ::: "memory" );
}

INLINE void ppc64_pte_t::create( word_t virt, word_t entry, word_t vsid,
		word_t second, word_t large )
{
    this->raw.word1 = entry;
    asm volatile( "eieio;" ::: "memory" );
    this->raw.word0 = 0;
    this->x.vsid = vsid;

    if (large)
	this->x.api = virt_to_api( virt ) & (~(1ul));
    else
	this->x.api = virt_to_api( virt );

    this->x.l = large;
    this->x.h = second;
    this->x.v = 1;
    asm volatile( "sync;" ::: "memory" );
}

INLINE void ppc64_pte_t::create( word_t virt, word_t entry, word_t vsid,
		word_t second, word_t large, word_t bolted )
{
    this->raw.word1 = entry;
    asm volatile( "eieio;" ::: "memory" );
    this->raw.word0 = 0;
    this->x.vsid = vsid;

    if (large)
	this->x.api = virt_to_api( virt ) & (~(1ul));
    else
	this->x.api = virt_to_api( virt );

    this->x.bolted = bolted;
    this->x.l = large;
    this->x.h = second;
    this->x.v = 1;
    asm volatile( "sync;" ::: "memory" );
}

INLINE word_t ppc64_htab_t::primary_hash( word_t virt, word_t vsid, bool large )
{
    if (large)
	return (vsid & HTAB_VSID_HASH_MASK) ^ ((virt >> POWERPC64_LARGE_BITS) & HTAB_EA_LARGE_MASK);
    else
	return (vsid & HTAB_VSID_HASH_MASK) ^ ((virt >> POWERPC64_PAGE_BITS) & HTAB_EA_PAGE_MASK);
}

INLINE word_t ppc64_htab_t::secondary_hash( word_t hash )
{
    // TODO: see the PPC eqv instruction; it can compute the complete
    // secondary hash in one instruction.
    return (~hash) & HTAB_VSID_HASH_MASK;
}

/**
 * ppc64_htab_t::reverse_hash: computes the virtual address which the pte maps.
 * @param pghash_pte The location of the pte, in the page hash.
 * #return The virtual address which the pte maps.
 */
INLINE word_t ppc64_htab_t::reverse_hash( ppc64_pte_t *pghash_pte )
{
    word_t vsid = pghash_pte->x.vsid;
    word_t api = pghash_pte->x.api;
    word_t page_index;
    word_t va;

    page_index = (word_t)pghash_pte;	/* Get PTE address */

    if (pghash_pte->x.h)		/* if secondary - invert the bits */
	page_index = ~page_index;

    /* Extract the Page Index */
    page_index = (page_index >> PTE_PAGE_INDEX_SHIFT);
    /* Add in the API and reverse hash with the VSID */
    page_index = (api << 11) | ((page_index ^ vsid) & PTE_PAGE_INDEX_MASK);

    /* If shift the page_index */
    if (pghash_pte->x.l)
	page_index = page_index << POWERPC64_LARGE_BITS;
    else
	page_index = page_index << POWERPC64_PAGE_BITS;

    /* Compute the Effective address */
    va = ((vsid << POWERPC64_SEGMENT_BITS) | page_index)
#if POWERPC64_USER_BITS != 64
	    & ((1ul << POWERPC64_USER_BITS)-1);
#else
    ;
#endif
    /* Remembering that the kernel is at a high address */
    if ((vsid >> CONFIG_POWERPC64_ESID_BITS) == 0)	/* Kernel segment */
	va = va
#if (CONFIG_POWERPC64_ESID_BITS + POWERPC64_SEGMENT_BITS) != 64
    // Add in the msb of the virtual address
	    | ( ~ ( ( 1ul << ( CONFIG_POWERPC64_ESID_BITS + POWERPC64_SEGMENT_BITS ) ) - 1 ) );
#else
    ;
#endif

    return va;
}

INLINE ppc64_pte_t* ppc64_htab_t::locate_pte( word_t virt, word_t vsid, 
	word_t slot, bool is_second_hash, bool large )
{
    // Create the hash.
    word_t hash = this->primary_hash( virt, vsid, large );
    if( is_second_hash )
	hash = this->secondary_hash( hash );

    // Go directly to the pte in the pteg, based on the pteg_slot stored
    // in the pgent.
    ppc64_pte_t *pte = get_pteg(hash);
    pte = &pte[ slot ];

    // Verify that the pte matches the search criteria.
    word_t api = ppc64_pte_t::virt_to_api( virt );

    if( (pte->x.v == 1) && (pte->x.vsid == vsid) && (pte->x.api == api) )
	return pte;

    return NULL;
}

#endif	/* !ASSEMBLY */

#endif	/* __ARCH__POWERPC64__PGHASH_H__ */
