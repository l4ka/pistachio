/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/resources.h
 * Description:   Resource definitions for ia64
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
 * $Id: resources.h,v 1.9 2004/04/13 06:32:19 cgray Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__RESOURCES_H__
#define __GLUE__V4_IA64__RESOURCES_H__

#include INC_GLUE(context.h)
#include INC_ARCH(fp.h)

#define HAVE_RESOURCE_TYPE_E
enum resource_type_e {
    COPY_AREA	= 0,
    BREAKPOINT	= 1,
    PERFMON	= 2,
    INTERRUPT_THREAD = 3
};

class thread_resources_t : public generic_thread_resources_t
{
    word_t	partner_rid;
    high_fp_t	*high_fp;	/* High floating-point registers */

public:
    void save (tcb_t * tcb, tcb_t * dest);
    void load (tcb_t * tcb);
    void purge (tcb_t * tcb);
    void init (tcb_t * tcb);
    void free (tcb_t * tcb);
    void dump (tcb_t * tcb);

    void enable_copy_area (tcb_t * tcb, tcb_t * partner);
    void disable_copy_area (tcb_t * tcb, bool disable_resource);

    void enable_global_breakpoint (tcb_t * tcb);
    void disable_global_breakpoint (tcb_t * tcb);

    void enable_global_perfmon (tcb_t * tcb);
    void disable_global_perfmon (tcb_t * tcb);

    void handle_disabled_fp (tcb_t * tcb, ia64_exception_context_t * frame);
    void save_fp (void) { high_fp->save (); }
    void load_fp (void) { high_fp->restore (); }
};


#endif /* !__GLUE__V4_IA64__RESOURCES_H__ */
