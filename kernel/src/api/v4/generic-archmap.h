/*********************************************************************
 *                
 * Copyright (C) 2005,  Karlsruhe University
 *                
 * File path:     generic/map.h
 * Description:   dummy architecture specific mapping declaration, for use by 
 *		  architectures without architecture specific fpages
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
 * $Id: generic-archmap.h,v 1.1 2005/05/19 08:35:40 stoess Exp $
 *                
 ********************************************************************/
#ifndef __GENERIC__MAP_H__
#define __GENERIC__MAP_H__

#include INC_API(fpage.h)
#include INC_API(tcb.h)

/*
 * arch_map_fpage()
 *
 * handles architecture-specific mappings
 */

INLINE void arch_map_fpage (tcb_t * src, fpage_t snd_fpage, word_t snd_base,
			    tcb_t * dst, fpage_t rcv_fpage, bool grant) 

{ }

/*
 * arch_unmap()
 *
 * handles architecture-specific revokings
 */

INLINE void arch_unmap_fpage (tcb_t * from, fpage_t fpage, bool flush) { }


/*
 * get_arch_specific_rcvwindow()
 *
 * returns complete architecture-specific address space 
 */


INLINE fpage_t acceptor_t::get_arch_specific_rcvwindow(tcb_t *dest) 
{  
    fpage_t fp;
    fp.raw = 0;
    return fp;
}



#endif /* !__GENERIC__MAP_H__ */
