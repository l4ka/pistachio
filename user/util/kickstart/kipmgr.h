/****************************************************************************
 *
 * Copyright (C) 2002-2003, 2006, Karlsruhe University
 *
 * File path:	kipmgr.h
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
 * $Id: kipmgr.h,v 1.12 2006/10/20 21:45:17 reichelt Exp $
 *
 ***************************************************************************/
#ifndef __KICKSTART__KIP_H__
#define __KICKSTART__KIP_H__

#include <l4/types.h>
#if defined(KICKSTART_AMD64)
#include "kcp-amd64.h"
#else
#include <l4/kcp.h>
#include <l4/kip.h>
#endif

class kip_manager_t
{
public:
    enum mem_bootloader_e
    { 
	desc_undefined		= 0,
	desc_init_table		= 1,
	desc_init_server	= 2,
	desc_boot_module	= 3,
    };

protected:
    L4_KernelConfigurationPage_t *kip;

    L4_Word_t mem_desc_cnt;
    L4_Word_t mem_desc_offset;
    L4_Word_t word_size;

    void set_val (L4_Word_t idx, L4_Word64_t value)
	{
	    if (word_size == 4)
		((L4_Word32_t *) kip)[idx] = L4_Word32_t (value);
	    else if (word_size == 8)
		((L4_Word64_t *) kip)[idx] = L4_Word64_t (value);
	}

    L4_Word64_t get_val (L4_Word_t idx)
	{
	    if (word_size == 4)
		return (L4_Word64_t) (((L4_Word32_t *) kip)[idx]);
	    else if (word_size == 8)
		return (L4_Word64_t) (((L4_Word64_t *) kip)[idx]);
	    else
		return 0;
	}

public:
    kip_manager_t();

    bool find_kip (L4_Word_t kernel_start, L4_Word_t kernel_end);

    void install_sigma0 (L4_Word_t mod_start, L4_Word_t mod_end,
                         L4_Word_t entry, L4_Word_t type);
    void install_root_task (L4_Word_t mod_start, L4_Word_t mod_end,
			    L4_Word_t entry, L4_Word_t type);
    void update_kip (L4_Word_t boot_info);
    bool dedicate_memory  (L4_Word64_t start, L4_Word64_t end,
			   L4_Word64_t type, L4_Word64_t sub_type);

    bool is_mem_region_free (L4_Word_t start, L4_Word_t size);
    L4_Word64_t get_phys_mem_max (void);
    L4_Word_t get_min_pagesize (void);
};

#endif	/* __KICKSTART__KIP_H__ */
