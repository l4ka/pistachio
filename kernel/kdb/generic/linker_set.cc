/*********************************************************************
 *                
 * Copyright (C) 2002, 2006,  Karlsruhe University
 *                
 * File path:     kdb/generic/linker_set.cc
 * Description:   Implementation of generic link time sets
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
 * $Id: linker_set.cc,v 1.7 2006/12/05 15:23:15 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <init.h>
#include <kdb/linker_set.h>


/*
 * Declared in linker script.
 */

extern word_t _start_setlist;
extern word_t _end_setlist;
extern word_t _start_sets;
extern word_t _end_sets;


/**
 * init_sets: Initialize all sets in the system.
 */
void init_sets (void) SECTION (SEC_INIT);
void init_sets (void)
{
    //linker_set_t * list_beg = (linker_set_t *) &_start_setlist;
    //linker_set_t * list_end = (linker_set_t *) &_end_setlist;
    linker_set_entry_t * sets_beg = (linker_set_entry_t *)
	(void *) &_start_sets;
    linker_set_entry_t * sets_end = (linker_set_entry_t *)
	(void *) &_end_sets;
    linker_set_entry_t * ent;

    /*
     * PS: Printf() may not yet be working!!!
     */

    /*
     * Parse through all set entries to initialze the sets.  Make sure
     * that all sets contain contigous arrays of set entries.
     */

    for (ent = sets_beg; ent < sets_end; ent++)
    {
	if (ent->set->entries++ == 0)
	{
	    /*
	     * Set is empty.  Initialize it.
	     */
	    ent->set->list = ent;
	    ent->set->curidx = 0;
	}
	else
	{
	    /*
	     * Move entry upwards until we concatenate it with the
	     * previous set entries.
	     */

	    linker_set_entry_t * m = ent;
	    while (m->set != m[-1].set)
	    {
		linker_set_entry_t tmp = m[-1];
		m[-1] = m[0];
		m[0] = tmp;
		if (m->set->list == m-1)
		    /* Update set's list pointer if needed. */
		    m->set->list = m;
		m--;
	    }
	}
    }
}


void linker_set_t::print (void)
{
    printf ("Set = %p,  Entries = %d\n", this, entries);
    for (word_t i = 0; i < entries; i++)
	printf ("%3d = %p\n", i, list[i].get_entry ());
}



/**
 * linker_set_t::reset: Reset linker set iterator.
 */
void linker_set_t::reset (void)
{
    curidx = 0;
}


/**
 * linker_set_t::next: Retrieves pointer to next entry in linker set,
 * or NULL if all entries have been iterated over.
 */
addr_t linker_set_t::next (void)
{
    if (curidx >= entries)
	return (addr_t) 0;
    return list[curidx++].get_entry ();
}


/**
 * linker_set_t::size: Returns size (number of entries) of linker set.
 */
word_t linker_set_t::size (void)
{
    return entries;
}


/**
 * linker_set_t::get: Returns indicated linker set entry.
 */
addr_t linker_set_t::get (word_t n)
{
    if (n >= entries)
	return NULL;
    return list[n].get_entry ();
}
