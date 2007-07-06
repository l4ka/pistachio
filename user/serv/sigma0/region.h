/*********************************************************************
 *                
 * Copyright (C) 2005,  Karlsruhe University
 *                
 * File path:     region.h
 * Description:   Generic regions
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
 * $Id: region.h,v 1.3 2005/06/02 14:11:08 joshua Exp $
 *                
 ********************************************************************/
#ifndef __REGION_H__
#define __REGION_H__

/**
 * Descriptor for a memory region.  A memory region has a start
 * address, an end address, and an owner.
 */
class region_t
{
public:

    L4_Word_t		low;
    L4_Word_t		high;
    L4_ThreadId_t	owner;

    region_t		*prev;
    region_t		*next;

    region_t (void) {}
    region_t (L4_Word_t low, L4_Word_t high, L4_ThreadId_t owner);

    void * operator new (L4_Size_t size);

    void swap (region_t * n);
    void remove (void);
    bool is_adjacent (const region_t & r);
    bool concatenate (region_t * reg);

    bool can_allocate (L4_Word_t addr, L4_Word_t log2size,
		       L4_ThreadId_t tid);
    L4_Fpage_t allocate (L4_Word_t addr, L4_Word_t log2size, L4_ThreadId_t tid,
			 L4_Fpage_t (*make_fpage) (L4_Word_t, int));
    L4_Fpage_t allocate (L4_Word_t log2size, L4_ThreadId_t tid,
			 L4_Fpage_t (*make_fpage) (L4_Word_t, int));
} __attribute__ ((packed));


/**
 * Opaque class for holding a region_t structure.
 */
class region_listent_t
{
    region_t	reg;

public:

    region_t * region (void)
	{ return &reg; }

    region_listent_t * next (void)
	{ return *(region_listent_t **) this; }

    void set_next (region_listent_t * n)
	{ *(region_listent_t **) this = n; }
} __attribute__ ((packed));


/**
 * List of memory regions.
 */
class region_list_t
{
    region_listent_t * list;

public:

    void add (L4_Word_t addr, L4_Word_t size);
    L4_Word_t contents (void);
    region_t * alloc (void);
    void free (region_t * r);
};


/**
 * A region pool is set of memory regions.
 */
class region_pool_t
{
    region_t first;
    region_t last;
    region_t * ptr;

public:

    void init (void);
    void dump (void);
    void insert (region_t * r);
    void remove (region_t * r);
    void insert (L4_Word_t low, L4_Word_t high, L4_ThreadId_t owner);
    void remove (L4_Word_t low, L4_Word_t high);
    void reset (void);
    region_t * next (void);
};


/**
 * List of free region_t structures.
 */
extern region_list_t region_list;


#endif /* !__REGION_H__ */
