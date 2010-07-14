/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/pgent-swtlb.h
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
#ifndef __ARCH__POWERPC__PGENT_SWTLB_H__
#define __ARCH__POWERPC__PGENT_SWTLB_H__


class space_t;
class mapnode_t;

#define HW_PGSHIFTS		{ 12, 22, 32 }

#define MDB_BUFLIST_SIZES	{ {12}, {8}, {4096}, {0} }
#define MDB_PGSHIFTS		{ 12, 22, 32 }
#define MDB_NUM_PGSIZES		(2)

class pgent_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t subtree	: 20;	// Pointer to subtree
	    word_t is_subtree	: 3;	// cache_subtree if subtree
	    word_t __pad	: 2;
	    word_t erpn		: 4;
	    word_t valid	: 3;	// != 0 if valid
	} tree;
	struct {
	    /* This is structured so that it can be "easily" inserted
	     * into the TLB
	     */
	    word_t epn		: 20;	// we only support 4k
	    word_t caching	: 3;	// cache_e
	    word_t referenced	: 1;	// 23
	    word_t changed	: 1;	// 24
	    word_t erpn		: 4;	// undef, UX, UW, UR, 25-28
	    word_t execute	: 1;	// SX 29
	    word_t write	: 1;	// SW 30
	    word_t read		: 1;	// SR 31
	} map;
    } __attribute__((packed));

    enum cache_e {
	cache_standard = 0,
	cache_inhibited = 1,
	cache_coherent = 2,
	cache_guarded = 3,
	cache_write_through = 4,
	cache_subtree = 7,
    };

    enum pgsize_e {
	size_4k = 0,
	size_4m = 1,
	size_4g = 2,
	size_max = size_4m
    };
private:

    // Linknode access 

    word_t get_linknode( );
    void set_linknode( word_t val );

public:

    // Predicates

    bool is_valid( space_t * s, pgsize_e pgsize );
    bool is_writable( space_t * s, pgsize_e pgsize );
    bool is_readable( space_t * s, pgsize_e pgsize );
    bool is_executable( space_t * s, pgsize_e pgsize );
    bool is_subtree( space_t * s, pgsize_e pgsize );
    bool is_kernel( space_t * s, pgsize_e pgsize );

    // Retrieval

    paddr_t address( space_t * s, pgsize_e pgsize );
    pgent_t * subtree( space_t * s, pgsize_e pgsize );
    mapnode_t * mapnode( space_t * s, pgsize_e pgsize, addr_t vaddr );
    addr_t vaddr( space_t * s, pgsize_e pgsize, mapnode_t * map );
    word_t rights (space_t * s, pgsize_e pgsize);
    word_t reference_bits( space_t *s, pgsize_e pgsize, addr_t vaddr );
    word_t attributes ( space_t * s, pgsize_e pgsize );

    // Modification

    void flush( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    void clear( space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    void make_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    void remove_subtree( space_t * s, pgsize_e pgsize, bool kernel );
    void set_entry( space_t * s, pgsize_e pgsize, paddr_t paddr,
			   word_t rwx, word_t attrib, bool kernel );
    void set_writable( space_t * s, pgsize_e pgsize );
    void set_readonly( space_t * s, pgsize_e pgsize );
    void update_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    void revoke_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    void set_rights( space_t *s, pgsize_e pgsize, word_t rwx );
    void reset_reference_bits( space_t *s, pgsize_e pgsize );
    void update_reference_bits( space_t *s, pgsize_e pgsz, word_t rwx );
    void set_accessed( space_t *s, pgsize_e pgsize, word_t flag );
    void set_dirty( space_t *s, pgsize_e pgsize, word_t flag );
    void set_attributes ( space_t * s, pgsize_e pgsize, word_t attr );
    void set_linknode( space_t * s, pgsize_e pgsize,
		       mapnode_t * map, addr_t vaddr );

    // Movement

    pgent_t * next( space_t * s, pgsize_e pgsize, word_t num );

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize);
};


#endif /* !__ARCH__POWERPC__PGENT_SWTLB_H__ */
