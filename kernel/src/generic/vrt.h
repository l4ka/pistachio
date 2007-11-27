/*********************************************************************
 *                
 * Copyright (C) 2005-2007,  Karlsruhe University
 *                
 * File path:     generic/vrt.h
 * Description:   Generic Variable Radix Tables
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
 * $Id: vrt.h,v 1.9 2006/06/12 17:02:30 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __VRT_H__
#define __VRT_H__

#include INC_API(fpage.h)
#include <mdb.h>

#if !defined(MAX_VRT_DEPTH)
#define MAX_VRT_DEPTH 10
#endif

class vrt_node_t;
class vrt_table_t;
class mdb_node_t;
class mdb_t;


/**
 * The vrt_t specifies a particular variable radix table.  Certain
 * operations on the VRT (e.g., map and mapctrl) are generic.  Other
 * operations like operating on access rights are defined on a per VRT
 * basis.
 */
class vrt_t
{
    vrt_table_t * root_table;

public:

    // VRT specific methods

    virtual word_t get_radix (word_t objsize);
    virtual word_t get_next_objsize (word_t objsize);
    virtual word_t get_vrt_size (void);
    virtual mdb_t * get_mapdb (void);
    virtual const char * get_name (void);

    // Node specific methods

    virtual void set_object (vrt_node_t * n, word_t n_sz, word_t paddr,
			     vrt_node_t * o, word_t o_sz, word_t access);
    virtual word_t get_address (vrt_node_t * n);
    virtual word_t make_misc (vrt_node_t * obj, mdb_node_t * map);
    virtual void dump (vrt_node_t * n);

    // Generic methods

    void set_table (vrt_table_t * t) { root_table = t; }
    vrt_table_t * get_table (void) { return root_table; }
    bool lookup (word_t addr, word_t * value, word_t * objsize);
    void map_fpage (fpage_t f_fp, word_t base, vrt_t * t_space,
		    fpage_t t_fp, bool grant);
    word_t mapctrl (fpage_t fp, mdb_t::ctrl_t ctrl,
		    word_t rights, word_t attrib);
    
    word_t flush (fpage_t fp)
	{ return mapctrl (fp, mdb_t::ctrl_t::flush (), 0, 0); }
};


/**
 * The vrt_node_t specfies either an object or table pointer within a
 * VRT table.
 */
class vrt_node_t
{
    union {
	word_t raw;
	struct {
	    word_t is_table_ptr	: 1;
	    word_t value	: BITS_WORD - 1;
	};
    };

public:

    // Predicates

    bool is_valid (void);
    bool is_table (void);

    // Retrieval

    word_t get_object (void);
    vrt_table_t * get_table (void);

    // Modification

    void clear (void);
    void set_object (vrt_t * vrt, word_t this_size, word_t paddr,
		     vrt_node_t * obj, word_t obj_size, word_t access);
    void set_object (word_t objvalue);
    void set_table (vrt_table_t * t);

    // Wrappers

    word_t get_address (vrt_t * vrt)
	{ return vrt->get_address (this); }

    friend class kdb_t;
};


/**
 * The vrt_table_t is an array of objects of a given size.  The size
 * of the array is variable and is specified upon creation of the
 * table.  The entries in the table are actually vrt_node_t
 * structures.  They can be an object or a pointer to a sub-table.
 *
 * The table also contains pointers to nodes in the mapping database.
 * These pointers are not stored directly in the table entry.  They
 * are stored in a shadow table directly after the main table.  The
 * reason for doing this is to lower the cache footprint for table
 * lookups and table scans.
 *
 * The table structure also supports short-circuiting the lookup from
 * higher up in the table (i.e., path compression).
 */
class vrt_table_t
{
    struct {
	word_t radix		: 6;
	word_t objsize		: 6;
	word_t __pad		: BITS_WORD - 12;
	word_t prefix		: BITS_WORD ;
	word_t entries		: BITS_WORD;
    };

public:

    void * operator new (size_t size, word_t radix_log2);
    void operator delete (void * t);

    word_t get_addr (word_t idx) { return (1UL << objsize) * idx; }

    // Predicates

    bool match_prefix (word_t addr);

    // Retrieval

    vrt_node_t * get_node (word_t addr);
    mdb_node_t * get_mapnode (word_t addr);
    vrt_table_t * get_table (word_t addr);
    word_t get_radix (void);
    word_t get_prefix (void);
    word_t get_objsize (void);
    word_t get_start_addr (void);
    word_t get_end_addr (void);

    // Modification

    void clear (word_t addr);
    void set_object (vrt_t * vrt, word_t addr, word_t paddr,
		     vrt_node_t * obj, word_t obj_size, word_t acc);
    void set_mapnode (word_t addr, mdb_node_t *);
    void set_table (word_t addr, vrt_table_t * t);
    void set_prefix (word_t p);
    void set_objsize (word_t s_log2);

    friend class kdb_t;
};





/*
**
**	vrt_node_t
**
*/


/**
 * Check if node is valid.
 * @return true if node is valid, false otherwise
 */
INLINE bool vrt_node_t::is_valid (void)
{
    return raw != 0;
}

/**
 * Check if node entry is valid a table pointer.
 * @return true if node is table pointer, false otherwise
 */
INLINE bool vrt_node_t::is_table (void)
{
    return is_table_ptr;
}

/**
 * Retrieve object stored in node.
 * @return object in raw format
 */
INLINE word_t vrt_node_t::get_object (void)
{
    return value;
}

/**
 * Retrieve table pointer stored in node.
 * @return table pointer, or NULL if there is no table
 */
INLINE vrt_table_t * vrt_node_t::get_table (void)
{
    return (vrt_table_t *) (is_table_ptr ? (value << 1) : 0);
}

