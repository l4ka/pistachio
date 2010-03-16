/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     src/arch/powerpc/page.h
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

#ifndef __ARCH__POWERPC__PAGE_H__
#define __ARCH__POWERPC__PAGE_H__

#define POWERPC_PAGE_BITS	12
#define POWERPC_PAGE_SIZE	(1 << POWERPC_PAGE_BITS)
#define POWERPC_PAGE_MASK	(~(POWERPC_PAGE_SIZE - 1))

#define PPC_PAGEDIR_BITS	22
#define PPC_PAGEDIR_SIZE	(1 << PPC_PAGEDIR_BITS)
#define PPC_PAGEDIR_MASK	~(PPC_PAGEDIR_SIZE - 1)

#define PPC_PAGE_GUARDED	(1 << 3)
#define PPC_PAGE_COHERENT	(1 << 4)
#define PPC_PAGE_CACHE_INHIBIT	(1 << 5)
#define PPC_PAGE_WRITE_THRU	(1 << 6)
#define PPC_PAGE_DIRTY		(1 << 7)
#define PPC_PAGE_ACCESSED	(1 << 8)
#define PPC_PAGE_FLAGS_MASK	0x000001fb
#define PPC_PAGE_PTE_MASK	0xfffff1fb
#define PPC_PAGE_REFERENCED_MASK	(PPC_PAGE_ACCESSED | PPC_PAGE_DIRTY)

#if defined(CONFIG_SMP)
#define PPC_PAGE_SMP_SAFE	PPC_PAGE_COHERENT
#else
#define PPC_PAGE_SMP_SAFE	0
#endif

#endif	/* __ARCH__POWERPC__PAGE_H__ */
