/*********************************************************************
 *                
 * Copyright (C) 1999-2003, 2005-2006,  Karlsruhe University
 *                
 * File path:     generic/mapping_alloc.cc
 * Description:   Memory management for mapping database
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
 * $Id: mapping_alloc.cc,v 1.13 2006/10/07 16:34:09 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <mapping.h>
#include <kdb/tracepoints.h>
#include <kmemory.h>

#if defined(CONFIG_NEW_MDB)
#include <mdb.h>
#undef MDB_NUM_PGSIZES
#define MDB_NUM_PGSIZES	16
#endif

//#define MDB_BUF_TRACEPOINTS

/*
 * Size of chunks allocated from kmem.
 */
#define MDB_ALLOC_CHUNKSZ	4096


#undef enable_interrupts
#undef disable_interrupts
#define enable_interrupts()
#define disable_interrupts()

#if defined(MDB_BUF_TRACEPOINTS)
DECLARE_TRACEPOINT (MDB_ALLOC_BUFFER);
DECLARE_TRACEPOINT (MDB_FREE_BUFFER);
#endif

DECLARE_KMEM_GROUP (kmem_mdb);

//#define MDB_DEBUG 

#ifdef MDB_DEBUG
#define ASSERT_BL(_bl_)							\
{									\
    mdb_mng_t *p = NULL, *m = (mdb_mng_t *) (_bl_)->list_of_lists;	\
    mdb_link_t *l;							\
    int cnt;								\
									\
    if (m) {								\
	ASSERT (m->prev_freelist == NULL);				\
	do {								\
	    ASSERT (m->prev_freelist == p);				\
	    ASSERT (m->freelist != NULL);				\
	    ASSERT (m->num_free > 0);					\
	    ASSERT (m->bl == (_bl_));					\
	    for (cnt = 0, l = m->freelist; l; cnt++)			\
		l = l->next;						\
	    ASSERT (cnt == m->num_free);				\
	    p = m;							\
	    m = m->next_freelist;					\
	} while (m);							\
    }									\
}

#else
# define ASSERT_BL(_bl_)
#endif

/*
 * We have AT MOST one entry for every MDB page size, one for
 * mapnode_t, one for dualnode_t. and one for the NULL entry.
 */
mdb_buflist_t mdb_buflists[MDB_NUM_PGSIZES + 3];

/*
 * Structure used for freelists.
 */
struct mdb_link_t {
    mdb_link_t		*next;
};

/*
 * Structure used for managing contents of a 4kb page.
 */
struct mdb_mng_t {
    mdb_link_t		*freelist;
    word_t		num_free;
    mdb_mng_t 		*next_freelist;
    mdb_mng_t		*prev_freelist;
    mdb_buflist_t 	*bl;
};


void mdb_add_size (word_t size)
{
    mdb_buflist_t * bl = mdb_buflists;

    // Avoid adding the same size twice
    for (word_t k = 0; bl->size; bl++, k++)
    {
	ASSERT (k < MDB_NUM_PGSIZES + 3);
	if (bl->size == size)
	    return;
    }

    bl->size = size;
    bl++;
    bl->size = 0;
}

#if defined(CONFIG_NEW_MDB)

MDB_INIT_FUNCTION (0, mdb_buflist_init)
{
    mdb_buflists[0].size = 0;
    mdb_add_size (sizeof (mdb_node_t));
    mdb_add_size (sizeof (mdb_table_t));
}

MDB_INIT_FUNCTION (2, mdb_buflist_init)
{
    mdb_buflist_t * bl;
    word_t i;

    /*
     * Initialize the remaining fields in the buflists array.  The max_
     * free field is calculated here so that we don't have to use
     * division in the mdb_alloc_buffer() function.
     */
    for (bl = mdb_buflists; bl->size; bl++)
    {
	bl->list_of_lists = NULL;
	if (bl->size >= KMEM_CHUNKSIZE)
	    continue;
	for (bl->max_free = 0, i = MDB_ALLOC_CHUNKSZ - bl->size;
	     i >= sizeof (mdb_mng_t);
	     i -= bl->size, bl->max_free++) {}
    }
}

#else /* !CONFIG_NEW_MDB */

void SECTION (".init") mdb_buflist_init (void)
{
    mdb_buflist_t * bl;
    word_t i;

    mdb_buflists[0].size = 0;

    /*
     * Add necessary allocation sizes.
     */

    mdb_add_size (sizeof (mapnode_t));
    mdb_add_size (sizeof (dualnode_t));
    for(i = 0; i < MDB_NUM_PGSIZES; i++)
	mdb_add_size ((1 << (mdb_pgshifts[i+1] - mdb_pgshifts[i])) *
		      sizeof (rootnode_t));

    /*
     * Initialize the remaining fields in the buflists array.  The max_
     * free field is calculated here so that we don't have to use
     * division in the mdb_alloc_buffer() function.
     */
    for (bl = mdb_buflists; bl->size; bl++)
    {
	bl->list_of_lists = NULL;
	if (bl->size >= KMEM_CHUNKSIZE)
	    continue;
	for (bl->max_free = 0, i = MDB_ALLOC_CHUNKSZ - bl->size;
	     i >= sizeof (mdb_mng_t);
	     i -= bl->size, bl->max_free++) {}
    }
}

#endif



