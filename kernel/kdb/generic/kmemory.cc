/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/generic/kmemory.cc
 * Description:   Kernel memory statistics dumper
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
 * $Id: kmemory.cc,v 1.4 2003/09/24 19:05:11 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kmemory.h>
#include <kdb/linker_set.h>

#define MAX_KMEM_SIZECNT 10
static word_t num_chunks[MAX_KMEM_SIZECNT+1];

#if defined(CONFIG_KMEM_TRACE)
DECLARE_SET(__kmem_groups);
#endif

DECLARE_KMEM_GROUP (kmem_misc);


/**
 * Dump kernel memory allocator statistics
 */
DECLARE_CMD (cmd_kmem_stats, statistics, 'k', "kmem",
	     "kernel memory allocator statistics");

CMD (cmd_kmem_stats, cg)
{
    word_t * kmem_free_list = kmem.kmem_free_list;
    word_t free_chunks = kmem.free_chunks;

    word_t num, *cur, *prev, i, j, idx, max_chunks;

    // Clear out statistics
    for (idx = 0; idx <= MAX_KMEM_SIZECNT; idx++)
	num_chunks[idx] = 0;
    max_chunks = 0;

    for (num = 1, cur = kmem_free_list, prev = NULL;
	 cur;
	 prev = cur, cur = (word_t *) *cur )
    {
	// Are chunks contigous?
	if ((word_t) prev + KMEM_CHUNKSIZE == (word_t) cur)
	    num++;
	else
	    num = 1;

	for (i = j = 1, idx = 0; i <= num; i++)
	{
	    if (i == j)
	    {
		// Number of chunks is power of 2
		word_t mask = (j * KMEM_CHUNKSIZE)-1;
		word_t tmp = (word_t) cur - ((j-1) * KMEM_CHUNKSIZE);

		// Check if chunks are properly aligned
		if ((tmp & mask) == 0)
		{	
		    if (idx < MAX_KMEM_SIZECNT)
			num_chunks[idx]++;
		    else
			num_chunks[MAX_KMEM_SIZECNT]++;
		    if (idx > max_chunks)
			max_chunks = idx;
		}

		j <<= 1;
		idx++;
	    }
	}
    }

    word_t freemem = (free_chunks*KMEM_CHUNKSIZE);
    printf ("Kernel memory statistics:\n"
	    "  Chunk size  = %d bytes\n"
	    "  Free chunks = %d (%d%cB)\n\n",
	    KMEM_CHUNKSIZE, free_chunks, 
	    freemem >= GB (1) ? freemem >> 30 :
	    freemem >= MB (1) ? freemem >> 20 : freemem >> 10,
	    freemem >= GB (1) ? 'G' : freemem >= MB (1) ? 'M' : 'K');

    printf ("  Consecutive   Number of\n");
    printf ("  chunks        matches\n");
    for (i = 0; i <= MAX_KMEM_SIZECNT; i++)
    {
	printf ("  %7d%c      %6d\n", (1 << i),
		i == MAX_KMEM_SIZECNT ? '+' : ' ',
		num_chunks[i]);
    }
    printf("\n  Max consecutive chunks: %d\n", 1 << max_chunks);

#if defined(CONFIG_KMEM_TRACE)

    printf ("\nKernel memory distribution:\n");
    __kmem_groups.reset ();
    kmem_group_t * group;
    while ((group = (kmem_group_t *) __kmem_groups.next ()) != NULL)
    {
	printf ("  %16s %6d %cB\n",
		group->name,
		group->mem >= GB (1) ? group->mem >> 30 :
		group->mem >= MB (1) ? group->mem >> 20 : group->mem >> 10,
		group->mem >= GB (1) ? 'G' :
		group->mem >= MB (1) ? 'M' : 'K');
    }
    printf ("\n");

#endif

    return CMD_NOQUIT;
}
