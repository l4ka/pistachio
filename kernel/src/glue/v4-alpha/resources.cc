/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  The University of New South Wales
 *                
 * File path:     glue/v4-alpha/resources.cc
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
 * $Id: resources.cc,v 1.2 2003/10/21 07:50:22 sjw Exp $
 *
 * Derived from glue/v4-powerpc/resources.cc
 *                
 ********************************************************************/

#include INC_API(tcb.h)

processor_resources_t processor_resources UNIT("cpulocal");

INLINE void thread_resources_t::deactivate_fpu( tcb_t *tcb )
{
    get_resources()->clear_fp_lazy_tcb();

    if(get_current_tcb() == tcb) {
	PAL::wrfen(0);
    } else {
	tcb->get_arch()->pcb.clear_fen();
    }
}

INLINE void thread_resources_t::activate_fpu( tcb_t *tcb )
{
    get_resources()->set_fp_lazy_tcb(tcb);

    if(get_current_tcb() == tcb) {
	PAL::wrfen(1);
    } else {
	tcb->get_arch()->pcb.set_fen();
    }
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
}

void thread_resources_t::purge( tcb_t *tcb )
{
    if( get_resources()->get_fp_lazy_tcb() == tcb )
	this->spill_fpu( tcb );
}

void thread_resources_t::init( tcb_t *tcb )
{
    tcb->resource_bits = 0;
}

void thread_resources_t::free( tcb_t *tcb )
{
    if( get_resources()->get_fp_lazy_tcb() == tcb )
	this->deactivate_fpu( tcb );
}

void thread_resources_t::spill_fpu( tcb_t *tcb )
{
    // Spill the registers.
    u64_t *start = this->fpu_state;

    /* HACK ALERT */
    PAL::wrfen(1);

    asm volatile (
	"stt     $f0,0(%0)\n\t"
	"stt     $f1,8(%0)\n\t"
	"stt     $f2,16(%0)\n\t"
	"stt     $f3,24(%0)\n\t"
	"stt     $f4,32(%0)\n\t"
	"stt     $f5,40(%0)\n\t"
	"stt     $f6,48(%0)\n\t"
	"stt     $f7,56(%0)\n\t"
	"stt     $f8,64(%0)\n\t"
	"stt     $f9,72(%0)\n\t"
	"stt     $f10,80(%0)\n\t"
	"stt     $f11,88(%0)\n\t"
	"stt     $f12,96(%0)\n\t"
	"stt     $f13,104(%0)\n\t"
	"stt     $f14,112(%0)\n\t"
	"stt     $f15,120(%0)\n\t"
	"stt     $f16,128(%0)\n\t"
	"stt     $f17,136(%0)\n\t"
	"stt     $f18,144(%0)\n\t"
	"stt     $f19,152(%0)\n\t"
	"stt     $f20,160(%0)\n\t"
	"stt     $f21,168(%0)\n\t"
	"stt     $f22,176(%0)\n\t"
	"stt     $f23,184(%0)\n\t"
	"stt     $f24,192(%0)\n\t"
	"stt     $f25,200(%0)\n\t"
	"stt     $f26,208(%0)\n\t"
	"stt     $f27,216(%0)\n\t"
	"stt     $f28,224(%0)\n\t" 
	"stt     $f29,232(%0)\n\t"
	"stt     $f30,240(%0)\n\t"
        "mf_fpcr $f0\n\t"
	"stt     $f0,248(%0)\n\t"
	: /* ouputs */
	: /* inputs */
	"r" (start)
	);

    PAL::wrfen(0); /* Could probably leave this be ... */

    this->deactivate_fpu( tcb );
}

void thread_resources_t::restore_fpu( tcb_t *tcb )
{
    // Load the registers.
    u64_t *start = this->fpu_state;

    this->activate_fpu( tcb );

    asm volatile (
	"ldt     $f0,248(%0)\n\t"
        "mt_fpcr $f0\n\t"
	"ldt     $f0,0(%0)\n\t"
	"ldt     $f1,8(%0)\n\t"
	"ldt     $f2,16(%0)\n\t"
	"ldt     $f3,24(%0)\n\t"
	"ldt     $f4,32(%0)\n\t"
	"ldt     $f5,40(%0)\n\t"
	"ldt     $f6,48(%0)\n\t"
	"ldt     $f7,56(%0)\n\t"
	"ldt     $f8,64(%0)\n\t"
	"ldt     $f9,72(%0)\n\t"
	"ldt     $f10,80(%0)\n\t"
	"ldt     $f11,88(%0)\n\t"
	"ldt     $f12,96(%0)\n\t"
	"ldt     $f13,104(%0)\n\t"
	"ldt     $f14,112(%0)\n\t"
	"ldt     $f15,120(%0)\n\t"
	"ldt     $f16,128(%0)\n\t"
	"ldt     $f17,136(%0)\n\t"
	"ldt     $f18,144(%0)\n\t"
	"ldt     $f19,152(%0)\n\t"
	"ldt     $f20,160(%0)\n\t"
	"ldt     $f21,168(%0)\n\t"
	"ldt     $f22,176(%0)\n\t"
	"ldt     $f23,184(%0)\n\t"
	"ldt     $f24,192(%0)\n\t"
	"ldt     $f25,200(%0)\n\t"
	"ldt     $f26,208(%0)\n\t"
	"ldt     $f27,216(%0)\n\t"
	"ldt     $f28,224(%0)\n\t" 
	"ldt     $f29,232(%0)\n\t"
	"ldt     $f30,240(%0)\n\t"
	: /* ouputs */
	: /* inputs */
	"r" (start)
	);
}

