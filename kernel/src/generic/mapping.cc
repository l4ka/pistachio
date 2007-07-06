/*********************************************************************
 *                
 * Copyright (C) 2000-2006,  Karlsruhe University
 *                
 * File path:     generic/mapping.cc
 * Description:   Generic mapping database implementation
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
 * $Id: mapping.cc,v 1.27 2006/10/07 16:34:09 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <mapping.h>
#include <linear_ptab.h>
#include <sync.h>

#include INC_API(tcb.h)

spinlock_t mdb_lock;

word_t mdb_pgshifts[] = MDB_PGSHIFTS;


// The sigma0_mapnode is initialized to own the whole address space.
static mapnode_t __sigma0_mapnode;
mapnode_t * sigma0_mapnode;

static rootnode_t * mdb_create_roots (mapnode_t::pgsize_e size) NOINLINE;
static dualnode_t * mdb_create_dual (mapnode_t * map, rootnode_t * root)
    NOINLINE;


/**
 * Initialize mapping database structures
 */
void SECTION (".init") init_mdb (void)
{
    dualnode_t *dual;

    void mdb_buflist_init (void);
    mdb_buflist_init ();

    mdb_lock.lock();

    // Frame table for the complete address space.
    dual = mdb_create_dual (NULL, mdb_create_roots (mapnode_t::size_max));

    // Let sigma0 own the whole address space.
    sigma0_mapnode = &__sigma0_mapnode;
    sigma0_mapnode->set_backlink ((mapnode_t *) NULL, (pgent_t *) NULL);
    sigma0_mapnode->set_space ((space_t *) 0);
    sigma0_mapnode->set_depth (0);
    sigma0_mapnode->set_next (dual);

    pgent_t::pgsize_e i;
    mapnode_t::pgsize_e j;

    // Sanity checking of pgshift arrays.
    for (i = (pgent_t::pgsize_e) 0; i < pgent_t::size_max; i++)
    {
	if (! is_page_size_valid (i))
	    continue;
	for (j = (mapnode_t::pgsize_e) 0; j < mapnode_t::size_max; j++)
	    if (hw_pgshifts[i] == mdb_pgshifts[j])
		break;
	if (j == mapnode_t::size_max)
	    panic ("mdb_pgshifts[] is not a superset of valid hw_pgshifts[]");
    }

    mdb_lock.unlock();
}



/*
 * Helper functions
 */

INLINE word_t mdb_arraysize (mapnode_t::pgsize_e pgsize)
{
    return 1 << (mdb_pgshifts[pgsize+1] - mdb_pgshifts[pgsize]);
}

INLINE word_t mdb_get_index (mapnode_t::pgsize_e size, addr_t addr)
{
    return ((word_t) addr >> mdb_pgshifts[size]) & (mdb_arraysize(size) - 1);
}

INLINE rootnode_t * mdb_index_root (mapnode_t::pgsize_e size, rootnode_t * r,
				    addr_t addr)
{
    return r + mdb_get_index (size, addr);
}

INLINE mapnode_t::pgsize_e mdb_pgsize (pgent_t::pgsize_e hw_pgsize)
{
    mapnode_t::pgsize_e s = (mapnode_t::pgsize_e) 0;
    while (mdb_pgshifts[s] < hw_pgshifts[hw_pgsize])
	s++;
    return s;
}

INLINE pgent_t::pgsize_e hw_pgsize (mapnode_t::pgsize_e mdb_pgsize)
{
    pgent_t::pgsize_e s = (pgent_t::pgsize_e) 0;
    while (hw_pgshifts[s] < mdb_pgshifts[mdb_pgsize])
	s++;
    return s;
}

static void remove_map (mapnode_t * pmap, mapnode_t * cmap)
{
    mapnode_t * nmap = cmap->get_nextmap ();

    if (pmap->is_next_map ())
	pmap->set_next (nmap);
    else
    {
	dualnode_t * dual = pmap->get_nextdual ();
	if (nmap)
	    dual->map = nmap;
	else
	{
	    // No more mappings, remove dual node
	    pmap->set_next (dual->root);
	    mdb_free_buffer ((addr_t) dual, sizeof (dualnode_t));
	}
    }

    mdb_free_buffer ((addr_t) cmap, sizeof (mapnode_t));

    if (nmap)
	nmap->set_backlink (pmap, nmap->get_pgent (cmap));
}

