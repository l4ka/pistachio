/*********************************************************************
 *                
 * Copyright (C) 2005-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/vrt_io.cc
 * Description:   VRT for thread objects
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
 * $Id: vrt_io.cc,v 1.4 2006/06/12 17:10:26 skoglund Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_GLUE(vrt_io.h)
#include INC_GLUE(io_space.h)

#include <kdb/tracepoints.h>
#include <debug.h>

word_t vrt_io_t::sizes[] = VRT_IO_SIZES;
word_t vrt_io_t::num_sizes = VRT_IO_NUMSIZES;


/**
 * Add buffer allocation sizes.
 */
MDB_INIT_FUNCTION (1, init_vrt_io_sizes)
{
    void mdb_add_size (word_t size);
    for (word_t i = 0; i < vrt_io_t::num_sizes; i++)
	mdb_add_size ((1UL << (vrt_io_t::sizes[i+1] - vrt_io_t::sizes[i])) *
		      (sizeof (vrt_node_t) + sizeof (mdb_node_t *)));
    mdb_add_size (sizeof (vrt_io_t));
}


/**
 * Allocate new VRT structure for implementing IO space.
 *
 * @param size		size of data structure
 *
 * @return new VRT for thread space
 */
void * vrt_io_t::operator new (word_t size)
{
    vrt_io_t * vrt = (vrt_io_t *) mdb_alloc_buffer (size);

    // Allocate and initialize root table

    vrt_io_t dummy;
    vrt_table_t * table =
	new (dummy.get_radix (sizes[num_sizes - 1])) vrt_table_t;
    table->set_prefix (0);
    table->set_objsize (sizes[num_sizes - 1]);
    vrt->set_table (table);

    vrt->init ();
    return vrt;
}

/**
 * Delete the VRT structure.
 *
 * @param size		size of data structure
 *
 * @return new VRT for IO space
 */
void vrt_io_t::operator delete (void * v)
{
    vrt_io_t * vrt = (vrt_io_t *) v;
    mdb_free_buffer (vrt, sizeof (mdb_node_t));
}

/**
 * Initialize the VRT for the IO space.
 */
void vrt_io_t::init (void)
{
#if defined(CONFIG_KDB)
    // Initialize name to "io<address>".

    word_t idx = 4 + sizeof (word_t) * 2;
    word_t num = (word_t) this;
    name[0] = 'i'; name[1] = 'o'; name[2] = '<';
    name[idx--] = 0; name[idx--] = '>';
    while (idx > 2)
    {
	name[idx--] = (num & 0xf) > 9 ?
	    (num & 0xf) - 10 + 'a' : (num & 0xf) + '0';
	num >>= 4;
    }
#endif

    count = 0;
}

/**
 * Populate complete space with idempotent mappings.  We don't care
 * about allocating mapping nodes for all the entries.  We just use
 * the root sigma0 mapping node for all entries.  The mapping database
 * will allocate entries on demand.
 */
void SECTION(".init") vrt_io_t::populate_sigma0 (void)
{
 
    vrt_table_t * t = get_table ();
    vrt_node_t * n = t->get_node (0);
    word_t addr = 0;
    for (word_t k = 0; k < (1UL << t->get_radix ()); k++, n++)
    {
	n->set_object ((addr & 0xffff) | (1UL << 16));
	t->set_mapnode (addr, sigma0_ionode);
	addr += (1UL << t->get_objsize ());
    }
}

/**
 * Get radix to use for table.
 * @param objsize	size of objects in table
 * @return radix to use for table
 */
word_t vrt_io_t::get_radix (word_t objsize)
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
word_t vrt_io_t::get_next_objsize (word_t objsize)
{
    ASSERT (objsize > sizes[0]);

    word_t k;
    for (k = num_sizes-1; objsize <= sizes[k]; k--) { }
    return sizes[k];
}

/**
 * Get size of VRT space.
 * @return log2 size of VRT space.
 */

word_t vrt_io_t::get_vrt_size (void)
{
    return sizes[num_sizes];
}

/**
 * Get mapping database associated with VRT.
 * @return mappings database associated with VRT
 */
mdb_t * vrt_io_t::get_mapdb (void)
{
    return &mdb_io;
}

/**
 * Get name of table structure.
 * @reurn name of table structure
 */
const char * vrt_io_t::get_name (void)
{
#if defined(CONFIG_KDB)
    return name;
#else
    return "";
#endif
}

/**
 * Copy IO port object and set valid bit.
 * @param n		destination object
 * @param paddr		physical address
 * @param o		source object
 * @param access	access rights
 */
void vrt_io_t::set_object (vrt_node_t * n, word_t n_sz, word_t paddr,
			   vrt_node_t * o, word_t o_sz, word_t access)
{
    n->set_object ((paddr & 0xffff) | (1UL << 16));
    zero_io_bitmap(get_space(), (paddr & 0xffff), n_sz);
}

/**
 * Get address (IO port) of object.
 * @param n		destination object
 * @return global port number of thread object
 */
word_t vrt_io_t::get_address (vrt_node_t * n)
{
    return get_port (n->get_object ());
}

/**
 * Dump VRT node information
 * @param n		destination object
 */
void vrt_io_t::dump (vrt_node_t * n)
{
    word_t value = n->get_object ();
    printf ("port: %04x\n", get_port (value));
}

/**
 * Create mapping node misc contents.
 * @param obj		vrt object
 * @param map		mapping node
 * @return pointer to current space
 */
word_t vrt_io_t::make_misc (vrt_node_t * obj, mdb_node_t * map)
{
    return (word_t) this->get_space();
}
