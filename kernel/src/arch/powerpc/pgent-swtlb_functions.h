/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/pgent-swtlb_functions.h
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
#pragma once

#include <kmemory.h>
#include INC_GLUE(space.h)

EXTERN_KMEM_GROUP (kmem_pgtab);

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
    return this->tree.valid != 0;
}

inline bool pgent_t::is_writable( space_t * s, pgsize_e pgsize )
{
    return this->map.write;
}

inline bool pgent_t::is_readable( space_t * s, pgsize_e pgsize )
{
    return this->map.read;
}

inline bool pgent_t::is_executable( space_t * s, pgsize_e pgsize )
{
    return this->map.execute;
}

inline bool pgent_t::is_subtree( space_t * s, pgsize_e pgsize )
{
    return pgsize == size_4m && 
	this->tree.is_subtree == cache_subtree;
}

inline bool pgent_t::is_kernel( space_t * s, pgsize_e pgsize )
{
    return s == get_kernel_space();
}

// Retrieval
inline paddr_t pgent_t::address( space_t * s, pgsize_e pgsize )
{
    return (paddr_t)(this->raw & POWERPC_PAGE_MASK) + 
	((paddr_t)this->map.erpn << 32);
}

inline pgent_t * pgent_t::subtree( space_t * s, pgsize_e pgsize )
{ 
    return (pgent_t *) this->address(s, pgsize); 
}

inline mapnode_t * pgent_t::mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr )
{ 
    return (mapnode_t *) (this->get_linknode() ^ (word_t) vaddr); 
}

inline addr_t pgent_t::vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map )
{ 
    return (addr_t) (this->get_linknode() ^ (word_t) map); 
}

inline word_t pgent_t::rights (space_t * s, pgsize_e pgsize)
{ 
    return ((is_readable(s, pgsize) ? (1<<2) : 0) | 
	    (is_writable(s, pgsize) ? (1<<1) : 0) |
	    (is_executable(s, pgsize) ? (1<<0) : 0));
}

inline word_t pgent_t::attributes ( space_t * s, pgsize_e pgsize )
{
    return (raw & PPC_PAGE_CACHE_INHIBIT) ? 1 : 0;
}

inline word_t pgent_t::reference_bits( space_t *s, pgsize_e pgsize, 
	addr_t vaddr )
{
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

// Modification

inline void pgent_t::flush( space_t *s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{

}

inline void pgent_t::clear( space_t * s, pgsize_e pgsize, bool kernel, 
	addr_t vaddr )
{ 
    pgent_t tmp;
    tmp.raw = this->raw;

    this->raw = 0;
    if( !kernel )
	this->set_linknode(0);
}

inline void pgent_t::make_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    this->raw = (word_t)kmem.alloc( kmem_pgtab, POWERPC_PAGE_SIZE * (kernel ? 1:2) );

    /* the following is a no-op */
    this->tree.is_subtree = cache_subtree;

    if( this->raw )
	this->tree.valid = 1;
}

inline void pgent_t::remove_subtree( space_t * s, pgsize_e pgsize, bool kernel )
{
    addr_t ptab = (addr_t) this->address( s, pgsize );
    this->raw = 0;

    kmem.free( kmem_pgtab, ptab, POWERPC_PAGE_SIZE * (kernel ? 1:2) );
}

inline void pgent_t::set_entry( space_t * s, pgsize_e pgsize, paddr_t paddr,
				word_t rwx, word_t attrib, bool kernel )
{
    this->raw = paddr & POWERPC_PAGE_MASK;
    this->map.erpn = (paddr >> 32) & 0xf;
    this->map.read = rwx >> 2 & 1;
    this->map.write = rwx >> 1 & 1;
    this->map.execute = rwx >> 0 & 1;
    this->map.caching = attrib;
}


inline void pgent_t::update_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if (rwx & 4) this->map.read = 1;
    if (rwx & 2) this->map.write = 1;
    if (rwx & 1) this->map.execute = 1;
}

inline void pgent_t::revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{ 
    if (rwx & 4) this->map.read = 0;
    if (rwx & 2) this->map.write = 0;
    if (rwx & 1) this->map.execute = 0;
}

inline void pgent_t::set_rights( space_t *s, pgsize_e pgsize, word_t rwx )
{
    this->map.read = rwx & 4 ? 1 : 0;
    this->map.write = rwx & 2 ? 1 : 0;
    this->map.execute = rwx & 1;
}

inline void pgent_t::set_attributes ( space_t * s, pgsize_e pgsize, word_t attrib )
{
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
    printf("%s",
	   map.caching == 1 ? "inhibit " : 
	   map.caching == 2 ? "coherent " :
	   map.caching == 3 ? "guarded " :
	   map.caching == 4 ? "write-through " : "");
}
