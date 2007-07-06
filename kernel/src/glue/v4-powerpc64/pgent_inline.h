/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/pgent_inline.h
 * Description:	Inlined functions for pgent_t (pgent.h).
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
 * $Id: pgent_inline.h,v 1.10 2006/11/17 17:02:04 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC64__PGENT_INLINE_H__
#define __GLUE__V4_POWERPC64__PGENT_INLINE_H__

#include <kmemory.h>

#include INC_GLUE(pgent.h)
#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)

EXTERN_KMEM_GROUP (kmem_pgtab);

extern word_t hw_pgshifts[];

INLINE word_t subtree_size (pgent_t::pgsize_e pgsize)
{
    ASSERT( pgsize != pgent_t::size_4k );
    return 1UL << (hw_pgshifts[pgsize] - hw_pgshifts[pgsize-1]);
}


// Page hash synchronization

inline void pgent_t::update_from_pghash( space_t * s, addr_t vaddr, bool large )
{
    ppc64_pte_t *pte;

    // Force the cpu to sync the tlb with the page hash before we read from it.
    sync();

    pte = get_pghash()->get_htab()->locate_pte( (word_t)vaddr, 
	    s->get_vsid(vaddr), this->map.pteg_slot, this->map.second_hash, large );

    if( pte )
    {
	this->map.referenced = pte->x.r;
	this->map.changed = pte->x.c;
    }
}

// Linknode access 

inline word_t pgent_t::get_linknode( space_t * s, pgsize_e pgsize )
{ 
//    return *(word_t *) ((word_t) this + (pgsize == size_4k) ? PPC64_SIZE_4k_LEVEL : PPC64_SIZE_16m_LEVEL ); 
    /* Presently, large pages are only supported for kernel */
    if ( pgsize != size_4k )
	printf("get_linknode failed: %p, %d, - %p\n", s, pgsize, this);
    ASSERT( pgsize == size_4k );
    return *(word_t *) ((word_t) this + PPC64_SIZE_4k_LEVEL ); 
}

inline void pgent_t::set_linknode( space_t * s, pgsize_e pgsize, word_t val )
{ 
//    *(word_t *) ((word_t) this + (pgsize == size_4k) ? PPC64_SIZE_4k_LEVEL : PPC64_SIZE_16m_LEVEL ) = val; 
    /* Presently, large pages are only supported for kernel */
    ASSERT( pgsize == size_4k );
    *(word_t *) ((word_t) this + PPC64_SIZE_4k_LEVEL ) = val; 
}


// Predicates

inline bool pgent_t::is_valid( space_t * s, pgsize_e pgsize )
{ 
    return (this->map.is_valid || this->tree.is_subtree);
}

inline bool pgent_t::is_writable( space_t * s, pgsize_e pgsize )
{ 
    return (this->map.pp == pgent_t::read_write);
}

inline bool pgent_t::is_readable( space_t * s, pgsize_e pgsize )
{ 
    return (this->map.pp != pgent_t::kernel_only);
}

inline bool pgent_t::is_executable( space_t * s, pgsize_e pgsize )
{ 
    return (this->map.noexecute == 0);
}

inline bool pgent_t::is_subtree( space_t * s, pgsize_e pgsize )
{ 
    return (this->tree.is_subtree && !this->tree.is_valid);
}

inline bool pgent_t::is_kernel( space_t * s, pgsize_e pgsize )
{
    return (this->map.pp == pgent_t::kernel_only);
}

// Retrieval

inline addr_t pgent_t::address( space_t * s, pgsize_e pgsize )
{ 
    return (addr_t)(this->map.rpn << POWERPC64_PAGE_BITS);
}
	
inline pgent_t * pgent_t::subtree( space_t * s, pgsize_e pgsize )
{ 
    return (pgent_t *) phys_to_virt( this->tree.subtree ); 
}

inline mapnode_t * pgent_t::mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr )
{ 
    return (mapnode_t *) (this->get_linknode(s, pgsize) ^ (word_t) vaddr); 
}

inline addr_t pgent_t::vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map )
{ 
    return (addr_t) (this->get_linknode(s, pgsize) ^ (word_t) map); 
}

