/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     kdb/platform/ppc44x/bgtree.cc
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
#include <debug.h>
#include <lib.h>
#include <kdb/console.h>
#include "bgtree.h"

extern addr_t setup_console_mapping(paddr_t paddr, int log2size);

bgtree_t bgtree_t::tree;

typedef struct {
    word_t data[4];
} fpu_reg_t __attribute__((aligned(16)));

static inline void store_fr0(fpu_reg_t *fp)
{
    asm volatile("stfpdx 0,0,%0\n" : : "b"(fp));
}

static inline void load_fr0(fpu_reg_t *fp)
{
    asm volatile("lfpdx 0,0,%0\n" : : "b"(fp));
}

bool bgtree_t::channel_t::send(bgtree_header_t hdr, bglink_hdr_t &lnkhdr, void *payload)
{
    if (get_status().inj_hdr >= 8)
	return false;

    // XXX: fix stack alignment
    static fpu_reg_t fp0;
    store_fr0(&fp0);

    send_header(&hdr);
    send_payload_block((char*)&lnkhdr);
    for (int idx = 0; idx < 15; idx++) 
	send_payload_block((char*)payload + idx * 16);
    load_fr0(&fp0);
    return true;
}

bool bgtree_t::channel_t::poll(bglink_hdr_t *lnkhdr, void *payload)
{
    bgtree_header_t hdr;
    if (get_status().rcv_hdr == 0)
	return false;

    // XXX: fix stack alignment
    static fpu_reg_t fp0;
    store_fr0(&fp0);

    hdr = get_header();
    rcv_payload_block(lnkhdr);
    for (int i = 0; i < 15; i++)
	rcv_payload_block((char*)payload + i * 16);
    load_fr0(&fp0);
    return true;
}

bool bgtree_t::channel_t::init(int channel, paddr_t pbase, size_t size)
{
    base_phys = pbase;
    base = setup_console_mapping(base_phys, 12);
    return true;
}


bool bgtree_t::init(fdt_t *fdt)
{
    fdt_property_t *prop;

    fdt_node_t *node = fdt->find_subtree("/plb/tree");
    if (!node)
	return false;

    if (! (prop = fdt->find_property_node(node, "dcr-reg")) )
	return false;
    dcr_base = prop->get_word(0);

    if (! (prop = fdt->find_property_node(node, "nodeid")) )
	return false;
    node_id = prop->get_word(0);

    if (! (prop = fdt->find_property_node(node, "reg")) )
	return false;

    for (int chnidx = 0; chnidx < 2; chnidx++)
	channel[chnidx].init(chnidx, prop->get_u64(chnidx * 3), 
			     prop->get_word(chnidx * 3 + 2));

    /* disable send and receive IRQs */
    mtdcrx(dcr_base + 0x45, 0);
    mtdcrx(dcr_base + 0x49, 0);

    /* clear anything that may be pending */
    mfdcrx(dcr_base + 0x44);
    mfdcrx(dcr_base + 0x48);

    return true;
}
