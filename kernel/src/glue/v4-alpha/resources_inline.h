/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/resources_inline.h
 * Description:   inline routines for resources.
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
 * $Id: resources_inline.h,v 1.3 2004/06/04 02:27:51 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_ALPHA__RESOURCES_INLINE_H__
#define __GLUE__V4_ALPHA__RESOURCES_INLINE_H__

#include INC_API(resources.h)

INLINE void thread_resources_t::alpha_fpu_unavail_exception( tcb_t *tcb )
{
    tcb_t *fp_tcb = get_resources()->get_fp_lazy_tcb();

    /* In our lazy floating point model, we should never see a floating point
     * exception if the current tcb already owns the floating point register
     * file.
     */
    ASSERT( fp_tcb != tcb );

    if( fp_tcb )
	fp_tcb->resources.spill_fpu( fp_tcb );

    this->restore_fpu( tcb );
}

#endif /* !__GLUE__V4_ALPHA__RESOURCES_H__ */
