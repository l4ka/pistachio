/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/resources.cc
 * Description:   thread resource management
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
 * $Id: resources.cc,v 1.9 2003/11/01 13:40:08 joshua Exp $
 *                
 ********************************************************************/

#include INC_API(tcb.h)

// TODO: ensure that this is initialized to NULL for each cpu, perhaps via 
// ctors.
tcb_t *_fp_lazy_tcb UNIT("cpulocal");

INLINE void thread_resources_t::deactivate_fpu( tcb_t *tcb )
{
    set_fp_lazy_tcb( NULL );
    // Disable fpu access for the tcb.
    except_regs_t *regs = get_user_except_regs(tcb);
    regs->srr1_flags = MSR_CLR( regs->srr1_flags, MSR_FP );
}

INLINE void thread_resources_t::activate_fpu( tcb_t *tcb )
{
    set_fp_lazy_tcb( tcb );
    // Enable fpu access for the tcb.
    except_regs_t *regs = get_user_except_regs(tcb);
    regs->srr1_flags = MSR_SET( regs->srr1_flags, MSR_FP );
}

void thread_resources_t::dump (tcb_t * tcb)
{
}

void thread_resources_t::save( tcb_t *tcb )
{
}

void thread_resources_t::load( tcb_t *tcb )
{
    if( EXPECT_TRUE(tcb->resource_bits == 0) )
	return;

    ppc_resource_bits_t *bits = (ppc_resource_bits_t *)&tcb->resource_bits;
    if( bits->copy_area_enabled() )
	this->enable_copy_area( tcb );
}

void thread_resources_t::purge( tcb_t *tcb )
{
    if( get_fp_lazy_tcb() == tcb )
	this->spill_fpu( tcb );
}

void thread_resources_t::init( tcb_t *tcb )
{
    tcb->resource_bits = 0;
    this->fpscr = 0;	// TODO: seed with an appropriate value!
}

void thread_resources_t::free( tcb_t *tcb )
{
    if( get_fp_lazy_tcb() == tcb )
	this->deactivate_fpu( tcb );
}

void thread_resources_t::spill_fpu( tcb_t *tcb )
{
    // Spill the registers.
    u64_t *start = this->fpu_state;
    asm volatile (
	    "stfd %%f0,    0(%0) ;"
	    "stfd %%f1,    8(%0) ;"
	    "stfd %%f2,   16(%0) ;"
	    "stfd %%f3,   24(%0) ;"
	    "stfd %%f4,   32(%0) ;"
	    "stfd %%f5,   40(%0) ;"
	    "stfd %%f6,   48(%0) ;"
	    "stfd %%f7,   56(%0) ;"
	    "stfd %%f8,   64(%0) ;"
	    "stfd %%f9,   72(%0) ;"
	    "stfd %%f10,  80(%0) ;"
	    "stfd %%f11,  88(%0) ;"
	    "stfd %%f12,  96(%0) ;"
	    "stfd %%f13, 104(%0) ;"
	    "stfd %%f14, 112(%0) ;"
	    "stfd %%f15, 120(%0) ;"
	    "stfd %%f16, 128(%0) ;"
	    "stfd %%f17, 136(%0) ;"
	    "stfd %%f18, 144(%0) ;"
	    "stfd %%f19, 152(%0) ;"
	    "stfd %%f20, 160(%0) ;"
	    "stfd %%f21, 168(%0) ;"
	    "stfd %%f22, 176(%0) ;"
	    "stfd %%f23, 184(%0) ;"
	    "stfd %%f24, 192(%0) ;"
	    "stfd %%f25, 200(%0) ;"
	    "stfd %%f26, 208(%0) ;"
	    "stfd %%f27, 216(%0) ;"
	    "stfd %%f28, 224(%0) ;"
	    "stfd %%f29, 232(%0) ;"
	    "stfd %%f30, 240(%0) ;"
	    "stfd %%f31, 248(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (start)
	    );

    /* Spill the fpscr.  Temporarily store it to an 8-byte location,
     * so that we can store it as a double and avoid an fp-double to
     * fp-single conversion.  Then we move it to our 4-byte tcb location.
     */
    u64_t fpscr_tmp;
    asm volatile (
	    "mffs %%f0 ;"
	    "stfd %%f0, 0(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (&fpscr_tmp)
	    );
    this->fpscr = (word_t)fpscr_tmp;

    this->deactivate_fpu( tcb );
}

void thread_resources_t::restore_fpu( tcb_t *tcb )
{
    /* Restore the fpscr.  We store it in the tcb as a 4-byte quantity,
     * but we need to load it as a floating point double to prevent
     * conversion from fp-single to fp-double.  So temporarily store
     * it in a 8-byte location.
     */
    u64_t fpscr_tmp = this->fpscr;
    asm volatile (
	    "lfd %%f0, 0(%0) ;"
	    "mtfsf 0xff, %%f0 ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (&fpscr_tmp)
	    );

    // Load the registers.
    u64_t *start = this->fpu_state;
    asm volatile (
	    "lfd %%f0,    0(%0) ;"
	    "lfd %%f1,    8(%0) ;"
	    "lfd %%f2,   16(%0) ;"
	    "lfd %%f3,   24(%0) ;"
	    "lfd %%f4,   32(%0) ;"
	    "lfd %%f5,   40(%0) ;"
	    "lfd %%f6,   48(%0) ;"
	    "lfd %%f7,   56(%0) ;"
	    "lfd %%f8,   64(%0) ;"
	    "lfd %%f9,   72(%0) ;"
	    "lfd %%f10,  80(%0) ;"
	    "lfd %%f11,  88(%0) ;"
	    "lfd %%f12,  96(%0) ;"
	    "lfd %%f13, 104(%0) ;"
	    "lfd %%f14, 112(%0) ;"
	    "lfd %%f15, 120(%0) ;"
	    "lfd %%f16, 128(%0) ;"
	    "lfd %%f17, 136(%0) ;"
	    "lfd %%f18, 144(%0) ;"
	    "lfd %%f19, 152(%0) ;"
	    "lfd %%f20, 160(%0) ;"
	    "lfd %%f21, 168(%0) ;"
	    "lfd %%f22, 176(%0) ;"
	    "lfd %%f23, 184(%0) ;"
	    "lfd %%f24, 192(%0) ;"
	    "lfd %%f25, 200(%0) ;"
	    "lfd %%f26, 208(%0) ;"
	    "lfd %%f27, 216(%0) ;"
	    "lfd %%f28, 224(%0) ;"
	    "lfd %%f29, 232(%0) ;"
	    "lfd %%f30, 240(%0) ;"
	    "lfd %%f31, 248(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (start)
	    );

    this->activate_fpu( tcb );
}

