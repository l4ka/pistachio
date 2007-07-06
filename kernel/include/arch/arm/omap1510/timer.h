/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/omap1510/timer.h
 * Description:   Functions which manipulate the OMAP1510 OS timer
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
 * $Id: timer.h,v 1.3 2004/08/12 11:54:52 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__OMAP1510__TIMER_H_
#define __ARCH__ARM__OMAP1510__TIMER_H_

#define TIMER_TICK_LENGTH	10000

#define TIMER1_BASE	0xFFFEC500
#define TIMER2_BASE	0xFFFEC600
#define TIMER3_BASE	0xFFFEC700

#define TIMER_CRTL	0x0
#define TIMER_LOAD	0x4
#define TIMER_READ	0x8

#define TIMER1_IRQ	26
#define TIMER2_IRQ	30
#define TIMER3_IRQ	16

#define VIRT_TIMER_BASE ((word_t) io_to_virt (TIMER1_BASE))

#define REG_TIMER_CTRL	*((volatile word_t *) (VIRT_TIMER_BASE + TIMER_CRTL)) 
#define REG_TIMER_LOAD	*((volatile word_t *) (VIRT_TIMER_BASE + TIMER_LOAD))

#endif /* __ARCH__ARM__OMAP1510__TIMER_H_*/
