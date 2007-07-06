/*********************************************************************
 *                
 * Copyright (C) 2005,  Karlsruhe University
 *                
 * File path:     kdb/generic/mdb.cc
 * Description:   Functions for debugging mapping databases
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
 * $Id: mdb.cc,v 1.6 2005/05/11 17:23:44 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include <mdb.h>
#include <mdb_mem.h>
#include <linear_ptab.h>


/*
 * Command group for mapping database.
 */

DECLARE_CMD_GROUP (mdb);


/**
 * Menu for mapping database
 */
DECLARE_CMD (cmd_mdb_menu, root, 'm', "mdb", "mapping database");

CMD (cmd_mdb_menu, cg)
{
    return mdb.interact (cg, "mdb");
}


/**
 * Dump memory mappings for given page frame.
 */
DECLARE_CMD (cmd_mdb_mem_dump, mdb, 'm', "dumpmem",
	     "dump specific memory mappings");

CMD (cmd_mdb_mem_dump, cg)
{
    word_t addr =  get_hex ("Address");
    if (addr == ABORT_MAGIC)
	return CMD_NOQUIT;

    dump_resource_map (&mdb_mem, sigma0_memnode, addr, 0);
    return CMD_NOQUIT;
}


/**
 * Dump all memory mappings.
 */
DECLARE_CMD (cmd_mdb_mem_dump_all, mdb, 'M', "dumpallmem",
	     "dump all memory mappings");

CMD (cmd_mdb_mem_dump_all, cg)
{
    extern mdb_node_t * sigma0_memnode;
    dump_table (&mdb_mem, sigma0_memnode->get_table (), 0);
    return CMD_NOQUIT;
}



/*
 * Helper functions
 */

static char * indent (word_t depth)
{
    static char spc[] = "                                              "
	"                                                              ";
    char * p = spc + sizeof (spc) - 1 - depth * 2;
    return p < spc || p >= spc + sizeof (spc) ? spc : p;
}

static word_t sz_num (word_t sz)
{
    return sz >= 30 ? (1UL << sz) >> 30 :
	sz >= 20 ? (1UL << sz) >> 20 :
	sz >= 10 ? (1UL << sz) >> 10 : (1UL << sz);
}

static const char * sz_suf (word_t sz)
{
    return sz >= 30 ? "G" : sz >= 20 ? "M" : sz >= 10 ? "K" : "";
}



/**
 * Dump mapping database table, including all subtrees and mapping
 * nodes.
 *
 * @param mdb		mapping databse
 * @param t		table to dump
 * @param depth		current recursion depth
 */
STATIC void kdb_t::dump_table (mdb_t * mdb, mdb_table_t * t, word_t depth)
{
    if (t == NULL)
	return;

    word_t paddr = t->prefix & ~(((1UL << t->objsize) << t->radix) - 1);

    printf ("%s%p table [objsize=%d%s  radix=%d  count=%d] (%p)\n",
	    indent (depth), paddr,
	    sz_num (t->objsize), sz_suf (t->objsize),
	    1UL << t->radix, t->count, t);

    mdb_tableent_t * te = t->get_entry (0);

    for (word_t k = 0;
	 k < (1UL << t->radix);
	 k++, te++, paddr += (1UL << t->objsize))
    {
	if (! te->is_valid ())
	    continue;

	if (te->is_table ())
	    dump_table (mdb, te->get_table (), depth + 1);

	if (te->get_node ())
	{
	    printf ("%s%p ", indent (depth + 1), paddr);

	    mdb_node_t * n = te->get_node ();
	    if (n == NULL)
	    {
		printf ("[null node]\n");
		continue;
	    }
	    word_t start_depth = n->get_depth ();

	    mdb->dump (n);
	    if (n->get_table ())
		dump_table (mdb, n->get_table (), depth + 2);

	    n = n->get_next ();
	    while (n != NULL)
	    {
		printf ("%s%ws ", indent (depth + 1 + n->get_depth ()
					  - start_depth), "");
		mdb->dump (n);
		if (n->get_table ())
		    dump_table (mdb, n->get_table (),
				depth + 2 + n->get_depth () - start_depth);
		n = n->get_next ();
	    }
	}
    }
}


/**
 * Dump part of mapping table that refers to object residing at a
 * particular address.
 * 
 * @param mdb		mapping database
 * @param table		mapping table
 * @param addr		physical address
 * @param depth		current recursion depth
 */
STATIC void kdb_t::dump_resource_table (mdb_t * mdb, mdb_table_t * table, word_t addr, word_t depth)
{
    while (table && table->match_prefix (addr))
    {
	printf ("%stable %p [objsize=%d%s  radix=%d  count=%d] (%p)\n",
		indent (depth - 1), table->get_prefix (),
		sz_num (table->objsize), sz_suf (table->objsize),
		1UL << table->radix, table->count, table);

	if (table->get_node (addr))
	    dump_resource_map (mdb, table->get_node (addr), addr, depth + 1);

	table = table->get_table (addr);
	depth++;
    }
}


/**
 * Dump a mapping tree that refers to object resifing at a particular
 * address,
 *
 * @param mdb		mapping database
 * @param node		mapping node
 * @param addr		physical address
 * @param depth		current recursion depth
 */
STATIC void kdb_t::dump_resource_map (mdb_t * mdb, mdb_node_t * node, word_t addr, word_t depth)
{
    word_t start_depth = node->get_depth ();
    while (node)
    {
	if (node->get_depth () > 0)
	{
	    printf ("%s", indent (depth - start_depth + node->get_depth ()-1));
	    mdb->dump (node);
	}
	if (node->get_table ())
	    dump_resource_table (mdb, node->get_table (), addr,
				 node->get_depth () - start_depth + depth + 1);
	node = node->get_next ();
    }
}
