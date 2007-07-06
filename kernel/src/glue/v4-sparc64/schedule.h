/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/schedule.h
 * Description:   
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
 * $Id: schedule.h,v 1.5 2004/06/28 06:54:30 philipd Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_SPARC64__SCHEDULE_H__
#define __GLUE__V4_SPARC64__SCHEDULE_H__

/**
 * @todo document me
 */
INLINE u64_t get_timer_tick_length()
{
    return TIMER_TICK_LENGTH;
}


/**
 * send the current processor to sleep
 */
INLINE void processor_sleep()
{
    pstate_t pstate;

    pstate.get();

    pstate.pstate.ie = 1;
    pstate.set();
    
    for(int i = 0; i < 100; i++) {
	asm volatile ("nop");
    }

    pstate.pstate.ie = 0;
    pstate.set();
}


#endif /* !__GLUE__V4_SPARC64__SCHEDULE_H__ */
