/*********************************************************************
 *                
 * Copyright (C) 2004-2007,  Karlsruhe University
 *                
 * File path:     generic/mdb.h
 * Description:   Classes and access methods for the generic mapping
 *		  database.
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
 * $Id: mdb.h,v 1.10 2007/01/08 14:08:10 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __MDB_H__
#define __MDB_H__

class mdb_node_t;
class mdb_tableent_t;
class mdb_table_t;


/**
 * The mdb_t specifies a particular mapping database, e.g., for page
 * frames, I/O ports, etc.  Certain operations on the mapping database
 * (e.g., map and mapctrl) are generic.  Other operations like
 * flushing cached entries for mappings, e.g., TLB entries in the case
 * of memory, must be defined on a per mapping database basis in
 * derived classes.
 */
class mdb_t
{
public:
    class ctrl_t {
    public:
	union {
	    struct {
		word_t __pad1		: 6;
		word_t mapctrl_self	: 1;
		word_t unmap		: 1;
		word_t set_rights	: 1;
		word_t reset_status	: 1;
		word_t deliver_status	: 1;
		word_t set_attribute	: 1;
		word_t __pad2		: BITS_WORD - 12;
	    };
	    word_t raw;
	};

	ctrl_t (void) {}
	ctrl_t (word_t num) { raw = num; }

	static ctrl_t flush (void)
	    {
		ctrl_t ctrl (0);
		ctrl.mapctrl_self = ctrl.unmap = ctrl.deliver_status = true;
		return ctrl;
	    }
	
	char *string()
	    {
		char *s = (char *) "~~~~~~";
		
		s[0] = (set_attribute ? 'm' : '~');
		s[1] = (deliver_status ? 'd' : '~');
		s[2] = (reset_status ? 'r' : '~');
		s[3] = (set_rights ? 'p' : '~');
		s[4] = (unmap ? 'u' : '~');
		s[5] = (mapctrl_self ? 'c' : '~');
		
		return s;
	    }

    };

    // Generic class for specifying properly aligned power of 2 sized
    // ranges using a single word.  SIZE indicates size of region
    // (log2), IDX indicates location, and C-bit if set (e.g., by
    // assigning a negative number) indicates the complete range.

    class range_t {
    public:
	union {
	    struct {
		word_t size		: 6;
		word_t idx		: BITS_WORD - 7;
		word_t c		: 1;
	    };
	    word_t raw;
	};

	range_t (word_t n) { raw = n; }

	range_t (addr_t b, word_t s)
	    {
		if (s >= 64)
		    raw = ~0UL;
		else
		{
		    c = 0;
		    idx = (word_t) b >> s;
		    size = s;
		}
	    }

	static range_t full (void)
	    {
		range_t range (~0UL);
		return range;
	    }

	bool in_range (word_t addr)
	    {
		return raw == ~0UL || (addr >> size) == idx;
	    }

	word_t get_size (void)
	    { return size; }

	word_t get_low (void)
	    { return raw == ~0UL ? 0 : idx << size; }

	word_t get_high (void)
	    { return raw == ~0UL ? ~0UL : ((idx + 1) << size) - 1; }
    };

    virtual word_t get_radix (word_t objsize);
    virtual word_t get_next_objsize (word_t objsize);
    virtual const char * get_name (void);

    // MDB specific node operations

    virtual void clear (mdb_node_t * node);
    virtual word_t get_rights (mdb_node_t * node);
    virtual void set_rights (mdb_node_t * node, word_t r);
    virtual void flush_cached_entry (mdb_node_t * node, range_t range);
    virtual bool allow_attribute_update (mdb_node_t * node);
    virtual void set_attribute (mdb_node_t * node, word_t attrib);
    virtual word_t get_phys_address (mdb_node_t * node);
    virtual word_t get_purged_status (mdb_node_t * node);
    virtual void reset_purged_status (mdb_node_t * node);
    virtual void update_purged_status (mdb_node_t * node, word_t status);
    virtual word_t get_effective_status (mdb_node_t * node);
    virtual word_t reset_effective_status (mdb_node_t * node);
    virtual void update_effective_status (mdb_node_t * node, word_t status);
    virtual void dump (mdb_node_t * node);

