/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/schedule.h
 * Description:   Scheduling functions
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
 * $Id: schedule.h,v 1.9 2003/09/24 19:04:37 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__SCHEDULE_H__
#define __GLUE__V4_IA64__SCHEDULE_H__

#include INC_GLUE(config.h)
#include INC_ARCH(psr.h)
#include INC_ARCH(cr.h)
#include INC_API(smp.h)

INLINE u64_t get_timer_tick_length (void)
{
    return TIMER_TICK_LENGTH;
}

/**
 * Sets the current processor asleep.
 */
INLINE void processor_sleep (void)
{
    cr_tpr_t old_tpr = cr_get_tpr ();

    cr_set_tpr (cr_tpr_t::all_enabled ());
    ia64_srlz_d ();
    enable_interrupts ();

    for (word_t i = 0; i < 100; i++);

    disable_interrupts ();
    cr_set_tpr (old_tpr);
}

#endif /* !__GLUE__V4_IA64__SCHEDULE_H__ */
