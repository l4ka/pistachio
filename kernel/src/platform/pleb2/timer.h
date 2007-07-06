/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/pleb2/timer.h
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
 * $Id: timer.h,v 1.2 2005/01/12 02:49:03 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__PLEB2__TIMER_H__
#define __PLATFORM__PLEB2__TIMER_H__

/* PXA255 Timer */
#define TIMER_TICK_LENGTH	5000
#define TIMER_POFFSET		0x0a00000
#define TIMER_VOFFSET		0x0001000
#define CLOCKS_POFFSET		0x1300000
#define CLOCKS_VOFFSET		0x0002000

#define TIMER_RATE		3686400

#if !defined(ASSEMBLY)

#include INC_CPU(cpu.h)

#define XSCALE_TIMERS		(IODEVICE_VADDR + TIMER_VOFFSET)

/* Match registers */
#define XSCALE_OS_TIMER_MR0	(*(volatile word_t *)(XSCALE_TIMERS + 0x00))
#define XSCALE_OS_TIMER_MR1	(*(volatile word_t *)(XSCALE_TIMERS + 0x04))
#define XSCALE_OS_TIMER_MR2	(*(volatile word_t *)(XSCALE_TIMERS + 0x08))
#define XSCALE_OS_TIMER_MR3	(*(volatile word_t *)(XSCALE_TIMERS + 0x0c))

/* Interrupt enable register */
#define XSCALE_OS_TIMER_IER	(*(volatile word_t *)(XSCALE_TIMERS + 0x1c))
/* Watchdog match enable register */
#define XSCALE_OS_TIMER_WMER	(*(volatile word_t *)(XSCALE_TIMERS + 0x18))
/* Timer count register */
#define XSCALE_OS_TIMER_TCR	(*(volatile word_t *)(XSCALE_TIMERS + 0x10))
/* Timer status register */
#define XSCALE_OS_TIMER_TSR	(*(volatile word_t *)(XSCALE_TIMERS + 0x14))

#define XSCALE_CLOCKS		(IODEVICE_VADDR + CLOCKS_VOFFSET)

#define XSCALE_CLOCKS_CCCR	(*(volatile word_t *)(XSCALE_CLOCKS + 0x00))

#endif

#endif /*__PLATFORM__PLEB2__TIMER_H__ */
