/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     generic/asid.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __ASID_H__
#define __ASID_H__


#include <debug.h>
#include <config.h>      /* CONFIG_MAX_NUM_ASIDS, CONFIG_PREEMPT_ASIDS */
#include INC_API(types.h)

class asid_t
{
public:
    enum {
	invalid = ~0U,
    };

    void init()
	{ asid = invalid; }

    void init_kernel(word_t kernel_asid)
	{ asid = kernel_asid; }

    word_t get()
	{ return this->asid; }
    
    void set(word_t asid)
	{ this->asid = asid; }

    bool is_valid()
	{ return asid != invalid; }

    void release()
	{ asid = invalid; }

    word_t asid;
    word_t timestamp;
};

/*
 * We use the ASID ref array for (1) referencing the address space
 * which uses the ASID or (2) when free to link unused ASIDs in a
 * list.  When reclaiming a used ASID we can assume that no entry is
 * in the free list which means we don't record if an entry is used or
 * not.
 */

template <class T, int SIZE>
class asid_manager_t
{
public:

    void init(word_t start, word_t end)
	{
	    free_list = NULL;
	    timestamp = 0;

	    for (word_t asid = 0; asid <= SIZE; asid++)
		asid_user[asid] = NULL;

	    for (word_t asid = start; asid <= end; asid++)
		free_asid(asid);
	}

    void free_asid(word_t asid)
	{
	    list_entry[asid] = free_list;
            word_t **fl = &list_entry[asid];                
            free_list = (word_t *) fl;
	}

    void allocate_asid(T* space)
	{
	    if (EXPECT_FALSE(!free_list))
		recycle_asid();
	    ASSERT(free_list);
	    word_t *head = free_list;
	    free_list = (word_t*)*head;
	    word_t asid = ((word_t)head - (word_t)&list_entry) / sizeof(word_t);
	    space->allocate_hw_asid(asid);
	    space->get_asid()->set(asid);
	}

    void recycle_asid()
	{
	    word_t oldest;
	    for (word_t idx = oldest = start; idx < end; idx++)
		if (asid_user[idx]->get_asid()->timestamp < 
		    asid_user[oldest]->get_asid()->timestamp)
		    oldest = idx;
	    printf("recycling ASID %x used by %p\n", oldest, asid_user[oldest]);
	    asid_user[oldest]->release_hw_asid(oldest);
	    asid_user[oldest]->get_asid()->release();
	    free_asid(oldest);
	}

    word_t reference(asid_t *asid)
	{
	    asid->timestamp = ++timestamp;
	    return asid->asid;
	}

private:
    word_t *free_list;
    union {
	T* asid_user[SIZE];
	word_t *list_entry[SIZE];
    };

    word_t start, end;
    word_t timestamp;
};

#endif /* !__ASID_H__ */
