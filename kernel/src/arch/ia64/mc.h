/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/mc.h
 * Description:   ia64 Machine Check support
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
 * $Id: mc.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__MC_H__
#define __ARCH__IA64__MC_H__


class processor_state_t
{
public:
    enum op_e {
	unkown		= 0,
	load		= 1,
	store		= 2,
	instr_fetch	= 3,
	data_prefetch	= 4,
	other_mem	= 5,
    };

    union {
	struct {
	    word_t __reserved1		: 2;
	    word_t rz			: 1;
	    word_t ra			: 1;
	    word_t me			: 1;
	    word_t mn			: 1;
	    word_t sy			: 1;
	    word_t co			: 1;

	    word_t ci			: 1;
	    word_t us			: 1;
	    word_t hd			: 1;
	    word_t tl			: 1;
	    word_t mi			: 1;
	    word_t pi	     		: 1;
	    word_t pm			: 1;
	    word_t dy			: 1;

	    word_t in			: 1;
	    word_t rs			: 1;
	    word_t cm			: 1;
	    word_t ex			: 1;
	    word_t cr			: 1;
	    word_t pc			: 1;
	    word_t dr			: 1;
	    word_t tr			: 1;

	    word_t rr			: 1;
	    word_t ar			: 1;
	    word_t br			: 1;
	    word_t pr			: 1;
	    word_t fp			: 1;
	    word_t b1			: 1;
	    word_t b0			: 1;
	    word_t gr			: 1;

	    word_t dsize		: 16;
	    word_t __reserved2		: 12;
	    word_t cc			: 1;
	    word_t tc			: 1;
	    word_t bc			: 1;
	    word_t uc			: 1;
	};
	word_t raw;
    };
};


#endif /* !__ARCH__IA64__MC_H__ */
