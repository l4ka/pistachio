/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/generic/mapping.cc
 * Description:   Mapping database dumping
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
 * $Id: mapping.cc,v 1.2 2003/09/24 19:05:11 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <mapping.h>
#include <linear_ptab.h>

static void dump_mdbmaps (mapnode_t * map, addr_t paddr,
			  mapnode_t::pgsize_e size,
			  rootnode_t * proot, char * spc);

static void dump_mdbroot (rootnode_t * root, addr_t paddr,
			  mapnode_t::pgsize_e size, char * spc);


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

INLINE pgent_t::pgsize_e hw_pgsize (mapnode_t::pgsize_e mdb_pgsize)
{
    pgent_t::pgsize_e s = (pgent_t::pgsize_e) 0;
    while (hw_pgshifts[s] < mdb_pgshifts[mdb_pgsize])
	s++;
    return s;
}


/**
 * cmd_dump_mdb: dump mapping database
 */
DECLARE_CMD (cmd_dump_mdb, root, 'm', "mdb", "dump mapping database");

CMD (cmd_dump_mdb, cg)
{
    static char spaces[] = "                                                ";

    addr_t paddr = (addr_t) get_hex ("Address");
    if ((word_t) paddr == ABORT_MAGIC)
	return CMD_NOQUIT;
    
    dump_mdbroot (mdb_index_root (mapnode_t::size_max, 
				  sigma0_mapnode->get_nextroot (), paddr),
		  paddr, mapnode_t::size_max, spaces + sizeof (spaces)-1);

    return CMD_NOQUIT;
}

static void dump_mdbmaps (mapnode_t * map, addr_t paddr,
			  mapnode_t::pgsize_e size,
			  rootnode_t * proot, char * spc)
{
    mapnode_t * pmap = NULL;

    while (map)
    {
	space_t * space = map->get_space ();
	pgent_t::pgsize_e hwsize = hw_pgsize (size);

	printf ("%s[%d] space=%p  vaddr=%p  pgent=%p  (%p)\n",
		spc - map->get_depth () * 2, map->get_depth (), space,
		(pmap ?
		 map->get_pgent (pmap)->vaddr (space, hwsize, map) :
		 map->get_pgent (proot)->vaddr (space, hwsize, map)),
		pmap ? map->get_pgent(pmap) : map->get_pgent(proot),
		map);
	
	pmap = map;
	if (map->is_next_root () || map->is_next_both () &&
	    map->get_nextroot () != NULL)
	{
	    dump_mdbroot (mdb_index_root (size-1, map->get_nextroot (), paddr),
			  paddr, size-1, spc - 2 - map->get_depth () * 2);
	}
	map = map->get_nextmap ();
    }
}

static void dump_mdbroot (rootnode_t * root, addr_t paddr,
			  mapnode_t::pgsize_e size, char * spc)
{
    printf ("%s%p: %d%cB %s (%p)\n",
	    spc, addr_mask (paddr,  ~((1 << mdb_pgshifts[size]) - 1)),
	    ((mdb_pgshifts[size] >= 30) ? 1 << (mdb_pgshifts[size] - 30) :
	     (mdb_pgshifts[size] >= 20) ? 1 << (mdb_pgshifts[size] - 20) :
	     1 << (mdb_pgshifts[size] - 10)),
	    ((mdb_pgshifts[size] >= 30) ? 'G' :
	     (mdb_pgshifts[size] >= 20) ? 'M' : 'K'),
	    root->is_next_both () ? "[root/map]" :
	    root->is_next_root () ? "[root]" : "[map]", root);

    if (root->is_next_map () || root->is_next_both ())
	dump_mdbmaps (root->get_map (), paddr, size, root, spc - 2);

    if (root->is_next_root () || root->is_next_both ())
	dump_mdbroot (mdb_index_root (size-1, root->get_root (), paddr), paddr,
		      size-1,  spc - 2);
}

