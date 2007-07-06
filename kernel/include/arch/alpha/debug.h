/*********************************************************************
 *                
 * Copyright (C) 2002, 2004,  University of New South Wales
 *                
 * File path:     arch/alpha/debug.h
 * Created:       24/07/2002 23:35:25 by Simon Winwood (sjw)
 * Description:   Debug support 
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
 * $Id: debug.h,v 1.15 2004/02/04 02:45:02 sgoetz Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__DEBUG_H__
#define __ARCH__ALPHA__DEBUG_H__

#include INC_GLUE(config.h)
#include INC_ARCH(palcalls.h)
#include INC_ARCH(page.h)
#include INC_PLAT(devspace.h)

INLINE int spin_forever(int pos = 0)
{
    PAL::halt();
    return 0;
}

#define SPIN_INTERRUPT  15 
#define SPIN_IDLE       30
#define SPIN_YIELD      45
#define SPIN_TIMER_INT  60
#define SPIN_IPC        75

#define POS_TO_ADDR(pos) (0xb8000 + 2 * (pos))

INLINE void spin(int pos = 0, int cpu = 0)
{
#if defined(CONFIG_SPIN_WHEELS)
    const char spin[] = {'|', '/', '-', '\\'};
    char next, current;

    current = devspace_t::dense_read8(POS_TO_ADDR(pos));
    next = spin[0];
    for(unsigned int i = 0; i < sizeof(spin); i++)
	if(current == spin[i]) {
	    next = spin[ (i + 1) % sizeof(spin) ];
	    break;
	}

    devspace_t::dense_write8(POS_TO_ADDR(pos), next);
#endif
}

INLINE void video_print_string(char *str, int pos) 
{
#if  defined(CONFIG_SPIN_WHEELS)
    for(char *c = str; *c; c++) {
	devspace_t::dense_write8(POS_TO_ADDR(pos++), *c);	
    }
#endif
}

INLINE void video_print_hex(word_t val, int pos) 
{
#if  defined(CONFIG_SPIN_WHEELS)
    int p = pos + 16;
    unsigned long mask = 0xf;
    int shift = 0;

    for(int i = 0; i < 16; i++, mask <<= 4, shift += 4) {
	u8_t c =  (((val & mask) >> shift) & 0xf);
	c += (c > 9) ? 'a' - 10 : '0';
	devspace_t::dense_write8(POS_TO_ADDR(p--), c);
    }
#endif
}

#define enter_kdebug(x) PAL::gentrap(0x2, (word_t) (x))

#define HERE()               printf("Here %s:%d\n", __PRETTY_FUNCTION__, __LINE__)

extern word_t handle_user_trap(word_t type, word_t arg);

#endif /* __ARCH__ALPHA__DEBUG_H__ */
