/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/pal.cc
 * Description:   IA-64 PAL management
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
 * $Id: pal.cc,v 1.6 2003/09/24 19:05:27 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_ARCH(tlb.h)
#include INC_ARCH(trmap.h)
#include INC_ARCH(pal.h)


/**
 * ia64_pal_code: physical location of IA-64 PAL functions
 */
addr_t ia64_pal_code;


/**
 * ia64_pal_entry: physical location of IA-64 PAL entry point
 */
addr_t ia64_pal_entry;


void SECTION (".init")
init_pal (void)
{
    addr_t pal_code = phys_to_virt (ia64_pal_code);

    /*
     * Ensure that both PAL data and instruction is mapped.
     */
    
    if ((! dtrmap.is_mapped (pal_code)) || (! itrmap.is_mapped (pal_code)))
    {
	translation_t tr (1, translation_t::write_back, 1, 1, 0,
			  translation_t::rwx,
			  ia64_pal_code, 0);
    
	if (! dtrmap.is_mapped (pal_code))
	    dtrmap.add_map (tr, pal_code, HUGE_PGSIZE, 0);

	if (! itrmap.is_mapped (pal_code))
	    itrmap.add_map (tr, pal_code, HUGE_PGSIZE, 0);
    }
}
