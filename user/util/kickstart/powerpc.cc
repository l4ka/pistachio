/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     powerpc.cc
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
#include <config.h>
#include <l4io.h>

#include "kickstart.h"
#include "fdt.h"

bool initialize_console(fdt_t *fdt);

/*
 * Loader formats supported for PowerPC
 */
loader_format_t loader_formats[] = {
    { "Flattened device tree", fdt_probe, fdt_init },
    NULL_LOADER
};


void fail(int ec)
{
    printf("PANIC: FAIL in line %d\n", ec);
    while(1);
}

void flush_dcache_range(L4_Word_t start, L4_Word_t end)
{
    printf("invalidate dcache %x-%x\n", start, end);
    for (; start < end; start += 32)
	asm("dcbf 0, %0" : : "b"(start));
}

void flush_cache()
{
    /* Should we flush the cache??? */
    flush_dcache_range((L4_Word_t)get_fdt_ptr(), 
		       ((L4_Word_t)get_fdt_ptr()) + get_fdt_ptr()->size);
}

static fdt_t *fdt_ptr;
fdt_t *get_fdt_ptr()
{
    return fdt_ptr;
}

extern void (*entry_secondary)(void);

void launch_kernel(L4_Word_t entry)
{
    void (*kernel)(void) = (void(*)(void))entry;

    entry_secondary = kernel; /* release APs */
    asm("msync; dcbi 0, %0" : : "b"(&entry_secondary));

    (*kernel)();
}

extern "C" void loader();
extern "C" void __loader(L4_Word_t r3, L4_Word_t r4, L4_Word_t r5, 
			 L4_Word_t r6, L4_Word_t r7)
{
    fdt_ptr = (fdt_t*)r3;

    if (!initialize_console(fdt_ptr))
	while(1);

    loader();
}
