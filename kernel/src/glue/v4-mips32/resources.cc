/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/resources.cc
 * Description:   MIPS32 thread resources
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
 * $Id: resources.cc,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(resources.h)
#include INC_ARCH(cp0regs.h)
#include INC_ARCH(mips_cpu.h)
#include <linear_ptab.h>


void thread_resources_t::save (tcb_t * tcb) {

    if( tcb->resource_bits.have_resource( COPY_AREA ) ) {
		
        pgent_t* pdir = tcb->space->get_pdir();
        word_t idx = page_table_index( pgent_t::size_4m, (addr_t)COPY_AREA_START );

        pdir[idx].raw = 0;
        pdir[idx+1].raw = 0;

        // XXX Flush tlb
        get_tlb()->flush( (word_t)tcb->space->get_asid()->get() );
    }
}


void thread_resources_t::load (tcb_t * tcb) {

    if (tcb->resource_bits.have_resource (COPY_AREA)) {
		
        tcb_t* partner = tcb->get_partner_tcb();
	
        pgent_t* src_pdir = tcb->space->get_pdir();
        pgent_t* dst_pdir = partner->space->get_pdir();

        word_t dst_idx = page_table_index( pgent_t::size_4m, (addr_t)this->copy_area );
        word_t src_idx = page_table_index( pgent_t::size_4m, (addr_t)COPY_AREA_START );

        src_pdir[src_idx].raw = dst_pdir[dst_idx].raw;
        src_pdir[src_idx+1].raw = dst_pdir[dst_idx+1].raw;
    }
}

void thread_resources_t::purge (tcb_t * tcb) {

}


void thread_resources_t::dump (tcb_t * tcb) {

}


void thread_resources_t::free (tcb_t * tcb) {

}


void thread_resources_t::init (tcb_t * tcb) {

    tcb->resource_bits.init();
    //get_resources()->clear_fp_lazy_tcb();
    //this->fpu_fpcsr = 0;
}


addr_t thread_resources_t::copy_area_real_address( addr_t addr ) {

    return (addr_t)( this->copy_area + (((word_t)(addr)) & 0x003fffff) );
}			


void thread_resources_t::enable_copy_area( tcb_t* src_tcb, addr_t* src_addr, tcb_t* dst_tcb, addr_t* dst_addr ) {
	
    if (src_tcb->resource_bits.have_resource (COPY_AREA)) {
        ASSERT(!"thread_resources_t::enable_copy_area called unexpectedly");
    }
    pgent_t* src_pdir = src_tcb->space->get_pdir();
    pgent_t* dst_pdir = dst_tcb->space->get_pdir();
	
    word_t dst_idx = page_table_index( pgent_t::size_4m, *dst_addr );
    word_t src_idx = page_table_index( pgent_t::size_4m, (addr_t)COPY_AREA_START );
    ASSERT( src_idx == 510 + 2 + 256 );

    this->copy_area = ((word_t)(*dst_addr)) & 0xffc00000;

    // set up temp mapping in source address space
	
    // XXX was wenn dest pdir eintrag noch nicht existiert ??	
    src_pdir[src_idx].raw = dst_pdir[dst_idx].raw;
    src_pdir[src_idx+1].raw = dst_pdir[dst_idx+1].raw;
	
    *dst_addr = (addr_t*)(COPY_AREA_START + (((word_t)(*dst_addr)) & 0x003fffff) );

    src_tcb->resource_bits += COPY_AREA;
}


void thread_resources_t::release_copy_area( tcb_t* tcb ) {

    if (tcb->resource_bits.have_resource (COPY_AREA)) {

        pgent_t* pdir = tcb->space->get_pdir();
        word_t idx = page_table_index( pgent_t::size_4m, (addr_t)COPY_AREA_START );

        pdir[idx].raw = 0;
        pdir[idx+1].raw = 0;

        // XXX Flush tlb
        get_tlb()->flush( (word_t)tcb->space->get_asid()->get() );
        tcb->resource_bits -= COPY_AREA;
    }
}
 

#if 0
// FPU support not tested
INLINE void thread_resources_t::deactivate_fpu( tcb_t *tcb ) {

    //get_resources()->clear_fp_lazy_tcb();
    //mips32_irq_context_t* context = (mips32_irq_context_t *)tcb->get_stack_top()-1;
    //context->status &= ~ST_CU1;
}


INLINE void thread_resources_t::activate_fpu( tcb_t *tcb ) {

    //get_resources()->set_fp_lazy_tcb( tcb );
    //mips32_irq_context_t * context = (mips32_irq_context_t *)tcb->get_stack_top()-1;
    //context->status |= ST_CU1;
}


