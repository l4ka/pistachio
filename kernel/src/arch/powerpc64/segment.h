/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/segment.h
 * Description:	PowerPC64 segmentation abstractions.
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
 * $Id: segment.h,v 1.2 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__SEGMENT_H__
#define __ARCH__POWERPC64__SEGMENT_H__

#include INC_GLUE(space.h)

class generic_segment_t
{
public:
    static inline void init_cpu( space_t *kspace, addr_t kbase );
    static inline void flush_segments();
    static inline void flush_segment_entry( addr_t addr );
    static inline void insert_entry( space_t *space, word_t vsid, word_t esid, bool large );
private:
};


#if CONFIG_POWERPC64_STAB
#include INC_ARCH(seghash.h)
#elif CONFIG_POWERPC64_SLB
#include INC_ARCH(slb.h)
#else
#error No segment table abstraction found
#endif


#endif /* __ARCH__POWERPC64__SEGMENT_H__ */
