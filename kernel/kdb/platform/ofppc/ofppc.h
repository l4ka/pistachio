/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	kdb/platform/ofppc/ofppc.h
 * Description:	OpenFirmware PPC support declarations.
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
 * $Id: ofppc.h,v 1.3 2003/09/24 19:05:20 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __KDB__PLATFORM__OFPPC__OFPPC_H__
#define __KDB__PLATFORM__OFPPC__OFPPC_H__

#if defined(CONFIG_KDB_CONS_OF1275)

#include <sync.h>

#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(pghash.h)

class of1275_space_t
{
protected:
    spinlock_t lock;

    word_t of1275_ptab_loc;
    word_t of1275_segments[16];
    word_t of1275_stack_top;
    word_t of1275_stack_bottom;

    word_t current_ptab_loc;
    word_t current_segments[16];

protected:
    word_t get_ptab_loc() { return ppc_get_sdr1(); }
    void get_segments( word_t segments[16] )
    {
	for( int i = 0; i < 16; i++ )
	    segments[i] = ppc_get_sr(i);
    }

    bool using_of1275_stack()
    {
	word_t stack = (word_t)&stack;
	return (stack >= this->of1275_stack_bottom) &&
	       (stack < this->of1275_stack_top);
    }

public:
    void init( word_t stack_top, word_t stack_bottom );

    word_t execute_of1275( word_t (*func)(void *), void *param );
};

INLINE of1275_space_t *get_of1275_space()
{
    extern of1275_space_t of1275_space;
    return &of1275_space;
}

#endif	/* CONFIG_KDB_CONS_OF1275 */

#endif	/* __KDB__PLATFORM__OFPPC__OFPPC_H__ */
