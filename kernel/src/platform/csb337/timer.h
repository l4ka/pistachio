/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/csb337/timer.h
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
 * $Id: timer.h,v 1.1 2004/08/12 10:58:53 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__CSB337__TIMER_H__
#define __PLATFORM__CSB337__TIMER_H__

#include	INC_PLAT(offsets.h)

/* Atmel AT91RM9200 System Timer */
#define ST_OFFSET	0xd00
#define ST_VADDR	(SYS_VADDR | ST_OFFSET)

#define		ST_SR_PITS	1	/* Period Interval Timer Status */
#define		ST_SR_WDOVF	2	/* Watchdog Overflow */
#define		ST_SR_RTTINC	4	/* Real-time Timer Increment */
#define		ST_SR_ALMS	8	/* Alarm Status */

#define		SLCK_RATE	32768
#define		TIMER_TICK_LENGTH	15625 /* usec */
#define		TIMER_PERIOD	512

#endif /*__PLATFORM__CSB337__TIMER_H__ */
