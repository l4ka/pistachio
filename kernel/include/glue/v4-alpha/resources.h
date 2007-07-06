/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/resources.h
 * Created:       17/09/2002 17:01:55 by Simon Winwood (sjw)
 * Description:   CPU resources 
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
 * $Id: resources.h,v 1.5 2004/06/04 02:27:51 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_ALPHA__RESOURCES_H__
#define __GLUE__V4_ALPHA__RESOURCES_H__

class thread_resources_t : public generic_thread_resources_t 
{
public:
    void dump(tcb_t * tcb);
    void save(tcb_t * tcb);
    void load(tcb_t * tcb);
    void purge(tcb_t * tcb);
    void init(tcb_t * tcb);
    void free(tcb_t * tcb);

public:
    void alpha_fpu_unavail_exception( tcb_t *tcb );

private:
    void spill_fpu( tcb_t *tcb );
    void restore_fpu( tcb_t *tcb );
    void deactivate_fpu( tcb_t *tcb );
    void activate_fpu( tcb_t *tcb );

private:
    u64_t fpu_state[32]; /* 31 FPRs and 1 FPSCR */
};

class processor_resources_t {
 public:
    void init_cpu(void) {
	fp_lazy_tcb = NULL;
    }

 public:
    tcb_t *get_fp_lazy_tcb() {
	return fp_lazy_tcb;
    }

    void set_fp_lazy_tcb( tcb_t *tcb ) {
	fp_lazy_tcb = tcb;
    }

    void clear_fp_lazy_tcb() {
	fp_lazy_tcb = NULL;
    }
    
 private:
    tcb_t *fp_lazy_tcb;
};

INLINE processor_resources_t *get_resources(void) 
{
    extern processor_resources_t processor_resources;
    return &processor_resources;
}

#endif /* __GLUE__V4_ALPHA__RESOURCES_H__ */
