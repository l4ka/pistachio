/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  University of New South Wales
 *                
 * File path:     arch/mips64/pgent.h
 * Description:   Generic page table manipulation for MIPS64
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
 * $Id: pgent.h,v 1.26 2006/11/17 17:07:12 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS64__PGENT_H__
#define __ARCH__MIPS64__PGENT_H__

#include <kmemory.h>
#include <debug.h>

#include INC_GLUE(config.h)
#include INC_GLUE(hwspace.h)
#include INC_ARCH(page.h)

#include INC_ARCH(tlb.h)


#define MIPS64_PTAB_SIZE	((1 << 10) * sizeof (u64_t))

#define MDB_PGSHIFTS		HW_PGSHIFTS


EXTERN_KMEM_GROUP (kmem_pgtab);

extern word_t hw_pgshifts[];

class mapnode_t;
class space_t;

class pgent_t
{
public:
    union {
	struct {
	    BITFIELD5(word_t,
		subtree		: 56,	/* Pointer to subtree */
		pgsize		: 4,	/* pgsize_e */
		is_subtree	: 1,	/* 1 if valid subtree */
		is_valid	: 1,	/* 0 for subtrees */
		__rv2		: 2	/* Defined by translation */
	    )
	} tree;
	struct {
	    BITFIELD6(word_t,
		__rv1		: 30,	/* Defined by translation */
		__pad		: 26,
		pgsize		: 4,	/* pgsize_e */
		is_subtree	: 1,	/* 0 for mapping */
		is_valid	: 1,	/* 1 if valid mapping */
		__rv2		: 2	/* Defined by translation */
	    )
	} map;
	u64_t		raw;
    };

    enum pgsize_e {
		size_4k = 0,
		size_16k,  /* 1	*/
		size_64k,  /* 2	*/
		size_256k, /* 3	*/
		size_1m,   /* 4	*/
		size_4m,   /* 5	*/
		size_16m,  /* 6	*/
#if CONFIG_MIPS64_ADDRESS_BITS == 40
		size_4g,   /* 7 */
		size_4t,   /* 8 */

		/* XXX stubs for below */
		size_64m,  /* 9*/
		size_256m, /* 10 */
		size_4p,  /* 11 */

		size_max = size_4g
#elif CONFIG_MIPS64_ADDRESS_BITS == 44
		size_64m,  /* 7	*/
		size_256m, /* 8	*/
		size_4g,   /* 9 */
		size_4t,   /* 10*/
		size_4p,  /* 11 */
		
		size_max = size_4g
#else
#error We only support 40 and 44 bit address spaces!
#endif
    };

private:

    // Linknode access 

    u64_t get_linknode (void)
	{ return *(u64_t *) ((word_t) this + MIPS64_PTAB_SIZE); }

    void set_linknode (u64_t val)
	{ *(u64_t *) ((word_t) this + MIPS64_PTAB_SIZE) = val; }

public:

    translation_t * translation (void)
	{ return (translation_t *) this; }

    // Predicates

    bool is_valid (space_t * s, pgsize_e pgsize)
	{
	    return map.is_valid || (map.is_subtree &&
				    map.pgsize < (word_t) pgsize);
	}

    bool is_subtree (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_4t || pgsize == size_4g || pgsize == size_4m)
		return tree.is_subtree;
	    else
		return map.pgsize < (word_t) pgsize;
	}

    bool is_readable (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->get_valid ();
	}

    bool is_writable (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->get_dirty () &&
		    translation ()->get_valid ();
	}

    bool is_executable (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->get_valid ();
	}

    bool is_kernel (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->is_kernel();
	}

    word_t attributes (space_t * s, pgsize_e pgsize)
	{
	    return translation ()->memattrib();
	}

    // Retrieval

    addr_t address (space_t * s, pgsize_e pgsize)
	{
	    return addr_mask (translation ()->phys_addr (),
			      ~((1UL << hw_pgshifts[pgsize]) - 1));
	}
	
    pgent_t * subtree (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_4t  || pgsize == size_4g || pgsize == size_4m)
		return (pgent_t *) tree_to_addr ((addr_t) raw);
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
	    if (translation ()->get_dirty ())
		rwx = 6;
	    else if (translation ()->get_valid())
		rwx = 4;
	    return rwx;
	}

    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    // XXX: Implement me
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
		tree.subtree = (word_t)
		    kmem.alloc (kmem_pgtab,
				kernel ? MIPS64_PTAB_SIZE : MIPS64_PTAB_SIZE * 2);
		tree.is_valid = 0;
		tree.is_subtree = 1;
		tree.pgsize = pgsize - 1;
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
		//addr_t ptab = mips64_phys_to_virt (7, (addr_t) tree.subtree);
		addr_t ptab = (addr_t) raw; /* FIXME */
		raw = 0;
		kmem.free (kmem_pgtab, tree_to_addr(ptab), 
			   kernel ? MIPS64_PTAB_SIZE : MIPS64_PTAB_SIZE * 2);
		break;
	    }
	    default:
		map.is_subtree = 1;
		map.is_valid = 0;
		map.pgsize = pgsize + 1;
		break;
	    }
	}

    /* This is technically an assignment operator */
    void set_entry(space_t *space, pgsize_e pgsize, pgent_t pgent)
	{
	    raw = pgent.raw;
	}

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
		    word_t rwx, bool kernel);
	{
	    translation_t newtr (
				 translation_t::l4default,
				 readable || writable, writable,
				 kernel, paddr);
	    raw = newtr.get_raw ();
	    map.is_valid = 1;
	    map.is_subtree = 0;
	    map.pgsize = pgsize;
	}

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
		    word_t rwx, word_t attrib, bool kernel)
	{
	    translation_t newtr (
	    			 (translation_t::memattrib_e)attrib,
				 readable || writable, writable,
				 kernel, paddr);
	    raw = newtr.get_raw ();
	    map.is_valid = 1;
	    map.is_subtree = 0;
	    map.pgsize = pgsize;
	}

    void set_attributes (space_t * s, pgsize_e pgsize, word_t attrib)
	{
	}

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    translation_t * tr = translation ();
	    if (rwx & 2) tr->set_dirty(0);
	}

    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    translation_t * tr = translation ();
	    if (rwx & 2) tr->set_dirty(1);
	}

    void reset_reference_bits (space_t * s, pgsize_e pgsize)
	{ translation ()->reset_reference_bits (); }

    void set_linknode (space_t * s, pgsize_e pgsize,
			      mapnode_t * map, addr_t vaddr)
	{ set_linknode ((u64_t) map ^ (u64_t) vaddr); }

    // Movement

    pgent_t * next (space_t * s, pgsize_e pgsize, word_t num)
	{
	    /* 2^ (22, 32, 42) -> next level */
	    switch (pgsize) {
	    case size_16k: case size_16m: //case size_16t:
		return this + 4*num;
	    case size_64k: case size_64m:
		return this + 16*num;
	    case size_256k: case size_256m:
		return this + 64*num;
	    case size_1m: //case size_1t:
		return this + 256*num;

	    case size_4p: case size_4t: case size_4g: case size_4m: case size_4k:
		return this + num;
	    }
	}

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	}
};


#endif /* !__ARCH__MIPS64__PGENT_H__ */
