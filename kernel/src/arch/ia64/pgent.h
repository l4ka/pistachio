/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     arch/ia64/pgent.h
 * Description:   Generic page table manipluation for IA-64
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
 * $Id: pgent.h,v 1.24 2006/11/17 17:08:42 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__PGENT_H__
#define __ARCH__IA64__PGENT_H__

#include <kmemory.h>
#include INC_GLUE(hwspace.h)
#include INC_ARCH(tlb.h)

#define IA64_PTAB_SIZE		((1 << 10) * sizeof (u64_t))
#define IA64_USER_PRIV		3
#define IA64_KERNEL_PRIV	0	


#define HW_PGSHIFTS		{12,13,14,16,18,20,22,24,26,28,32,42,52}
#define HW_VALID_PGSIZES	((1 << 12) | (1 << 13) | \
				 (1 << 14) | (1 << 16) | \
				 (1 << 18) | (1 << 20) | \
				 (1 << 22) | (1 << 24) | \
				 (1 << 26) | (1 << 28))

#define MDB_BUFLIST_SIZES	{ {24}, {16}, {32}, {8192}, {128}, {0} }
#define MDB_PGSHIFTS		{12,13,14,16,18,20,22,24,26,28,32,42,52}
#define MDB_NUM_PGSIZES		(12)


EXTERN_KMEM_GROUP (kmem_pgtab);

extern word_t hw_pgshifts[];

class mapnode_t;
class space_t;

class pgent_t
{
public:
    union {
	struct {
	    word_t	subtree		: 61;	// Pointer to subtree
	    word_t	__pad		: 1;
	    word_t	is_subtree	: 1;	// 1 if valid subtree
	    word_t	is_valid	: 1;	// 0 for subtrees
	} tree;
	struct {
	    word_t	__rv		: 53;	// Defined by translation
	    word_t	pgsize		: 4;	// pgsize_e
	    word_t	__pad		: 4;
	    word_t	executed	: 1;
	    word_t	is_subtree	: 1;	// 0 for mapping
	    word_t	is_valid	: 1;	// 1 if valid mapping
	} map;
	u64_t		raw;
    };

    enum pgsize_e {
	size_4k = 0,
	size_8k,   // 1
	size_16k,  // 2
	size_64k,  // 3
	size_256k, // 4 
	size_1m,   // 5
	size_4m,   // 6
	size_16m,  // 7
	size_64m,  // 8
	size_256m, // 9
	size_4g,   // 10
	size_4t,   // 11
	size_4p,   // 12 	// We only allow for 4PB virtual space
	size_max = size_4t
    };

private:

    // Linknode access 

    u64_t get_linknode (void)
	{ return *(u64_t *) ((word_t) this + IA64_PTAB_SIZE); }

    void set_linknode (u64_t val)
	{ *(u64_t *) ((word_t) this + IA64_PTAB_SIZE) = val; }

public:

    translation_t * translation (void)
	{ return (translation_t *) this; }

    void set_referenced (void)
	{ translation ()->set_accessed (); }

    void set_written (void)
	{ translation ()->set_dirty (); }

    void set_executed (void)
	{ map.executed = 1; }


    // Predicates

    bool is_valid (space_t * s, pgsize_e pgsize)
	{
	    return map.is_valid || (map.is_subtree &&
				    map.pgsize < (word_t) pgsize);
	}

