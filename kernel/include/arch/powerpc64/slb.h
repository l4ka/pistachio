/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/slb.h
 * Description:	PowerPC64 segment lookaside buffer abstractions.
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
 * $Id: slb.h,v 1.6 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__SLB_H__
#define __ARCH__POWERPC64__SLB_H__


#define POWERPC64_SEG_SHIFT	28
#define POWERPC64_SEG_MASK	((1ul << 36)-1)
#define ESID(x)			(((x) >> POWERPC64_SEG_SHIFT) & POWERPC64_SEG_MASK)

#if !defined(ASSEMBLY)

#include INC_ARCH(ppc64_registers.h)

class slbent_t
{
public:
    union {
	struct {
	    word_t esid: 36;	/* Effective segment ID */
	    word_t v:     1;	/* Entry valid (v=1) or invalid */
	    word_t null1:15;	/* padding to a 64b boundary */
	    word_t index:12;	/* Index to select SLB entry. Used by slbmte */
	} X;
	word_t raw;
    } esid;
    union {
	struct {
	    word_t vsid: 52;	/* Virtual segment ID */
	    word_t ks:    1;	/* Supervisor (privileged) state storage key */
	    word_t kp:    1;	/* Problem state storage key */
	    word_t n:     1;	/* No-execute if n=1 */
	    word_t l:     1;	/* Virt pages are large (l=1) or 4KB (l=0) */
	    word_t c:     1;	/* Class */
	    word_t resv0: 7;	/* Padding to a 64b boundary */
	} X;
	word_t raw;
    } vsid;

private:
};

class segment_t : public generic_segment_t
{
public:
    static inline void init_cpu( space_t *kspace, addr_t kbase );

    static inline void flush_segments()
    {
	__asm__ __volatile__("isync; slbia; isync":::"memory");
    }

    inline void flush_segment_entry( addr_t addr )
    {
	__asm__ __volatile__("sync; slbie %0; sync":: "r" (addr) :"memory");
    }

    static inline void insert_entry( space_t *space, word_t vsid, word_t esid, bool large );
};


INLINE void segment_t::init_cpu( space_t *kspace, addr_t kbase )
{
    slbent_t kern_ent;

    word_t k_vsid = kspace->get_vsid( kbase );

    /* Setup the kernel VSID_ASID */
    __asm__ __volatile__ ( "mtasr %0;" :: "r" (0) );

    flush_segments();

    /* Create an entry for the first kernel segment (256MB)
     * in SLB index 0. This entry is not evicted by a TLBIA
     */

    kern_ent.esid.raw = 0;	    /* Clear word */
    kern_ent.esid.X.esid = ESID((word_t)kbase);
    kern_ent.esid.X.v = 1;

    kern_ent.vsid.raw = 0;	    /* Clear word */
    kern_ent.vsid.X.vsid = k_vsid;
    kern_ent.vsid.X.kp = 1;	    /* Key to restrict user (with PTE PP bits) */
    kern_ent.vsid.X.l = 1;	    /* Kernel 16M pages */
    kern_ent.vsid.X.c = 1;	    /* Kernel class */

    __asm__ __volatile__ ("isync" : : : "memory");
    __asm__ __volatile__ (
	    "slbmte  %0,%1"
	    :: "r" (kern_ent.vsid.raw),   /* VSID data */
	       "r" (kern_ent.esid.raw));  /* ESID data */
    /* Order update  */
    __asm__ __volatile__ ("isync" : : : "memory");
}


/* Insert an entry into the SLB
 */
INLINE void segment_t::insert_entry( space_t *space, word_t vsid, word_t esid, bool large )
{
    word_t index;
    slbent_t entry;

/* XXX temporary check */
ASSERT( (vsid >> VSID_REVERSE_SHIFT) == 0 );

    /* Search for an empty SLB entry.
     * Don't search entry 0, its the kernel.
     */
    for(index = 1; index < CONFIG_POWERPC64_SLBENTRIES; index++)
    {
	__asm__ __volatile__(
	    "slbmfee  %0,%1"	    /* Move from SLB entry, ESID */
            : "=r" (entry.esid)
	    : "r" (index)
	);

        if(!entry.esid.X.v)
	{
            /* Create the new SLB entry
             */
	    entry.esid.raw = 0;
	    entry.esid.X.esid = esid;
	    entry.esid.X.v = 1;
	    entry.esid.X.index = index;

	    entry.vsid.raw = 0;
	    entry.vsid.X.vsid = vsid;
	    entry.vsid.X.kp = 1;
	    entry.vsid.X.l = large ? 1 : 0;
	    entry.vsid.X.c = 1;	    /* Kernel class */
                                                                                                                                                       
            /* slbie is not needed as no previous mapping existed. */
            /* Order update  */
            __asm__ __volatile__ ("isync" ::: "memory");
            __asm__ __volatile__ (
		"slbmte  %0,%1"
                :: "r" (entry.vsid.raw),
                   "r" (entry.esid.raw)
	    );
	    /* Order update  */
            __asm__ __volatile__ ("isync" ::: "memory");
            return;
        }
    }

    /* Create the new SLB entry
     */
    entry.esid.raw = 0;
    entry.esid.X.esid = esid;
    entry.esid.X.v = 1;
    entry.esid.X.index = ppc64_get_tb() & 0x0f;

    entry.vsid.raw = 0;
    entry.vsid.X.vsid = vsid;
    entry.vsid.X.kp = 1;
    entry.vsid.X.l = large ? 1 : 0;
    entry.vsid.X.c = 1;	    /* Kernel class */

    __asm__ __volatile__ ("isync" ::: "memory");    /* Order update */
    __asm__ __volatile__ (
	"slbmte  %0,%1"
        :: "r" (entry.vsid.raw),
	   "r" (entry.esid.raw)
    );
    __asm__ __volatile__ ("isync" ::: "memory" );   /* Order update */
}

#endif

#endif /* __ARCH__POWERPC64__SLB_H__ */