    // Maptree operations

    mdb_node_t * map (mdb_node_t * f_node, void * obj, word_t objsize,
		      word_t addr, word_t in_rights, word_t out_rights);
    word_t mapctrl (mdb_node_t * node, range_t range, ctrl_t ctrl,
		    word_t rights, word_t attrib);
    word_t flush (mdb_node_t * node);

    void delete_node (mdb_node_t * node);

    friend class kdb_t;
};


/**
 * The mdb_node_t is the main structure for keeping track of recursive
 * mappings.  The structure consists of a doubly linked list of all
 * the mapping nodes within a mapping tree where all the nodes in the
 * tree refer to objects of the same size.
 *
 * If there exists a mapping to a sub-object, i.e., a partial mapping,
 * the mdb_node_t will contain a pointer to an mdb_table_t.  The table
 * structure will, in addition to the table of smaller object sizes,
 * also contain a pointer to the next mapping node of the same size.
 *
 * The mapping nodes within a tree are listed in a depth first order.
 * To determine the exact tree structure, a depth field indicates the
 * current depth within the mapping tree.  As such, the complete
 * subtree of a mapping node can be found by parsing through the
 * linked list until encountering a node with the same depth as the
 * current node.  This scheme enables implementation of non-recursive
 * algorithms for parsing the mapping tree.
 *
 * The parent node of a mapping can always be found by parsing the
 * linked list backwards until a node with a lower depth is
 * encountered.  This applies even if the current mapping node has a
 * parent node which refers to a mapping of a larger object size.
 */
class mdb_node_t
{
    struct {
	word_t in_rights	: 4;
	word_t out_rights	: 4;
	word_t obj_size		: 6;
	word_t depth		: BITS_WORD - 14;
	word_t prev		: BITS_WORD;
	word_t next_is_table	: 1;
	word_t next		: BITS_WORD - 1;
	word_t object_ptr	: BITS_WORD;
	word_t misc		: BITS_WORD;
    };

public:

    void * operator new (word_t size);
    void operator delete (void * n);
    mdb_node_t (void);

    // Predicates

    bool is_my_parent (mdb_node_t * n);

    // Retrieval

    mdb_node_t * get_prev (void);
    mdb_node_t * get_next (void);
    mdb_table_t * get_table (void);
    word_t get_depth (void);
    void * get_object (void);
    word_t get_objsize (void);
    word_t get_misc (void);
    word_t get_inrights (void);
    word_t get_outrights (void);

    // Modification

    void set_prev (mdb_node_t * p);
    void set_next (mdb_node_t * n);
    void set_table (mdb_table_t * t);
    void remove_table (void);
    void set_depth (word_t d);
    void set_object (void * o);
    void set_objsize (word_t s);
    void set_misc (word_t m);
    void set_inrights (word_t r);
    void set_outrights (word_t r);

    // Miscellaneous

    mdb_node_t * get_parent (void);

    // Mapping DB specific.  We don't use virtual functions in
    // mdb_node_t and derive classes because we don't want to
    // associate a vtable with every mdb_node_t object.

