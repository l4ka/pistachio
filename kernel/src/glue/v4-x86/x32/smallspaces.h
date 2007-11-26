/*********************************************************************
 *                
 * Copyright (C) 2003, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/smallspaces.h
 * Description:   Small space id handling
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
 * $Id: smallspaces.h,v 1.4 2003/09/24 19:04:36 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X32__SMALLSPACES_H__
#define __GLUE_V4_X86__X32__SMALLSPACES_H__

#include INC_GLUE(config.h)

class space_t;

class smallspace_id_t
{
    union {
	u32_t	raw;
	u8_t	id;
    };

public:

    /**
     * Check whether small space id indicates a small space or not.
     * @return true if space id indicates small space, false otherwise
     */
    bool is_small (void)
	{
	    return id != 0;
	}

    /**
     * Set small space id to indicate large address space.
     */
    void set_large (void)
	{
	    raw = 0;
	}

    /**
     * Set small space id to indicate small address space.
     * @param idx	index into small space area (4MB stepping)
     * @param size	size of small space in megabytes
     */
    void set_small (word_t idx, word_t size)
	{
	    id = ((idx & ~(size - 1)) >> 1) | (size >> 2);
	}

    /**
     * Get size of small space (in bytes).
     * @return size of small space (in bytes)
     */
    word_t size (void)
	{
	    if (id == 0)
		return 0;

	    word_t size = (1UL << 22);
	    for (word_t mask = 1; (id & mask) == 0; mask <<= 1, size <<= 1)
		;
	    return size;
	}

    /**
     * Get offset of small space within small space area (in bytes).
     * @return offset of small space (in bytes)
     */
    word_t offset (void)
	{
	    if (id == 0)
		return 0;

	    word_t mask = 1;
	    for (; (id & mask) == 0; mask <<= 1)
		;
	    return (id & ~mask) << 21;
	}

    void set_raw (word_t r) { raw = r; }
    word_t get_raw (void) { return raw; }
};


bool is_smallspace(space_t *s);


#endif /* !__GLUE_V4_X86__X32__SMALLSPACES_H__ */
