/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/ia64/specials.h
 * Description:   ia64 specific functions and defines
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
 * $Id: specials.h,v 1.7 2003/09/24 19:06:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__SPECIALS_H__
#define __L4__IA64__SPECIALS_H__


/*
 * Architecture specific helper functions.
 */

L4_INLINE int __L4_Msb (L4_Word_t w) __attribute__ ((const));

L4_INLINE int __L4_Msb (L4_Word_t w)
{
    int bitnum;

    if (w == 0)
	return 64;

    for (bitnum = 0, w >>= 1; w != 0; bitnum++)
	w >>= 1;

    return bitnum;
}




/*
 * Cacheability hints for string items.
 */

#define L4_CacheNonTemporalL1		(L4_CacheAllocationHint_t) { raw: 1 })
#define L4_CacheNonTemporalL2		(L4_CacheAllocationHint_t) { raw: 2 })
#define L4_CacheNonTemporalAllLevels	(L4_CacheAllocationHint_t) { raw: 3 })




/*
 * Memory attributes for MemoryControl system call.
 */

#define L4_WriteBackMemory		1
#define L4_WriteCoalescingMemory	7
#define L4_UncacheableMemory		5
#define L4_UncacheableExportedMemory	6
#define L4_NaTPageMemory		8




/*
 * Architecture specific memory descriptor types.
 */

#define L4_ACPIMemoryType		(0x1f)




/*
 * PCI config fpages
 */

typedef union {
    L4_Word_t	raw;
    struct {
	L4_Word_t	rwx:4;
	L4_Word_t	__two:6;
	L4_Word_t	s:6;
	L4_Word_t	p:48;
    } X;
} L4_PCIConfigFpage_t;

L4_INLINE L4_Fpage_t L4_PCIConfigFpage (L4_Word_t BaseAddress, int FpageSize)
{
    L4_PCIConfigFpage_t fp;
    L4_Fpage_t ret;
    L4_Word_t msb = __L4_Msb (FpageSize);

    fp.X.p = BaseAddress;
    fp.X.__two = 2;
    fp.X.s = (1UL << msb) < (L4_Word_t) FpageSize ? msb + 1 : msb;
    fp.X.rwx = L4_NoAccess;

    ret.raw = fp.raw;
    return ret;
}

L4_INLINE L4_Fpage_t L4_PCIConfigFpageLog2 (L4_Word_t BaseAddress,
					    int FpageSize)
{
    L4_PCIConfigFpage_t fp;
    L4_Fpage_t ret;

    fp.X.p = BaseAddress;
    fp.X.__two = 2;
    fp.X.s = FpageSize;
    fp.X.rwx = L4_NoAccess;

    ret.raw = fp.raw;
    return ret;
}




#endif /* !__L4__IA64__SPECIALS_H__ */
