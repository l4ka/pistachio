/*********************************************************************
 *                
 * Copyright (C) 2003-2007,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-x86/x32/space.cc
 * Description:   Various space management stuff
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
 * $Id: space.cc,v 1.8 2006/10/19 22:57:37 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <linear_ptab.h>


void get_ptab_dump_ranges (addr_t * vaddr, word_t * num,
			   pgent_t::pgsize_e *max_size)
{
    const pgent_t::pgsize_e max = pgent_t::size_max;

    *vaddr = (addr_t) 0;
    *num = page_table_size (max);
    *max_size = max;

    switch (get_choice ("Memory area", "Complete/User/Kernel/Tcb/Manual"
#if defined(CONFIG_X86_SMALL_SPACES)
			"/Small spaces"
#endif
#if defined(CONFIG_X86_IO_FLEXPAGES)
			"/tSsm"
#endif
			, 'c'))
    {
    case 'c':
	break;
    case 'u':
	*num = page_table_index (max, (addr_t) USER_AREA_END);
	break;
    case 'k':
	*vaddr = (addr_t) KERNEL_AREA_START;
	*num = page_table_index (max, (addr_t) KERNEL_AREA_END) - 
	    page_table_index (max, (addr_t) KERNEL_AREA_START);
	break;
    case 't':
	*vaddr = (addr_t) KTCB_AREA_START;
	*num = page_table_index (max, (addr_t) KTCB_AREA_SIZE);
	break;
    case 'm':	
    {
	*vaddr =   (addr_t) get_hex ("start address", 0UL, "00000000");
	addr_t end_vaddr =   (addr_t) get_hex ("end address", 0UL, "00000000");
	*num = page_table_index (max, (addr_t) ((word_t) end_vaddr - (word_t) *vaddr));
	break;
    }
#if defined(CONFIG_X86_SMALL_SPACES)
    case 's':
	*vaddr = (addr_t) SMALLSPACE_AREA_START;
	*num = page_table_index (max, (addr_t) SMALLSPACE_AREA_SIZE);
	break;
#endif
    }
}
