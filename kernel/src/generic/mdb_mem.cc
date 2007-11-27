/*********************************************************************
 *                
 * Copyright (C) 2005-2007,  Karlsruhe University
 *                
 * File path:     generic/mdb_mem.cc
 * Description:   Memory specific generic mapping database functions
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
 * $Id: mdb_mem.cc,v 1.9 2007/01/08 14:11:23 skoglund Exp $
 *                
 ********************************************************************/
#include <mdb_mem.h>
#include <linear_ptab.h>
#include INC_ARCH(pgent.h)
#include INC_API(tcb.h)
#include INC_GLUE(space.h)


/**
 * Mapping database for all physical page frames in the system.
 */
mdb_mem_t mdb_mem;

word_t mdb_mem_t::sizes[] = MDB_MEM_SIZES;
word_t mdb_mem_t::num_sizes = MDB_MEM_NUMSIZES;


/**
 * Sigma0 memory mapping node.
 */
mdb_node_t * sigma0_memnode;


/**
 * Dummy page rable entry to associate with the sigma0 mapping node.
 * Needed in order to retrieve access rights when mapping from sigma0.
 */
static pgent_t sigma0_pgent;


/**
 * Add buffer allocation sizes.
 */
MDB_INIT_FUNCTION (1, init_mdb_mem_sizes)
{
    void mdb_add_size (word_t size);
    for (word_t i = 0; i < mdb_mem.num_sizes; i++)
	mdb_add_size ((1UL << (mdb_mem.sizes[i+1] - mdb_mem.sizes[i])) *
		      sizeof (mdb_tableent_t));
}


/**
 * Initialize the mapping database for memory.  A single mapping node
 * that covers the complete address space is created and owned by
 * sigma0.  All other mappings are derived from this node.
 */
MDB_INIT_FUNCTION (3, init_mdb_mem)
{
    sigma0_memnode = new mdb_node_t;
    ASSERT (sigma0_memnode);

    sigma0_memnode->set_prev (sigma0_memnode);
    sigma0_memnode->set_next (NULL);
    sigma0_memnode->set_depth (0);
    sigma0_memnode->set_inrights (~0UL);
    sigma0_memnode->set_outrights (~0UL);
    sigma0_memnode->set_object (&sigma0_pgent);
    sigma0_memnode->set_objsize (mdb_mem.sizes[mdb_mem.num_sizes]);
    sigma0_memnode->reset_purged_status (&mdb_mem);
    sigma0_memnode->reset_effective_status (&mdb_mem);

    // Set the access rights for the dummy sigma0 pgent to full
    // permissions.

    extern space_t * sigma0_space;
    sigma0_pgent.update_rights (sigma0_space, pgent_t::size_max + 1, ~0UL);

    // Sanity checking of page size arrays.

    pgent_t::pgsize_e i;
    word_t j;

    for (i = (pgent_t::pgsize_e) 0; i < pgent_t::size_max; i++)
    {
	if (! is_page_size_valid (i))
	    continue;
	for (j = 0; j < mdb_mem.num_sizes; j++)
	    if (hw_pgshifts[i] == mdb_mem.sizes[j])
		break;
 	if (j == mdb_mem.num_sizes)
	    panic ("mdb_mem.sizes[] is not a superset of valid hw_pgshifts[]");
    }
}



/*
 * Helper functions.
 */


INLINE pgent_t::pgsize_e pgsize (mdb_node_t * node)
{
    mdb_mem_misc_t misc (node->get_misc ());
    return (pgent_t::pgsize_e) misc.pgsize;
}

INLINE word_t purged_status (mdb_node_t * node)
{
    mdb_mem_misc_t misc (node->get_misc ());
    return misc.purged_status;
}

INLINE space_t * space (mdb_node_t * node)
{
    mdb_mem_misc_t misc (node->get_misc ());
    return (space_t *) (misc.space << 8);
}



/*
 * MDB specific functions.
 */


/**
 * Get radix to use for mapping table.
 * @param objsize	size of objects in table
 * @return radix to use for mapping table
 */
word_t mdb_mem_t::get_radix (word_t objsize)
{
    ASSERT (objsize < sizes[num_sizes]);

    word_t k;
    for (k = 0; objsize >= sizes[k]; k++) {}
    return sizes[k] - objsize;
}

/**
 * Get the next smaller valid object size.
 * @param objsize	size of object
 * @return size of next smaller object size
 */
word_t mdb_mem_t::get_next_objsize (word_t objsize)
{
    ASSERT (objsize > sizes[0]);

    word_t k;
    for (k = num_sizes-1; objsize <= sizes[k]; k--) {}
    return sizes[k];
}

/**
 * Get name of mapping database.
 * @return mapping database name
 */
const char * SECTION(SEC_KDEBUG) mdb_mem_t::get_name (void)
{
    return "mem";
}


/*
 * MDB specific mapping node functions.
 */


/**
 * Clear the mapping and flush any cached entries.
 * @param node		mapping node
 */
void mdb_mem_t::clear (mdb_node_t * node)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    addr_t vaddr = pg->vaddr (space (node), pgsize (node), node);
    pg->clear (space (node), pgsize (node), 0, vaddr);
    pg->flush (space (node), pgsize (node), 0, vaddr);
    space (node)->flush_tlbent (get_current_space (), vaddr,
				node->get_objsize ());
}

