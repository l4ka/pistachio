/*********************************************************************
 *                
 * Copyright (C) 2004-2007,  Karlsruhe University
 *                
 * File path:     generic/mdb.cc
 * Description:   Generic mapping database
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
 * $Id: mdb.cc,v 1.16 2007/01/08 14:08:10 skoglund Exp $
 *                
 ********************************************************************/
#include <mdb.h>
#include <sync.h>
#include INC_GLUE(mdb.h)

#include <kdb/tracepoints.h>
#include <debug.h>

#if !defined(MAX_MDB_RECURSION)
#define MAX_MDB_RECURSION 10
#endif


DECLARE_TRACEPOINT (MDB_MAP);
DECLARE_TRACEPOINT (MDB_MAPCTRL);

DEFINE_SPINLOCK (mdb_lock);


/**
 * Initialize mapping databases.  Invoke the init function for each
 * mapping database type.
 */
void SECTION (".init") init_mdb (void)
{
    extern mdb_init_func_t _start_mdb_funcs[];
    extern mdb_init_func_t _end_mdb_funcs[];

    for (mdb_init_func_t * fdesc = _start_mdb_funcs;
	 fdesc < _end_mdb_funcs;
	 fdesc++)
    {
	(*fdesc->function) ();
    }
}


/**
 * Allocate a new mapping node.
 *
 * @param size		size of new node
 *
 * @return new mapping node
 */
void * mdb_node_t::operator new (size_t size)
{
    return (void *) mdb_alloc_buffer (size);
}


/**
 * Remove mapping node.  Any tables that falls empty as a result of
 * the delete operation are not deleted.  They have to be deleted
 * explicitly.  Further, the operation assumes that all mapping nodes
 * below the current one have already been deleted.  We can't use the
 * delete operator instead of this function since we need to know
 * which mapping database the node resides in (i.e., we need to know
 * how to retrieve the physical address the node refers to).
 *
 * @param node		node to remove
 */
void mdb_t::delete_node (mdb_node_t * node)
{
    if (node == NULL)
	return;

    mdb_node_t * p = node->get_prev ();

    ASSERT (node->get_next () == NULL ||
	    node->get_next ()->get_depth () <= node->get_depth ());

    if (p->get_next () == node)
    {
	// We can simply remove entry directly from linked list.

	p->set_next (node->get_next ());
    }
    else
    {
	word_t addr = node->get_phys_address (this);
	mdb_table_t * table = p->get_table ();
	ASSERT (table);

	// Find table containing pointer to mapping node and remove
	// corresponding table entry.

	while (table->get_objsize () > node->get_objsize ())
	{
	    table = table->get_table (addr);
	    ASSERT (table);
	}

	ASSERT (table->get_node (addr) == node);
	table->set_node (addr, node->get_next ());
    }

    if (node->get_next ())
	node->get_next ()->set_prev (p);

    node->clear (this);
    mdb_free_buffer (node, sizeof (mdb_node_t));
}


/**
 * Allocate a new mapping table.  Allocates both the table control
 * structure and the table entries.  Further, the table control
 * structure is partly initialized.
 *
 * @param size		size of control structure
 * @param radix_log2	size of table
 *
 * @return new mapping table
 */
void * mdb_table_t::operator new (size_t size, word_t radix_log2)
{
    mdb_table_t * t = (mdb_table_t *) mdb_alloc_buffer (sizeof (mdb_table_t));

    t->node = NULL;
    t->radix = radix_log2;
    t->count = 0;
    t->entries = (word_t) mdb_alloc_buffer (sizeof (mdb_tableent_t) * 
					    (1UL << radix_log2));

    mdb_tableent_t * e = t->get_entry (0);
    for (word_t k = 0; k < (1UL << radix_log2); k++, e++)
	     e->clear ();

    return (void *) t;
}


/**
 * Remove table.  The operation assumes that the table is empty.
 *
 * @param t		table to remove
 */
void mdb_table_t::operator delete (void * t)
{
    if (t == NULL)
	return;

    mdb_table_t * table = (mdb_table_t *) t;
    ASSERT (table->get_count () == 0);

    mdb_free_buffer ((void *) table->entries,
		     sizeof (mdb_tableent_t) * (1UL << table->get_radix ()));
    mdb_free_buffer (table, sizeof (mdb_table_t));
}


