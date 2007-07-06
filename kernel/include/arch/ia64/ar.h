/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/ar.h
 * Description:   IA64 application registers
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
 * $Id: ar.h,v 1.7 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__AR_H__
#define __ARCH__IA64__AR_H__


#define FRAME_MARKER(input, local, output) \
    (((input + local) << 7) + (input + local + output))

#if !defined(ASSEMBLY)

class ar_pfs_t
{
public:
    union {
	struct {
	    word_t sof			: 7;
	    word_t sol			: 7;
	    word_t sor			: 4;
	    word_t rrb_gr		: 7;
	    word_t rrb_fr		: 7;
	    word_t rrb_pr		: 6;
	    word_t __rv1		: 14;
	    word_t previous_ec		: 6;
	    word_t __rv2		: 4;
	    word_t previous_priv	: 2;
	};
	u64_t raw;
    };

    word_t locals (void)
	{ return sol; }

    word_t outputs (void)
	{ return sof - sol; }

    word_t framesize (void)
	{ return sof; }
};

class ar_fpsr_t
{
public:
    union {
	struct {
	    word_t traps_vd	: 1;
	    word_t traps_dd	: 1;
	    word_t traps_zd	: 1;
	    word_t traps_od	: 1;
	    word_t traps_ud	: 1;
	    word_t traps_id	: 1;

	    word_t sf0		: 13;
	    word_t sf1		: 13;
	    word_t sf2		: 13;
	    word_t sf3		: 13;
	    word_t __rv		: 6;
	};
	u64_t raw;
    };

    enum ctrl_e {
	ftz		= (1 << 0),
	wre		= (1 << 1),
	pc_0		= (0 << 2),
	pc_1		= (1 << 2),
	pc_2		= (2 << 2),
	pc_3		= (3 << 2),
	rc_zero		= (0 << 4),
	rc_up		= (1 << 4),
	rc_down		= (2 << 4),
	rc_nearest	= (2 << 4),
	td		= (1 << 6)
    };

    enum flags_e {
	v = (1 << 7),
	d = (1 << 8),
	z = (1 << 9),
	o = (1 << 10),
	u = (1 << 11),
	i = (1 << 12)
    };

    void enable_all_traps (void)
	{
	    traps_vd = traps_dd = traps_zd = traps_od =
		traps_ud = traps_id = 0;
	}

    void disable_all_traps (void)
	{
	    traps_vd = traps_dd = traps_zd = traps_od =
		traps_ud = traps_id = 1;
	}

    void set_sf0 (word_t sf) { sf0 = sf; }
    void set_sf1 (word_t sf) { sf1 = sf; }
    void set_sf2 (word_t sf) { sf2 = sf; }
    void set_sf3 (word_t sf) { sf3 = sf; }
};


INLINE word_t ar_get_itc (void)
{
    word_t itc;
    __asm__ __volatile__ ("mov %0 = ar.itc" :"=r" (itc));
    return itc;
}

INLINE void ar_set_itc (word_t count)
{
    __asm__ __volatile__ ("mov ar.itc = %0" ::"r" (count));
}

#endif /* !ASSEMBLY */

#endif /* !__ARCH__IA64__AR_H__ */
