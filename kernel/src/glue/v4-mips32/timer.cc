/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/timer.cc
 * Description:   MIPS32 CP0 Timer implementation
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
 * $Id: timer.cc,v 1.1 2006/02/23 21:07:46 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>

#include INC_API(schedule.h)

#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(syscalls.h)

#include INC_ARCH(cp0regs.h)


timer_t timer;

unsigned timer_interrupt;

static u64_t ticks_passed;


extern "C" SYS_CLOCK() {
    //printf("SysClock returns %x %x\n", (word_t)(ticks_passed >> 32), (word_t)(ticks_passed & 0xffffffff) );
    return_clock( ticks_passed );
}


extern "C" void handle_timer_interrupt(word_t irq, mips32_irq_context_t* frame) {

    u32_t compare = (u32_t)read_32bit_cp0_register(CP0_COMPARE);
    u32_t counter = (u32_t)read_32bit_cp0_register(CP0_COUNT);
    u32_t difference;

    u32_t pmclach;
    timer_interrupt = 1;
	
    if( EXPECT_TRUE( counter > compare ) ) {
        difference = counter - compare;
        pmclach = difference / TIMER_PERIOD + 1;
    }
    else if( counter < compare ) {
        difference = (u32_t)0xffffffff - compare + 1 + counter;
        pmclach = difference / TIMER_PERIOD + 1;
    }
    else {
        //difference = 1;
        pmclach = 1;
    }
	
    ticks_passed += pmclach;
    compare += pmclach * TIMER_PERIOD;
    ASSERT( compare > counter );
    write_32bit_cp0_register (CP0_COMPARE, compare);

    while( 1 ) {
        counter = (u32_t)read_32bit_cp0_register(CP0_COUNT);
        if( counter > compare ) {
            compare += TIMER_PERIOD;
            ticks_passed++;
            write_32bit_cp0_register (CP0_COMPARE, compare);
        }
        else {
            break;
        }
    }

    //u32_t counter = (u32_t)read_32bit_cp0_register(CP0_COUNT);
    //timer_interrupt = 1;
    //write_32bit_cp0_register (CP0_COMPARE, counter + TIMER_PERIOD);
    //MAGIC_BREAKPOINT;
	
    get_current_scheduler()->handle_timer_interrupt();
}


void SECTION (".init") timer_t::init_global(void) {

    get_interrupt_ctrl()->register_interrupt_handler(7, handle_timer_interrupt);
}


void SECTION (".init") timer_t::init_cpu(void) {

    write_32bit_cp0_register(CP0_COUNT, 0);
    write_32bit_cp0_register(CP0_COMPARE, TIMER_PERIOD );
    ticks_passed = 0;

    get_interrupt_ctrl()->unmask(7);
}
