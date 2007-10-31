/*********************************************************************
 *                
 * Copyright (C) 2005-2007,  Karlsruhe University
 *                
 * File path:     generic/vrt.cc
 * Description:   Generic Variable Radix Table for managing mappings
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
 * $Id: vrt.cc,v 1.14 2006/11/14 18:54:54 skoglund Exp $
 *                
 ********************************************************************/
#include <vrt.h>
#include <mdb.h>
#include <kdb/tracepoints.h>

#include INC_API (fpage.h)


DECLARE_TRACEPOINT (VRT_MAP);
DECLARE_TRACEPOINT (VRT_OVERMAP);
DECLARE_TRACEPOINT (VRT_MAPCTRL);


/**
 * Add buffer allocation sizes.
 */
MDB_INIT_FUNCTION (1, init_vrt_sizes)
{
    void mdb_add_size (word_t size);
    mdb_add_size (sizeof (vrt_table_t));
}


/**
 * Create and partially initialize a new table object.  Allocates both
 * the table control structure and the table entries themselves.
 *
 * @param size		size of control structure
 * @param radix_log2	size of table
 *
 * @return new table object
 */
void * vrt_table_t::operator new (size_t size, word_t radix_log2)
{
    vrt_table_t * t = (vrt_table_t *) mdb_alloc_buffer (size);

    t->radix = radix_log2;
    t->entries = (word_t)
	mdb_alloc_buffer ((sizeof (vrt_node_t) + sizeof (mdb_node_t *)) *
			  (1UL << radix_log2));

    vrt_node_t * n = t->get_node (0);
    for (word_t k = 0; k < (1UL << radix_log2); k++, n++)
	     n->clear ();

    return (void *) t;
}


/**
 * Remove table.  The operation assumes that the table is empty.
 *
 * @param t		table to remove
 */
void vrt_table_t::operator delete (void * t)
{
    if (t == NULL)
	return;

    vrt_table_t * table = (vrt_table_t *) t;

    mdb_free_buffer ((void *) table->entries,
		     (sizeof (vrt_node_t) + sizeof (mdb_node_t *)) *
		     (1UL << table->get_radix ()));
    mdb_free_buffer (table, sizeof (vrt_table_t));
}


/**
 * Lookup object in table.
 *
 * @param addr		address of mapping
 * @param value		returned object value
 * @param objsize	returned object size
 *
 * @return true if object found, false otherwise
 */
bool vrt_t::lookup (word_t addr, word_t * value, word_t * objsize)
{
    vrt_table_t * table = get_table ();
    vrt_node_t * node;

    for (;;)
    {
	if (! table->match_prefix (addr))
	    return false;

	node = table->get_node (addr);

	if (! node->is_valid ())
	    return false;

	if (! node->is_table ())
	{
	    *objsize = table->get_objsize ();
	    *value = node->get_object ();
	    return true;
	}

	table = node->get_table ();
    }

    /* NOTREACHED */
    return false;
}


/**
 * Perform a map operation within current table/space.
 *
 * @param f_fp		flexpage for current space
 * @param base		send base to use for mapping
 * @param t_space	destination table/space
 * @param t_fp		flexpage for destination space
 * @param grant		true if map operation is a grant
 */
