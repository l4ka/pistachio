/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/resource_functions.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_POWERPC__RESOURCE_FUNCTIONS_H__
#include INC_API(resources.h)

INLINE void thread_resources_t::fpu_unavail_exception( tcb_t *tcb )
{
    tcb_t *fp_tcb = get_fp_lazy_tcb();

    /* In our lazy floating point model, we should never see a floating point
     * exception if the current tcb already owns the floating point register
     * file.
     */
    ASSERT( fp_tcb != tcb );

    if( fp_tcb )
	fp_tcb->resources.spill_fpu( fp_tcb );
    this->restore_fpu( tcb );
}

INLINE addr_t thread_resources_t::copy_area_real_address( tcb_t *src, addr_t addr )
{
    return addr_offset(addr, copy_area_offset - COPY_AREA_START);
}

INLINE void thread_resources_t::setup_copy_area( tcb_t *src, addr_t *saddr, 
						 tcb_t *dst, addr_t *daddr )
{
    copy_area_offset = (word_t)*daddr & ~(COPY_AREA_SIZE - 1);
    *daddr = addr_offset((addr_t)COPY_AREA_START, (word_t)*daddr & (COPY_AREA_SIZE - 1));
    //TRACEF("copy area: offset: %08x, dst: %p\n", copy_area_offset, *daddr);
    src->resource_bits += COPY_AREA;
}

#ifdef CONFIG_PPC_MMU_SEGMENTS
INLINE void thread_resources_t::enable_copy_area( tcb_t *src )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&src->resource_bits;

    tcb_t *partner = tcb_t::get_tcb( src->get_partner() );
    ppc_segment_t partner_seg = partner->get_space()->get_segment_id();

    // Change the copy area segment register to point into the target space.
#warning VU: copy area code is inorrect for tunnelled PFs
    isync();
    ppc_set_sr( COPY_AREA_SEGMENT, 
		partner_seg.raw | bits->get_copy_area_dst_seg() );
    isync();
}
INLINE void thread_resources_t::flush_copy_area( tcb_t *tcb ) { }

#elif defined(CONFIG_PPC_MMU_TLB)
INLINE void thread_resources_t::enable_copy_area( tcb_t *src ) { }
INLINE void thread_resources_t::flush_copy_area( tcb_t *tcb )
{
    space_t *space = tcb->get_space();
    space->flush_tlb(space, (addr_t)COPY_AREA_START, (addr_t)COPY_AREA_END);
}
#endif

INLINE void thread_resources_t::disable_copy_area( tcb_t *tcb )
{
    if (tcb->resource_bits.have_resource(COPY_AREA))
    {
	//TRACEF("disable copy area\n");
	flush_copy_area(tcb);
	tcb->resource_bits -= COPY_AREA;
    }
}

INLINE void thread_resources_t::set_kernel_ipc( tcb_t *tcb )
{
    tcb->resource_bits += KERNEL_IPC;
}

INLINE void thread_resources_t::clr_kernel_ipc( tcb_t *tcb )
{
    tcb->resource_bits -= KERNEL_IPC;
}

INLINE void thread_resources_t::set_kernel_thread( tcb_t *tcb )
{
    tcb->resource_bits += KERNEL_THREAD;
}


#endif /* !__GLUE__V4_POWERPC__RESOURCE_FUNCTIONS_H__ */