    bool is_subtree (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_4t || pgsize == size_4g || pgsize == size_4m)
		return map.is_subtree;
	    else
		return map.pgsize < (word_t) pgsize;
	}

    bool is_readable (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->access_rights () != translation_t::xp_rx;
	}

    bool is_writable (space_t * s, pgsize_e pgsize)
	{
	    return (translation ()->access_rights () & 0x02) &&
	      (translation ()->access_rights () != translation_t::xp_rx);
	}

    bool is_executable (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->access_rights () & 0x01;
	}

    bool is_kernel (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->privilege_level () == IA64_KERNEL_PRIV;
	}

    // Retrieval

    addr_t address (space_t * s, pgsize_e pgsize)
	{
	    return addr_mask (translation ()->phys_addr (),
			      ~((1UL << hw_pgshifts[pgsize]) - 1));
	}
	
    pgent_t * subtree (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_4t || pgsize == size_4g || pgsize == size_4m)
		return (pgent_t *) phys_to_virt (tree.subtree);
	    else
		return (pgent_t *) this;
	}

    mapnode_t * mapnode (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{ return (mapnode_t *) (get_linknode () ^ (u64_t) vaddr); }

    addr_t vaddr (space_t * s, pgsize_e pgsize, mapnode_t * map)
	{ return (addr_t) (get_linknode () ^ (u64_t) map); }

    word_t reference_bits (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{
	    word_t rwx = 0;
	    if (translation ()->is_dirty ())
		rwx = 6;
	    else if (translation ()->is_accessed ())
		rwx = 4;
	    if (map.executed)
		rwx |= 5;
	    return rwx;
	}

    word_t attributes (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->memattrib();
	}

    // Modification

    void clear (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
	{
	    switch (pgsize)
	    {
	    case size_4m: case size_4g: case size_4t: case size_4p:
		raw = 0;
		break;
	    default:
		tree.is_valid = 0;
		tree.is_subtree = 1;
		break;
	    }
	    if (! kernel)
		set_linknode (0);
	}

    void flush (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
	{
	}

    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    switch (pgsize) {
	    case size_4t: case size_4g: case size_4m:
		tree.subtree = (word_t) virt_to_phys
		    (kmem.alloc (kmem_pgtab, kernel
				 ? IA64_PTAB_SIZE : IA64_PTAB_SIZE * 2));
		tree.is_valid = 0;
		tree.is_subtree = 1;
		break;
	    default:
		map.is_valid = 0;
		map.is_subtree = 1;
		map.pgsize = pgsize - 1;
		break;
	    }
	}

    void remove_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    switch (pgsize) {
	    case size_4p: case size_4t: case size_4g: case size_4m:
	    {
		addr_t ptab = (addr_t) phys_to_virt (tree.subtree);
		raw = 0;
		kmem.free (kmem_pgtab,
			   ptab, kernel ? IA64_PTAB_SIZE : IA64_PTAB_SIZE * 2);
		break;
	    }
	    default:
		map.is_subtree = 1;
		map.is_valid = 0;
		map.pgsize = pgsize + 1;
		break;
	    }
	}

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
		    word_t rwx, word_t attrib, bool kernel)
	{
	    translation_t newtr (true, (translation_t::memattrib_e) attrib,
				 kernel, kernel,
				 kernel ? IA64_KERNEL_PRIV : IA64_USER_PRIV,
				 (translation_t::access_rights_e) (rwx & 3),
				 paddr, true);
	    raw = newtr.get_raw ();
	    map.is_valid = 1;
	    map.is_subtree = 0;
	    map.executed = 0;
	    map.pgsize = pgsize;
	}

    void set_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    rwx &= 3;
	    translation ()->set_access_rights
		((translation_t::access_rights_e) rwx);
	}

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    translation_t * tr = translation ();
	    word_t rights = (word_t) tr->access_rights ();
	    if (rights & 4)
		return;
	    if (rwx & 1) rights &= ~1UL;
	    if (rwx & 2) rights &= ~2UL;
	    tr->set_access_rights ((translation_t::access_rights_e) rights);
	}

    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    translation_t * tr = translation ();
	    word_t rights = (word_t) tr->access_rights ();
	    if (rights & 4)
		return;
	    if (rwx & 1) rights |= 1;
	    if (rwx & 2) rights |= 2;
	    tr->set_access_rights ((translation_t::access_rights_e) rights);
	}

    void reset_reference_bits (space_t * s, pgsize_e pgsize)
	{ translation ()->reset_reference_bits (); map.executed = 0; }

    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    if (rwx) translation ()->set_accessed ();
	    if (rwx & 0x2) translation ()->set_dirty ();
	    if (rwx & 0x1) map.executed = 1;
	}

    void set_attributes (space_t * s, pgsize_e pgsize, word_t attrib)
	{
	    translation ()->set_memattrib
		((translation_t::memattrib_e) attrib);
	}

    void set_linknode (space_t * s, pgsize_e pgsize,
			      mapnode_t * map, addr_t vaddr)
	{ set_linknode ((u64_t) map ^ (u64_t) vaddr); }

    // Movement

    pgent_t * next (space_t * s, pgsize_e pgsize, word_t num)
	{
	    switch (pgsize) {
	    default:
	    case size_4p: case size_4t: case size_4g:
	    case size_4m: case size_4k:
		return this + num;
	    case size_8k:
		return this + 2*num;
	    case size_16k: case size_16m:
		return this + 4*num;
	    case size_64k: case size_64m:
		return this + 16*num;
	    case size_256k: case size_256m:
		return this + 64*num;
	    case size_1m:
		return this + 256*num;
	    }
	}

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	}
};


#endif /* !__ARCH__IA64__PGENT_H__ */
