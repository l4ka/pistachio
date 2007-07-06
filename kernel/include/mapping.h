/*********************************************************************
 *                
 * Copyright (C) 2000, 2001, 2002, 2003, 2005,  Karlsruhe University
 *                
 * File path:     mapping.h
 * Description:   Generic mapping databse structures
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
 * $Id: mapping.h,v 1.7 2005/12/22 12:35:58 stoess Exp $
 *                
 ********************************************************************/
#ifndef __MAPPING_H__
#define __MAPPING_H__

class space_t;
class pgent_t;
class mapnode_t;
class rootnode_t;

#include INC_ARCH(pgent.h)
#include INC_API(fpage.h)


/**
 * mdb_pgshifts: page sizes supported by the mapping database
 *
 * The mdb_pgshifts[] is an architecture specific arrray defining the
 * page sizes that should be supported by the mapping databse.
 * Actually, it defines the shift numbers rather than the page sizes.
 * The last entry in the array should define the whole address space.
 * E.g, if page sizes of 1MB, 64KB, 4KB, and 1KB are supported, the
 * array would (on a 32 bit address space) contain the values: 10, 12,
 * 16, 20, and 32.
 *
 */
extern word_t mdb_pgshifts[];


class mdb_mng_t;
class mdb_buflist_t
{
public:
    word_t	size;
    mdb_mng_t	*list_of_lists;
    word_t	max_free;
};


/**
 * mdbbuflists: allocation sizes for mapping database
 *
 * The mdb_buflists[] array defines the buffer sizes that will be
 * needed by the mapping database.  These sizes depend on the page
 * sizes to be supported, and must also contain the number 3*wordsize
 * (for mapnode_t) and 2*wordsize (for dualnote_t).  The array should
 * be terminated by defining a zero buffer size.
 *
 */
extern mdb_buflist_t mdb_buflists[];


/* Top level mapping node for sigma0. */
extern mapnode_t * sigma0_mapnode;



/**
 * dualnode_t: node containing pointers to root array and mapping tree
 */
class dualnode_t
{
public:
    mapnode_t	*map;
    rootnode_t	*root;
};

#ifndef MDB_SPACE_BITS
#define MDB_SPACE_BITS	(BITS_WORD - 11)
#endif

/**
 * mapnode_t: node for holding a mapping databse entry
 */
class mapnode_t
{
    union {
	struct {
	    word_t is_prev_root		: 1;
	    word_t prev_ptr		: BITS_WORD - 1;
	    word_t tree_depth		: BITS_WORD - MDB_SPACE_BITS - 3;
	    word_t rwx			: 3;
	    word_t space		: MDB_SPACE_BITS;
	    word_t is_next_root		: 1;
	    word_t is_next_map		: 1;
	    word_t next_ptr		: BITS_WORD - 2;
	} x;
	word_t raw[3];
    };

public:

    enum pgsize_e {
	size_max = MDB_NUM_PGSIZES-1
    };

    // Previous pointer and backing pointer

    pgent_t * get_pgent (mapnode_t * prev)
	{
	    return (pgent_t *) ((x.prev_ptr << 1) ^ (word_t) prev);
	}

    pgent_t * get_pgent (rootnode_t * prev)
	{
	    return (pgent_t *) ((x.prev_ptr << 1) ^ (word_t) prev);
	}

    mapnode_t * get_prevmap (pgent_t * pg)
	{
	    if (x.is_prev_root)
		return (mapnode_t *) NULL;
	    else
		return (mapnode_t *) ((x.prev_ptr << 1) ^ (word_t) pg);
	}

    rootnode_t * get_prevroot (pgent_t * pg)
	{
	    if (! x.is_prev_root)
		return (rootnode_t *) NULL;
	    else
		return (rootnode_t *) ((x.prev_ptr << 1) ^ (word_t) pg);
	}

    void set_backlink (mapnode_t * prev, pgent_t * pg)
	{
	    x.prev_ptr = ((word_t) prev ^ (word_t) pg) >> 1;
	    x.is_prev_root = 0;
	}

    void set_backlink (rootnode_t * prev, pgent_t * pg)
	{
	    x.prev_ptr = ((word_t) prev ^ (word_t) pg) >> 1;
	    x.is_prev_root = 1;
	}

    bool is_prev_root (void)
	{
	    return x.is_prev_root;
	}


    // Next pointer

    mapnode_t * get_nextmap (void)
	{
	    if (! x.is_next_map)
		return (mapnode_t *) NULL;
	    else if (x.is_next_root)
		return (mapnode_t *) ((dualnode_t *) (x.next_ptr << 2))->map;
	    else
		return (mapnode_t *) (x.next_ptr << 2);
	}

    rootnode_t * get_nextroot (void)
	{
	    if (! x.is_next_root)
		return (rootnode_t *) NULL;
	    else if (x.is_next_map)
		return (rootnode_t *) ((dualnode_t *) (x.next_ptr << 2))->root;
	    else
		return (rootnode_t *) (x.next_ptr << 2);
	}

    dualnode_t * get_nextdual (void)
	{
	    if (x.is_next_root && x.is_next_map)
		return (dualnode_t *) (x.next_ptr << 2);
	    else
		return (dualnode_t *) NULL;
	}

    void set_next (mapnode_t * map)
	{
	    x.next_ptr = ((word_t) map) >> 2;
	    x.is_next_root = 0;
	    x.is_next_map = 1;
	}

    void set_next (rootnode_t *root)
	{
	    x.next_ptr = ((word_t) root) >> 2;
	    x.is_next_root = 1;
	    x.is_next_map = 0;
	}

