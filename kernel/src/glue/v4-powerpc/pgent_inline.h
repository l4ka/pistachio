/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/pgent_inline.h
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
 * $Id: pgent_inline.h,v 1.15 2007/01/14 21:36:19 uhlig Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__PGENT_INLINE_H__
#define __GLUE__V4_POWERPC__PGENT_INLINE_H__

#include <kmemory.h>

#include INC_GLUE(pgent.h)
#include INC_GLUE(space.h)
#include INC_GLUE(pghash.h)

EXTERN_KMEM_GROUP (kmem_pgtab);

// Page hash synchronization

inline void pgent_t::update_from_pghash( space_t * s, addr_t vaddr )
{
    ppc_translation_t *pte;

    // Force the cpu to sync the tlb with the page hash before we read from it.
    sync();

    pte = get_pghash()->get_htab()->locate_pte( (word_t)vaddr, 
	    s->get_vsid(vaddr), 
	    this->map.pteg_slot, this->map.second_hash );
    if( pte )
    {
	this->map.referenced = pte->x.r;
	this->map.changed = pte->x.c;
    }
}

// Linknode access 

inline word_t pgent_t::get_linknode( void )
{ 
    return *(word_t *) ((word_t) this + POWERPC_PAGE_SIZE); 
}

inline void pgent_t::set_linknode( word_t val )
{ 
    *(word_t *) ((word_t) this + POWERPC_PAGE_SIZE) = val; 
}


// Predicates

inline bool pgent_t::is_valid( space_t * s, pgsize_e pgsize )
{ 
    if( pgsize == pgent_t::size_4m )
	return this->tree.valid;
    else
	return this->raw != 0;
}

inline bool pgent_t::is_writable( space_t * s, pgsize_e pgsize )
{ 
    return this->map.pp != pgent_t::read_only;
}

inline bool pgent_t::is_readable( space_t * s, pgsize_e pgsize )
{ 
    return this->is_valid( s, pgsize );
}

inline bool pgent_t::is_executable( space_t * s, pgsize_e pgsize )
{ 
    return this->is_valid( s, pgsize );
}

inline bool pgent_t::is_subtree( space_t * s, pgsize_e pgsize )
{ 
    return (pgsize == size_4m); 
}

inline bool pgent_t::is_kernel( space_t * s, pgsize_e pgsize )
{
    return s == get_kernel_space();
}

// Retrieval

inline addr_t pgent_t::address( space_t * s, pgsize_e pgsize )
{ 
    return (addr_t)(this->raw & POWERPC_PAGE_MASK);
}
	
inline pgent_t * pgent_t::subtree( space_t * s, pgsize_e pgsize )
{ 
    return (pgent_t *) phys_to_virt( this->address(s, pgsize) ); 
}

inline mapnode_t * pgent_t::mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr )
{ 
    return (mapnode_t *) (this->get_linknode() ^ (word_t) vaddr); 
}

inline addr_t pgent_t::vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map )
{ 
    return (addr_t) (this->get_linknode() ^ (word_t) map); 
}

inline word_t pgent_t::attributes ( space_t * s, pgsize_e pgsize )
{
    return (raw & PPC_PAGE_CACHE_INHIBIT) ? 1 : 0;
}

inline word_t pgent_t::reference_bits( space_t *s, pgsize_e pgsize, 
	addr_t vaddr )
{
    this->update_from_pghash( s, vaddr );

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

inline word_t pgent_t::get_translation( space_t *s, pgsize_e pgsize )
{
    return this->raw & PPC_PAGE_PTE_MASK;
}

// Modification

inline void pgent_t::flush( space_t *s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{
    get_pghash()->flush_4k_mapping( s, vaddr, this );
}

inline void pgent_t::clear( space_t * s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{ 
    pgent_t tmp;
    tmp.raw = this->raw;

    this->raw = 0;
    if( !kernel )
	this->set_linknode(0);
    
    tmp.flush( s, pgsize, kernel, vaddr );
}

inline void pgent_t::make_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    addr_t page = kmem.alloc( kmem_pgtab, POWERPC_PAGE_SIZE * (kernel ? 1:2) );

    this->raw = (word_t)virt_to_phys( page );
    if( this->raw )
	this->tree.valid = 1;
}

inline void pgent_t::remove_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    addr_t ptab = this->address( s, pgsize );
    this->raw = 0;

    kmem.free( kmem_pgtab, phys_to_virt(ptab), 
	    POWERPC_PAGE_SIZE * (kernel ? 1:2) );
}

inline void pgent_t::set_entry( space_t * s, pgsize_e pgsize, addr_t paddr,
				word_t rwx, word_t attrib, bool kernel )
{
    word_t attr = rwx & 2 ? pgent_t::read_write : pgent_t::read_only;
    if( attrib )
	attr |= PPC_PAGE_CACHE_INHIBIT;

    this->raw = ((word_t)paddr & POWERPC_PAGE_MASK) | 
	(attr & PPC_PAGE_FLAGS_MASK);
}

inline void pgent_t::set_writable( space_t * s, pgsize_e pgsize )
{ 
    this->map.pp = pgent_t::read_write;
}

inline void pgent_t::set_readonly( space_t * s, pgsize_e pgsize )
{ 
    this->map.pp = pgent_t::read_only;
}

inline void pgent_t::update_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if( rwx & 2 ) 
	this->set_writable( s, pgsize );
}

inline void pgent_t::revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if( rwx & 2) 
	this->set_readonly( s, pgsize );
}

inline void pgent_t::set_attributes ( space_t * s, pgsize_e pgsize, word_t attrib )
{
    if (attrib)
	raw |= PPC_PAGE_CACHE_INHIBIT;
    else
	raw &= ~PPC_PAGE_CACHE_INHIBIT;
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
    this->set_linknode ((word_t) map ^ (word_t) vaddr); 
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

#endif	/* __GLUE__V4_POWERPC__PGENT_INLINE_H__ */
