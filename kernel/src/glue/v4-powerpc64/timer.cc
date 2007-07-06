/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/timer.cc
 * Description:   Timer management functions
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
 * $Id: timer.cc,v 1.5 2004/06/04 06:38:41 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE(timer.h)
#include INC_ARCH(1275tree.h)
#include INC_ARCH(ppc64_registers.h)

timer_t timer UNIT("cpulocal");

static word_t timer_interval;

SECTION(".init")
void timer_t::init_global()
{
    u32_t *prop_val, len, timebase_hz;
    of1275_device_t *cpu = get_of1275_tree()->find_device_type( "cpu" );

    cpu->get_prop( "timebase-frequency", (char **)&prop_val, &len);

    timebase_hz = *prop_val;

    timer_interval = timebase_hz / TIMER_TICK_LENGTH;

    TRACE_INIT( "Timebase frequency %dHz\n",timebase_hz );
}

SECTION(".init")
void timer_t::init_cpu()
{
    // Initialize the time base to 0. */
    ppc64_set_tbl( 0 );	// Make sure that tbu won't be upset by a carry.
    ppc64_set_tbu( 0 );	// Clear the tbu.
    ppc64_set_tbl( 0 );	// Clear the tbl.

    this->last_time_base = 0;
}

#define except_return()			\
do {					\
    asm volatile (			\
	"mtlr %0 ;"			\
	"ld %%r1, 0(%1) ;"		\
	"blr ;"				\
	:				\
	: "r" (__builtin_return_address(0)), \
	  "b" (__builtin_frame_address(0)) \
    );					\
    while(1);				\
} while(0)


extern "C" void decrementer_handler( powerpc64_irq_context_t *context )
{
    word_t tb, next_dec;

    do {
	get_timer()->last_time_base += timer_interval;
    } while ( get_timer()->last_time_base < (tb = ppc64_get_tb()) );

    next_dec = get_timer()->last_time_base - tb;

    ppc64_set_dec(next_dec);

    /* Do this last as we may reschedule */
    get_current_scheduler()->handle_timer_interrupt();

    except_return();
}