/**
 * Get effective access rights of mapping.
 * @param node		mapping node
 * @return effective access rights.
 */
word_t mdb_mem_t::get_rights (mdb_node_t * node)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    return pg->rights (space (node), pgsize (node));
}

/**
 * Set effective access rights for mapping.  Note that the TLB entry
 * for the mapping is not flushed.  This must be done explecitly
 * afterwards.
 * @param node		mapping node
 * @param r		new access rights
 */
void mdb_mem_t::set_rights (mdb_node_t * node, word_t r)
{ 
    pgent_t * pg = (pgent_t *) node->get_object ();
    return pg->set_rights (space (node), pgsize (node), r);
}

/**
 * Flush TLB entry for mapping.
 * @param node		mapping node
 */
void mdb_mem_t::flush_cached_entry (mdb_node_t * node, range_t range)
{
    if (space (node)->does_tlbflush_pay (range.get_size ()))
	return;

    pgent_t * pg = (pgent_t *) node->get_object ();
    addr_t vaddr = pg->vaddr (space (node), pgsize (node), node);
    pg->flush (space (node), pgsize (node), 0, vaddr);
    space (node)->flush_tlbent (get_current_space (), vaddr,
				node->get_objsize ());
}

/**
 * Check whether attribute updates are allowed for mapping.
 * @param node		mapping node
 * @return true if attribute updates are allowed
 */
#include INC_API(space.h)
bool mdb_mem_t::allow_attribute_update (mdb_node_t * node)
{
    // XXX: Temporary solution for V4
    return is_privileged_space (get_current_space ());
}

/**
 * Set attribute for mapping.
 * @param node		mapping node
 * @param attrib	new attribute
 */
void mdb_mem_t::set_attribute (mdb_node_t * node, word_t attrib)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    return pg->set_attributes (space (node), pgsize (node), attrib);
}

/**
 * Get physical address of mapping.
 * @param node		mapping node
 * @return physical address of page frame
 */
word_t mdb_mem_t::get_phys_address (mdb_node_t * node)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    return (word_t) pg->address (space (node), pgsize (node));
}

/**
 * Get the purged status bits for mapping.
 * @param node		mapping node
 * @return purged status bits
 */
word_t mdb_mem_t::get_purged_status (mdb_node_t * node)
{
    return purged_status (node);
}

/**
 * Reset the purged status bits for mapping.
 * @param node		mapping node
 */
void mdb_mem_t::reset_purged_status (mdb_node_t * node)
{
    node->set_misc (mdb_mem_misc (space (node), pgsize (node)));
}

/**
 * Update the purged status bits for mapping.
 * @param node		mapping node
 * @param status	new status bits
 */
void mdb_mem_t::update_purged_status (mdb_node_t * node, word_t status)
{
    node->set_misc (mdb_mem_misc (space (node),
				  pgsize (node),
				  purged_status (node) | status));
}

/**
 * Get effective status bits for mapping.
 * @param node		mapping node
 * @return effective status bits
 */
word_t mdb_mem_t::get_effective_status (mdb_node_t * node)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    addr_t vaddr = pg->vaddr (space (node), pgsize (node), node);
    return pg->reference_bits (space (node), pgsize (node), vaddr);
}

/**
 * Reset effective status bits for mapping.
 * @param node		mapping node
 * @return effective status bits
 */
word_t mdb_mem_t::reset_effective_status (mdb_node_t * node)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    addr_t vaddr = pg->vaddr (space (node), pgsize (node), node);
    word_t rwx = pg->reference_bits (space (node), pgsize (node), vaddr);
    pg->reset_reference_bits (space (node), pgsize (node));
    return rwx;
}

/**
 * Update effective status bits for mapping.
 * @param node		mapping node
 * @param status	new status bits
 */
void mdb_mem_t::update_effective_status (mdb_node_t * node, word_t status)
{
    pgent_t * pg = (pgent_t *) node->get_object ();
    pg->update_reference_bits (space (node), pgsize (node), status);
}

/**
 * Dump contents of mapping node.
 * @param node		mapping node
 */
void SECTION(SEC_KDEBUG) mdb_mem_t::dump (mdb_node_t * node)
{
    printf ("[%d] ", node->get_depth ());

    pgent_t * pg = (pgent_t *) node->get_object ();
    space_t * spc = space (node);
    pgent_t::pgsize_e psz =  pgsize (node);
    addr_t vaddr = pg->vaddr (spc, psz, node);
 
    printf ("vaddr: %p  spc: %p  ", vaddr, spc);
    printf ("rights: [o=%c%c%c i=%c%c%c e=%c%c%c] (%p)\n",
	    node->get_outrights () & 0x4 ? 'r' : '~',
	    node->get_outrights () & 0x2 ? 'w' : '~',
	    node->get_outrights () & 0x1 ? 'x' : '~',
	    node->get_inrights () & 0x4 ? 'r' : '~',
	    node->get_inrights () & 0x2 ? 'w' : '~',
	    node->get_inrights () & 0x1 ? 'x' : '~',
	    get_rights (node) & 0x4 ? 'r' : '~',
	    get_rights (node) & 0x2 ? 'w' : '~',
	    get_rights (node) & 0x1 ? 'x' : '~', node);
}
