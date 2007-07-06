/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/pgtab.h
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
 * $Id: pgtab.h,v 1.11 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__PGTAB_H__
#define __ARCH__POWERPC__PGTAB_H__

#include INC_ARCH(page.h)

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

#if 0

class ppc_pgent_t
{
private:
    union {
	/* This is structured so that it can be directly inserted into
	 * the page hash table, while masking out the hash pte's reserved bits.
	 */
	struct {
	    u32_t rpn		: 20;
	    u32_t pteg_slot	: 3;
	    u32_t referenced	: 1;
	    u32_t changed	: 1;
	    u32_t wimg		: 4;
	    u32_t second_hash	: 1;
	    u32_t pp		: 2;
	} pg;
	u32_t raw;
    };

public:
    /* The available permissions depend on the Ks and Kp bits in the segment
     * descriptors.  We set Ks=0 and Kp=0, so we have read-only or read-write.
     */
    enum permission_e {
	unused1 = 0,	// read/write
	unused2 = 1,	// read/write
	read_write = 2,
	read_only = 3
    };

    // predicates
    inline bool is_valid() 
	{ return this->raw != 0; }

    inline bool is_writable() 
	{ return this->pg.pp != ppc_pgent_t::read_only; }

    inline bool is_readable()
	{ return this->is_valid(); }

    inline bool is_executable()
	{ return this->is_valid(); }

    inline bool is_accessed() 
	{ return this->pg.referenced; }

    inline bool is_dirty() 
	{ return this->pg.changed; }

    inline bool is_second_hash()
	{ return this->pg.second_hash; }

    // retrieval
    inline addr_t get_address() 
	{ return (addr_t) (this->raw & POWERPC_PAGE_MASK); }

    inline ppc_pgent_t * get_ptab() 
	{ return (ppc_pgent_t *) (this->raw & POWERPC_PAGE_MASK); }

    inline word_t get_pte() 
	{ return this->raw & PPC_PAGE_PTE_MASK; }

    inline u32_t get_raw()
	{ return this->raw; }

    inline u32_t get_pteg_slot()
	{ return this->pg.pteg_slot; }

    inline permission_e get_permissions()
	{ return (permission_e)this->pg.pp; }

    // modification
    inline void clear()
	{ this->raw = 0; }

    inline void reset_accessed()
	{ this->pg.referenced = 0; }

    inline void reset_dirty()
	{ this->pg.changed = 0; }

    inline void set_accessed( word_t flag )
	{ this->pg.referenced |= flag; }

    inline void set_dirty( word_t flag )
	{ this->pg.changed |= flag; }

    inline void set_pte( word_t pte )
	{ this->raw = PPC_PAGE_PTE_MASK & pte; }

    inline void set_entry( addr_t addr, u32_t attrib )
	{ 
	    this->raw = ((u32_t)addr & POWERPC_PAGE_MASK) | 
		(attrib & PPC_PAGE_FLAGS_MASK); 
	}

    inline void set_ptab_entry( addr_t addr, u32_t attrib )
	{ 
	    this->raw = ((u32_t)addr & POWERPC_PAGE_MASK) | 
		(attrib & PPC_PAGE_FLAGS_MASK); 
	}

    inline void copy( const ppc_pgent_t pgent )
	{ this->raw = pgent.raw; }

    inline void set_writable( void )
	{ this->pg.pp = ppc_pgent_t::read_write; }

    inline void set_readonly( void )
	{ this->pg.pp = ppc_pgent_t::read_only; }

    inline void set_hash( word_t is_second_hash )
	{ this->pg.second_hash = is_second_hash ; }

    inline void set_pteg_slot( u32_t slot )
	{ this->pg.pteg_slot = slot; }

    static inline u32_t get_pdir_idx( addr_t addr )
	{ return (u32_t)addr >> PPC_PAGEDIR_BITS; }

    static inline u32_t get_ptab_idx( addr_t addr )
	{ return ((u32_t)addr & ~PPC_PAGEDIR_MASK) >> POWERPC_PAGE_BITS; }
};

#endif

#endif /* __ARCH__POWERPC__PGTAB_H__ */