/**
 * Create new mapping node entry in the mapping database.
 *
 * @param f_node	mapping node to map from
 * @param obj		object to map
 * @param objsize	size of object (log2)
 * @param addr		physical address of object
 * @param out_rights	outbound access rights of mapping
 * @param in_rights	inbound access rights of mapping
 *
 * @return pointer to new mapping node
 *
 * Create new mapping node entry for specified object and hook it into
 * the mapping databse with F_NODE as the parent.  New mapping tables
 * will be created when necessary.
 */
mdb_node_t * mdb_t::map (mdb_node_t * f_node,
			 void * obj, word_t objsize, word_t addr,
			 word_t out_rights, word_t in_rights)
{
    TRACEPOINT (MDB_MAP, "%s::map (%p, %p, %d, %p, %x, %x)",
		get_name (),  f_node,  obj, objsize,   addr,  (out_rights & 7UL), 
		(in_rights & 7UL));

    mdb_lock.lock();

    // Allocate and initialize new mapping node.

    mdb_node_t * newnode = new mdb_node_t;
    ASSERT (newnode);
    newnode->set_depth (f_node->get_depth () + 1);
    newnode->set_objsize (objsize);
    newnode->set_object (obj);
    newnode->set_prev (f_node);
    newnode->set_inrights (in_rights);
    newnode->set_outrights (out_rights);
    newnode->set_rights (this, f_node->get_rights (this) &
			 out_rights & in_rights);
    newnode->reset_purged_status (this);

    if (objsize == f_node->get_objsize ())
    {
	// New node is of the same object size as the current node.
	// We can simply insert the node directly below current node.

	newnode->set_next (f_node->get_next ());
	if (f_node->get_next ())
	    f_node->get_next ()->set_prev (newnode);
	f_node->set_next (newnode);
	mdb_lock.unlock();
	return newnode;
    }

    // We can not simply put new mapping node below current node
    // because the new node has a smaller object size.  We either need
    // to create a sub-table for holding the mapping, or use an
    // existing table.

    mdb_table_t * table = f_node->get_table ();
    mdb_table_t * prev_table = NULL;

    for (;;)
    {
	if (table != NULL && table->match_prefix (addr))
	{
	    if (table->get_objsize () == objsize)
	    {
		// New mapping belongs within the current mapping
		// table.  We can simply insert the new mapping node
		// directly into the subtree below the appropriate
		// mapping table entry.

		mdb_node_t * n = table->get_node (addr);
		newnode->set_next (n);
		if (n)
		    n->set_prev (newnode);
		table->set_node (addr, newnode);
		mdb_lock.unlock();
		return newnode;
	    }
	    else if (table->get_objsize () > objsize)
	    {
		// Encountered a mapping table which should act as a
		// parent table for a table containing the new object.

		prev_table = table;
		table = table->get_table (addr);
		if (table)
		    // Recurse into sub-table.
		    continue;
	    }
	    else if (table->get_objsize () < objsize)
	    {
		// The new table should contain the current mapping
		// table within one of the table entries.  This will
		// be taken care of below (table != NULL).
	    }

	    // Create a new mapping table just below mapping node or
	    // mapping table.  We perform path compression to avoid
	    // unnecessary lookup and memory overhead.  The radix for
	    // the new table is based on the object size of the
	    // entries within the table.

	    mdb_table_t * newtable = new (get_radix (objsize)) mdb_table_t;

	    newtable->set_prefix (addr);
	    newtable->set_objsize (objsize);
	    newtable->set_node (addr, newnode);
	    newnode->set_next (NULL);

	    if (prev_table)
		prev_table->set_table (addr, newtable);
	    else
		f_node->set_table (newtable);

	    if (table != NULL)
	    {
		// There already existed a table which did path
		// compression and containing objects of smaller sizes
		// than the current one.  Make sure that this table is
		// inserted into the newly created table.  Do this
		// insertion after inserting newtable above so as to
		// avoid overwriting the auxiliary mapnode pointer in
		// table.

		newtable->set_table (table->get_prefix (), table);
	    }

	    mdb_lock.unlock();
	    return newnode;
	}

	// The address prefix for the looked up table (if present)
	// does not match that of the current object.  This indicates
	// that there is no overlap whatsoever, or that we must create
	// a new table which contains the old table in one of its
	// entries.  If there was no table below mapping node we
	// simply create one.

	mdb_table_t * newtable = new (get_radix (objsize)) mdb_table_t;
	mdb_node_t * auxnode = f_node->get_next ();
	
	newtable->set_prefix (addr);
	newtable->set_objsize (objsize);
	newtable->set_node (addr, newnode);
	newnode->set_next (NULL);

	if (table != NULL)
	{
	    auxnode = table->get_node ();
	    table->set_node (NULL);

	    // Remove old table now to avoid propagating auxiliary
	    // mapping node upwards during the set_table operations
	    // below.

	    if (prev_table)
		prev_table->remove_table (table->get_prefix ());
	    else
		f_node->remove_table ();

	    if (newtable->match_prefix (table->get_prefix ()))
	    {
		// There is an overlap between an existing table and the
		// newly created table.  The overlap occured because the
		// old table did path compression, and should be treated
		// as a sub-table for the newly created table.

		ASSERT (table->get_objsize () < objsize);
		newtable->set_table (table->get_prefix (), table);
	    }
	    else
	    {
		// We have two sub-tables that do not overlap, but
		// that resides under the same parent table entry or
		// mapping node.  We need to create an intermediate
		// table that can help us hold both tables.

		// The intermediate table will hold objects of sizes
		// which is the maximum valid object size that does
		// not generate a mapping table where the two
		// subtables map to the same table entry.

		word_t imask;
		word_t isize = prev_table ?
		    prev_table->get_objsize () : f_node->get_objsize ();

		do {
		    isize = get_next_objsize (isize);
		    imask = get_radix (isize) + isize;
		    imask = ((imask == sizeof (word_t) * 8) ? ~0UL :
			     ((1UL << imask) - 1)) & ~((1UL << isize) - 1);
		} while ((addr & imask) == (table->get_prefix () & imask));

		ASSERT (isize > objsize);
		ASSERT (isize > table->get_objsize ());

		mdb_table_t * itable = new (get_radix (isize)) mdb_table_t;

		itable->set_prefix (addr);
		itable->set_objsize (isize);

		itable->set_table (addr, newtable);
		itable->set_table (table->get_prefix (), table);

		// It's ok to just operate on the intermediate table
		// from here on.

		newtable = itable;
	    }
	}
	
	if (prev_table)
	    prev_table->set_table (addr, newtable);
	else
	    f_node->set_table (newtable);

	newtable->set_node (auxnode);
	mdb_lock.unlock();
	return newnode;
    }
}


