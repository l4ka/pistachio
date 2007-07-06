/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:    api/v4/interrupt.h 
 * Description:  Interrupt handling and abstraction
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
 * $Id: interrupt.h,v 1.4 2003/09/24 19:04:24 skoglund Exp $
 *
 *********************************************************************/
#ifndef __API__V4__INTERRUPT_H__
#define __API__V4__INTERRUPT_H__

#include INC_GLUE(intctrl.h)

/**
 * handle_interrupt: callback function for interrupt handling in V4
 * param irq: IRQ number
 */
void handle_interrupt(word_t irq);

/**
 * initializes interrupt threads
 */
void init_interrupt_threads();

/**
 * thread control for interrupt threads, sets handler function
 */
bool thread_control_interrupt(threadid_t irq_tid, threadid_t handler_tid);

#ifdef CONFIG_SMP
/**
 * migration function for interrupt threads
 * deals with re-routing
 */
void migrate_interrupt_start (tcb_t * tcb);
void migrate_interrupt_end (tcb_t * tcb);
#endif

#endif /* __API__V4__INTERRUPT_H__ */
