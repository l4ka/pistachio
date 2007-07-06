/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/context.h
 * Description:   Various context management classes
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
 * $Id: context.h,v 1.28 2003/09/24 19:04:36 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__CONTEXT_H__
#define __GLUE__V4_IA64__CONTEXT_H__

#include INC_ARCH(psr.h)
#include INC_ARCH(cr.h)
#include INC_ARCH(ar.h)

typedef struct {
    word_t raw[2];
} ia64_float;


/**
 * Frame contains minimal information for a thread switch context.
 * The SP and IP must be located at the same offset from the end of
 * the frame as the SP and IP in the exception context frame.
 */
class ia64_switch_context_t
{
public:
    word_t	__pad;
    word_t	num_dirty;	// Only used in syscall stubs
    word_t	rsc;		// Only used in syscall stubs
    word_t	gp;		// Only used in syscall stubs

    word_t	pfs;
    word_t	cfm;
    addr_t	ip;
    word_t *	bspstore;
    word_t	rnat;
    word_t	unat;
    word_t	pr;
    psr_t	psr;
    addr_t	sp;		// Only used in syscall stubs
    addr_t	rp;
};


class ia64_exception_context_t
{
public:
    word_t	exception_num;
    word_t	__pad1;

    ia64_float	f6;
    ia64_float	f7;
    ia64_float	f8;
    ia64_float	f9;
    ia64_float	f10;
    ia64_float	f11;
    ia64_float	f12;
    ia64_float	f13;
    ia64_float	f14;
    ia64_float	f15;

    addr_t	bspstore_kern;	// For dumping reg stack contents

    word_t	r1;
    word_t	r2;
    word_t	r3;
    word_t	r4;
    word_t	r5;
    word_t	r6;
    word_t	r7;
    word_t	r8;
    word_t	r9;
    word_t	r10;
    word_t	r11;
    word_t	r13;
    word_t	r14;
    word_t	r15;
    word_t	r16;
    word_t	r17;
    word_t	r18;
    word_t	r19;
    word_t	r20;
    word_t	r21;
    word_t	r22;
    word_t	r23;
    word_t	r24;
    word_t	r25;
    word_t	r26;
    word_t	r27;
    word_t	r28;
    word_t	r29;
    word_t	r30;
    word_t	r31;

    word_t	b0;
    word_t	b1;
    word_t	b2;
    word_t	b3;
    word_t	b4;
    word_t	b5;
    word_t	b6;
    word_t	b7;

    word_t	pr;

    ar_pfs_t	pfs;
    word_t	num_dirty;
    addr_t	bspstore;
    word_t	rsc;
    word_t	ccv;
    word_t	unat;
    word_t	rnat;
    word_t	lc;
    word_t	ec;
    ar_fpsr_t	fpsr;

    addr_t	iipa;
    cr_isr_t	isr;
    ar_pfs_t	ifs;
    word_t	iim;
    addr_t	ifa;
    addr_t	iha;
    word_t	unat_kern;
    word_t	rnat_kern;
    cr_tpr_t	tpr;

    // Remaining fields overlap with switch context
    psr_t	ipsr;
    addr_t	r12;
    addr_t	iip;
};


#endif /* !__GLUE__V4_IA64__CONTEXT_H__ */