inline word_t pgent_t::reference_bits( space_t *s, pgsize_e pgsize, 
	addr_t vaddr )
{
    this->update_from_pghash( s, vaddr, (pgsize == pgent_t::size_4k) ? 0 : 1);

    word_t rwx = 0;
    if( this->map.referenced ) rwx = 5;
    if( this->map.changed )    rwx |= 6;
    return rwx;
}

inline void pgent_t::update_reference_bits( space_t *s, pgsize_e pgsize,
					    word_t rwx )
{
    if (rwx) this->map.referenced = 1;
    if (rwx & 0x2) this->map.changed = 1;
}

inline word_t pgent_t::get_pte( space_t *s )
{
    return this->raw & PPC64_PAGE_PTE_MASK;
}

inline word_t pgent_t::attributes( space_t * s, pgsize_e pgsize )
{
    return this->map.wimg;
}

// Modification

inline void pgent_t::flush( space_t *s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{
    get_pghash()->flush_mapping( s, vaddr, this, pgsize );
}

inline void pgent_t::clear( space_t * s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{ 
    pgent_t tmp;
    tmp.raw = this->raw;

    this->raw = 0;
    if( !kernel )
	this->set_linknode(s, pgsize, 0);
    
    tmp.flush( s, pgsize, kernel, vaddr );
}

inline void pgent_t::make_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    /* Presently, large pages are only supported for kernel */
    addr_t page = kmem.alloc( kmem_pgtab,
		    ( subtree_size(pgsize) * sizeof(word_t) ) *
		    ( ( (pgsize-1) == size_4k ) ? ( kernel ? 1 : 2 ) : 1 ) );

    this->tree.subtree = (word_t)virt_to_phys( page );
    this->tree.is_subtree = 1;
    this->tree.is_valid = 0;
}

inline void pgent_t::remove_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    addr_t ptab = this->subtree( s, pgsize );
    this->raw = 0;

    kmem.free( kmem_pgtab, ptab, 
		    ( subtree_size(pgsize) * sizeof(word_t) ) *
		    ( ( (pgsize-1) == size_4k ) ? ( kernel ? 1 : 2 ) : 1 ) );

}

inline void pgent_t::set_entry( space_t * s, pgsize_e pgsize, addr_t paddr,
				word_t rwx, word_t attrib, bool kernel )
{
    this->map.pp = kernel ? pgent_t::kernel_only : (rwx & 2 ?  pgent_t::read_write : pgent_t::read_only);

    this->map.wimg = attrib;

    this->map.rpn = (word_t)paddr >> POWERPC64_PAGE_BITS;
    this->map.is_valid = 1;
    this->map.noexecute = ! (rwx & 1);
}

inline void pgent_t::update_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if( rwx & 2 ) 
	this->map.pp = pgent_t::read_write;
    if( rwx & 1 ) 
	this->map.noexecute = 0;
}

inline void pgent_t::revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if( rwx & 2 ) 
	this->map.pp = pgent_t::read_only;
    if( rwx & 1 ) 
	this->map.noexecute = 1;
}

inline void pgent_t::set_attributes( space_t *s, pgsize_e pgsize, word_t attrib )
{
    this->map.wimg = attrib;
}

inline void pgent_t::reset_reference_bits( space_t *s, pgsize_e pgsize )
{ 
    this->map.referenced = 0;
    this->map.changed = 0;
}

inline void pgent_t::set_accessed( space_t *s, pgsize_e pgsize, word_t flag )
{
    this->map.referenced |= flag;
}

inline void pgent_t::set_dirty( space_t *s, pgsize_e pgsize, word_t flag )
{
    this->map.changed |= flag;
}

inline void pgent_t::set_linknode( space_t * s, pgsize_e pgsize,
	mapnode_t * map, addr_t vaddr )
{ 
    this->set_linknode (s, pgsize, (word_t) map ^ (word_t) vaddr); 
}

// Movement

inline pgent_t * pgent_t::next( space_t * s, pgsize_e pgsize, word_t num )
{ 
    return this + num; 
}

// Debug

inline void pgent_t::dump_misc (space_t * s, pgsize_e pgsize)
{
}

#endif	/* __GLUE__V4_POWERPC64__PGENT_INLINE_H__ */
