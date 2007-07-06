/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/intctrl.cc
 * Description:   Implementation of interrupt control functionality 
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
 * $Id: intctrl.cc,v 1.16 2004/04/05 06:21:53 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(intctrl.h)
#include INC_ARCH(addrspace.h)
#include INC_ARCH(mips_cpu.h)
#include INC_PLAT(cache.h)

/* XXX - Should be in a header file. Need to provide an optimised version */
extern "C" void *memcpy( void *dst, const void *src, word_t size );

intctrl_t intctrl;

word_t exception_handlers[32];
word_t interrupt_handlers[8];

extern "C" void spurious_interrupt(word_t irq, mips64_irq_context_t * frame)
{
    printf("L4 Mips: Spurious interrupt %d\n", irq);
    enter_kdebug("Spurious interrupt");
}

extern "C" void intctrl_t_handle_irq(word_t irq, mips64_irq_context_t * frame)
{
    get_interrupt_ctrl()->mask(irq);
    handle_interrupt(irq);
}

/**
 * Setup MIPS exception vector jump table
 */
static void SECTION (".init")
setup_exception_vectors()
{
    extern void (*_mips64_interrupt);
    extern void (*_mips64_tlb_mod);
    extern void (*_mips64_stlb_miss);
    extern void (*_mips64_l4syscall);

    get_interrupt_ctrl()->register_exception_handler(0, &_mips64_interrupt);
    get_interrupt_ctrl()->register_exception_handler(1, &_mips64_tlb_mod);

    get_interrupt_ctrl()->register_exception_handler(2, &_mips64_stlb_miss);
    get_interrupt_ctrl()->register_exception_handler(3, &_mips64_stlb_miss);

    get_interrupt_ctrl()->register_exception_handler(8, &_mips64_l4syscall);

    get_interrupt_ctrl()->register_interrupt_handler(2, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(3, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(4, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(5, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(6, intctrl_t_handle_irq);
    cache_t::flush_cache_all();
}

#if CONFIG_IPC_FASTPATH
#define __mips64_interrupt __mips64_interrupt_fp
#endif

/**
 * Setup the MIPS architecture interrupts
 */
void SECTION (".init")
intctrl_t::init_arch(void) 
{
    unsigned int i;
    /* declare assembly functions */
    extern char __mips64_tlb_refill;
    extern char __mips64_xtlb_refill;
    extern char __mips64_cache_error;
    extern char __mips64_interrupt;
    extern char __mips64_extra_vector;
    extern void (*_mips64_exception);

#if CONFIG_IPC_FASTPATH
    TRACE_INIT("init_arch: MIPS64 using FASTPATH\n");
#endif

    mips_cpu::cli();

    /* Copy the MIPS exception vectors to KSEG0 0xFFFFFFFF80000000  */
    memcpy((void *)(KSEG0), &__mips64_tlb_refill, 0x80);
    memcpy((void *)(KSEG0 + 0x080), &__mips64_xtlb_refill, 0x80);
    memcpy((void *)(KSEG0 + 0x100), &__mips64_cache_error, 0x80);
    memcpy((void *)(KSEG0 + 0x180), &__mips64_interrupt, 0x80);
    /* Some MIPS CPU's have an extra vector for interrupts */
    memcpy((void *)(KSEG0 + 0x200), &__mips64_extra_vector, 0x80);

    cache_t::flush_cache_all();

    for (i=0; i<32; i++)
	exception_handlers[i] = (word_t)&_mips64_exception;
    for (i=0; i<8; i++)
	interrupt_handlers[i] = (word_t)spurious_interrupt;

    setup_exception_vectors();
}

void SECTION (".init")
intctrl_t::init_cpu(void) 
{
    /* Mask out all interrupts */
    mips_cpu::clear_cp0_status(ST_IM);
    get_idle_tcb()->arch.int_mask = 0;

    /* Clear BEV: set vector base to 0xFFFFFFFF80000000 */
    mips_cpu::clear_cp0_status(ST_BEV);
}

