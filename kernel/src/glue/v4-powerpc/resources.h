/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/resources.h
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
 * $Id: resources.h,v 1.9 2003/09/24 19:04:51 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__RESOURCES_H__
#define __GLUE__V4_POWERPC__RESOURCES_H__

class ppc_resource_bits_t
{
public:
    union {
	struct {
	    word_t copy_area_dst_seg : 4;
	    word_t copy_area : 1;
	    word_t in_kernel_ipc : 1;
	    word_t kernel_thread : 1;
	} x;
	word_t raw;
    };

public:
    bool copy_area_enabled()
    {
	return this->x.copy_area != 0;
    }

    void disable_copy_area()
    {
	this->x.copy_area = 0;
	this->x.copy_area_dst_seg = 0;
    }

    void enable_copy_area( addr_t dst_addr )
    {
	this->x.copy_area_dst_seg = word_t(dst_addr) >> 28;
	this->x.copy_area = 1;
    }

    word_t get_copy_area_dst_seg()
    {
	return this->x.copy_area_dst_seg;
    }

    void set_kernel_ipc()
    {
	this->x.in_kernel_ipc = 1;
    }

    void clr_kernel_ipc()
    {
	this->x.in_kernel_ipc = 0;
    }

    void set_kernel_thread()
    {
	this->x.kernel_thread = 1;
    }
};

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

    addr_t copy_area_address( addr_t addr );
    addr_t copy_area_real_address( tcb_t *src, addr_t addr );
    void setup_copy_area( tcb_t *src, tcb_t *dst, addr_t dst_addr );
    void enable_copy_area( tcb_t *src );
    void disable_copy_area( tcb_t *src );

    void set_kernel_ipc( tcb_t *tcb );
    void clr_kernel_ipc( tcb_t *tcb );

    void set_kernel_thread( tcb_t *tcb );

private:
    void spill_fpu( tcb_t *tcb );
    void restore_fpu( tcb_t *tcb );
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
    word_t fpscr;
    u64_t fpu_state[32];
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
