/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/intctrl.cc
 * Description:   MIPS32 interrupt controller implementation
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
 * $Id: intctrl.cc,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include INC_GLUE(intctrl.h)
#include INC_GLUE(config.h)
#include <generic/lib.h>

intctrl_t intctrl;

word_t exception_handlers[32];
word_t interrupt_handlers[8];


extern "C" void spurious_interrupt( word_t irq, mips32_irq_context_t* frame ) {

    printf("L4 Mips: Spurious interrupt %d\n", irq);
    enter_kdebug("Spurious interrupt");
}


extern "C" void intctrl_t_handle_irq(word_t irq, mips32_irq_context_t* frame) {

    intctrl_t::mask(irq);
    handle_interrupt(irq);
}


void SECTION (".init") intctrl_t::init_intctrl() {

    extern char	__general_except, __general_except_end;
    extern char	__tlbmiss_except, __tlbmiss_except_end;
    extern char	__extra_except, __extra_except_end;

    extern char	mips32_exception;
    extern char	mips32_interrupt;
    extern char	mips32_l4syscall;
    extern char	mips32_tlb_invalid;
    extern char	mips32_tlb_mod;

    //ASSERT( (word_t)(&__tlbmiss_except_end - &__tlbmiss_except) <= 0x80 && "Exception vector exceeds 0x80 bytes" );
    //ASSERT( (word_t)(&__general_except_end - &__general_except) <= 0x80 && "Exception vector exceeds 0x80 bytes" );
    //ASSERT( (word_t)(&__extra_except_end - &__extra_except) <= 0x80 && "Exception vector exceeds 0x80 bytes" );

    /* copy exception vectors */
    memcpy( (void *)(KSEG1_BASE), &__tlbmiss_except, &__tlbmiss_except_end - &__tlbmiss_except );
    memcpy( (void *)(KSEG1_BASE + 0x180), &__general_except, &__general_except_end - &__general_except );
    memcpy( (void *)(KSEG1_BASE + 0x200), &__extra_except, &__extra_except_end - &__extra_except );
	
    /* setup exception vector jump table */
    for( unsigned i = 0; i < 32; i++ )
        exception_handlers[i] = (word_t)&mips32_exception;

    for( unsigned i = 0; i < 8; i++ )
        interrupt_handlers[i] = (word_t)spurious_interrupt;

    get_interrupt_ctrl()->register_exception_handler(0, &mips32_interrupt);
    get_interrupt_ctrl()->register_exception_handler(1, &mips32_tlb_mod);
    get_interrupt_ctrl()->register_exception_handler(2, &mips32_tlb_invalid);
    get_interrupt_ctrl()->register_exception_handler(3, &mips32_tlb_invalid);
    get_interrupt_ctrl()->register_exception_handler(8, &mips32_l4syscall);

    get_interrupt_ctrl()->register_interrupt_handler(2, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(3, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(4, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(5, intctrl_t_handle_irq);
    get_interrupt_ctrl()->register_interrupt_handler(6, intctrl_t_handle_irq);
 
    //XXX ?? flush cache
}
