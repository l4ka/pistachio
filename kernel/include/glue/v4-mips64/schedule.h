/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-template/schedule.h
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
 * $Id: schedule.h,v 1.8 2004/06/04 02:32:31 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS64__SCHEDULE_H__
#define __GLUE__V4_MIPS64__SCHEDULE_H__

#include INC_API(tcb.h)
#include INC_GLUE(config.h)
#include INC_ARCH(mips_cpu.h)

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
    mips_cpu::sti();
    mips_cpu::sleep();
    mips_cpu::cli();
    mips_cpu::restore_mask(get_idle_tcb()->arch.int_mask);
}

#endif /* !__GLUE__V4_MIPS64__SCHEDULE_H__ */
