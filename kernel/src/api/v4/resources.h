/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     api/v4/resources.h
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
 * $Id: resources.h,v 1.5 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__RESOURCES_H__
#define __API__V4__RESOURCES_H__

#include <bitmask.h>


class tcb_t;

class generic_thread_resources_t
{
public:
    void dump(tcb_t * tcb) { }
    void save(tcb_t * tcb) { }
    void load(tcb_t * tcb) { }
    void purge(tcb_t * tcb) { }
    void init(tcb_t * tcb) { }
    void free(tcb_t * tcb) { }
};

#include INC_GLUE(resources.h)


#if !defined(HAVE_RESOURCE_TYPE_E)
typedef word_t	resource_bits_t;
#else


/**
 * Abstract class for handling resource bit settings.
 */
class resource_bits_t
{
    bitmask_t<word_t>	resource_bits;

public:

    /**
     * Intialize resources (i.e., clear all resources).
     */
    inline void init (void)
	{ resource_bits.clear(); }

    /**
     * Clear all resources.
     */
    inline void clear (void)
	{ resource_bits.clear(); }

    /**
     * Add resource to resource bits.
     * @param t		type of resource
     * @return new resource bits
     */
    inline resource_bits_t operator += (resource_type_e t)
	{ 
	    resource_bits += (int) t;
	    return *this;
	}

    /**
     * Remove resource from resource bits.
     * @param t		type of resource
     * @return new resource bits
     */
    inline resource_bits_t operator -= (resource_type_e t)
	{
	    resource_bits -= (int) t;
	    return *this;
	}

    /**
     * Check if any resouces are registered.
     * @return true if any resources are registered, false otherwise
     */
    bool have_resources (void)
	{
	    return (word_t) resource_bits != 0;
	}

    /**
     * Check if indicated resource is registered.
     * @param t		type of resource
     * @return true if resource is registered, false otherwise
     */
    bool have_resource (resource_type_e t)
	{
	    return resource_bits.is_set ((int) t);
	}

    /**
     * Convert resource bits to a word (e.g., for printing).
     * @return the resource mask
     */
    inline operator word_t (void)
	{
	    return (word_t) resource_bits;
	}
};

#endif


#endif /* !__API__V4__RESOURCES_H__ */