    void clear (mdb_t * mdb)
	{ mdb->clear (this); }
    word_t get_rights (mdb_t * mdb)
	{ return mdb->get_rights (this); }
    void set_rights (mdb_t * mdb, word_t r)
	{ mdb->set_rights (this, r); }
    void flush_cached_entry (mdb_t * mdb, mdb_t::range_t range)
	{ mdb->flush_cached_entry (this, range); }
    bool allow_attribute_update (mdb_t * mdb)
	{ return mdb->allow_attribute_update (this); }
    void set_attribute (mdb_t * mdb, word_t attrib)
	{ mdb->set_attribute (this, attrib); }
    word_t get_phys_address (mdb_t * mdb)
	{ return mdb->get_phys_address (this); }
    word_t get_purged_status (mdb_t * mdb)
	{ return mdb->get_purged_status (this); }
    void reset_purged_status (mdb_t * mdb)
	{ mdb->reset_purged_status (this); }
    void update_purged_status (mdb_t * mdb, word_t status)
	{ return mdb->update_purged_status (this, status); }
    word_t get_effective_status (mdb_t * mdb)
	{ return mdb->get_effective_status (this); }
    word_t reset_effective_status (mdb_t * mdb)
	{ return mdb->reset_effective_status (this); }
    void update_effective_status (mdb_t * mdb, word_t status)
	{ mdb->update_effective_status (this, status); }

    friend class kdb_t;
};



/**
 * mdb_tableent_t is an antry in a mapping database table.  It
 * contains a pointer to mapping node, another mdb table, or both.  If
 * the table entry contains a pointer to another table, the table
 * structure will contain a pointer to any potential mapping nodes.
 */
class mdb_tableent_t
{
    struct {
	word_t	ptr_is_table	: 1;
	word_t	ptr		: BITS_WORD - 1;
    };

public:

    // Predicates

    bool is_valid (void);
    bool is_table (void);

    // Retrieval

    mdb_table_t * get_table (void);
    mdb_node_t * get_node (void);

    // Modification

    void set_table (mdb_table_t * t);
    void set_node (mdb_node_t * n);
    void clear (void);

    friend class mdb_table_t;
    friend class kdb_t;
};



/**
 * The mdb_table_t is an array of objects of a given size.  The size
 * of the array itself is variable and specified upon creation.
 * Entries within the table can be pointers to mapping nodes,
 * sub-tables, or both.  A count field keeps track of how many entries
 * are currently populated.  When the count reaches 0 it is safe to
 * remove the table without the risk of unreferenced pointers.
 *
 * The table structure also supports short-circuiting the lookup from
 * higher up in the mapping database hierarchy (i.e., path
 * compression).  To determine whether path compression is possible
 * the user should check the table address prefix before accesing the
 * table entries.  Note that all the access methods operating on table
 * entries assume that this check has already been carried out.
 */
class mdb_table_t
{
    struct {
	word_t node		: BITS_WORD;
	word_t radix		: 6;
	word_t objsize		: 6;
	word_t count		: BITS_WORD - 12;
	word_t prefix		: BITS_WORD ;
	word_t entries		: BITS_WORD;
    };

public:

    void * operator new (size_t size, word_t radix_log2);
    void operator delete (void * t);

    word_t get_addr (word_t idx) { return (1UL << objsize) * idx; }
    word_t get_mask (void) { return ((1UL << objsize) << radix) - 1; }

    // Predicates

    bool match_prefix (word_t addr);

    // Retrieval

    mdb_tableent_t * get_entry (word_t addr);
    mdb_node_t * get_node (void);
    mdb_node_t * get_node (word_t addr);
    mdb_table_t * get_table (word_t addr);
    word_t get_radix (void);
    word_t get_count (void);
    word_t get_prefix (void);
    word_t get_objsize (void);

    // Modification

    void remove_node (word_t addr);
    void remove_table (word_t addr);
    void set_node (mdb_node_t * n);
    void set_node (word_t addr, mdb_node_t * n);
    void set_table (word_t addr, mdb_table_t * t);
    void set_prefix (word_t p);
    void set_objsize (word_t s_log2);

    friend class kdb_t;
};






/*
**
**	mdb_t
**
*/


/**
 * Flush mapping tree.
 * @param n		root of tree to flush
 * @return status bits of flushed tree
 */
INLINE word_t mdb_t::flush (mdb_node_t * node)
{
    return mapctrl (node, range_t::full (), ctrl_t::flush (), 0, 0);
}





