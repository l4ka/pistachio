/*********************************************************************
 *                
 * Copyright (C) 2003  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/ktcb.h
 * Description:   Kernel TCB state 
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
 * $Id: ktcb.h,v 1.5 2004/06/28 06:53:19 philipd Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__KTCB_H__
#define __GLUE__V4_SPARC64__KTCB_H__

class arch_ktcb_t {
public:
    /* Top of the KTCB stack inside the kernel's pinned area. */
    word_t pinned_stack_top;

    /* Area used to save trap state registers on thread switches. */
    tstate_t tstate;  // (u64_t)  Trap State.
    tpc_t    tpc;     // (word_t) Trap PC.
    tnpc_t   tnpc;    // (word_t) Trap Next PC.
    tl_t     tl;      // (u8_t)   Trap Level Register.
    pil_t    pil;     // (u8_t)   Processor Interrupt Level.

    /* Number of window frames saved in the UTCB. */
    word_t saved_windows;

    /* CWP saved in switch_to */
    cwp_t saved_cwp;
};


#endif /* !__GLUE__V4_SPARC64__KTCB_H__ */