static void NOINLINE remove_map (rootnode_t * proot, mapnode_t * cmap)
{
    mapnode_t * nmap = cmap->get_nextmap ();

    if (proot->is_next_map ())
	proot->set_ptr (nmap);
    else
    {
	dualnode_t * dual = proot->get_dual ();
	if (nmap)
	    dual->map = nmap;
	else
	{
	    // No more mappings, remove dual node
	    proot->set_ptr (dual->root);
	    mdb_free_buffer ((addr_t) dual, sizeof (dualnode_t));
	}
    }

    mdb_free_buffer ((addr_t) cmap, sizeof (mapnode_t));

    if (nmap)
	nmap->set_backlink (proot, nmap->get_pgent (cmap));
}


/**
 * Inserts mapping into mapping database
 * 
 * @param f_map		node to map from
 * @param f_pg		page table entry of source mapping
 * @param f_hwpgsize	page size of source mapping
 * @param f_addr	virtual address of source mapping
 * @param t_pg		page table entry for destination mapping
 * @param t_hwpgsize	page size of destination mapping
 * @param t_space	space of destination mapping
 * @param grant		grant or map
 * 
 * @returns mapping node of the destination mapping
 */
mapnode_t * mdb_map (mapnode_t * f_map, pgent_t * f_pg,
		     pgent_t::pgsize_e f_hwpgsize, addr_t f_addr,
		     pgent_t * t_pg, pgent_t::pgsize_e t_hwpgsize,
		     space_t * t_space, bool grant)
{
    rootnode_t *root, *proot;
    mapnode_t *nmap;

    //space_t::begin_update();
    mdb_lock.lock();

    // Grant operations simply reuse the old mapping node
    if (grant)
    {
	if (f_map->is_prev_root ())
	    f_map->set_backlink (f_map->get_prevroot (f_pg), t_pg);
	else
	    f_map->set_backlink (f_map->get_prevmap (f_pg), t_pg);

	f_map->set_space (t_space);
	mdb_lock.unlock();
	//space_t::end_update();
	return f_map;
    }

    // Convert to mapping database pagesizes
    mapnode_t::pgsize_e f_pgsize = mdb_pgsize (f_hwpgsize);
    mapnode_t::pgsize_e t_pgsize = mdb_pgsize (t_hwpgsize);

    mapnode_t * newmap = (mapnode_t *) mdb_alloc_buffer (sizeof (mapnode_t));

    if (f_pgsize == t_pgsize)
    {
	// Hook new node directly below mapping node
	nmap = f_map->get_nextmap ();
	newmap->set_backlink (f_map, t_pg);
	newmap->set_space (t_space);
	newmap->set_depth (f_map->get_depth () + 1);
	newmap->set_next (nmap);

	// Fixup prev->next pointer
	if (f_map->is_next_root ())
	    f_map->set_next (mdb_create_dual (newmap, f_map->get_nextroot ()));
	else if (f_map->is_next_both ())
	    f_map->get_nextdual ()->map = newmap;
	else
	    f_map->set_next (newmap);

	// Fixup next->prev pointer
	if (nmap)
	    nmap->set_backlink (newmap, nmap->get_pgent (f_map));

	mdb_lock.unlock();
	//space_t::end_update();
	return newmap;
    }

    root = NULL;

    while (f_pgsize > t_pgsize)
    {
	// Need to traverse into subtrees
	f_pgsize--;
	proot = root;

	if (proot)
	{
	    nmap = proot->get_map ();
	    root = proot->get_root ();
	}
	else
	{
	    // This is for 1st iteration only
	    nmap = f_map->get_nextmap ();
	    root = f_map->get_nextroot ();
	}

	if (! root)
	{
	    // New array needs to be created
	    root = mdb_create_roots (f_pgsize);

	    if (proot)
		// Insert below previous root node
		if (nmap)
		    proot->set_ptr (mdb_create_dual (nmap, root));
		else
		    proot->set_ptr (root);
	    else
		// Insert below original mapping node
		if (nmap)
		    f_map->set_next (mdb_create_dual (nmap, root));
		else
		    f_map->set_next (root);
	}

	// Traverse into subtree
	root = mdb_index_root (f_pgsize, root, f_addr);
    }

    // Insert mapping below root node
    nmap = root->get_map ();
    newmap->set_backlink (root, t_pg);
    newmap->set_space (t_space);
    newmap->set_depth (f_map->get_depth () + 1);
    newmap->set_next (root->get_map ());

//    printf ("newmap=%p  prev=%p:%p  space=%p  depth=%p  next=%p:%p:%p\n",
//	    newmap, newmap->get_prevmap(t_pg), newmap->get_prevroot(t_pg),
//	    newmap->get_space(), newmap->get_depth(),
//	    newmap->get_nextmap(), newmap->get_nextroot(),
//	    newmap->get_nextdual());

    // Fixup root->next pointer
    if (root->is_next_root ())
	root->set_ptr (mdb_create_dual (newmap, root->get_root ()));
    else if (root->is_next_both ())
	root->get_dual ()->map = newmap;
    else
	root->set_ptr (newmap);

    // Fixup next->prev pointer
    if (nmap)
	nmap->set_backlink (newmap, nmap->get_pgent (root));

    mdb_lock.unlock();
    //space_t::end_update();
    return newmap;
}



