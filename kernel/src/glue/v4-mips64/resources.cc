/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     glue/v4-mips64/resources.cc
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
 * $Id: resources.cc,v 1.6 2004/04/07 03:09:18 cvansch Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(mips_cpu.h)

#include <kdb/tracepoints.h>

/* Processor specific resources that are handled lazily
 * such as the floating point registers. This keeps a copy
 * of the resource owner for lazy replacement
 */
processor_resources_t processor_resources UNIT("cpulocal");

DECLARE_TRACEPOINT (DISABLED_FPU);

void thread_resources_t::save (tcb_t * tcb)
{
}

void thread_resources_t::load (tcb_t * tcb)
{
}

void thread_resources_t::purge (tcb_t * tcb)
{
}

void thread_resources_t::free (tcb_t * tcb)
{
}

void thread_resources_t::init (tcb_t * tcb)
{
    tcb->resource_bits.init ();

    get_resources()->clear_fp_lazy_tcb();
    this->fpu_fpcsr = 0;
}

INLINE void thread_resources_t::deactivate_fpu( tcb_t *tcb )
{
    get_resources()->clear_fp_lazy_tcb();

    mips64_irq_context_t * context =
	(mips64_irq_context_t *) tcb->get_stack_top () - 1;

    context->status &= ~ST_CU1;
}

INLINE void thread_resources_t::activate_fpu( tcb_t *tcb )
{
    get_resources()->set_fp_lazy_tcb( tcb );
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) tcb->get_stack_top () - 1;

    context->status |= ST_CU1;
}

void thread_resources_t::spill_fpu( tcb_t *tcb )
{
    // Spill the registers.
    u64_t *start = this->fpu_gprs;
    asm volatile (
	    "s.d  $f0,	  0(%0) ;"
	    "s.d  $f1,    8(%0) ;"
	    "s.d  $f2,   16(%0) ;"
	    "s.d  $f3,   24(%0) ;"
	    "s.d  $f4,   32(%0) ;"
	    "s.d  $f5,   40(%0) ;"
	    "s.d  $f6,   48(%0) ;"
	    "s.d  $f7,   56(%0) ;"
	    "s.d  $f8,   64(%0) ;"
	    "s.d  $f9,   72(%0) ;"
	    "s.d  $f10,  80(%0) ;"
	    "s.d  $f11,  88(%0) ;"
	    "s.d  $f12,  96(%0) ;"
	    "s.d  $f13, 104(%0) ;"
	    "s.d  $f14, 112(%0) ;"
	    "s.d  $f15, 120(%0) ;"
	    "s.d  $f16, 128(%0) ;"
	    "s.d  $f17, 136(%0) ;"
	    "s.d  $f18, 144(%0) ;"
	    "s.d  $f19, 152(%0) ;"
	    "s.d  $f20, 160(%0) ;"
	    "s.d  $f21, 168(%0) ;"
	    "s.d  $f22, 176(%0) ;"
	    "s.d  $f23, 184(%0) ;"
	    "s.d  $f24, 192(%0) ;"
	    "s.d  $f25, 200(%0) ;"
	    "s.d  $f26, 208(%0) ;"
	    "s.d  $f27, 216(%0) ;"
	    "s.d  $f28, 224(%0) ;"
	    "s.d  $f29, 232(%0) ;"
	    "s.d  $f30, 240(%0) ;"
	    "s.d  $f31, 248(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (start)
	    );

    // Save the FPCSR
    asm volatile (
	    "cfc1   %0, $31 ;"
	    : /* outputs */
	      "=r" (this->fpu_fpcsr)
	    );

    this->deactivate_fpu( tcb );
}

INLINE void thread_resources_t::restore_fpu( tcb_t *tcb )
{
    this->activate_fpu( tcb );

    // Restore the registers.
    u64_t *start = this->fpu_gprs;
    asm volatile (
	    "l.d  $f0,	  0(%0) ;"
	    "l.d  $f1,    8(%0) ;"
	    "l.d  $f2,   16(%0) ;"
	    "l.d  $f3,   24(%0) ;"
	    "l.d  $f4,   32(%0) ;"
	    "l.d  $f5,   40(%0) ;"
	    "l.d  $f6,   48(%0) ;"
	    "l.d  $f7,   56(%0) ;"
	    "l.d  $f8,   64(%0) ;"
	    "l.d  $f9,   72(%0) ;"
	    "l.d  $f10,  80(%0) ;"
	    "l.d  $f11,  88(%0) ;"
	    "l.d  $f12,  96(%0) ;"
	    "l.d  $f13, 104(%0) ;"
	    "l.d  $f14, 112(%0) ;"
	    "l.d  $f15, 120(%0) ;"
	    "l.d  $f16, 128(%0) ;"
	    "l.d  $f17, 136(%0) ;"
	    "l.d  $f18, 144(%0) ;"
	    "l.d  $f19, 152(%0) ;"
	    "l.d  $f20, 160(%0) ;"
	    "l.d  $f21, 168(%0) ;"
	    "l.d  $f22, 176(%0) ;"
	    "l.d  $f23, 184(%0) ;"
	    "l.d  $f24, 192(%0) ;"
	    "l.d  $f25, 200(%0) ;"
	    "l.d  $f26, 208(%0) ;"
	    "l.d  $f27, 216(%0) ;"
	    "l.d  $f28, 224(%0) ;"
	    "l.d  $f29, 232(%0) ;"
	    "l.d  $f30, 240(%0) ;"
	    "l.d  $f31, 248(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (start)
	    );

    // Restore the FPCSR
    asm volatile (
	    "ctc1   %0, $31 ;"
	    : /* outputs */
	    : /* inputs */
	      "r" (this->fpu_fpcsr)
	    );
}

void thread_resources_t::mips64_fpu_unavail_exception( tcb_t *tcb, mips64_irq_context_t *context )
{
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    TRACEPOINT (DISABLED_FPU,
		printf ("FPU disabled exception:  cur=%p  owner=%p\n",
			tcb, fp_tcb));

    /* In our lazy floating point model, we should never see a floating point
     * exception if the current tcb already owns the floating point register
     * file.
     */
    ASSERT( fp_tcb != tcb );

    mips_cpu::enable_fpu();

    if( fp_tcb )
	fp_tcb->resources.spill_fpu( fp_tcb );

    this->restore_fpu( tcb );
}

void thread_resources_t::mips64_fpu_spill( tcb_t *tcb )
{
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    ASSERT( tcb );

    if (tcb == fp_tcb) {
	mips_cpu::enable_fpu();
	fp_tcb->resources.spill_fpu( fp_tcb );
    }
}
