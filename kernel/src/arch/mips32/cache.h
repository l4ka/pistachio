/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/cache.h
 * Description:   Cache controller for MIPS32
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
 * $Id: cache.h,v 1.1 2006/02/23 21:07:39 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS32__CACHE_H__
#define __ARCH__MIPS32__CACHE_H__

/*
 * Cache Operation on MIPS32-4K processors ( see user manual p.129 )
 * Note: ErrCtl[WST,SPR] must be cleared
 */

#define Index_Invalidate_I      0x00 // 000 00
#define Index_Invalidate_D      0x01 // 000 01

#define Index_LoadTag_I		0x04 // 001 00
#define Index_LoadTag_D		0x05 // 001 01

#define Index_StoreTag_I	0x08 // 010 00
#define Index_StoreTag_D	0x09 // 010 01

#define Index_HitInvalidate_I   0x10 // 100 00
#define Index_HitInvalidate_D   0x11 // 100 01

#define Index_Fill_I		0x14 // 101 00

#define Index_FetchAndLook_I	0x1C // 111 00
#define Index_FetchAndLook_D	0x1D // 111 01


void init_cache();


#endif /* !__ARCH__MIPS32__CACHE_H__ */