/*
**
**	mdb_node_t
**
*/


/**
 * Constructor for new mapping node.  Make sure that the initial next
 * pointer in the object is not treated as a pointer to a table.
 */
INLINE mdb_node_t::mdb_node_t (void)
{
    next = next_is_table = 0;
}

/**
 * Check if node is my parent.
 * @param n		node to check against
 * @return true if node is my parent, false otherwise
 */
INLINE bool mdb_node_t::is_my_parent (mdb_node_t * n)
{
    return get_depth () == n->get_depth () + 1;
}

/**
 * Retrieve pointer to previous mapping node.
 * @return pointer to previous mappings node
 */
INLINE mdb_node_t * mdb_node_t::get_prev (void)
{
    return (mdb_node_t *) prev;
}

/**
 * Retrieve pointer to mapping table.
 * @return pointer to mapping table, or NULL if no table exists
 */
INLINE mdb_table_t * mdb_node_t::get_table (void)
{
    if (! next_is_table)
	return NULL;

    return (mdb_table_t *) (next << 1);
}

/**
 * Retrieve pointer to next mapping node.
 * @return pointer to next mappings node, or NULL if no next node exists
 */
INLINE mdb_node_t * mdb_node_t::get_next (void)
{
    if (next_is_table)
	return get_table ()->get_node ();

    return (mdb_node_t *) (next << 1);
}

/**
 * Retrieve current mapping depth.
 * @return current mapping depth
 */
INLINE word_t mdb_node_t::get_depth (void)
{
    return depth;
}

/**
 * Retrieve current object pointer.
 * @return current object pointer
 */
INLINE void * mdb_node_t::get_object (void)
{
    return (void *) object_ptr;
}

/**
 * Retrieve size of current object,
 * @return size of current object (log 2)
 */
INLINE word_t mdb_node_t::get_objsize (void)
{
    return obj_size;
}

/**
 * Retrieve auxiliary value of mapnode.
 * @return auxiliary value
 */
INLINE word_t mdb_node_t::get_misc (void)
{
    return misc;
}

/**
 * Retrieve inbound access rights.
 * @return inbound access rights.
 */
INLINE word_t mdb_node_t::get_inrights (void)
{
    return in_rights;
}

/**
 * Retrieve outbound access rights.
 * @return outbound access rights.
 */
INLINE word_t mdb_node_t::get_outrights (void)
{
    return out_rights;
}

/**
 * Set pointer to previous mapping node.
 * @param p	new pointer
 */
INLINE void mdb_node_t::set_prev (mdb_node_t * p)
{
    prev = (word_t) p;
}

/**
 * Set pointer to next mapping node.
 * @param n	new pointer
 */
INLINE void mdb_node_t::set_next (mdb_node_t * n)
{
    if (next_is_table)
	get_table ()->set_node (n);
    else
	next = ((word_t) n) >> 1;
}

/**
 * Set pointer to mapping table.  Old table, if present, is not
 * implicitly destroyed first.
 * @param t	new pointer
 */
INLINE void mdb_node_t::set_table (mdb_table_t * t)
{
    t->set_node (get_next ());
    if (next_is_table)
	get_table ()->set_node (NULL);
    next = ((word_t) t) >> 1;
    next_is_table = 1;
}

/**
 * Remove pointer to table.
 */
INLINE void mdb_node_t::remove_table (void)
{
    next = ((word_t) get_table ()->get_node ()) >> 1;
    next_is_table = 0;
}

/**
 * Set current mapping depth.
 * @param d	new depth.
 */
INLINE void mdb_node_t::set_depth (word_t d)
{
    depth = d;
}

/**
 * Set current object pointer.
 * @param o	new object pointer.
 */
INLINE void mdb_node_t::set_object (void * o)
{
    object_ptr = (word_t) o;
}

/**
 * Set current object size.
 * @param s	new object size (log 2).
 */
INLINE void mdb_node_t::set_objsize (word_t s)
{
    obj_size = s;
}

