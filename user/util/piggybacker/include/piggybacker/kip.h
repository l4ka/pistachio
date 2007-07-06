/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/kip.h
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
 * $Id: kip.h,v 1.3 2003/10/27 00:26:15 cvansch Exp $
 *
 ***************************************************************************/
#ifndef __PIGGYBACKER__INCLUDE__KIP_H__
#define __PIGGYBACKER__INCLUDE__KIP_H__

#include <l4/types.h>
#include <l4/kcp.h>
#include <l4/kip.h>

class kip_server_t
{
public:
    L4_Word_t ip;
    L4_Word_t start;
    L4_Word_t end;

    void clear() { this->ip = this->start = this->end = 0; }
};

class kip_manager_t
{
protected:
    L4_KernelConfigurationPage_t *kip_src;
    L4_KernelConfigurationPage_t *kip_dst;

    void install_module( L4_Word_t mod_start, L4_Word_t mod_end, kip_server_t *server );

    enum server_e {
	sigma0 = 0,
	root_task,
	kernel,
	tot,
    };
    kip_server_t servers[tot];

    L4_Word_t boot_info;
    L4_Word_t mem_desc_cnt;

public:
    bool find_kip( L4_Word_t kernel_start );
    void install_sigma0( L4_Word_t mod_start, L4_Word_t mod_end );
    void install_root_task( L4_Word_t mod_start, L4_Word_t mod_end );
    void install_kernel( L4_Word_t mod_start, L4_Word_t mod_end );
    L4_Word_t first_avail_page();

    bool virt_to_phys( L4_Word_t virt, L4_Word_t elf_start, L4_Word_t *phys );

    void update_kip();
    void set_boot_info( L4_Word_t val ) { this->boot_info = val; }
    void setup_main_memory( L4_Word_t start, L4_Word_t end );
    void dedicate_memory( L4_Word_t start, L4_Word_t end, L4_Word_t type, L4_Word_t sub_type );

    void init();
};

#endif	/* __PIGGYBACKER__INCLUDE__KIP_H__ */
