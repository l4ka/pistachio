/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	glue/v4-powerpc64/pgent.h
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
 * $Id: pgent.h,v 1.10 2006/11/17 17:02:04 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC64__PGENT_H__
#define __GLUE__V4_POWERPC64__PGENT_H__

#include INC_ARCH(pgtab.h)
#include INC_ARCH(pghash.h)

class space_t;
class mapnode_t;

/* Page table tree layout
 *
 * We support 56 bits of address space only!
 *
 * Top 6-8-9-9-6-6 Leaf
 *
 * This supports hardware page sizes 4k and 16M
 * 
 */

#if CONFIG_PLAT_OFPOWER4 || CONFIG_CPU_POWERPC64_PPC970

#define HW_PGSHIFTS		{ 12, 18, 24, 33, 42, 50, 56 }
#define HW_VALID_PGSIZES	((1 << 12))

#define MDB_PGSHIFTS		{ 12, 18, 24, 34, 42, 50, 56 }
#define MDB_NUM_PGSIZES		(6)

#define PPC64_SIZE_4k_LEVEL	((1ul << 6) * 8)
#define PPC64_SIZE_16m_LEVEL	((1ul << 9) * 8)

#elif CONFIG_PLAT_OFPOWER3

#define HW_PGSHIFTS		{ 12, 20, 28, 36, 44, 51, 58, 64 }
#define HW_VALID_PGSIZES	((1 << 12))

#define MDB_PGSHIFTS		{ 12, 20, 28, 36, 44, 51, 58, 64 }
#define MDB_NUM_PGSIZES		(7)

#define PPC64_SIZE_4k_LEVEL	((1ul << 8) * 8)

#endif

class pgent_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t is_valid	: 1;	// 1 if a valid entry, 0 if subtree
	    word_t is_subtree	: 1;	// 1 if a valid subtree, second_hash if valid entry.
	    word_t __pad	: 14;
	    word_t subtree	: 48;	// Pointer to subtree (physical)
	} tree;
	struct {
	    /* This is structured so that it can be directly inserted into
	     * the page hash table, while masking out the hardware's reserved
	     * bits.
	     */
	    word_t is_valid	: 1;	// 1 if a valid entry
	    word_t second_hash	: 1;	// 1 if used the second hash, must be 0 if not valid.
	    word_t rpn		: 50;
	    word_t pteg_slot	: 3;	// The index into the pte group.
	    word_t referenced	: 1;
	    word_t changed	: 1;
	    word_t wimg		: 4;
	    word_t noexecute	: 1;
	    word_t pp		: 2;
	} map;
    };

    enum pgsize_e {
#if CONFIG_PLAT_OFPOWER4 || CONFIG_CPU_POWERPC64_PPC970
	size_4k = 0,	// 12
	size_256k = 1,	// 18
	size_16m = 2,	// 24
	size_8g = 3,	// 33
	size_4t = 4,	// 42
	size_1p = 5,	// 50
	size_max = size_1p,
	size_64p = 6,	// 56
	size_16e = 7	// 64
#elif CONFIG_PLAT_OFPOWER3
	size_4k = 0,	// 12
	size_1m = 1,	// 20
	size_256m = 2,	// 28
	size_64g = 3,	// 36
	size_16t = 4,	// 44
	size_2p = 5,	// 51
	size_256p = 6,	// 58
	size_max = size_256p,
	size_16e = 7,	// 64
#endif
    };

    enum permission_e {
	kernel_only = 0,	// Kernel RW, User None
	user_read_only = 1,	// Kernel RW, User Read Only
	read_write = 2,		// Both RW
	read_only = 3		// Both RO
    };

    enum wimg_e {
	guarded = 1,
	coherent = 2,
	cache_inhibit = 4,
	write_through = 8,
	l4default = coherent
    };

private:

    // Page hash synchronization

    inline void update_from_pghash( space_t * s, addr_t vaddr, bool large );

    // Linknode access 

    inline word_t get_linknode( space_t * s, pgsize_e pgsize );
    inline void set_linknode( space_t * s, pgsize_e pgsize, word_t val );

public:

    // Predicates

    inline bool is_valid( space_t * s, pgsize_e pgsize );
    inline bool is_writable( space_t * s, pgsize_e pgsize );
    inline bool is_readable( space_t * s, pgsize_e pgsize );
    inline bool is_executable( space_t * s, pgsize_e pgsize );
    inline bool is_subtree( space_t * s, pgsize_e pgsize );
    inline bool is_kernel( space_t * s, pgsize_e pgsize );
    /* inline bool is_kernel_writeable( space_t * s, pgsize_e pgsize ); */

    // Retrieval

    inline addr_t address( space_t * s, pgsize_e pgsize );
    inline pgent_t * subtree( space_t * s, pgsize_e pgsize );
    inline mapnode_t * mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr );
    inline addr_t vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map );
    inline word_t reference_bits( space_t *s, pgsize_e pgsize, addr_t vaddr );
    inline word_t get_pte( space_t *s );
    inline word_t attributes( space_t * s, pgsize_e pgsize );

    // Modification

    inline void flush( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    inline void clear( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    inline void make_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    inline void remove_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    inline void set_entry( space_t * s, pgsize_e pgsize, addr_t paddr,
			   word_t attrib, bool kernel = false );
    inline void update_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    inline void revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    inline void reset_reference_bits( space_t *s, pgsize_e pgsize );
    inline void update_reference_bits( space_t *s, pgsize_e pgsz, word_t rwx );
    inline void set_accessed( space_t *s, pgsize_e pgsize, word_t flag );
    inline void set_dirty( space_t *s, pgsize_e pgsize, word_t flag );
    inline void set_linknode( space_t * s, pgsize_e pgsize,
	    mapnode_t * map, addr_t vaddr );
    inline void set_attributes( space_t *s, pgsize_e pgsize, word_t attrib );

    // Movement

    inline pgent_t * next( space_t * s, pgsize_e pgsize, word_t num );

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize);
};

#endif	/* __GLUE__V4_POWERPC64__PGENT_H__ */