    void set_next (dualnode_t * dual)
	{
	    x.next_ptr = ((word_t) dual) >> 2;
	    x.is_next_root = 1;
	    x.is_next_map = 1;
	}

    bool is_next_root (void)
	{
	    return x.is_next_root && !x.is_next_map;
	}

    bool is_next_map (void)
	{
	    return x.is_next_map && !x.is_next_root;
	}

    bool is_next_both (void)
	{ 
	    return x.is_next_root && x.is_next_map;
	}

    // Address space identifier

    space_t * get_space (void)
	{
	    return (space_t *) (x.space << (BITS_WORD - MDB_SPACE_BITS));
	}

    void set_space (space_t * space)
	{
	    x.space = ((word_t) space) >> (BITS_WORD - MDB_SPACE_BITS);
	}

    // Referenced bits

    word_t get_rwx (void)
	{
	    return x.rwx;
	}

    void set_rwx (word_t rwx)
	{
	    x.rwx = rwx;
	}

    void update_rwx (word_t rwx)
	{
	    x.rwx |= rwx;
	}

    // Tree depth

    word_t get_depth (void)
	{
	    return x.tree_depth;
	}

    void set_depth (word_t depth)
	{
	    x.tree_depth = depth;
	}

} __attribute__ ((packed));



/**
 * rootnode_t: root array node representing a physical page frame
 */
class rootnode_t
{
public:
    union {
	struct {
	    word_t is_next_root		: 1;
	    word_t is_next_map		: 1;
	    word_t next_ptr		: BITS_WORD - 2;
	} x;
	word_t raw;
    };

    mapnode_t * get_map (void)
	{
	    if (! x.is_next_map)
		return (mapnode_t *) NULL;
	    else if (x.is_next_root)
		return (mapnode_t *) ((dualnode_t *) (x.next_ptr << 2))->map;
	    else
		return (mapnode_t *) (x.next_ptr << 2);
	}

    rootnode_t * get_root (void)
	{
	    if (! x.is_next_root)
		return (rootnode_t *) NULL;
	    else if (x.is_next_map)
		return (rootnode_t *) ((dualnode_t *) (x.next_ptr << 2))->root;
	    else
		return (rootnode_t *) (x.next_ptr << 2);
	}

    dualnode_t * get_dual (void)
	{
	    if (x.is_next_root && x.is_next_map)
		return (dualnode_t *) (x.next_ptr << 2);
	    else
		return (dualnode_t *) NULL;
	}

    void set_ptr (mapnode_t * map)
	{
	    x.next_ptr = (word_t) map >> 2;
	    x.is_next_root = 0;
	    x.is_next_map = 1;
	}

    void set_ptr (rootnode_t * root)
	{
	    x.next_ptr = (word_t) root >> 2;
	    x.is_next_root = 1;
	    x.is_next_map = 0;
	}

    void set_ptr (dualnode_t * dual)
	{
	    x.next_ptr = (word_t) dual >> 2;
	    x.is_next_root = 1;
	    x.is_next_map = 1;
	}

    bool is_next_root (void)
	{
	    return x.is_next_root && !x.is_next_map;
	}

    bool is_next_map (void)
	{
	    return x.is_next_map && !x.is_next_root;
	}

    bool is_next_both (void)
	{ 
	    return x.is_next_root && x.is_next_map;
	}
};



/**
 * mdb_pgshifts: array of bit-shifts for mapping db page tables
 *
 * Array containing the actual page sizes (as bit-shifts) for the
 * various mapping databage page size numbers.  Array is indexed by
 * page size number.  Last entry must be the bit-shift for the
 * complete mappable address space.
 */
extern word_t mdb_pgshifts[];


/*
 * Define some operators on the pgsize enum to make code more
 * readable.
 */

INLINE mapnode_t::pgsize_e operator-- (mapnode_t::pgsize_e & l, int)
{
    mapnode_t::pgsize_e ret = l;
    l = (mapnode_t::pgsize_e) ((word_t) l - 1);
    return ret;
}

INLINE mapnode_t::pgsize_e operator++ (mapnode_t::pgsize_e & l, int)
{
    mapnode_t::pgsize_e ret = l;
    l = (mapnode_t::pgsize_e) ((word_t) l + 1);
    return ret;
}

INLINE mapnode_t::pgsize_e operator+ (mapnode_t::pgsize_e l, int r)
{
    return (mapnode_t::pgsize_e) ((word_t) l + r);
}

INLINE mapnode_t::pgsize_e operator- (mapnode_t::pgsize_e l, int r)
{
    return (mapnode_t::pgsize_e) ((word_t) l - r);
}



/*
 * Function prototypes
 */

/* From generic/mapping.cc */
mapnode_t * mdb_map (mapnode_t * f_map, pgent_t * f_pg,
		     pgent_t::pgsize_e f_hwpgsize, addr_t f_addr,
		     pgent_t * t_pg, pgent_t::pgsize_e t_hwpgsize,
		     space_t * t_space, bool grant);
word_t mdb_flush (mapnode_t * f_map, pgent_t * f_pg,
		  pgent_t::pgsize_e f_hwpgsize, addr_t f_addr,
		  pgent_t::pgsize_e t_hwpgsize, fpage_t fp, bool unmap_self);
void init_mdb (void);

/* From generic/mapping_alloc.cc */
addr_t mdb_alloc_buffer (word_t size);
void mdb_free_buffer (addr_t addr, word_t size);


#endif /* !__MAPPING_H__ */
