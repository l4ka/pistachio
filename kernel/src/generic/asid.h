/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     asid.h
 * Created:       01/08/2002 17:01:52 by Simon Winwood (sjw)
 * Description:   Generic ASID management 
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
 * $Id: asid.h,v 1.5 2005/02/09 11:52:08 ud3 Exp $
 *                
 ********************************************************************/

#ifndef __ASID_H__
#define __ASID_H__

/* WARNING!!! This file is _not_ SMP-safe */

/* This file contains generic routines for ASID management.  Hardware usually
 * supports a limited number of ASIDs (21264 has ~ 256), and the number of address
 * spaces may be greater than this.  The implication of this is that an ASID may need 
 * to be used by a number of address spaces (only one space at a time), and hence 
 * need to be managed like any other resource 
 *
 * This means that ASIDs may be pre-empted and managed in a LRU manner (or equivelent).
 */

#include <debug.h>
#include <config.h>      /* CONFIG_MAX_NUM_ASIDS, CONFIG_PREEMPT_ASIDS */
#include INC_API(types.h)

typedef s32_t hw_asid_t;

class asid_t 
{
 public:
    enum valid_e {
	invalid = -1
    };

    void init(void) {
	asid = invalid;
    }

    /* For architectures that have a seperate kernel address space. */
    void init_kernel(hw_asid_t kernel_asid) {
        asid = kernel_asid;
    }

    bool is_valid(void) {
	return asid != invalid;
    }

    /* This will allocate, reference, and return the asid */
    hw_asid_t get(void);
    
    void release(void);

    /* Methods asid_cache_t requires */
    /* This just returns the value */
    hw_asid_t value(void) {
	return asid;
    }

    void preempt(void)
    {
	asid = invalid;
    }

 private:
    hw_asid_t asid;
};

/* This implementation is pretty straight forward and could (should) be optimised */
class asid_cache_t 
{
 public:
    /**
     * Initialise ASID cache
     *
     * This method initialises the ASID cache.  Note that this should
     * be called before set_valid.
     **/    
    void init(void) {
	first_free = lru_head = -1;

	/* Initialise all asids to invalid */
	for(int i = 0; i < CONFIG_MAX_NUM_ASIDS; i++) {
	    asids[i].prev = asids[i].next = -1;
	    asids[i].asid = (asid_t *) -1;
	} 
    }

    /**
     * Sets a valid range of ASIDs
     * @param start     The first valid asid (inclusive)
     * @param end       The last valid asid (inclusive)
     *
     * This method sets a range of ASIDs to be valid (allocatable).
     * Note that the parameters are inclusive.  Multiple invocations
     * allowed.
     **/    
    void set_valid(hw_asid_t start, hw_asid_t end);
    
    /**
     * Used to reference ASIDs to enable efficient pre-emption
     * @param asid      The ASID to reference.
     *
     * This method manages the LRU list (if any) of ASIDS to enable
     * more efficient target selection during ASID pre-emption.
     *
     **/    
    void reference(asid_t *asid);

    /**
     * Allocates an ASID
     * @param asid      The destination ASID (used during preemption).
     *
     *
     **/    
    hw_asid_t allocate(asid_t *asid);

    void release(asid_t *asid);

    asid_t *lookup(hw_asid_t hw_asid) {
	return asids[hw_asid].asid;
    }
 private:
    void move_asid(hw_asid_t idx, bool remove, bool insert);

    struct asid_link_t {
	asid_t *asid;
	hw_asid_t prev, next; /* index of previous and next asid */
    } asids[CONFIG_MAX_NUM_ASIDS];

    hw_asid_t first_free;
    hw_asid_t lru_head;
};

INLINE asid_cache_t *get_asid_cache(void)
{
    extern asid_cache_t asid_cache;
    return &asid_cache;
}

INLINE hw_asid_t asid_t::get(void) 
{
    if(EXPECT_FALSE(!is_valid()))
	asid = get_asid_cache()->allocate(this);
    else 
	get_asid_cache()->reference(this);
    
    return asid;
}
    
INLINE void asid_t::release(void) 
{
    get_asid_cache()->release(this);
    asid = invalid;
}

INLINE void asid_cache_t::set_valid(hw_asid_t start, hw_asid_t end)
{
    hw_asid_t *prev, idx, old_head;
    old_head = first_free;
    prev = &first_free;

    for(idx = start; idx <= end; idx++) {
	if(asids[idx].asid != (asid_t *) -1)
	    continue;
	    
	*prev = idx;
	asids[idx].asid = 0;
	prev = &asids[idx].next;
    }
    
    *prev = old_head;
}

INLINE hw_asid_t asid_cache_t::allocate(asid_t *asid)
{
    ASSERT(!asid->is_valid());
    hw_asid_t idx = asid_t::invalid;

    if(first_free == -1) {
#ifdef CONFIG_PREEMPT_ASIDS 
	idx = asids[lru_head].prev;
	move_asid(idx, true, false);
	asids[idx].asid->preempt();

#else
	enter_kdebug("Exhausted ASIDs");
#endif /* CONFIG_PREEMPT_ASIDS */
    } else {
	idx = first_free;
	first_free = asids[first_free].next;
    }
    
    move_asid(idx, false, true);
    asids[idx].asid = asid;
    return idx;
}

INLINE void asid_cache_t::release(asid_t *asid)
{
    if(!asid->is_valid()) {
	return;
    }

    hw_asid_t idx = asid->value();

    move_asid(idx, true, false);

    asids[idx].asid = 0;
    asids[idx].next = first_free;

    first_free = idx;
}

INLINE void asid_cache_t::reference(asid_t *asid)
{
    ASSERT(asid->is_valid());
    move_asid(asid->value(), true, true);
}

/**
 * Moves an ASID to the top of the LRU list
 * @param idx           The index of the ASID to be moved
 * @param inserted      Whether the ASID is in the LRU list (and therefore whether it needs to be removed first)
 *
 * This function manages the ASID LRU list.  It also encapsulates all
 * the LRU complexity, and hence allows for easy (and neat)
 * conditional compilation.
 **/
INLINE void asid_cache_t::move_asid(hw_asid_t idx, bool remove, bool insert)
{
#ifdef CONFIG_PREEMPT_ASIDS
    hw_asid_t prev, next;
    if(remove) {
	/* remove from list */
	prev = asids[idx].prev;
	next = asids[idx].next;

	/* sjw (02/08/2002): This may happen if a space deletes itself --- impossible? */
	if(lru_head == idx) 
	    lru_head = asids[idx].next;

	if(prev != next) {
	    asids[prev].next = next;
	    asids[next].prev = prev;
	} else {
	    lru_head = -1;
	}
    }

    if(insert) {
	if(lru_head != -1) {
	    prev = asids[lru_head].prev;
	    next = lru_head;
	} else {
	    /* sjw (02/08/2002): Slightly inefficient ... */
	    next = prev = idx;
	}

	asids[next].prev = idx;
	asids[prev].next = idx;
	asids[idx].prev = prev;
	asids[idx].next = next;
	
	lru_head = idx;
    }
#endif /* CONFIG_PREEMPT_ASIDS */
}

#endif /* !__ASID_H__ */
