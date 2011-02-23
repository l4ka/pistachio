/*********************************************************************
 *                
 * Copyright (C) 1999-2011,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/resources.h
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
#ifndef __GLUE__V4_POWERPC__RESOURCES_H__
#define __GLUE__V4_POWERPC__RESOURCES_H__

#define HAVE_RESOURCE_TYPE_E
enum resource_type_e {
    KERNEL_THREAD	= 0, /* disables fast path */
    KERNEL_IPC		= 1, /* disables fast path */
    FPU			= 2,
    COPY_AREA		= 3,
    SOFTHVM		= 4,
};


#define FPU_REGS	32

/* support for extended floating point state like AltiVec or Double Hummer */
#ifdef CONFIG_SUBPLAT_440_BGP
#define FPU_EXTRA_REGS	32
#else
#define FPU_EXTRA_REGS	0
#endif

class thread_resources_t : public generic_thread_resources_t
{
public:
    void dump(tcb_t * tcb);
    void save( tcb_t *tcb );
    void load( tcb_t *tcb );
    void purge( tcb_t *tcb );
    void init( tcb_t *tcb );
    void free( tcb_t *tcb );

public:
    void fpu_unavail_exception( tcb_t *tcb );

    addr_t copy_area_real_address( tcb_t *src, addr_t addr );
    void setup_copy_area( tcb_t *src, addr_t *saddr, tcb_t *dst, addr_t *daddr);
    void enable_copy_area( tcb_t *src );
    void disable_copy_area( tcb_t *src );
    void flush_copy_area( tcb_t *src );

    void set_kernel_ipc( tcb_t *tcb );
    void clr_kernel_ipc( tcb_t *tcb );
    void set_kernel_thread( tcb_t *tcb );

#ifdef CONFIG_X_PPC_SOFTHVM
    void enable_hvm_mode( tcb_t *tcb);
    void disable_hvm_mode( tcb_t *tcb);
#endif

    void spill_fpu( tcb_t *tcb );
    void restore_fpu( tcb_t *tcb );
    void reown_fpu( tcb_t *tcb, tcb_t *new_owner );

private:
    void deactivate_fpu( tcb_t *tcb );
    void activate_fpu( tcb_t *tcb );

    addr_t change_segment( addr_t addr, word_t segment )
    {
	word_t tmp = (word_t)addr;
	tmp &= 0x0fffffff;
	tmp |= segment << 28;
	return (addr_t)tmp;
    }

private:
#ifdef CONFIG_X_PPC_SOFTHVM
    static tcb_t *last_hvm_tcb; 
#endif
    word_t copy_area_offset;
    word_t fpscr;
    u64_t fpu_state[FPU_REGS + FPU_EXTRA_REGS] __attribute__((aligned(16)));
};


INLINE tcb_t *get_fp_lazy_tcb()
{
    extern tcb_t *_fp_lazy_tcb;
    return _fp_lazy_tcb;
}

INLINE void set_fp_lazy_tcb( tcb_t *tcb )
{
    extern tcb_t *_fp_lazy_tcb;
    _fp_lazy_tcb = tcb;
}


#endif /* !__GLUE__V4_POWERPC__RESOURCES_H__ */