void thread_resources_t::spill_fpu( tcb_t *tcb ) {
	
    word_t* start = this->fpu_gprs;
   
    __asm__ __volatile__ (
        "s.s  $f0,	0(%0);\n\t"
        "s.s  $f1,  4(%0);\n\t"
        "s.s  $f2,  8(%0);\n\t"
        "s.s  $f3,  12(%0);\n\t"
        "s.s  $f4,  16(%0);\n\t"
        "s.s  $f5,  20(%0);\n\t"
        "s.s  $f6,  24(%0);\n\t"
        "s.s  $f7,  28(%0);\n\t"
        "s.s  $f8,  32(%0);\n\t"
        "s.s  $f9,  36(%0);\n\t"
        "s.s  $f10, 40(%0);\n\t"
        "s.s  $f11, 44(%0);\n\t"
        "s.s  $f12, 48(%0);\n\t"
        "s.s  $f13, 52(%0);\n\t"
        "s.s  $f14, 56(%0);\n\t"
        "s.s  $f15, 60(%0);\n\t"
        "s.s  $f16, 64(%0);\n\t"
        "s.s  $f17, 68(%0);\n\t"
        "s.s  $f18, 72(%0);\n\t"
        "s.s  $f19, 76(%0);\n\t"
        "s.s  $f20, 80(%0);\n\t"
        "s.s  $f21, 84(%0);\n\t"
        "s.s  $f22, 88(%0);\n\t"
        "s.s  $f23, 92(%0);\n\t"
        "s.s  $f24, 96(%0);\n\t"
        "s.s  $f25, 100(%0);\n\t"
        "s.s  $f26, 104(%0);\n\t"
        "s.s  $f27, 108(%0);\n\t"
        "s.s  $f28, 112(%0);\n\t"
        "s.s  $f29, 116(%0);\n\t"
        "s.s  $f30, 120(%0);\n\t"
        "s.s  $f31, 124(%0);"
        ::"b" (start)
        );

    // Save the FPCSR
    __asm__ __volatile__ (
        "cfc1   %0, $31 ;"
        : "=r" (this->fpu_fpcsr)
	);

    this->deactivate_fpu( tcb );
}


INLINE void thread_resources_t::restore_fpu( tcb_t *tcb )
{
    this->activate_fpu( tcb );

    word_t *start = this->fpu_gprs;

    __asm__ __volatile__ (
        "l.s  $f0,	0(%0);\n\t"
        "l.s  $f1,  4(%0);\n\t"
        "l.s  $f2,  8(%0);\n\t"
        "l.s  $f3,  12(%0);\n\t"
        "l.s  $f4,  16(%0);\n\t"
        "l.s  $f5,  20(%0);\n\t"
        "l.s  $f6,  24(%0);\n\t"
        "l.s  $f7,  28(%0);\n\t"
        "l.s  $f8,  32(%0);\n\t"
        "l.s  $f9,  36(%0);\n\t"
        "l.s  $f10, 40(%0);\n\t"
        "l.s  $f11, 44(%0);\n\t"
        "l.s  $f12, 48(%0);\n\t"
        "l.s  $f13, 52(%0);\n\t"
        "l.s  $f14, 56(%0);\n\t"
        "l.s  $f15, 60(%0);\n\t"
        "l.s  $f16, 64(%0);\n\t"
        "l.s  $f17, 68(%0);\n\t"
        "l.s  $f18, 72(%0);\n\t"
        "l.s  $f19, 76(%0);\n\t"
        "l.s  $f20, 80(%0);\n\t"
        "l.s  $f21, 84(%0);\n\t"
        "l.s  $f22, 88(%0);\n\t"
        "l.s  $f23, 92(%0);\n\t"
        "l.s  $f24, 96(%0);\n\t"
        "l.s  $f25, 100(%0);\n\t"
        "l.s  $f26, 104(%0);\n\t"
        "l.s  $f27, 108(%0);\n\t"
        "l.s  $f28, 112(%0);\n\t"
        "l.s  $f29, 116(%0);\n\t"
        "l.s  $f30, 120(%0);\n\t"
        "l.s  $f31, 124(%0);"
        : : "b" (start)
	);

    // Restore the FPCSR
    __asm__ __volatile__ (
        "ctc1   %0, $31 ;"
        : : "r" (this->fpu_fpcsr)
	);
}


void thread_resources_t::mips32_fpu_unavail_exception( tcb_t *tcb, mips32_irq_context_t* context ) {
	
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    /* In our lazy floating point model, we should never see a floating point
     * exception if the current tcb already owns the floating point register
     * file.
     */
    ASSERT( fp_tcb != tcb );

    get_mips_cpu()->enable_fpu();

    if( fp_tcb ) {
        fp_tcb->resources.spill_fpu( fp_tcb );
    }

    this->restore_fpu( tcb );
}


void thread_resources_t::mips32_fpu_spill( tcb_t *tcb ) {
	
    tcb_t * fp_tcb = get_resources()->get_fp_lazy_tcb();

    ASSERT( tcb );

    if( tcb == fp_tcb ) {
	get_mips_cpu()->enable_fpu();
        fp_tcb->resources.spill_fpu( fp_tcb );
    }
}
#endif
