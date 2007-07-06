/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, University of New South Wales
 *                
 * File path:   glue/v4-sparc64/ultrasparc/pgent_inline.h
 * Description: Inlined functions for pgent_t
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
 * $Id: pgent_inline.h,v 1.6 2006/11/17 17:00:38 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__ULTRASPARC__PGENT_INLINE_H__
#define __GLUE__V4_SPARC64__ULTRASPARC__PGENT_INLINE_H__

#include <kmemory.h>

#include INC_GLUE_API_ARCH(space.h)

EXTERN_KMEM_GROUP(kmem_space);

INLINE tsb_data_t* pgent_t::tsb_data(void)
{
    return (tsb_data_t*)&this->raw;
}

// Predicates

INLINE bool pgent_t::is_valid(space_t * s, pgsize_e pgsize)
{
    return leaf.is_valid || is_subtree(s, pgsize);
}

INLINE bool pgent_t::is_subtree(space_t * s, pgsize_e pgsize)
{
    if(pgsize < size_16m) {
	return leaf.pgsize < (word_t)pgsize;
    } else {
	return tree.is_subtree;
    }
}

INLINE bool pgent_t::is_readable(space_t * s, pgsize_e pgsize)
{
    return leaf.access_bits & READ_ACCESS_BIT;
}

INLINE bool pgent_t::is_writable(space_t * s, pgsize_e pgsize)
{    
    return leaf.access_bits & WRITE_ACCESS_BIT;
}

INLINE bool pgent_t::is_executable(space_t * s, pgsize_e pgsize)
{
    return leaf.access_bits & EXECUTE_ACCESS_BIT;
}

INLINE bool pgent_t::is_kernel(space_t * s, pgsize_e pgsize)
{
    return tsb_data()->privileged;
}

// Retrieval

INLINE addr_t pgent_t::address(space_t * s, pgsize_e pgsize)
{
    return (addr_t)(tsb_data()->pfn << SPARC64_PAGE_BITS);
}

INLINE pgent_t * pgent_t::subtree(space_t * s, pgsize_e pgsize)
{
    if (pgsize < size_16m) {
	return this;
    } else {
	return (pgent_t *) phys_to_virt((addr_t)tree.subtree);
    }
}

INLINE mapnode_t * pgent_t::mapnode(space_t * s, pgsize_e pgsize, addr_t vaddr)
{
    return (mapnode_t *) (this->get_linknode() ^ (word_t) vaddr);
}

INLINE addr_t pgent_t::vaddr(space_t * s, pgsize_e pgsize, mapnode_t * map)
{
    return (addr_t) (this->get_linknode() ^ (word_t) map);
}

INLINE word_t pgent_t::reference_bits(space_t * s, pgsize_e pgsize, addr_t vaddr)
{
    tsbent_t *tsbent;
    u16_t index = tsb_t::index(vaddr, (tlb_t::pgsize_e)pgsize);

    if(pgsize == size_512k || pgsize == size_4m) {
	/* have to search multiple TSB entries */
	UNIMPLEMENTED();
    }

    if(leaf.in_itsb) {
	tsb_t::get(index, &tsbent, 
		   (pgsize == size_8k) ? tsb_t::i8k_tsb : tsb_t::i64k_tsb);

	leaf.ref_bits |= tsbent->get_ref_bits();
    }

    if(leaf.in_dtsb) {
	tsb_t::get(index, &tsbent, 
		   (pgsize == size_8k) ? tsb_t::d8k_tsb : tsb_t::d64k_tsb);

	leaf.ref_bits |= tsbent->get_ref_bits();
    }

    return leaf.ref_bits;
}

INLINE word_t pgent_t::attributes(space_t * s, pgsize_e pgsize)
{
    tsb_data_t* tsbent = this->tsb_data();
    return tsbent->cache_attrib;
}

// Modification

