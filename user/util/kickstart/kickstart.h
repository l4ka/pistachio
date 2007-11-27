/*********************************************************************
 *                
 * Copyright (C) 2004, 2007,  Karlsruhe University
 *                
 * File path:     kickstart.h
 * Description:   Global kickstart definitions
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
 * $Id: kickstart.h,v 1.2 2004/04/15 21:11:53 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KICKSTART__KICKSTART_H__
#define __KICKSTART__KICKSTART_H__

#include <l4/types.h>

#ifndef NULL
#define NULL	0
#endif


/**
 * A particular type of loader format (e.g., MBI compiant loader).
 */
class loader_format_t
{
public:

    /**
     * String describing current loader format.
     */
    const char * name;

    /**
     * Detect if a valid loader format of this particular type is present.
     * @returns true if format found, false otherwise
     */
    bool (*probe)(void);

    /**
     * Initialize everything according loader format.
     * @returns enrty point for kernel
     */
    L4_Word_t (*init)(void);
};

#define NULL_LOADER { "null", NULL, NULL }


/**
 * NULL terminated array of loader formats.
 */
extern loader_format_t loader_formats[];



// Prototypes for architecture-specific functions
void launch_kernel (L4_Word_t entry);
void flush_cache (void);
void fail (int ec);


// Helper functions
#define FAIL() do { fail(__LINE__); } while (0)

L4_INLINE L4_Word_t align_up (L4_Word_t addr, L4_Word_t size)
{
    return (addr + size - 1) & ~(size - 1);
}


#endif /* !__KICKSTART__KICKSTART_H__ */