/**
 * Set auxiliary value for mappings node.
 * @param m	new auxiliary value
 */
INLINE void mdb_node_t::set_misc (word_t m)
{
    misc = m;
}

/**
 * Set inbound access rights.
 * @param inbound access rights.
 */
INLINE void mdb_node_t::set_inrights (word_t r)
{
    in_rights = r;
}

/**
 * Set outbound access rights.
 * @param outbound access rights.
 */
INLINE void mdb_node_t::set_outrights (word_t r)
{
    out_rights = r;
}





/*
**
**	mdb_tableent_t
**
*/


/**
 * Check if table entry is valid.
 * @return true if entry is valid, false otherwise
 */
INLINE bool mdb_tableent_t::is_valid (void)
{
    return ptr != 0;
}

/**
 * Check if table entry points to table.
 * @return true if entry contains table pointer, false otherwise
 */
INLINE bool mdb_tableent_t::is_table (void)
{
    return ptr_is_table != 0;
}

/**
 * Retrieve pointer to mapping table.
 * @return pointer to mapping table, or NULL if no table exists
 */
INLINE mdb_table_t * mdb_tableent_t::get_table (void)
{
    if (! ptr_is_table)
	return NULL;

    return (mdb_table_t *) (ptr << 1);
}

/**
 * Retrieve pointer to mapping node.
 * @return pointer to next mappings node, or NULL if no next node exists
 */
INLINE mdb_node_t * mdb_tableent_t::get_node (void)
{
    if (ptr_is_table)
	return get_table ()->get_node ();

    return (mdb_node_t *) (ptr << 1);
}

/**
 * Set pointer to mapping table.  Old table, if present, is not
 * implicitly destroyed first.
 * @param t	new pointer
 */
INLINE void mdb_tableent_t::set_table (mdb_table_t * t)
{
    t->set_node (get_node ());
    if (ptr_is_table)
	get_table ()->set_node (NULL);
    ptr = ((word_t) t) >> 1;
    ptr_is_table = 1;
}

/**
 * Set pointer to mapping node.
 * @param n	new pointer
 */
INLINE void mdb_tableent_t::set_node (mdb_node_t * n)
{
    if (ptr_is_table)
	get_table ()->set_node (n);
    else
	ptr = ((word_t) n) >> 1;
}

/**
 * Clear table entry.
 */
INLINE void mdb_tableent_t::clear (void)
{
    ptr = ptr_is_table = 0;
}




/*
**
**	mdb_table_t
**
*/


/**
 * Check if the table prefix matches the supplied address.
 * @param addr		address to match
 * @return true if prefix matches, false otherwise
 */
INLINE bool mdb_table_t::match_prefix (word_t addr)
{
    return ((addr ^ prefix) & ~get_mask ()) == 0;
}

/**
 * Retrieve auxiliary mapping node pointer.
 * @return pointer to mapping node, or NULL if no node exists
 */
INLINE mdb_node_t * mdb_table_t::get_node (void)
{
    return (mdb_node_t *) node;
}

/**
 * Retrieve mapping table entry.
 * @param addr		address to use for indexing
 * @return pointer to mapping table entry
 */
INLINE mdb_tableent_t * mdb_table_t::get_entry (word_t addr)
{
    return (mdb_tableent_t *) entries +
	((addr >> objsize) & ((1UL << radix) - 1));
}

/**
 * Retrieve mapping node from within mapping table.
 * @param addr		address to use for indexing
 * @return pointer to mapping node, or NULL if no node exists
 */
INLINE mdb_node_t * mdb_table_t::get_node (word_t addr)
{
    return get_entry (addr)->get_node ();
}

/**
 * Retrieve sub-table from within mapping table.
 * @param addr		address to use for indexing
 * @return pointer to mapping table, or NULL if no table exists
 */
INLINE mdb_table_t * mdb_table_t::get_table (word_t addr)
{
    return get_entry (addr)->get_table ();
}

