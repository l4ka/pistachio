/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/ppc44x.h
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __ARCH__POWERPC__IBM450_H__
#define __ARCH__POWERPC__IBM450_H__

#ifndef __ASSEMBLY__
asm(".macro lfpdx   frt, idx, reg; .long ((31<<26)|((\\frt)<<21)|(\\idx<<16)|(\\reg<<11)|(462<<1)); .endm");
asm(".macro lfpdux  frt, idx, reg; .long ((31<<26)|((\\frt)<<21)|(\\idx<<16)|(\\reg<<11)|(494<<1)); .endm");
asm(".macro stfpdx  frt, idx, reg; .long ((31<<26)|((\\frt)<<21)|(\\idx<<16)|(\\reg<<11)|(974<<1)); .endm");
asm(".macro stfpdux frt, idx, reg; .long ((31<<26)|((\\frt)<<21)|(\\idx<<16)|(\\reg<<11)|(1006<<1)); .endm");

extern inline word_t ppc_get_dcrx(word_t dcrn)
{
    word_t value;
    asm volatile ("mfdcrx %0,%1": "=r" (value) : "r" (dcrn) : "memory");
    return value;
}

extern inline void ppc_set_dcrx(word_t dcrn, word_t value)
{
    asm volatile("mtdcrx %0,%1": :"r" (dcrn), "r" (value) : "memory");
}


#endif

#endif /* !__ARCH__POWERPC__IBM450_H__*/
