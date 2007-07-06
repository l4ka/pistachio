/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/pgtab.h
 * Description:	Defines a page table entry compatible with the page hash entry.
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
 * $Id: pgtab.h,v 1.3 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__PGTAB_H__
#define __ARCH__POWERPC64__PGTAB_H__

#include INC_ARCH(page.h)

/* These defines are dependant on the hash page table structure
 * which is defined in pgent.h
 */
#define PPC64_PAGE_NOEXECUTE		(1ul << 2)
#define PPC64_PAGE_GUARDED		(1ul << 3)
#define PPC64_PAGE_COHERENT		(1ul << 4)
#define PPC64_PAGE_CACHE_INHIBIT	(1ul << 5)
#define PPC64_PAGE_WRITE_THRU		(1ul << 6)
#define PPC64_PAGE_DIRTY		(1ul << 7)
#define PPC64_PAGE_ACCESSED		(1ul << 8)
#define PPC64_PAGE_FLAGS_MASK		0x40000000000001fful
#define PPC64_PAGE_PTE_MASK		0x3ffffffffffff1fful
#define PPC64_PAGE_REFERENCED_MASK	(PPC_PAGE_ACCESSED | PPC_PAGE_DIRTY)

#if defined(CONFIG_SMP)
#define PPC64_PAGE_SMP_SAFE	PPC64_PAGE_COHERENT
#else
#define PPC64_PAGE_SMP_SAFE	0
#endif

#endif /* __ARCH__POWERPC64__PGTAB_H__ */

