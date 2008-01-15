/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     generic/kmemory.cc
 * Description:   very simple kernel memory allocator
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
 * $Id: kmemory.cc,v 1.26 2007/01/22 21:06:59 skoglund Exp $
 *                
 ********************************************************************/
#include <kmemory.h>
#include <debug.h>
#include <init.h>
#include <kdb/tracepoints.h>
#include <sync.h>

//#define DEBUG_KMEM

#ifdef DEBUG_KMEM
# define ALLOC_TRACE	TRACE
# define FREE_TRACE	TRACE
#else
# define ALLOC_TRACE(x...)
# define FREE_TRACE(x...)
#endif

DECLARE_TRACEPOINT (KMEM_ALLOC);
DECLARE_TRACEPOINT (KMEM_FREE);

/* THE kernel memory allocator */
kmem_t kmem;

#if 0
#define KMEM_CHECK						\
{								\
    word_t num = 0;						\
    word_t * ptr = kmem_free_list;				\
    while(ptr)							\
    {								\
	ptr = (word_t*)(*ptr);					\
	num++;							\
    }								\
    if (num != free_chunks)					\
    {								\
	TRACEF("inconsistent kmem list: %d != %d (free_chunks)"	\
	       "\ncaller=%p\n",					\
	    num, free_chunks, __builtin_return_address(0));	\
	ptr = kmem_free_list;					\
	while(ptr) {						\
	    printf("%p -> ", ptr);				\
	    ptr = (word_t*)*ptr;				\
	}							\
	enter_kdebug("broken kmem");				\
    }								\
}
#else
#define KMEM_CHECK
#endif

SECTION(SEC_INIT) void kmem_t::init(void * start, void * end)
{
#define ISIZE ((word_t) end - (word_t) start)
    TRACE_INIT ("Initializing kernel memory (%p-%p) [%d%c]\n", start, end,
		ISIZE >= GB (1) ? ISIZE >> 30 :
		ISIZE >= MB (1) ? ISIZE >> 20 : ISIZE >> 10,
		ISIZE >= GB (1) ? 'G' : ISIZE >= MB (1) ? 'M' : 'K');

    /* initialize members */
    kmem_free_list = NULL;
    free_chunks = 0;
    
    /* do the real work */
    free(start, (word_t)end - (word_t)start);
#if 0
    kernel_info_page.reserved_mem0_low  = (word_t) text_paddr;
    kernel_info_page.reserved_mem0_high = (word_t) _end_text_p;
    kernel_info_page.reserved_mem1_low  = virt_to_phys((word_t) kmem_start);
    kernel_info_page.reserved_mem1_high = (virt_to_phys((word_t) kmem_end)  + (PAGE_SIZE-1)) & PAGE_MASK;
#endif
}

/* the stupid version */
void kmem_t::free(void * address, word_t size)
{
    word_t* p;
    word_t* prev, *curr;

    spinlock.lock();

    FREE_TRACE("kmem_free(%p, %x)\n", address, size);
    TRACEPOINT (KMEM_FREE,
		"kmem_free (%p, %d [%d%c]), ip: %p\n",
		address, size,
		size >= GB (1) ? size >> 30 :
		size >= MB (1) ? size >> 20 : size >> 10,
		size >= GB (1) ? 'G' : size >= MB (1) ? 'M' : 'K',
		__builtin_return_address (0));

    KMEM_CHECK;

    size = max(size, KMEM_CHUNKSIZE);
    ASSERT((size % KMEM_CHUNKSIZE) == 0);

    for (p = (word_t*)address;
	 p < ((word_t*)(((word_t)address) + size - KMEM_CHUNKSIZE));
	 p = (word_t*) *p)
	*p = (word_t) p + KMEM_CHUNKSIZE; /* write next pointer */
    
    /* find the place to insert */
    for (prev = (word_t*) (void *) &kmem_free_list, curr = kmem_free_list;
	 curr && (address > curr);
	 prev = curr, curr = (word_t*) *curr);
    /* and insert there */
    FREE_TRACE("prev %p/%p, curr %p, p: %p, \n", prev, *prev, curr, p); 
    *prev = (word_t) address; *p = (word_t) curr;

    /* update counter */
    free_chunks += (size/KMEM_CHUNKSIZE);
    FREE_TRACE("kmem: free chunks=%x\n", free_chunks);
    KMEM_CHECK;

    spinlock.unlock();
}