/*
 * Function mdb_alloc_buffer (size)
 *
 *    Allocate a buffer of the indicated size.  It is assumed the size
 *    is at least 8, and a multiple of 4.
 *
 */
addr_t mdb_alloc_buffer (word_t size)
{
    mdb_buflist_t * bl;
    mdb_link_t * buf;
    mdb_mng_t * mng;

    /*
     * Find the correct buffer list.
     */
    for (bl = mdb_buflists; bl->size != size; bl++)
    {
	if (bl->size == 0)
	{
	    panic ("Illegal MDB buffer allocation size (%d) (fn=%p).\n",
		   size, __builtin_return_address (0));
	}
    }

#if defined(MDB_BUF_TRACEPOINTS)
    TRACEPOINT (MDB_ALLOC_BUFFER,
		printf ("mdb_alloc_buffer (%d), ip: %p\n",
			size, __builtin_return_address (0)));
#endif

    /*
     * Forward large buffer requests directly to the kmem allocator.
     */
    if (size >= KMEM_CHUNKSIZE)
	return kmem.alloc (kmem_mdb, size);

    disable_interrupts ();

    /*
     * Get pool of available buffers.
     */
    mng = bl->list_of_lists;
    if (mng == NULL)
    {
	mdb_link_t *b, *n, *p, *ed;

	enable_interrupts ();

	/*
	 * First slot of page is dedicated to management strucures.
	 */
	mng = (mdb_mng_t *) kmem.alloc (kmem_mdb, MDB_ALLOC_CHUNKSZ);
	mng->freelist		= (mdb_link_t *)
	    ((word_t) mng + MDB_ALLOC_CHUNKSZ - bl->max_free*size);
	mng->num_free		= bl->max_free;
	mng->prev_freelist	= (mdb_mng_t *) NULL;
	mng->bl			= bl;

	/*
	 * Initialize buffers.
	 */
	ed = (mdb_link_t *) ((word_t) mng + MDB_ALLOC_CHUNKSZ - size);
	for (b = mng->freelist; b <= ed; b = n)
	{
	    n = (mdb_link_t *) ((word_t) b + size);
	    b->next = n;
	}
	p = (mdb_link_t *) ((word_t) b - size);
	p->next = (mdb_link_t *) NULL;

	disable_interrupts ();

	/*
	 * Update list of freelists.
	 */
	mng->next_freelist = bl->list_of_lists;
	bl->list_of_lists = mng;
	if (mng->next_freelist)
	{
	    mng->next_freelist->prev_freelist = mng;
	}
    }

    ASSERT_BL (bl);

    /*
     * Remove buffer from freelist inside page.
     */
    buf = mng->freelist;
    mng->freelist = buf->next;
    mng->num_free--;

    if (mng->num_free == 0)
    {
	/*
	 * All buffers in page have been used.  Remove freelist from
	 * list_of_lists.
	 */
	bl->list_of_lists = mng->next_freelist;
	if (bl->list_of_lists != NULL)
	    bl->list_of_lists->prev_freelist = (mdb_mng_t *) NULL;
    }

    ASSERT_BL (bl);
    enable_interrupts ();

    return (addr_t) buf;
}


/*
 * Function mdb_free_buffer (addr)
 *
 *    Free buffer at address `addr'.
 *
 */
void mdb_free_buffer (addr_t addr, word_t size)
{
    mdb_buflist_t *bl;
    mdb_mng_t *mng;
    mdb_link_t *buf;

#if defined(MDB_BUF_TRACEPOINTS)
    TRACEPOINT (MDB_FREE_BUFFER,
		printf ("mdb_free_buffer (%p, %d), ip: %p\n",
			addr, size, __builtin_return_address (0)));
#endif

    /*
     * Forward large buffer requests directly to the kmem allocator.
     */
    if (size >= KMEM_CHUNKSIZE)
    {
	kmem.free (kmem_mdb, addr, size);
	return;
    }

    buf = (mdb_link_t *) addr;
    mng = (mdb_mng_t *) ((word_t) addr & ~(MDB_ALLOC_CHUNKSZ-1));
    bl = mng->bl;

    ASSERT_BL (bl);
    disable_interrupts ();

    /*
     * Put buffer into local pool of buffers.
     */
    buf->next = mng->freelist;
    mng->freelist = buf;
    mng->num_free++;

    if (mng->num_free == 1)
    {
	/*
	 * Buffer pool has gone from empty to non-empty.  Put pool back
	 * into list of pools.
	 */
	if (bl->list_of_lists) 
	    bl->list_of_lists->prev_freelist = mng;
	mng->next_freelist = bl->list_of_lists;
	mng->prev_freelist = (mdb_mng_t *) NULL;
	bl->list_of_lists = mng;

    }
    else if (mng->num_free == bl->max_free)
    {
	/*
	 * We have freed up all buffers in the frame.  Give frame back
	 * to kernel memory allocator.
	 */
	if (mng->next_freelist)
	    mng->next_freelist->prev_freelist = mng->prev_freelist;

	if (bl->list_of_lists == mng)
	    bl->list_of_lists = mng->next_freelist;
	else if (mng->prev_freelist)
	    mng->prev_freelist->next_freelist = mng->next_freelist;

	kmem.free (kmem_mdb, (addr_t) mng, MDB_ALLOC_CHUNKSZ);
    }

    ASSERT_BL (bl);
    enable_interrupts ();
}

