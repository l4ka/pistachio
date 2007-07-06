/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     glue/v4-powerpc/schedule.h
 * Description:   scheduling functions
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
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__SCHEDULE_H__
#define __GLUE__V4_POWERPC__SCHEDULE_H__

#include INC_API(smp.h)

INLINE u32_t get_timer_tick_length()
{
    return TIMER_TICK_LENGTH;
}

/**
 * sets the current processor asleep
 */
INLINE void processor_sleep()
{
#if defined(CONFIG_SMP)
    // Blip the interrupts.
    ppc_enable_interrupts();
    ppc_disable_interrupts();

    process_xcpu_mailbox();
#else
    /* Enable external interrupts and power savings mode.  If the cpu is
     * configured for power savings, then when we set msr, it will go to sleep.
     * The timer handler will clear MSR[EE] and MSR[POW].
     * So we can test either of those bits to determine whether the timer
     * interrupt has fired.  A while loop will wait for a timer exception
     * on CPU's which are not configured for power savings mode.
     */
    word_t msr = ppc_get_msr();
    msr = MSR_SET(msr, MSR_EE);
    msr = MSR_SET(msr, MSR_POW);
    ppc_set_msr( msr );

    /* Wait for the timer exception. */
    while( MSR_BIT(msr, MSR_EE) )
	msr = ppc_get_msr();
#endif
}

INLINE word_t processor_wake( word_t msr )
{
    /* Disable the power savings mode and the external interrupts. */
    msr = MSR_CLR( msr, MSR_POW );
    msr = MSR_CLR( msr, MSR_EE );
    return msr;
}

#endif /* __GLUE__V4_POWERPC__SCHEDULE_H__ */
