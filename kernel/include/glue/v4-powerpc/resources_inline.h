/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/resources_inline.h
 * Description:   powerpc specific resources
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
 * $Id: resources_inline.h,v 1.7 2003/09/24 19:04:51 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__RESOURCES_INLINE_H__
#define __GLUE__V4_POWERPC__RESOURCES_INLINE_H__

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

INLINE addr_t thread_resources_t::copy_area_address( addr_t addr )
{
    addr = change_segment( addr, COPY_AREA_SEGMENT );
    return addr;
}

INLINE addr_t thread_resources_t::copy_area_real_address( tcb_t *src, addr_t addr )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&src->resource_bits;

    addr =  change_segment( addr, bits->get_copy_area_dst_seg() );
    return addr;
}

INLINE void thread_resources_t::setup_copy_area( tcb_t *src, tcb_t *dst, 
	addr_t dst_addr )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&src->resource_bits;
    bits->enable_copy_area( dst_addr );
}

INLINE void thread_resources_t::enable_copy_area( tcb_t *src )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&src->resource_bits;

    tcb_t *partner = src->get_space()->get_tcb( src->get_partner() );
    ppc_segment_t partner_seg = partner->get_space()->get_segment_id();

    // Change the copy area segment register to point into the target space.
    isync();
    ppc_set_sr( COPY_AREA_SEGMENT, 
	    partner_seg.raw | bits->get_copy_area_dst_seg() );
    isync();
}

INLINE void thread_resources_t::disable_copy_area( tcb_t *tcb )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&tcb->resource_bits;

    bits->disable_copy_area();
}

INLINE void thread_resources_t::set_kernel_ipc( tcb_t *tcb )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&tcb->resource_bits;

    bits->set_kernel_ipc();
}

INLINE void thread_resources_t::clr_kernel_ipc( tcb_t *tcb )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&tcb->resource_bits;

    bits->clr_kernel_ipc();
}

INLINE void thread_resources_t::set_kernel_thread( tcb_t *tcb )
{
    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&tcb->resource_bits;

    bits->set_kernel_thread();
}

#endif /* !__GLUE__V4_POWERPC__RESOURCES_H__ */
