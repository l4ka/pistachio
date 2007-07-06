/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/seghash.h
 * Description:	PowerPC64 segment hash abstractions.
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
 * $Id: seghash.h,v 1.5 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__SEGHASH_H__
#define __ARCH__POWERPC64__SEGHASH_H__

#include INC_ARCH(stab.h)

#define POWERPC64_SEG_SHIFT	28
#define POWERPC64_SEG_MASK	((1ul << 36)-1)
#define ESID(x)			(((x) >> POWERPC64_SEG_SHIFT) & POWERPC64_SEG_MASK)

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
    /* Setup the kernel segment table */
    __asm__ __volatile__ ( "mtasr %0;" :: "r" ( kspace->get_seg_table()->get_asr() ) );

    flush_segments();

    word_t vsid = kspace->get_vsid( kbase );

    insert_entry( kspace, vsid, ESID((word_t)kbase), false );
}

INLINE void segment_t::insert_entry( space_t *space, word_t vsid, word_t esid, bool large )
{
    ppc64_stab_t *stab = space->get_seg_table();

    //printf( "segment: insert_entry: space = %p. vsid = %p, esid = %p \n", space, vsid, esid );
    ppc64_ste_t *ste = stab->find_insertion( vsid, esid );

    ste->set_entry( esid, 0, 1, 0, vsid );
}

#endif	/* __ARCH__POWERPC64__PGHASH_H__ */