/**
 * Retrieve population count for table.
 * @return number of valid table entries
 */
INLINE word_t mdb_table_t::get_count (void)
{
    return count;
}

/**
 * Retrieve number of entries in table.
 * @return number table entries
 */
INLINE word_t mdb_table_t::get_radix (void)
{
    return radix;
}

/**
 * Retrieve address prefix for mapping table.
 * @return address prefix for mapping table
 */
INLINE word_t mdb_table_t::get_prefix (void)
{
    return prefix;
}

/**
 * Retrieve object size for table entries.
 * @return object size of table entries (log2)
 */
INLINE word_t mdb_table_t::get_objsize (void)
{
    return objsize;
}

/**
 * Remove node pointer within mapping table
 * @param addr		address to use for indexing
 */
INLINE void mdb_table_t::remove_node (word_t addr)
{
    mdb_tableent_t * e = get_entry (addr);
    if (e->is_valid () && ! e->is_table ())
	count--;
    e->set_node (NULL);
}

/**
 * Remove sub-table from mapping table.
 * @param addr		address to use for indexing
 */
INLINE void mdb_table_t::remove_table (word_t addr)
{
    mdb_tableent_t * e = get_entry (addr);
    if (! e->is_table ())
	return;

    mdb_node_t * n = e->get_node ();
    if (n == NULL)
	count--;
    e->ptr_is_table = 0;
    e->ptr = ((word_t) n) >> 1;
}

/**
 * Set auxiliary mapping node pointer.
 * @param n		new mapping node pointer
 */
INLINE void mdb_table_t::set_node (mdb_node_t * n)
{
    node = (word_t) n;
}

/**
 * Set mapping node pointer within mapping table.
 * @param addr		address to use for indexing
 * @param n		new mapping node pointer
 */
INLINE void mdb_table_t::set_node (word_t addr, mdb_node_t * n)
{
    if (n == NULL)
	remove_node (addr);
    else
    {
	mdb_tableent_t * e = get_entry (addr);
	if (! e->is_valid ())
	    count++;
	e->set_node (n);
    }
}

/**
 * Set sub-table pointer within mapping table.
 * @param addr		address to use for indexing
 * @param t		new mapping table
 */
INLINE void mdb_table_t::set_table (word_t addr, mdb_table_t * t)
{
    if (t == NULL)
	remove_table (addr);
    else
    {
	mdb_tableent_t * e = get_entry (addr);
	if (! e->is_valid ())
	    count++;
	e->set_table (t);
    }
}

/**
 * Set address prefix for mapping table.
 * @param p		new prefix
 */
INLINE void mdb_table_t::set_prefix (word_t p)
{
    prefix = p;
}

/**
 * Set size of objects in mapping table.
 * @param s		object size (log2)
 */
INLINE void mdb_table_t::set_objsize (word_t s)
{
    objsize = s;
}


/* From generic/mapping_alloc.cc */
addr_t mdb_alloc_buffer (word_t size);
void mdb_free_buffer (addr_t addr, word_t size);



/**
 * Data structure for holding MDB init functions.
 */
typedef struct {
    word_t priority;
    void (*function)(void);
} mdb_init_func_t;


/**
 * Declare an MDB init function.  The MDB init functions are called in
 * the order according to their priorities.
 *
 * @param prio		priority
 * @param func		function pointer
 */
#define MDB_INIT_FUNCTION(prio, func)					\
    void __attribute__ ((__section__ (".init")))			\
	__mdb_init__##prio##_##func (void);				\
    mdb_init_func_t __mdb_init__##prio##_##func##_ent			\
	__attribute__ ((__section__ (".mdb_funcs."#prio), __unused__)) 	\
	= { prio, __mdb_init__##prio##_##func };			\
    void __attribute__ ((__section__ (".init")))			\
	__mdb_init__##prio##_##func (void)


#endif /* !__MDB_H__ */