/**
 * Clear node.
 */
INLINE void vrt_node_t::clear (void)
{
    raw = 0;
}

/**
 * Copy contents into object.
 * @param vrt		vrt object
 * @param this_size	size of current object
 * @param paddr		physical address
 * @param obj		object to copy
 * @param obj_size	size of object to copy
 * @param access	access right to apply to new object
 */
INLINE void vrt_node_t::set_object (vrt_t * vrt, word_t this_size,
				    word_t paddr, vrt_node_t * obj,
				    word_t obj_size, word_t access)
{
    vrt->set_object (this, this_size, paddr,
		     obj, obj_size, access);
    is_table_ptr = 0;
}

/**
 * Set raw contents of objects.
 * @param value		raw contents of object
 */
INLINE void vrt_node_t::set_object (word_t objvalue)
{
    value = objvalue;
    is_table_ptr = 0;
}

/**
 * Set table pointer.
 * @param t		pointer to table
 */
INLINE void vrt_node_t::set_table (vrt_table_t * t)
{
    value = (word_t) t >> 1;
    is_table_ptr = 1;
}





/*
**
**	vrt_table_t
**
*/


/**
 * Check if the table prefix matches the supplied address.
 * @param addr		address to match
 * @return true if prefix matches, false otherwise
 */
INLINE bool vrt_table_t::match_prefix (word_t addr)
{
    // Need to do shift operation twice instead of adding objsize and
    // radix, or else gcc somehow manages to optimize away the
    // operation altogether.
    return ((addr ^ prefix) & ~(((1UL << objsize) << radix) - 1)) == 0;
}

/**
 * Retrieve node entry.
 * @param addr		address to use for indexing
 * @return pointer to table entry
 */
INLINE vrt_node_t * vrt_table_t::get_node (word_t addr)
{
    return (vrt_node_t *) entries +
	((addr >> objsize) & ((1UL << radix) - 1));
}

/**
 * Retrieve table entry.
 * @param addr		address to use for indexing
 * @return pointer to table entry
 */
INLINE mdb_node_t * vrt_table_t::get_mapnode (word_t addr)
{
    mdb_node_t ** map_ptrs = (mdb_node_t **)
	((vrt_node_t *) entries + (1UL << radix));
    return map_ptrs[(addr >> objsize) & ((1UL << radix) - 1)];
}

/**
 * Retrieve sub-table from within mapping table.
 * @param addr		address to use for indexing
 * @return pointer to mapping table, or NULL if no table exists
 */
INLINE vrt_table_t * vrt_table_t::get_table (word_t addr)
{
    return get_node (addr)->get_table ();
}

/**
 * Retrieve number of entries in table.
 * @return number table entries (log 2)
 */
INLINE word_t vrt_table_t::get_radix (void)
{
    return radix;
}

/**
 * Retrieve address prefix for table.
 * @return address prefix for table
 */
INLINE word_t vrt_table_t::get_prefix (void)
{
    return prefix;
}

/**
 * Retrieve object size for table entries.
 * @return object size of table entries (log2)
 */
INLINE word_t vrt_table_t::get_objsize (void)
{
    return objsize;
}

/**
 * Retrive start address of table (undefined if table spans an address
 * range equal to word size).
 * @return address of first entry in table
 */
INLINE word_t vrt_table_t::get_start_addr (void)
{
    return prefix & (~0UL << (objsize + radix));
}

/**
 * Retrive end address of table (undefined if table spans an address
 * range equal to word size).
 * @return address next address after table ends
 */
INLINE word_t vrt_table_t::get_end_addr (void)
{
    return get_start_addr () + (1UL << (objsize + radix));
}

/**
 * Clear entry from table.
 * @param addr		address to use for indexing
 */
INLINE void vrt_table_t::clear (word_t addr)
{
    vrt_node_t * n = get_node (addr);
    n->clear ();
}

/**
 * Modify node object within mapping table.
 * @param addr		address to use for indexing
 * @param paddr		physical address
 * @param obj		source object
 * @param obj_size	size of source object
 * @param access	access rights
 */
INLINE void vrt_table_t::set_object (vrt_t * vrt, word_t addr, word_t paddr,
				     vrt_node_t * obj, word_t obj_size,
				     word_t access)
{
    if (obj == NULL)
	clear (addr);
    else
    {
	vrt_node_t * n = get_node (addr);
	n->set_object (vrt, get_objsize (), paddr, obj, obj_size, access);
    }
}

/**
 * Set pointer to mapping database node.
 * @param addr		address to use for indexing
 * @param map		pointer to mapping database node
 */
INLINE void vrt_table_t::set_mapnode (word_t addr, mdb_node_t * map)
{
    mdb_node_t ** map_ptrs = (mdb_node_t **)
	((vrt_node_t *) entries + (1UL << radix));
    map_ptrs[(addr >> objsize) & ((1UL << radix) - 1)] = map;
}

/**
 * Set sub-table pointer within table.
 * @param addr		address to use for indexing
 * @param t		new mapping table
 */
INLINE void vrt_table_t::set_table (word_t addr, vrt_table_t * t)
{
    if (t == NULL)
	clear (addr);
    else
    {
	vrt_node_t * n = get_node (addr);
	n->set_table (t);
    }
}

/**
 * Set address prefix for table.
 * @param p		new prefix
 */
INLINE void vrt_table_t::set_prefix (word_t p)
{
    prefix = p;
}

/**
 * Set size of objects in table.
 * @param s		object size (log2)
 */
INLINE void vrt_table_t::set_objsize (word_t s)
{
    objsize = s;
}



#endif /* !__VRT_H__ */
