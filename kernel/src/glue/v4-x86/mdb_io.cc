/*********************************************************************
 *                
 * Copyright (C) 2005-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/mdb_io.cc
 * Description:   IO port specific generic mappings database functions
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
 * $Id: mdb_io.cc,v 1.5 2007/01/08 14:08:11 skoglund Exp $
 *                
 ********************************************************************/
#include <vrt.h>
#include INC_GLUE(mdb_io.h)
#include INC_GLUE(io_space.h)



/**
 * Mapping database for all physical page frames in the system.
 */
mdb_io_t mdb_io;

word_t mdb_io_t::sizes[] = MDB_IO_SIZES;
word_t mdb_io_t::num_sizes = MDB_IO_NUMSIZES;


/**
 * Sigma0 IO port mapping node.
 */
mdb_node_t * sigma0_ionode;


/**
 * Dummy VRT entry to associate with the sigma0 mapping node.  Needed
 * in order to retrieve access rights when mapping from sigma0.
*/
static vrt_node_t sigma0_ioent;





/**
 * Add buffer allocation sizes.
 */
MDB_INIT_FUNCTION (1, init_mdb_io_sizes)
{
    void mdb_add_size (word_t size);
    for (word_t i = 0; i < mdb_io.num_sizes; i++)
	mdb_add_size ((1UL << (mdb_io.sizes[i+1] - mdb_io.sizes[i])) *
		      sizeof (mdb_tableent_t));
}


/**
 * Initialize the mapping database for IO ports.  A single mapping node
 * that covers the complete address space is created and owned by
 * sigma0.  All other mappings are derived from this node.
 */
MDB_INIT_FUNCTION (3, init_mdb_io)
{
    sigma0_ionode = new mdb_node_t;

    sigma0_ionode->set_prev (sigma0_ionode);
    sigma0_ionode->set_next (NULL);
    sigma0_ionode->set_depth (0);
    sigma0_ionode->set_inrights (~0UL);
    sigma0_ionode->set_outrights (~0UL);
    sigma0_ionode->set_object (&sigma0_ioent);
    sigma0_ionode->set_objsize (mdb_io.sizes[mdb_io.num_sizes]);
    // Set the access rights for the dummy sigma0 pgent to full
    // permissions.

    vrt_io_t::set_rights (&sigma0_ioent, vrt_io_t::fullrights);
    // Sanity checking of object sizes.

    extern vrt_io_t vrt_io;
    word_t i, j;

    for (i = 0; i < vrt_io.num_sizes; i++)
    {
	for (j = 0; j < mdb_io.num_sizes; j++)
	    if (vrt_io.sizes[i] == mdb_io.sizes[j])
		break;
 	if (j == mdb_io.num_sizes)
	    panic ("mdb_io.sizes[] is not a superset of vrt_io.sizes[]");
    }
}


/*
 * MDB specific functions.
 */


/**
 * Get radix to use for mapping table.
 * @param objsize	size of objects in table
 * @return radix to use for mapping table
 */
word_t mdb_io_t::get_radix (word_t objsize)
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
word_t mdb_io_t::get_next_objsize (word_t objsize)
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
const char * SECTION(SEC_KDEBUG) mdb_io_t::get_name (void)
{
    return "io";
}


/*
 * MDB specific mapping node functions.
 */


/**
 * Clear the mapping and flush any cached entries.
 * @param node		mapping node
 */
void mdb_io_t::clear (mdb_node_t * node)
{
    vrt_node_t * n = (vrt_node_t *) node->get_object ();
    set_io_bitmap((space_t *)node->get_misc(),
		  vrt_io_t::get_port (n->get_object()), node->get_objsize());
    n->clear ();

}

/**
 * Get effective access rights of mapping.
 * @param node		mapping node
 * @return effective access rights.
 */
word_t mdb_io_t::get_rights (mdb_node_t * node)
{
    vrt_node_t * n = (vrt_node_t *) node->get_object ();
    return (word_t) vrt_io_t::get_rights (n->get_object ());
}

/**
 * Set effective access rights for mapping.  Note that the TLB entry
 * for the mapping is not flushed.  This must be done explecitly
 * afterwards.
 * @param node		mapping node
 * @param r		new access rights
 */
void mdb_io_t::set_rights (mdb_node_t * node, word_t r)
{ 
    vrt_node_t * n = (vrt_node_t *) node->get_object ();
    vrt_io_t::set_rights (n, (vrt_io_t::rights_e) r);
}

/**
 * Flush TLB entry for mapping.
 * @param node		mapping node
 */
void mdb_io_t::flush_cached_entry (mdb_node_t * node, range_t range)
{
}

/**
 * Check whether attribute updates are allowed for mapping.
 * @param node		mapping node
 * @return true if attribute updates are allowed
 */
bool mdb_io_t::allow_attribute_update (mdb_node_t * node)
{
    return false;
}

/**
 * Set attribute for mapping.
 * @param node		mapping node
 * @param attrib	new attribute
 */
void mdb_io_t::set_attribute (mdb_node_t * node, word_t attrib)
{
}

/**
 * Get physical address of mapping.  The physical address is the
 * global IO port number.
 * @param node		mapping node
 * @return global IO port number of mapping
 */
word_t mdb_io_t::get_phys_address (mdb_node_t * node)
{
    vrt_node_t * n = (vrt_node_t *) node->get_object ();
    return vrt_io_t::get_port (n->get_object ());
}

/**
 * Get the purged status bits for mapping.
 * @param node		mapping node
 * @return purged status bits
 */
word_t mdb_io_t::get_purged_status (mdb_node_t * node)
{
    return 0;
}

/**
 * Reset the purged status bits for mapping.
 * @param node		mapping node
 */
void mdb_io_t::reset_purged_status (mdb_node_t * node)
{
}

/**
 * Get effective status bits for mapping.
 * @param node		mapping node
 * @return effective status bits
 */
word_t mdb_io_t::get_effective_status (mdb_node_t * node)
{
    return 0;
}

/**
 * Reset effective status bits for mapping.
 * @param node		mapping node
 * @return effective status bits
 */
word_t mdb_io_t::reset_effective_status (mdb_node_t * node)
{
    return 0;
}

/**
 * Update effective status bits for mapping.
 * @param node		mapping node
 * @param status	new status bits
 */
void mdb_io_t::update_effective_status (mdb_node_t * node, word_t status)
{
}


/**
 * Update the purged status bits for mapping.
 * @param node		mapping node
 * @param status	new status bits
 */
void mdb_io_t::update_purged_status (mdb_node_t * node, word_t status)
{
}

/**
 * Dump contents of mapping node.
 * @param node		mapping node
 */
void mdb_io_t::dump (mdb_node_t * node)
{
    vrt_node_t * n = (vrt_node_t *) node->get_object ();
    word_t value = n->get_object ();

    printf ("[%d] ", node->get_depth ());
    printf ("port: %04x-%04x, space: %p\n", 
	    vrt_io_t::get_port (value),
	    vrt_io_t::get_port (value) + (1UL << node->get_objsize ()) - 1,
	    node->get_misc());
}

