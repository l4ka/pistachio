/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     kdb/platform/ppc44x/bgtree.h
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
#ifndef __PLATFORM__PPC44X__BGTREE_H__
#define __PLATFORM__PPC44X__BGTREE_H__

#include INC_ARCH(ibm450.h)
#include INC_ARCH(io.h)
#include INC_PLAT(fdt.h)

// tree ifc memory offsets
#define BGP_TRx_DI		(0x00U)
#define BGP_TRx_HI		(0x10U)
#define BGP_TRx_DR		(0x20U)
#define BGP_TRx_HR		(0x30U)
#define BGP_TRx_Sx		(0x40U)

#define BGP_NUM_CHANNEL		2

/* hardware header */
struct bgtree_header_t
{
    union {
	word_t raw;
	struct {
	    word_t pclass	: 4;
	    word_t p2p		: 1;
	    word_t irq		: 1;
	    word_t vector	: 24;
	    word_t csum_mode	: 2;
	} p2p;
	struct {
	    word_t pclass	: 4;
	    word_t p2p		: 1;
	    word_t irq		: 1;
	    word_t op		: 3;
	    word_t opsize	: 7;
	    word_t tag		: 14;
	    word_t csum_mode	: 2;
	} bcast;
    };

    void set_p2p(word_t pclass, word_t vector, bool irq = false)
	{
	    raw = 0;
	    p2p.pclass = pclass;
	    p2p.irq = irq;
	    p2p.vector = vector;
	    p2p.p2p = 1;
	}

    void set_broadcast(word_t pclass, word_t tag = 0, bool irq = false)
	{
	    raw = 0;
	    bcast.pclass = pclass;
	    bcast.irq = irq;
	    bcast.tag = tag;
	}
} __attribute__((packed));


struct bgtree_status_t 
{
    union {
	word_t raw;
	struct {
	    word_t inj_pkt	: 4;
	    word_t inj_qwords	: 4;
	    word_t		: 4;      
	    word_t inj_hdr	: 4;
	    word_t rcv_pkt	: 4;
	    word_t rcv_qwords	: 4;
	    word_t		: 3;
	    word_t irq		: 1;
	    word_t rcv_hdr	: 4;
	};
    };
} __attribute__((packed));

/* link layer */
struct bglink_hdr_t
{
    word_t dst_key; 
    word_t src_key; 
    u16_t conn_id; 
    u8_t this_pkt; 
    u8_t total_pkt;
    u16_t lnk_proto;	// 1 eth, 2 con, 3...
    u16_t optional;	// for encapsulated protocol use
} __attribute__((packed));



class bgtree_t
{
public:
    static void fpu_memcpy_16(void *dst, void *src)
	{
	    asm volatile("lfpdx 0,0,%0\n"
			 "stfpdx 0,0,%1\n"
			 :
			 : "b"(src), "b"(dst)
			 : "fr0", "memory");
	}

    static void in128(addr_t reg, void *ptr)
	{ fpu_memcpy_16(ptr, reg); }

    static void out128(addr_t reg, void *ptr)
	{ fpu_memcpy_16(reg, ptr); }

    // device registers
    struct channel_t 
    {
    public:
	addr_t base;		// virtual base address of tree
	paddr_t base_phys;	// phys location
	
	void send_header(bgtree_header_t *hdr)
	    { out_be32(addr_offset(base, BGP_TRx_HI), hdr->raw); }

	void send_payload_block(void *payload)
	    { out128(addr_offset(base, BGP_TRx_DI), payload); }

	void rcv_payload_block(void *payload)
	    { in128(addr_offset(base, BGP_TRx_DR), payload); }

	bgtree_header_t get_header()
	    { 
		bgtree_header_t hdr;
		hdr.raw = in_be32(addr_offset(base, BGP_TRx_HR)); 
		return hdr;
	    }
	
	bgtree_status_t get_status()
	    { 
		bgtree_status_t status;
		status.raw = in_be32(addr_offset(base, BGP_TRx_Sx)); 
		return status;
	    }

	bool init(int channel, paddr_t pbase, size_t size);
	bool send(bgtree_header_t hdr, bglink_hdr_t &lnkhdr, void *payload);
	bool poll(bglink_hdr_t *lnkhdr, void *payload);
    };

    channel_t channel[BGP_NUM_CHANNEL];

    word_t dcr_base;
    word_t curr_conn;
    word_t node_id;	// self

public:
    bool init(fdt_t *fdt);
    void init_link_hdr(bglink_hdr_t *hdr) 
	{
	    hdr->src_key = this->node_id;
	    hdr->conn_id = this->curr_conn++;
	}
	    
	

    /* global instanciation */
    static bgtree_t tree;
    static bgtree_t* get_device(fdt_t *fdt, word_t handle) 
	{ return &tree; }
};

#endif /* !__PLATFORM__PPC44X__BGTREE_H__ */
