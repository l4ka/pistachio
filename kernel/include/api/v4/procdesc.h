/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     api/v4/procdesc.h
 * Description:   Processor descriptors for kernel interface page
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
 * $Id: procdesc.h,v 1.5 2006/10/18 11:47:10 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__PROCDESC_H__
#define __API__V4__PROCDESC_H__

#if !defined(ASSEMBLY)

// make sure sizeof(procdesc_t) == 2^n
class procdesc_t
{
public:
    word_t external_freq;
    word_t internal_freq;
    word_t freq_change;

    /*
     * Usage:
     *   ia64	ITC frequency
     */
    word_t arch1;

public:
    void set_external_frequency(word_t freq)
	{ external_freq = freq; }
    
    void set_internal_frequency(word_t freq)
	{ internal_freq = freq; }
};


#endif /* !ASSEMBLY */

#if !defined(KIP_PROC_DESC_LOG2SIZE)
#if defined(CONFIG_IS_64BIT)
#define KIP_PROC_DESC_LOG2SIZE	5
#elif defined(CONFIG_IS_32BIT)
#define KIP_PROC_DESC_LOG2SIZE	4
#endif
#endif /* !defined(KIP_PROC_DESC_LOG2SIZE) */

#endif /* !__API__V4__PROCDESC_H__ */
