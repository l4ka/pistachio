/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/itc_timer.h
 * Description:   ITC based timer
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
 * $Id: itc_timer.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__ITC_TIMER_H__
#define __GLUE__V4_IA64__ITC_TIMER_H__

#include <timer.h>

class itc_ptimer_t : public generic_periodic_timer_t
{
    word_t _rate;

public:
    void setup (word_t rate);
    void init_cpulocal (word_t vector);
    void init_global (word_t vector);
    word_t get_rate (void) { return _rate; }
};

INLINE itc_ptimer_t * get_itc_ptimer (void)
{
    extern itc_ptimer_t itc_ptimer;
    return &itc_ptimer;
}


#endif /* !__GLUE__V4_IA64__ITC_TIMER_H__ */