INLINE void pgent_t::clear(space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
{
    pgent_t tmp;
    tmp.raw = raw;

    raw = 0;
    if(!kernel) {
	set_linknode(0);
    }

    tmp.flush(s, pgsize, kernel, vaddr);
}

INLINE void pgent_t::insert(space_t * s, pgent_t::pgsize_e pgsize,
			    addr_t vaddr, tsb_t::tsb_e tsb)
{
    u16_t index = tsb_t::index(vaddr, (tlb_t::pgsize_e)pgsize);

    tsbent_t* tsbent;
    tsb_t::get(index, &tsbent, tsb);

    if(tsbent->get_ref_bits() &&
       tsbent->get_asid() != INVALID_CONTEXT
       && tsbent->get_tlb_valid())
    {
	space_t* evicted_space;
	pgent_t* evicted_pgent;
	pgent_t::pgsize_e evicted_pgsize;

	evicted_space = space_t::lookup_space(tsbent->get_asid());
	if(evicted_space == NULL) {
	    evicted_space = get_kernel_space();
	}

	if(!evicted_space->lookup_mapping(tsbent->get_va(index),
					  &evicted_pgent, &evicted_pgsize))
	{
	    enter_kdebug("Evicting invalid mapping!\n");
	}

	evicted_pgent->leaf.ref_bits |= tsbent->get_ref_bits();
    }

    tsbent->set_data(tsb_data());
    tsbent->set_va(vaddr);
    tsbent->set_asid(s->get_context());

    if(tsb == tsb_t::d8k_tsb || tsb == tsb_t::d64k_tsb) {
	leaf.in_dtsb = 1;
    } else {
	leaf.in_itsb = 1;
    }
}

INLINE void pgent_t::flush(space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
{
    if(pgsize == size_512k || pgsize == size_4m) {
	/* have to search multiple TSB entries */
	UNIMPLEMENTED();
    }

    /* remove the entry from the TSBs, updating reference bits in the process */
    tsbent_t *tsbent;
    u16_t index = tsb_t::index(vaddr, (tlb_t::pgsize_e)pgsize);

    if(leaf.in_itsb) {
	tsb_t::get(index, &tsbent, 
		   (pgsize == size_8k) ? tsb_t::i8k_tsb : tsb_t::i64k_tsb);

	leaf.ref_bits |= tsbent->get_ref_bits();
	tsbent->set_asid(INVALID_CONTEXT);
	leaf.in_itsb = 0;
    }

    if(leaf.in_dtsb) {
	tsb_t::get(index, &tsbent, 
		   (pgsize == size_8k) ? tsb_t::d8k_tsb : tsb_t::d64k_tsb);

	leaf.ref_bits |= tsbent->get_ref_bits();
	tsbent->set_asid(INVALID_CONTEXT);
	leaf.in_dtsb = 0;
    }

    mmu_t::unmap(s->get_context(), vaddr, tlb_t::all_tlb);
}

INLINE void pgent_t::make_subtree(space_t * s, pgsize_e pgsize, bool kernel)
{
    if(pgsize >= size_16m) {
	/* All levels are 10 bits worth of VA, except for the leaf level which has 
	 * 11 bits and is also twice the size to accomodate the SPT */
	addr_t page = kmem.alloc(kmem_space, sizeof(word_t) * 
				 ((pgsize == size_16m) ? (1 << 11) * 2 : (1 << 10)));
	this->tree.subtree = (word_t)virt_to_phys(page);
	this->tree.is_subtree = 1;
	this->tree.is_valid = 0;
    } else {
	leaf.is_valid = 0;
	leaf.pgsize = pgsize - 1;
    }
}

INLINE void pgent_t::remove_subtree(space_t * s, pgsize_e pgsize, bool kernel)
{
    if(pgsize >= size_16m) {
	addr_t page = this->subtree(s, pgsize);
	this->raw = 0;

	kmem.free(kmem_space, page, sizeof(word_t) *
		  (pgsize == size_16m) ? (1 << 11) * 2 : (1 << 10));
    }
}

INLINE void pgent_t::set_entry(space_t * s, pgsize_e pgsize, addr_t paddr,
			       word_t rwx, word_t attrib, bool kernel)
{
    this->raw = 0;
    this->leaf.access_bits = (rwx & 4 ? READ_ACCESS_BIT : 0) |
	(rwx & 2 ? WRITE_ACCESS_BIT : 0) |
	(rwx & 1 ? EXECUTE_ACCESS_BIT : 0);
    this->leaf.pgsize = pgsize;

    tsb_data_t* tsbent = this->tsb_data();
    tsbent->privileged = kernel;
    tsbent->pfn = (word_t)paddr >> SPARC64_PAGE_BITS;
    tsbent->tlb_valid = 1;
    tsbent->cache_attrib = attrib;

    this->leaf.is_valid = 1;
}

INLINE void pgent_t::revoke_rights(space_t * s, pgsize_e pgsize, word_t rwx)
{
    this->leaf.access_bits &= ~rwx;
}

INLINE void pgent_t::update_rights(space_t * s, pgsize_e pgsize, word_t rwx)
{
    this->leaf.access_bits |= rwx;
}

INLINE void pgent_t::reset_reference_bits(space_t * s, pgsize_e pgsize)
{
    leaf.ref_bits = 0;
}

INLINE void pgent_t::update_reference_bits(space_t *s, pgsize_e pgsize,
					   word_t rwx)
{
    leaf.ref_bits |= rwx;
}

INLINE void pgent_t::set_attributes(space_t * s, pgsize_e pgsize,
				    word_t attrib)
{
    tsb_data_t* tsbent = this->tsb_data();
    tsbent->cache_attrib = attrib;
}


INLINE void pgent_t::set_linknode(space_t * s, pgsize_e pgsize, 
				  mapnode_t * map, addr_t vaddr)
{
    set_linknode((word_t) map ^ (word_t) vaddr);
}

// Movement

INLINE pgent_t * pgent_t::next(space_t * s, pgsize_e pgsize, word_t num)
{
    switch(pgsize) {
	/* Following four sizes are the MMU-supported leaf entry sizes; all appear
	 * in the bottom level of the page table */
    case size_8k:
	return this + num;
    case size_64k:
	return this + 8*num;
    case size_512k:
	return this + 64*num;
    case size_4m:
	return this + 512*num;

    default:
	return this + num;
    }
}

// Linknode access

#define PT_BOTTOMLEVEL_SIZE	((1 << 11) * sizeof(pgent_t))

INLINE u64_t pgent_t::get_linknode(void)
{
    return *(word_t *)((word_t) this + PT_BOTTOMLEVEL_SIZE);
}

INLINE void pgent_t::set_linknode(u64_t val)
{
    *(word_t *)((word_t) this + PT_BOTTOMLEVEL_SIZE) = val;
}

#endif /* !__GLUE__V4_SPARC64__ULTRASPARC__PGENT_INLINE_H__ */