void vrt_t::map_fpage (fpage_t f_fp, word_t base, vrt_t * t_space,
		       fpage_t t_fp, bool grant)
{
    TRACEPOINT (VRT_MAP, "%s::%s (fp=%p [%p,%d] base=%p) to %s (fp=%p [%p,%d])\n",
			get_name (), grant ? "grant" : "map",
			f_fp.raw, f_fp.get_base (), f_fp.get_size_log2 (),
			base, t_space->get_name (), t_fp.raw,
			t_fp.get_base (), t_fp.get_size_log2 ());

    // Determine the exact location for where to map from and where to
    // map to.

    word_t f_addr, t_addr, f_num, t_num, mapsize, offset;
    if (f_fp.get_size_log2 () <= t_fp.get_size_log2 ())
    {
	f_num = f_fp.get_size_log2 ();
	base &= base_mask (t_fp, f_num);
	f_addr = (word_t) address (f_fp, f_num);
	t_addr = (word_t) addr_offset
	    (addr_mask (address (t_fp, f_num),
			~base_mask (t_fp, f_num)), base);
    }
    else
    {
	f_num = t_fp.get_size_log2 ();
	base &= base_mask (f_fp, f_num);
	f_addr = (word_t) addr_offset
	    (addr_mask (address (f_fp, f_num),
			~base_mask (f_fp, f_num)), base);
	t_addr = (word_t) address (t_fp, f_num);
    }

    // Determine the size and number of objects to map.

    if (f_num > get_vrt_size ())
	f_num = get_vrt_size ();
    mapsize = get_next_objsize (f_num + 1);
    f_num = t_num = 1UL << (f_num - mapsize);

    mdb_node_t * map;
    mdb_node_t * f_map;
    vrt_table_t * f_table = get_table ();
    vrt_table_t * t_table = t_space->get_table ();
    vrt_node_t * f_node = f_table->get_node (f_addr);
    vrt_node_t * t_node = t_table->get_node (t_addr);
    word_t f_off = 0;

    // Arrays to use for recursion.  We don't want to do full function
    // recursion because of stack space requirements.

    vrt_table_t * r_ftable[MAX_VRT_DEPTH];
    vrt_node_t *  r_fnode[MAX_VRT_DEPTH];
    word_t	  r_fnum[MAX_VRT_DEPTH];
    vrt_table_t * r_ttable[MAX_VRT_DEPTH];
    vrt_node_t *  r_tnode[MAX_VRT_DEPTH];
    word_t	  r_tnum[MAX_VRT_DEPTH];
    word_t f_depth = 0, t_depth = 0;

    while (f_num > 0 || t_num > 0)
    {
	if (! f_node->is_valid () ||
	    (f_node->is_table () &&
	     ! f_node->get_table ()->match_prefix (f_addr)))
	{
	    // There exist no valid object in source space.  Skip the
	    // whole mapping (possibly recursing up from destination
	    // space first).

	    while (t_table->get_objsize () < f_table->get_objsize ())
	    {
		t_depth--;
		t_table = r_ttable[t_depth];
		t_node = r_tnode[t_depth];
		t_num = r_tnum[t_depth];
	    }

	    if (t_table->get_objsize () == f_table->get_objsize ())
		goto Next_receiver_entry;

	    // Destination obj size > source obj size.
	    goto Next_sender_entry;
	}


	if (f_table->get_objsize () > mapsize && f_node->is_table ())
	{
	    // We are working on too large source nodes.  Skip down
	    // into subtable.

	    f_table = f_node->get_table ();
	    f_node = f_table->get_node (f_addr);
	    continue;
	}
	else if (f_node->is_table ())
	{
	    // Mappings in sender space are small.  Need to map every
	    // single object within subtable(s).

	    r_ftable[f_depth] = f_table;
	    r_fnode[f_depth] = f_node + 1;
	    r_fnum[f_depth] = f_num - 1;
	    f_depth++;

	    f_table = f_node->get_table ();
	    f_node = f_table->get_node (0);
	    f_num = 1UL << f_table->get_radix ();
	    continue;
	}
	else if (f_table->get_objsize () > mapsize)
	{
	    // We are mapping smaller sub-objects out of a larger
	    // object.

	    f_num = 1;
	}

	// We have now finished looking up the mapping in the source
	// space.  Current status is:
	//
	//   f_node - object to map from
	//   f_table - table where object resides
	//   f_num - number of f_node objects (of this size) to map
	//
	// Next we have to find the location in the destination space
	// where to insert the new object(s).

	if ((t_table->get_objsize () > f_table->get_objsize ()) ||
	    (t_table->get_objsize () > mapsize))
	{
	    // We are currently working on too large receive sizes.
	    // We need to either recurse into a subtable, or if no
	    // such table exist, create a subtable.

	    r_ttable[t_depth] = t_table;
	    r_tnode[t_depth] = t_node + 1;
	    r_tnum[t_depth] = t_num - 1;
	    t_depth++;

	    if (t_node->is_valid () && ! t_node->is_table ())
	    {
		// We are overmapping a large object with a smaller
		// one.  We need to unmap the larger object.

		TRACEPOINT (VRT_OVERMAP, "%s overmap: faddr=%p fsz=%d "
			    "taddr=%p tsz=%d (single entry)\n",
			    t_space->get_name (),
			    f_addr, f_table->get_objsize (),
			    t_addr, t_table->get_objsize ());

		t_space->get_mapdb ()->flush (t_table->get_mapnode (t_addr));
		ASSERT (! t_node->is_valid ());

		// We might have unmapped the source mapping during
		// the flush operation.

		if (! f_node->is_valid ())
		    goto Next_receiver_entry;
	    }

	    if ((! t_node->is_valid ()) ||
		(t_node->is_table () &&
		 ! t_node->get_table ()->match_prefix (t_addr)))
	    {
		// Appropriate subtable does not exist.  Create a
		// table according to the wanted object size.  We
		// perform path compression to avoid unnecessary
		// lookup and memory overhead.

		word_t newsize = f_table->get_objsize () < mapsize ?
		    f_table->get_objsize () : mapsize;
		vrt_table_t * newtable =
		    new (get_radix (newsize)) vrt_table_t;

		newtable->set_prefix (t_addr);
		newtable->set_objsize (newsize);

		if (t_node->is_valid ())
		{
		    ASSERT (t_node->is_table ());

		    if (newtable->match_prefix
			(t_node->get_table ()->get_prefix ()))
		    {
			// There exists a sub-table that is completely
			// contained within the new table.  Remove it.

			TRACEPOINT (VRT_OVERMAP,"%s overmap: faddr=%p fsz=%d "
				    "taddr=%p (subtable <%p,%p>)\n",
				    t_space->get_name (),
				    f_addr, f_table->get_objsize (),
				    t_addr, t_node->get_table ()->get_start_addr (),
				    t_node->get_table ()->get_end_addr ());

			word_t num = 1;
			word_t addr = t_addr;
			word_t start_depth = t_depth;

			while (num > 0)
			{
			    if (! t_node->is_valid ())
			    {
				// Skip invalid entries.
			    }
			    else if (t_node->is_table ())
			    {
				// We must unmap each single object in
				// subtable.

				r_ttable[t_depth] = t_table;
				r_tnode[t_depth] = t_node;
				r_tnum[t_depth] = num - 1;
				t_depth++;

				t_table = t_node->get_table ();
				t_node = t_table->get_node (0);
				num = 1UL << t_table->get_radix ();
				continue;
			    }
			    else
			    {
				// Unmap object from destination space.

				t_space->get_mapdb ()->flush
				    (t_table->get_mapnode (addr));
			    }

			    // Skip to next table entry.

			    addr += 1UL << t_table->get_objsize ();
			    if (t_depth > start_depth)
				t_node++;
			    num--;

			    while (num == 0 && t_depth > start_depth)
			    {
				// Recurse up an remove subtable.

				t_depth--;
				t_table = r_ttable[t_depth];
				t_node = r_tnode[t_depth];
				num = r_tnum[t_depth];

				delete t_node->get_table ();
				t_node->clear ();
				if (t_depth > start_depth)
				    t_node++;
			    }

			    // Don't delete the source node

			    if (t_depth == start_depth)
				break;
			}

			// We might have unmapped the source mapping during
			// the flush operation.

			if (! f_node->is_valid ())
			{
			    delete newtable;
			    goto Next_receiver_entry;
			}
		    }
		    else
		    {
			// There is a conflict between the newly
			// created table and a previously existing
			// table (i.e., both tables fall under the
			// same parent table entry).  We need to
			// create an intermediate table that can
			// resolve the conflict.

			// The intermediate table will hold objects of
			// sizes which is the maximum valid object
			// size that does not generate a mapping table
			// where the two subtables map to the same
			// table entry.

			word_t imask;
			word_t isize = t_table->get_objsize ();

			do {
			    isize = get_next_objsize (isize);
			    imask = get_radix (isize) + isize;
			    imask = (imask == sizeof (word_t) * 8 ?
				     ~0UL : ((1UL << imask) - 1))
				& ~((1UL << isize) - 1);
			} while ((t_addr & imask) ==
				 (t_node->get_table ()->get_prefix () &
				  imask));

			ASSERT (isize > newsize);
			ASSERT (isize > t_node->get_table ()->get_objsize ());

			vrt_table_t * itable =
			    new (get_radix (isize)) vrt_table_t;

			itable->set_prefix (t_addr);
			itable->set_objsize (isize);
			itable->set_table (t_addr, newtable);
			itable->set_table
			    (t_node->get_table ()->get_prefix (),
			     t_node->get_table ());
			newtable = itable;
		    }
		}

		t_table->set_table (t_addr, newtable);
	    }

	    ASSERT (t_node->is_valid () && t_node->is_table () &&
		    t_node->get_table ()->match_prefix (t_addr));

	    if (t_node->get_table ()->get_objsize () >= mapsize)
	    {
		// We still haven't recursed down to the object size
		// we are supposed to operate on.

		t_table = t_node->get_table ();
		t_node = t_table->get_node (t_addr);
		continue;
	    }

	    t_table = t_node->get_table ();
	    t_node = t_table->get_node (t_addr);
	    t_num = 1UL << t_table->get_radix ();
	    continue;
	}

	else if (t_node->is_valid () &&
		 t_table->get_objsize () <= f_table->get_objsize ())
	{
	    // We are ovemapping a node or a whole subtree.  Unmap
	    // each single entry in the subtree.

	    TRACEPOINT (VRT_OVERMAP, "%s overmap: faddr=%p fsz=%d "
			"taddr=%p tsz=%d (%s)\n",
			get_name (),
			f_addr, f_table->get_objsize (),
			t_addr, t_table->get_objsize (),
			t_node->is_table () ?
			"subtable" : "single entry");

	    word_t num = 1;
	    word_t addr = t_addr;
	    word_t start_depth = t_depth;

	    while (num > 0)
	    {
		if (! t_node->is_valid ())
		{
		    // Skip invalid entries.
		}
		else if (t_node->is_table ())
		{
		    // We must unmap each single object in
		    // subtable.

		    r_ttable[t_depth] = t_table;
		    r_tnode[t_depth] = t_node;
		    r_tnum[t_depth] = num - 1;
		    t_depth++;

		    t_table = t_node->get_table ();
		    t_node = t_table->get_node (0);
		    num = 1UL << t_table->get_radix ();
		    continue;
		}
		else
		{
		    // Unmap object from destination space.

		    t_space->get_mapdb ()->flush
			(t_table->get_mapnode (addr));
		}

		// Skip to next table entry.

		addr += 1UL << t_table->get_objsize ();
		if (t_depth > start_depth)
		    t_node++;
		num--;

		while (num == 0 && t_depth > start_depth)
		{
		    // Recurse up an remove subtable.

		    t_depth--;
		    t_table = r_ttable[t_depth];
		    t_node = r_tnode[t_depth];
		    num = r_tnum[t_depth];

		    delete t_node->get_table ();
		    t_node->clear ();
		    if (t_depth > start_depth)
			t_node++;
		}
	    }

	    // We might have unmapped the source mapping during
	    // the flush operation.

	    if (! f_node->is_valid ())
		goto Next_receiver_entry;
	}

	// We have now finished looking up the mapping in both the
	// source and the destination space
	//
	//   f_node  - object to map from
	//   f_table - table where source object resides
	//   f_num   - number of f_node objects (of this size) to map
	//   t_node  - object to map to
	//   t_table - table where destination object resides
	//   t_num   - number of t_node objects (of this size) to map
	//
	// Further, the t_node is guaranteed to be invalid at this
	// point.

	ASSERT (! t_node->is_valid ());

	offset = f_addr
	    & ((1UL << f_table->get_objsize ()) - 1)
	    & ~((1UL << t_table->get_objsize ()) - 1);

	f_map = f_table->get_mapnode (f_addr);
	if (grant)
	    f_map = f_map->get_parent ();

	t_table->set_object (t_space, t_addr,
			     f_node->get_address (this) + offset + f_off,
			     f_node, f_table->get_objsize (),
			     f_fp.get_access ());
	map = get_mapdb ()->map (f_map, t_node, t_table->get_objsize (),
				 f_node->get_address (this) + offset + f_off,
				 f_fp.get_access (), ~0UL);
	map->set_misc (t_space->make_misc (t_node, map));
	t_table->set_mapnode (t_addr, map);

    Next_receiver_entry:

	t_addr += 1UL << t_table->get_objsize ();
	t_num--;

	if (t_num > 0)
	{
	    t_node++;
	    if (t_table->get_objsize () < f_table->get_objsize ())
	    {
		// We are mapping smaller objects out of a large
		// object.  Update the object offset for the mapping
		// in the desination space and do not skip to the next
		// source object yet.

		f_off += 1UL << t_table->get_objsize ();
		continue;
	    }
	}
	else if (t_table->get_objsize () < f_table->get_objsize () &&
		 f_table->get_objsize () <= mapsize)
	{
	    // We have finished mapping a subtable.  Recurse up to
	    // parent table.

	    do {
		f_off += 1UL << t_table->get_objsize ();
		t_depth--;
		t_table = r_ttable[t_depth];
		t_node = r_tnode[t_depth];
		t_num = r_tnum[t_depth];
	    } while (t_num == 0 &&
		     t_table->get_objsize () < f_table->get_objsize ());

	    // If mapping large object into smaller objects, don't
	    // skip to the next source object yet.

	    if (t_num > 0 &&
		t_table->get_objsize () < f_table->get_objsize ())
		continue;
	}

    Next_sender_entry:

	if (grant)
	    get_mapdb ()->flush (f_table->get_mapnode (f_addr));

	f_addr += 1UL << f_table->get_objsize ();
	f_num--;
	f_off = 0;

	if (f_num > 0)
	{
	    f_node++;
	    continue;
	}
	else if (f_depth > 0)
	{
	    // We have finished mapping a subtable.  Recurse up to the
	    // parent table.

	    do {
		f_depth--;
		f_table = r_ftable[f_depth];
		f_node = r_fnode[f_depth];
		f_num = r_fnum[f_depth];
	    } while (f_num == 0 && f_depth > 0);

	    // We may also need to recurse up in the receiver space
	    // now.

	    while (t_num == 0 && t_depth > 0)
	    {
		t_depth--;
		f_off += 1UL << t_table->get_objsize ();
		t_table = r_ttable[t_depth];
		t_node = r_tnode[t_depth];
		t_num = r_tnum[t_depth];
	    }
	}
	else
	{
	    // Finished.
	    t_num = 0;
	}
    }
}


