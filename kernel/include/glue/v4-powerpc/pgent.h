/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/pgent.h
 * Description:	The page table entry.  Compatible with the generic linear
 * 		page table walker.
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
 * $Id: pgent.h,v 1.15 2007/01/14 21:36:19 uhlig Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__PGENT_H__
#define __GLUE__V4_POWERPC__PGENT_H__

#include INC_ARCH(pgtab.h)
#include INC_ARCH(pghash.h)

class space_t;
class mapnode_t;

#define HW_PGSHIFTS		{ 12, 22, 32 }
#define HW_VALID_PGSIZES	(1 << 12)

#define MDB_BUFLIST_SIZES	{ {12}, {8}, {4096}, {0} }
#define MDB_PGSHIFTS		{ 12, 22, 32 }
#define MDB_NUM_PGSIZES		(2)


class pgent_t
{
public:
    union {
	word_t		raw;
	struct {
	    word_t subtree	: 20;	// Pointer to subtree
	    word_t __pad	: 11;
	    word_t valid	: 1;	// 1 if a valid subtree.
	} tree;
	struct {
	    /* This is structured so that it can be directly inserted into
	     * the page hash table, while masking out the hardware's reserved
	     * bits.
	     */
	    word_t rpn		: 20;
	    word_t pteg_slot	: 3;	// The index into the pte group.
	    word_t referenced	: 1;
	    word_t changed	: 1;
	    word_t wimg		: 4;
	    word_t second_hash	: 1;	// 1 if used the second hash.
	    word_t pp		: 2;
	} map;
    };

    enum pgsize_e {
	size_4k = 0,
	size_4m = 1,
	size_4g = 2,
	size_max = size_4m
    };

    enum permission_e {
	unused1 = 0,	// read/write
	unused2 = 1,	// read/write
	read_write = 2,
	read_only = 3
    };

private:

    // Page hash synchronization

    inline void update_from_pghash( space_t * s, addr_t vaddr );

    // Linknode access 

    inline word_t get_linknode( void );
    inline void set_linknode( word_t val );

public:

    // Predicates

    inline bool is_valid( space_t * s, pgsize_e pgsize );
    inline bool is_writable( space_t * s, pgsize_e pgsize );
    inline bool is_readable( space_t * s, pgsize_e pgsize );
    inline bool is_executable( space_t * s, pgsize_e pgsize );
    inline bool is_subtree( space_t * s, pgsize_e pgsize );
    inline bool is_kernel( space_t * s, pgsize_e pgsize );

    // Retrieval

    inline addr_t address( space_t * s, pgsize_e pgsize );
    inline pgent_t * subtree( space_t * s, pgsize_e pgsize );
    inline mapnode_t * mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr );
    inline addr_t vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map );
    inline word_t reference_bits( space_t *s, pgsize_e pgsize, addr_t vaddr );
    inline word_t get_translation( space_t *s, pgsize_e pgsize );
    inline word_t attributes ( space_t * s, pgsize_e pgsize );

    // Modification

    inline void flush( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    inline void clear( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    inline void make_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    inline void remove_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    inline void set_entry( space_t * s, pgsize_e pgsize, addr_t paddr,
			   word_t rwx, word_t attrib, bool kernel );
    inline void set_writable( space_t * s, pgsize_e pgsize );
    inline void set_readonly( space_t * s, pgsize_e pgsize );
    inline void update_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    inline void revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    inline void reset_reference_bits( space_t *s, pgsize_e pgsize );
    inline void update_reference_bits( space_t *s, pgsize_e pgsz, word_t rwx );
    inline void set_accessed( space_t *s, pgsize_e pgsize, word_t flag );
    inline void set_dirty( space_t *s, pgsize_e pgsize, word_t flag );
    inline void set_attributes ( space_t * s, pgsize_e pgsize, word_t attr );
    inline void set_linknode( space_t * s, pgsize_e pgsize,
	    mapnode_t * map, addr_t vaddr );

    // Movement

    pgent_t * next( space_t * s, pgsize_e pgsize, word_t num );

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize);
};

#endif	/* __GLUE__V4_POWERPC__PGENT_H__ */