/* the stupid version */
void * kmem_t::alloc(word_t size)
{
    word_t*	prev;
    word_t*	curr;
    word_t*	tmp;
    word_t	i;

    spinlock.lock();
    
    ALLOC_TRACE("%s(%d) kfl: %p\n", __FUNCTION__, size, kmem_free_list);
    TRACEPOINT (KMEM_ALLOC, "kmem_alloc (%d [%d%c]), ip: %p\n",
		size, size >= GB (1) ? size >> 30 :
		size >= MB (1) ? size >> 20 : size >> 10,
		size >= GB (1) ? 'G' : size >= MB (1) ? 'M' : 'K',
		__builtin_return_address (0));
    KMEM_CHECK;
    
    size = max(size, KMEM_CHUNKSIZE);
    ASSERT((size % KMEM_CHUNKSIZE) == 0);

    for (prev = (word_t*) (void *) &kmem_free_list, curr = kmem_free_list;
	 curr;
	 prev = curr, curr = (word_t*) *curr)
    {
	ALLOC_TRACE("curr=%x\n", curr);
	/* properly aligned ??? */
	if (!((word_t) curr & (size - 1)))
	{
	    ALLOC_TRACE("%s(%d):%d: curr=%x\n", __FUNCTION__, 
			size, __LINE__, curr);

	    tmp = (word_t*) *curr;
	    for (i = 1; tmp && (i < (size / KMEM_CHUNKSIZE)); i++)
	    {
		ALLOC_TRACE("%s():%d: i=%d, tmp=%x\n", 
			    __FUNCTION__, __LINE__, i, tmp);

		if ((word_t) tmp != ((word_t) curr + KMEM_CHUNKSIZE*i))
		{
		    ALLOC_TRACE("skip: %x\n", curr);
		    tmp = 0;
		    break;
		};
		tmp = (word_t*) *tmp;
	    }
	    if (tmp)
	    {
		/* dequeue */
		*prev = (word_t) tmp;

		/* zero the page */
		for (word_t i = 0; i < (size / sizeof(word_t)); i++)
		    curr[i] = 0;

		/* update counter */
		free_chunks -= (size/KMEM_CHUNKSIZE);

		/* successful return */
		ALLOC_TRACE("kmalloc: %x->%p (%d), kfl: %p, caller: %p\n", 
			    size, curr, free_chunks, kmem_free_list, __builtin_return_address(0));
		KMEM_CHECK;

#if 0
		TRACEPOINT (KMEM_ALLOC, ("kmem_alloc (%d bytes), ip=%x "
					    "==> %p\n",
					    size, __builtin_return_address(0),
					    curr),
			       printf ("kmem_alloc (%d [%d%c]), ip: %p "
				       "==> %p\n",
				       size, size >= GB (1) ? size >> 30 :
				       size >= MB (1) ? size >> 20 :
				       size >> 10, size >= GB (1) ? 'G' :
				       size >= MB (1) ? 'M' : 'K',
				       __builtin_return_address (0), curr));

#endif

		spinlock.unlock();

		return curr;
	    }
	}
    }
#if 0
    word_t * tmp1 = kmem_free_list;
    while(tmp1) {
	printf("%p -> ", tmp1);
	tmp1 = (word_t*)*tmp1;
    }
#endif
    enter_kdebug("kmem_alloc: out of kernel memory");

    spinlock.unlock();
    return NULL;
}

#define ALIGN(x)    (x & mask)

/* the stupid aligned version */
void * kmem_t::alloc_aligned(word_t size, word_t alignment, word_t mask)
{
    word_t*	prev;
    word_t*	curr;
    word_t*	tmp;
    word_t	i;

    word_t	align = ALIGN(alignment);

    spinlock.lock();
    
    ALLOC_TRACE("%s(%d) kfl: %p\n", __FUNCTION__, size, kmem_free_list);
    TRACEPOINT (KMEM_ALLOC, "kmem_alloc (%d), ip: %p\n", size, __builtin_return_address (0));
    KMEM_CHECK;
    
    size = max(size, KMEM_CHUNKSIZE);
    ASSERT((size % KMEM_CHUNKSIZE) == 0);

    for (prev = (word_t*) (void *) &kmem_free_list, curr = kmem_free_list;
	 curr;
	 prev = curr, curr = (word_t*) *curr)
    {
	ALLOC_TRACE("curr=%x\n", curr);
	/* properly aligned ??? */
	if ((ALIGN((word_t)curr) == align) && (!((word_t) curr & ((size)- 1))))
	{
	    ALLOC_TRACE("%s(%d):%d: curr=%x\n", __FUNCTION__, 
			size, __LINE__, curr);

	    tmp = (word_t*) *curr;
	    for (i = 1; tmp && (i < (size / KMEM_CHUNKSIZE)); i++)
	    {
		ALLOC_TRACE("%s():%d: i=%d, tmp=%x\n", 
			    __FUNCTION__, __LINE__, i, tmp);

		if ((word_t) tmp != ((word_t) curr + KMEM_CHUNKSIZE*i))
		{
		    ALLOC_TRACE("skip: %x\n", curr);
		    tmp = 0;
		    break;
		};
		tmp = (word_t*) *tmp;
	    }
	    if (tmp)
	    {
		/* dequeue */
		*prev = (word_t) tmp;

		/* zero the page */
		for (word_t i = 0; i < (size / sizeof(word_t)); i++)
		    curr[i] = 0;

		/* update counter */
		free_chunks -= (size/KMEM_CHUNKSIZE);

		/* successful return */
		ALLOC_TRACE("kmalloc: %x->%p (%d), kfl: %p, caller: %p\n", 
			    size, curr, free_chunks, kmem_free_list, __builtin_return_address(0));
		KMEM_CHECK;

#if 0
		TRACEPOINT (KMEM_ALLOC, ("kmem_alloc (%d bytes), ip=%x "
					    "==> %p\n",
					    size, __builtin_return_address(0),
					    curr));
#endif

		spinlock.unlock();

		return curr;
	    }
	}
    }
#if 0
    word_t * tmp1 = kmem_free_list;
    while(tmp1) {
	printf("%p -> ", tmp1);
	tmp1 = (word_t*)*tmp1;
    }
#endif
    enter_kdebug("kmem_alloc: out of kernel memory");

    spinlock.unlock();
    return NULL;
}