/**
 * Perform a map control operation within current table/space.
 *
 * @param fp		flexpage to perform mapctrl on
 * @param ctrl		type of operation
 * @param rights	new access rights
 * @param attrib	new attributes
 *
 * @return status bits for flexpage
 */
word_t vrt_t::mapctrl (fpage_t fp, mdb_t::ctrl_t ctrl,
		       word_t rights, word_t attrib)
{
    TRACEPOINT (VRT_MAPCTRL,
		"%s::mapctrl (%p [%p,%d], %x [%s] %x, %p)\n",
		get_name (),
		fp.raw, fp.get_base (), fp.get_size_log2 (),
		ctrl.raw, ctrl.string(), rights & 0x7, attrib);

    // Determine size and number of objects to mapctrl.  Make sure
    // that we stay within the bounds of the space even if user
    // specified an fpage larger than the space.

    word_t mapsize = get_next_objsize (fp.get_size_log2 () + 1);
    word_t num = get_vrt_size ();
    if (num > fp.get_size_log2 ())
	num = fp.get_size_log2 ();
    num = 1UL << (num - mapsize);

    word_t vaddr = (word_t) address (fp, mapsize);
    vrt_table_t * table = get_table ();
    vrt_node_t * node = table->get_node (vaddr);
    word_t status = 0;

    // Arrays to use for recursion.  We don't want to do full function
    // recursion because of stack space requirements.

    vrt_table_t * r_table[MAX_VRT_DEPTH];
    vrt_node_t *  r_node[MAX_VRT_DEPTH];
    word_t	  r_num[MAX_VRT_DEPTH];
    word_t depth = 0;

    while (num)
    {
	if (! node->is_valid () ||
	    (node->is_table () &&
	     ! fp.is_range_overlapping
	     ((addr_t) node->get_table ()->get_start_addr (),
	      (addr_t) node->get_table ()->get_end_addr ())))
	{
	    // There exist no valid object in source space.  Skip the
	    // whole mapping.

	    goto Next_entry;
	}

	if (table->get_objsize () > mapsize && node->is_table ())
	{
	    // We are working on too large source nodes.  Skip down
	    // into subtable.

	    table = node->get_table ();
	    node = table->get_node (vaddr);
	    continue;
	}
	else if (node->is_table ())
	{
	    // Mappings in space are small.  Need to mapctrl every
	    // single object within subtable(s).

	    r_table[depth] = table;
	    r_node[depth] = node;
	    r_num[depth] = num - 1;
	    depth++;

	    table = node->get_table ();
	    node = table->get_node (0);
	    num = 1UL << table->get_radix ();
	    continue;
	}
	else if (table->get_objsize () > mapsize)
	{
	    // We are performing mapctrl on too small a mapping.
	    // Increase the scope of the mapctrl operation.

	    num = 1;
	    mapsize = table->get_objsize ();
	}

	get_mapdb ()->mapctrl (table->get_mapnode (vaddr),
			       mdb_t::range_t (fp.get_base (),
					       fp.get_size_log2 ()),
			       ctrl, rights, attrib);

    Next_entry:

	vaddr += 1UL << table->get_objsize ();
	num--;

	if (num > 0)
	    node++;
	else if (depth > 0)
	{
	    // We have finished parsing a subtable.  Recurse up to the
	    // parent table.  Delete table if we are doing unmap in
	    // the current space.

	    do {
		depth--;
		table = r_table[depth];
		node = r_node[depth];
		num = r_num[depth];

		if (ctrl.unmap && ctrl.mapctrl_self)
		{
		    delete node->get_table ();
		    node->clear ();
		}
		node++;
	    } while (num == 0 && depth > 0);
	}
    }

    return status;
}