/**
 * Modify or delete mapping tree.
 *
 * @param node		mapping node to start operation on
 * @param range		range in which mapctrl is performed
 * @param ctrl		type of operation
 * @param rights	new rights for mapping tree
 * @param attrib	new attribute for mapping tree
 *
 * @return old status rights
 *
 * Modify or delete mapping tree rooted at NODE.  Deletion occurs if
 * the unmap field in CTRL is set.  If the set_rights field is set,
 * the in-rights or out-rights of the mapping node is updated, and the
 * new rights (specified in RIGHTS) are propagater throughout the
 * tree.  The reset_status and deliver_status resets and/or delivers
 * the status bits in the mapping tree.  The set_attribute field
 * indicates that the attributes for the mapping tree should be
 * updated according to ATTRIB.  The RANGE parameter can be used to
 * limit the operation to only parts of the affected area (only
 * applicable if mapctrl_self is not specified).
 */
word_t mdb_t::mapctrl (mdb_node_t * node, range_t range,
		       mdb_t::ctrl_t ctrl, word_t rights, word_t attrib)
{
    TRACEPOINT (MDB_MAPCTRL,
		"%s::mapctrl (%p, <%x,%x>, %x [%s], %p) phys=%p\n",
		(word_t) get_name (), (word_t) node, 
		range.get_low (), range.get_high (),
		ctrl.raw, (word_t) ctrl.string(),
		rights & 0x7, node->get_phys_address (this));

    mdb_lock.lock();

    // If we are unmapping the whole node there is no need to perform
    // any updates on it first.  We do need to read out the status
    // bits, though, or else the contents of these bits will be lost
    // to nodes higher up in the mapping tree.

    if (ctrl.unmap)
    {
	ctrl.set_rights = false;
	ctrl.set_attribute = false;
	ctrl.reset_status = false;
	ctrl.deliver_status = true;
    }

    // If we reset the status bits we also need to read out the
    // current bits so that nodes higher up in the tree can read them
    // out later.

    else if (ctrl.reset_status)
	ctrl.deliver_status = true;

    if (ctrl.set_attribute && ! node->allow_attribute_update (this))
	ctrl.set_attribute = false;

    // When updating permissions we seperately keep track of whether
    // we revoke rights and/or extend rights.  This is because of
    // implications on correctness and algorithm efficiency.
    //
    // If we revoke rights we must also make sure that any cached
    // effective access rights (e.g., in the TLB) are flushed
    // immediately.  Such flushing is not required for rights
    // extension.  When extending the access rights updates to caches
    // can happen lazily.
    //
    // If we are only revoking access rights it is sufficient to apply
    // a fixed bitmask to the effective access rights within a whole
    // subtree.  When extending the access rights, however, such a
    // scheme is not possible.  Instead, the effective access rights
    // of the parent must be looked up to determine the effective
    // access rigths of the current node.  Since we avoid recursive
    // algorithms, the lookup requires searching backwards in the
    // linked list of mapping nodes until parent is found.  This can
    // be a relatively expensive operation, and is as such avoided if
    // possible.

    bool revoke_rights = false;
    bool extend_rights = false;
    word_t revoke_mask = 0;

    if (ctrl.set_rights && ctrl.mapctrl_self)
    {
	if (ctrl.mapctrl_self)
	{
	    // Changing the inbound rights for the mapping.

	    word_t old_rights = node->get_inrights ();
	    if (old_rights == rights)
	    {
		// Permissions not changed.  No need to perform any
		// updates throughout the subtree.
		ctrl.set_rights = false;
	    }
	    else
	    {
		node->set_inrights (rights);
		revoke_rights = (old_rights & rights) != old_rights;
		extend_rights = (old_rights | rights) != old_rights;
		revoke_mask = ~((old_rights & rights) ^ old_rights);
	    }
	}
	else
	{
	    // Changing outbound rights for the mapping.  We need to
	    // change the rights in every mapping node which is a
	    // child of the current one (i.e., with a depth count one
	    // more than the start depth).
	}
    }

    mdb_node_t * startnode = node;
    mdb_node_t * parent = node;
    mdb_node_t * prev;
    word_t status_bits = 0;

    // The algorithm used to parse the mapping tree is not recursive.
    // However, due to the need to parse sub tables we need to have
    // some sort of recursion.  Instead of doing a full recursive
    // function call we keep arrays of the variables we need to stack.
    // The maximum depth of this recursion is typically low (< 16) and
    // can be determined upon compile time.

    mdb_table_t * r_table[MAX_MDB_RECURSION];
    struct {
	word_t idx : BITS_WORD - 1;
	word_t mod : 1;
    } r_values[MAX_MDB_RECURSION];
    word_t recurse_level = 0;

    mdb_table_t * table = NULL;
    word_t tableidx = 0;
    bool do_modify = false;

    if (! ctrl.mapctrl_self)
	goto Skip_node_modifications;

    if (ctrl.deliver_status)
	status_bits |= node->get_purged_status (this);

    // We never do partial mapctrl if operating on self.

    range = range_t::full ();

    // Tha map control algorithm works as follows.
    //
    // A main loop parses through every mapping node in the subtree.
    // The termination criteria is when we either reach the end of the
    // linked list representing the subtree, or that the current node
    // in the linked list has a depth counter that indicates that it
    // is a sibling or is higher up in the mapping hiearchy than the
    // start node.  After the main loop terminates, the alorithm
    // potentially backs up towards the start node again to remove
    // mappings or reset reference bits.
    //
    // The main loop can be divided into three major parts:
    //
    //   1. Process the current mapping node (setting attributes,
    //      setting permissions, or delivering status information).
    //   2. Go to next mapping node.
    //   3. Select new mapping node to process in case we've reached
    //      the end of a subtree.  Step 3 can again be divided into 4
    //      steps.
    //        a) Perform unmap: Parse linked list backwards to remove
    //           mapping nodes.  Remove any mapping tables that fall
    //           empty.
    //        b) Perform status reset: Parse linked list backwards to
    //           update status bits.
    //        c) Recurse down into subtable, skip to next table entry
    //           in current table, or parse up from previous subtable.
    //	      d) Handle the case where we still have not found a new
    //	         mapping node to process.
    //
    // The different steps are marked as "--- PART X ---" in the code
    // below.

    do {
	// --- PART 1 ---

	do_modify = true;

	if (ctrl.deliver_status)
	    status_bits |= node->get_effective_status (this);

	if (ctrl.set_attribute)
	{
	    node->set_attribute (this, attrib);
	    node->flush_cached_entry (this, range);
	}

	if (ctrl.set_rights)
	{
	    if ((! ctrl.mapctrl_self) && node->is_my_parent (startnode))
	    {
		// We need to update the outbound rights for the
		// current mapping.  Also determine whether we need to
		// revoke or extend the access rights within the
		// subtree.

		word_t old_rights = node->get_outrights ();
		node->set_outrights (rights);
		revoke_rights = (old_rights & rights) != old_rights;
		extend_rights = (old_rights | rights) != old_rights;
		revoke_mask = ~((old_rights & rights) ^ old_rights);
	    }

	    // Update the effective access rights for the mapping.
	    // Try to perform as little works as possible while doing
	    // so.

	    if (revoke_rights && (! extend_rights))
	    {
		node->set_rights (this, node->get_rights (this) &
				  revoke_mask);
	    }
	    else if (extend_rights)
	    {
		if (! node->is_my_parent (parent))
		    parent = node->get_parent ();

		node->set_rights (this, parent->get_rights (this) &
				  node->get_outrights () &
				  node->get_inrights ());
	    }

	    // If rights were revoked we mush make sure that any
	    // cached entries (e.g., in TLB) are flushed immediately.

	    if (revoke_rights)
		node->flush_cached_entry (this, range);
	}

    Skip_node_modifications:

	// --- PART 2 ---

	// Finished operating on the mapping node.  Find next node to
	// operate on.

	prev = node;

	if (node->get_table ())
	{
	    // We need to recurse into a subtable.  Record the current
	    // table and table entry we are working on.  Initially,
	    // these will be NULL-entries.  Continue recursion if
	    // selected entry in new table is another subtable.

	    mdb_table_t * nexttab = node->get_table ();

	    do {
		ASSERT (recurse_level < MAX_MDB_RECURSION);
		r_table[recurse_level] = table;
		r_values[recurse_level].idx = tableidx;
		r_values[recurse_level++].mod = do_modify;
		table = nexttab;

		if (! do_modify &&
		    nexttab->match_prefix (range.get_low ()) &&
		    (nexttab->get_radix () + nexttab->get_objsize ()) >
		    range.get_size ())
		{
		    // We should only operate on a subset of table
		    // entries.

		    word_t a = range.get_low ();
		    node = table->get_node (a);
		    nexttab = table->get_table (a);
		    tableidx = ((word_t) table->get_entry (a) -
				(word_t) table->get_entry (0))
			/ sizeof (mdb_tableent_t);
		}
		else
		{
		    // We should operate on all table entries.

		    node = table->get_node (0);
		    tableidx = 0;
		    nexttab = table->get_table (0);
		}
	    } while (nexttab);
	}
	else
	{
	    node = node->get_next ();
	}

	// --- PART 3 ---

	if (node == NULL)
	{
	    // We have reached the end of a subtree, or alternatively,
	    // chosen an emptry subtree from a mapping table.

	    // If we have not recursed into any subtables it indicates
	    // that we have only operated on a simple tree of a single
	    // object size.  Just exit the main mapctrl loop and
	    // perform potential cleanup at the end of the mapctrl
	    // function.

	    if (recurse_level == 0)
		break;

	    // --- PART 3a ----

	    while (ctrl.unmap)
	    {
		// We must keep a well defined mapping tree at all
		// times.  That is, we don't want to get to the point
		// where a mapping tree A->B->C turns into A->C (e.g.,
		// due to a preempted unmap operation).  Such a
		// transformation is conceptually wrong (A never
		// mapped to C) and will cause problems if A attempts
		// to perform a tagged mapctrl on the mapping which
		// used to go to B.  Further, we can get into problems
		// since the depth counts are no longer contigous,
		// causing subtrees to suddenly hook themselves into
		// other subtrees, etc.
		//
		// To prevent not well-defined mapping trees, we
		// always delete mapping trees from bottom up.  That
		// is, when we encounter the end of a linked list we
		// start deleting nodes from the end of the list.

		// Since we terminate the mapctrl loop above if the
		// recursion depth is zero, we know that the backwards
		// delete operation will terminate in a table entry.
		// The termination criteria in the while loop below
		// detects this.

		ASSERT (recurse_level > 0);

		if (prev->get_table () != NULL)
		    break;

		do {
		    node = prev;
		    prev = prev->get_prev ();
		    delete_node (node);
		} while (prev->get_table () == NULL);

		table->remove_node (table->get_addr (tableidx));
		node = NULL;

		// Table may fall empty due to delete operation.  If
		// so, delete it and recurse up to the parent table.
		// Continue deleting tables if they also happen to
		// fall empty.

		while (table != NULL && table->get_count () == 0)
		{
		    mdb_table_t * t = table;

		    ASSERT (recurse_level > 0);
		    recurse_level--;
		    table = r_table[recurse_level];
		    tableidx = r_values[recurse_level].idx;
		    do_modify = r_values[recurse_level].mod;

		    if (prev->get_table () == t)
			prev->remove_table ();
		    else
		    {
			ASSERT (table);
			ASSERT (table->get_table
				(table->get_addr (tableidx)) == t);
			table->remove_table (table->get_addr (tableidx));
		    }

		    // The deleted table may have had an auxiliary
		    // mapping node that ended up in the parent table
		    // or parent node.  If so, continue working on
		    // that node.

		    node = t->get_node ();
		    delete t;
		}

		if (table == NULL) ASSERT (recurse_level == 0);
		else ASSERT (recurse_level > 0);

		if (prev->get_table ())
		{
		    // If the previous node still contains a table it
		    // must mean that we still have a number of
		    // subtrees within the table that we
		    // (potientially) need to delete.  Make sure that
		    // we don't continue with the regular mapping node
		    // before processing the table.

		    ASSERT (table != NULL);
		    ASSERT (table->get_count () > 0);
		    ASSERT (prev->get_table ()->get_count () > 0);
		    node = NULL;
		    break;
		}
		else
		{
		    // The node no longer contains a table.  This
		    // indicates that all the subtrees beneath the
		    // mapping table have been deleted.

		    if (prev->get_next () != NULL)
		    {
			// Continue performing mapcontrol on next node.
			node = prev->get_next ();
			break;
		    }
		    else if (recurse_level == 0)
		    {
			// Exit from main loop.
			ASSERT (table == NULL);
			break;
		    }
		    else
			// Deleting current subtree.
			continue;
		}
	    }

	    // --- PART 3b ---

	    if (ctrl.reset_status && prev->get_table () == NULL)
	    {
		// Reset the reference status bits and propagate the
		// old bit contents upwards to the parent of the table
		// we are currently processing.  We don't have to deal
		// with startnode and mapctr_self here since we know
		// that we are operating within a subtable.

		word_t status = 0;
		mdb_node_t * p = prev->get_prev ();
		node = prev;

		do {
		    status |= node->reset_effective_status (this);
		    node->flush_cached_entry (this, range);
		    node->update_purged_status (this, status);

		    if (! node->is_my_parent (p))
		    {
			node->get_parent ()->
			    update_effective_status (this, status);
			status = 0;
		    }

		    node = p;
		    p = p->get_prev ();
		    
		} while (p->get_next () == node);

		p->update_effective_status (this, status);
		node = NULL;
	    }

	    // --- PART 3c ---

	    // We have now possibly finished processing a whole
	    // subtree, and the (table, tableidx) tuple indicates
	    // which mapping table entry we are currently working on
	    // (if any).  It might be that we have just recursed up
	    // from a subtable.  If so, and we have not found a proper
	    // node to work on yet, we try to search through the
	    // mapping table entries for valid nodes or recurse into
	    // subtables if they exist.

	    while (node == NULL && table != NULL)
	    {
		if (tableidx < (1UL << table->get_radix ()) - 1)
		{
		    // Try next table entry.  Recurse into subtable(s) if
		    // necessary.

 		    ASSERT (recurse_level > 0);
		    tableidx++;

		    if (! r_values[recurse_level - 1].mod &&
			! range.in_range ((table->get_prefix () &
					   ~table->get_mask ()) +
					  table->get_addr (tableidx)))
		    {
			// We have reached the end of a mapctrl range
			// in a table that should only be partially
			// processed.  Force algorithm to recurse up
			// to previous level.

			tableidx = (1UL << table->get_radix ()) - 1;
			continue;
		    }

		    while (table->get_table (table->get_addr (tableidx)))
		    {
			ASSERT (recurse_level < MAX_MDB_RECURSION);
			r_table[recurse_level] = table;
			r_values[recurse_level].idx = tableidx;
			r_values[recurse_level++].mod = do_modify;
			table = table->get_table (table->get_addr (tableidx));
			tableidx = 0;
		    }
		    node = table->get_node (table->get_addr (tableidx));
		}
		else
		{
		    // Reached end of mapping table.  Recurse up to
		    // previous mapping table.  Before continuing to
		    // parse parent table, we first try to get the
		    // next mapping node out of the current table
		    // structure.  If table has fallen empty because
		    // of an unmap operation it would have been
		    // deleted in step 3a.

		    node = table->get_node ();

 		    ASSERT (recurse_level > 0);
		    recurse_level--;
		    table = r_table[recurse_level];
		    tableidx = r_values[recurse_level].idx;
		    do_modify = r_values[recurse_level].mod;
		}
	    }

	    // --- PART 3d ---

	    // If we still have not found a valid mapping node it
	    // means that we are finished parsing the whole subtree.
	    // Exit the main loop.

	    if (node == NULL)
	    {
		ASSERT (recurse_level == 0);
		if (ctrl.unmap)
		    ASSERT (prev->get_next () == NULL);
		break;
	    }
	}

    } while (node->get_depth () > startnode->get_depth ());

    if (ctrl.unmap)
    {
	// Remove the mapping tree we have just parsed thorugh and
	// propagate the status bits upwards to the parent of the
	// deleted mapping tree.

	node = prev;
	prev = node->get_prev ();

	while (node != startnode)
	{
	    status_bits |= node->get_effective_status (this);
	    delete_node (node);
	    node = prev;
	    prev = node->get_prev ();
	}

	if (ctrl.mapctrl_self)
	{
	    status_bits |= startnode->get_effective_status (this);
	    startnode->get_parent ()->update_effective_status
		(this, status_bits);
	    delete_node (startnode);
	}
	else
	    startnode->update_effective_status (this, status_bits);
    }

    if (ctrl.reset_status)
    {
	// We've reached the end of the subtree.  Reset and propagate
	// reference status bits upwards to the starting node.

	word_t status = 0;
	node = prev;
	prev = node->get_prev ();

	while (node != startnode)
	{
	    status |= node->reset_effective_status (this);
	    node->flush_cached_entry (this, range);
	    node->update_purged_status (this, status);

	    if (! node->is_my_parent (prev))
	    {
		node->get_parent ()->update_effective_status (this, status);
		status = 0;
	    }

	    node = prev;
	    prev = prev->get_prev ();
	}

	node->update_effective_status (this, status);

	if (ctrl.mapctrl_self)
	{
	    // If we also reset the bits in the startnode, make sure
	    // that we propagate the bits to the parent of the
	    // startnode.

	    status |= startnode->reset_effective_status (this);
	    startnode->flush_cached_entry (this, range);
	    startnode->reset_purged_status (this);
	    startnode->get_parent ()->update_effective_status (this, status);
	}
    }

    mdb_lock.unlock();
    return status_bits;
}





/**
 * Locate parent of current mapping node.
 * @return pointer to parent
 */
mdb_node_t * mdb_node_t::get_parent (void)
{
    mdb_node_t * p = this;

    do { p = p->get_prev (); } while (p->get_depth () >= get_depth ());

    return p;
}