/**
 * Flush mapping recursively from mapping database
 *
 * @param f_map		node to flush from
 * @param f_pg		page table entry of source mapping
 * @param f_hwpgsize	page size of source mapping
 * @param f_addr	virtual address of source mapping
 * @param t_hwpgsize	page size to flush
 * @param fp		fpage specified to fpage_unmap()
 * @param unmap_self	flush self or just just child mapping
 *
 * Recursively revokes access attributes of indicated mapping.  If all
 * access rights are revoked, the mapping will be recursively removed
 * from the mapping database.
 *
 * @returns logical OR of all involved mappings' access attributes
 */
word_t mdb_flush (mapnode_t * f_map, pgent_t * f_pg,
		  pgent_t::pgsize_e f_hwpgsize, addr_t f_addr,
		  pgent_t::pgsize_e t_hwpgsize, fpage_t fp, bool unmap_self)
{
    dualnode_t *dual;
    mapnode_t *pmap, *nmap;
    rootnode_t *root, *proot, *nroot;
    word_t rcnt, startdepth, rwx;
    addr_t vaddr;
    space_t *space, *parent_space;
    pgent_t *parent_pg;
    pgent_t::pgsize_e parent_pgsize;

    mapnode_t * r_nmap[mapnode_t::size_max];
    rootnode_t * r_root[mapnode_t::size_max];
    word_t  r_rcnt[mapnode_t::size_max];
    word_t  r_prev[mapnode_t::size_max];  /* Bit 0 set indicates mapping.
					     Bit 0 cleared indicates root. */
    rcnt = 0;
    startdepth = f_map->get_depth ();
    root = NULL;

    if (unmap_self)
    {
	// Read and reset the reference bits stored in the mapping node.
	// These reference bits were updated when someone further up in
	// the mappings tree performed an reference bit read-and-reset
	// (i.e., unmap).  Also clear the reference bits in the page table
	// entry to make sure that the mapping node bits are not modified
	// in the algortihm below.

	space = f_map->get_space ();
	vaddr = f_pg->vaddr (space, f_hwpgsize, f_map);

	rwx = f_map->get_rwx () |
	    f_pg->reference_bits (space, f_hwpgsize, vaddr);
	f_map->set_rwx (0);
	f_pg->reset_reference_bits (space, f_hwpgsize);

	parent_pg = NULL;
    }
    else
    {
	// Do not take the reference bits of current mapping into
	// account unless we have a flush operation.  Instead, we
	// record the reference bits for the whole subtree in the
	// current page table entry.

	rwx = 0;
	parent_pg = f_pg;
	parent_space = f_map->get_space ();
	parent_pgsize = f_hwpgsize;
    }

    // Convert to mapping database pagesizes
    mapnode_t::pgsize_e f_pgsize = mdb_pgsize (f_hwpgsize);
    mapnode_t::pgsize_e t_pgsize = mdb_pgsize (t_hwpgsize);

    mdb_lock.lock();

    do {
	// Variables `f_map' and `f_pg' are valid at this point

	dual  = f_map->get_nextdual ();
	nroot = f_map->get_nextroot ();
	nmap  = f_map->get_nextmap ();
	pmap  = f_map->get_prevmap (f_pg);
	proot = f_map->get_prevroot (f_pg);
	space = f_map->get_space ();

	f_hwpgsize = hw_pgsize (f_pgsize);

	// Variable contents at this point:
	//
	//   f_map - the current mapping node
	//   f_pg  - the current pgent node
	//   pmap  - the previous mapping node (or NULL if prev is root)
	//   proot - the previous root node (or NULL if prev is map)
	//   nmap  - the next mapping node (may be NULL)
	//   dual  - next dual node (or NULL if no such node)
	//   root  - Current root array pointer (or NULL)

//	printf("New: f_map=%p  dual=%p  nmap=%p  pmap=%p  proot=%p  "
//	       "fsize=%d   tsize=%d  root=%p\n",
//	       f_map, dual, nmap, pmap, proot, f_pgsize, t_pgsize, root);

	vaddr = f_pg->vaddr (space, f_hwpgsize, f_map);

	if (unmap_self)
	{
	    // Update reference bits
	    f_map->update_rwx
		(f_pg->reference_bits (space, f_hwpgsize, vaddr));
	    rwx |= f_pg->reference_bits (space, f_hwpgsize, vaddr);

	    ASSERT (f_pgsize <= t_pgsize);

	    if (fp.is_rwx ())
	    {
		// Revoke all access rights (i.e., remove node)
		if (pmap)
		    remove_map (pmap, f_map);
		else
		    remove_map (proot, f_map);
		f_pg->clear(space, f_hwpgsize, false, vaddr);
	    }
	    else
	    {
		// Revoke access rights
		f_pg->revoke_rights (space, f_hwpgsize, fp.get_rwx ());
		f_pg->reset_reference_bits (space, f_hwpgsize);
		f_pg->flush (space, f_hwpgsize, false, vaddr);
		pmap = f_map;
		proot = NULL;
	    }

	    // We might have to flush some TLB entries
	    if (! space->does_tlbflush_pay (fp.get_size_log2 ()))
		space->flush_tlbent (get_current_space (), vaddr,
				     page_shift (f_hwpgsize));
	}
	else
	{
	    f_pg->reset_reference_bits (space, f_hwpgsize);
	    pmap = f_map;
	    proot = NULL;
	}
	f_map = NULL;

	// Variables `f_map' and `f_pg' are no longer valid here

	if (nroot)
	{
	    f_pgsize--;

	    if (f_pgsize < t_pgsize)
	    {
		// Recurse into subarray before checking mappings
		ASSERT (f_pgsize < mapnode_t::size_max);
		r_prev[f_pgsize] = pmap ? (word_t) pmap | 1 : (word_t) proot;
		r_nmap[f_pgsize] = nmap;
		r_root[f_pgsize] = root;
		r_rcnt[f_pgsize] = rcnt;

		root = nroot - 1;
		rcnt = mdb_arraysize (f_pgsize);

		if (dual && fp.is_rwx () && unmap_self)
		    mdb_free_buffer ((addr_t) dual, sizeof (dualnode_t));
	    }
	    else
	    {
		// We may use the virtual address f_addr for indexing
		// here since the alignment within the page will be
		// the same as with the physical address.
		root = mdb_index_root (f_pgsize, nroot, f_addr) - 1;
		rcnt = 1;
	    }
	}
	else
	{
	    if (nmap)
	    {
		if (pmap)
		    f_pg = nmap->get_pgent (pmap);
		else
		    f_pg = nmap->get_pgent (proot);
		f_map = nmap;
	    }
	    else if ((f_pgsize < t_pgsize)  && root)
	    {
		// Recurse up from subarray
		if (fp.is_rwx ())
		{
		    // Revoke all access rights (i.e., remove subtree)
		    root -= mdb_arraysize (f_pgsize) - 1;
		    mdb_free_buffer ((addr_t) root,
				     mdb_arraysize (f_pgsize) *
				     sizeof (rootnode_t));
		}

		ASSERT (f_pgsize < mapnode_t::size_max);
		f_map = r_nmap[f_pgsize];
		root  = r_root[f_pgsize];
		rcnt  = r_rcnt[f_pgsize];
		if (r_prev[f_pgsize] & 1)
		{
		    proot = NULL;
		    pmap = (mapnode_t *) (r_prev[f_pgsize] & ~1UL);
		    if (f_map)
			f_pg = f_map->get_pgent (pmap);
		}
		else
		{
		    proot = (rootnode_t *) r_prev[f_pgsize];
		    pmap = NULL;
		    if (f_map)
			f_pg = f_map->get_pgent (proot);
		}

		f_pgsize++;
	    }
	}

	// If f_map now is non-nil, the variables f_map, f_pg, pmap
	// and proot will all be valid.  Otherwise, root and rcnt will
	// be valid.

	while ((! f_map) && (rcnt > 0))
	{
	    rcnt--;
	    root++;

	    dual  = root->get_dual ();
	    nroot = root->get_root ();
	    f_map = root->get_map ();

	    if (nroot)
	    {
		// Recurse into subarray before checking mappings
		f_pgsize--;

		if (fp.is_rwx ())
		    root->set_ptr (f_map); // Remove subarray

		ASSERT (f_pgsize < mapnode_t::size_max);
		r_prev[f_pgsize] = (word_t) root;
		r_nmap[f_pgsize] = f_map;
		r_root[f_pgsize] = root;
		r_rcnt[f_pgsize] = rcnt;

		f_map = NULL;
		root = nroot - 1;
		rcnt = mdb_arraysize (f_pgsize);

		if (dual && fp.is_rwx ())
		    mdb_free_buffer ((addr_t) dual, sizeof (dualnode_t));
	    }
	    else
	    {
		if (f_map)
		{
		    f_pg = f_map->get_pgent (root);
		    pmap = NULL;
		    proot = root;
		}
	    }
	}

	if (f_pgsize <= t_pgsize)
	    // From now on we will unmap all nodes
	    unmap_self = true;

    } while (f_map && f_map->get_depth () > startdepth);

    // Update the reference bits in the page table entry of the parent
    // mapping.
    // XXX: Handle the case where one does flush instead of unmap.
    if (parent_pg)
	parent_pg->update_reference_bits (parent_space, parent_pgsize, rwx);
   
    mdb_lock.unlock();

    return rwx;
}


/**
 * Create root array
 *
 * @param size	page size for root nodes
 *
 * Allocates and initializes a new root array.
 *
 * @returns pointer to array
 */
static NOINLINE rootnode_t * mdb_create_roots (mapnode_t::pgsize_e size)
{
    rootnode_t *newnodes, *n;

    word_t num = mdb_arraysize (size);
    newnodes = (rootnode_t *) mdb_alloc_buffer (sizeof (rootnode_t) * num);

    for (n = newnodes; num--; n++)
	n->set_ptr ((mapnode_t *) NULL);

    return newnodes;
}


/**
 * Create a dual node
 * 
 * @param map	map pointer of node
 * @param root	root pointer of node
 * 
 * Allocates and initializes a new dual node.
 * 
 * @returns pointer to dual node
 */
static NOINLINE dualnode_t * mdb_create_dual (mapnode_t * map,
					      rootnode_t * root)
{
    dualnode_t * dual = (dualnode_t *) mdb_alloc_buffer (sizeof (dualnode_t));
    dual->map = map;
    dual->root = root;
    return dual;
}
