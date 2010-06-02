/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     kdb/platform/ppc44x/bluegene.cc
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
#include <sync.h>
#include INC_ARCH(cache.h)
#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(ibm450.h)
#include INC_PLAT(fdt.h)

extern addr_t setup_console_mapping(paddr_t paddr, int log2size);

#ifdef CONFIG_KDB_CONS_BGP_JTAG

class bgp_mailbox_t 
{
public:
    volatile unsigned short command;	// comand; upper bit=ack
    unsigned short len;			// length (does not include header)
    unsigned short result;		// return code from reader
    unsigned short crc;			// 0=no CRC
    char data[0];

public:
    enum mb_commands_e {
	cmd_print = 2,
    };

};

typedef struct jtag_console_t 
{
    u64_t mb_phys;
    bgp_mailbox_t *mb;
    word_t size;
    word_t dcr_set;
    word_t dcr_clear;
    word_t dcr_mask;

    void send_command(int command)
	{ 
	    mb->command = command;
	    asm volatile("sync");
	    ppc_set_dcr(dcr_set, dcr_mask);
	    
	    do {
		ppc_cache_invalidate_block((word_t)&mb->command);
	    } while(!(mb->command & 0x8000));
	}

    void putc(char c)
	{
	    if (!mb) return;

	    mb->data[mb->len++] = c;
	
	    if (mb->len >= size || c == '\n')
	    {
		send_command(bgp_mailbox_t::cmd_print);
		mb->len = 0;
	    }
	}

    bool init(fdt_t *fdt)
	{
	    /* initialize only once */
	    if (mb)
		return true;
	    
	    fdt_property_t *prop;
	    fdt_node_t *node = fdt->find_subtree("/jtag/console0");

	    if (! (prop = fdt->find_property_node(node, "reg")) )
		return false;

	    size = prop->get_word(2);
	    mb_phys = prop->get_u64(0);
#warning fix uboot
	    mb_phys |= 0x700000000ULL;

	    if (! (prop = fdt->find_property_node(node, "dcr-reg")) )
		return false;

	    dcr_set = prop->get_word(0);
	    dcr_clear = prop->get_word(1);
	    
	    if (! (prop = fdt->find_property_node(node, "dcr-mask")) )
		return false;
	    
	    dcr_mask = prop->get_word(0);

	    mb = (bgp_mailbox_t*)setup_console_mapping(mb_phys, 14);
	    return true;
	}
};

void init_bgtree();
static jtag_console_t cons;

static void init_jtag()
{
    cons.init(get_fdt());
    init_bgtree();
}

const char kbd_ret[] = "1q2q";
static char getc_jtag(bool block) 
{
    static int cnt = 0;

    if (cnt < 1)
	return kbd_ret[cnt++];

    if (block)
	while(1);

    return 0; 
}

static void putc_jtag(char c) 
{
    cons.putc(c);
}
#endif

#if defined(CONFIG_KDB_CONS_BGP_TREE)

#include "bgtree.h"

#define BUF_SIZE		240

class tree_console_t {
private:
    // console buffers...
    char out_buf[BUF_SIZE] __attribute__((aligned(16)));
    char in_buf[BUF_SIZE] __attribute__((aligned(16)));
    int out_len;
    int in_len;
    int in_head;

    word_t send_id;	// console send id
    word_t rcv_id;	// console receive id
    int dest_node;	// destination node
    word_t proto_id;
    word_t route;
    word_t channel;
    bgtree_t *tree;
    spinlock_t lock;

public:
    bool init(fdt_t *fdt);
    void flush_outbuf();
    void poll();
    void inject(except_regs_t *frame);

    void putc(char c)
	{ 
	    lock.lock();
	    out_buf[out_len++] = c;
	    if (out_len >= BUF_SIZE || c == '\n')
		flush_outbuf();
	    lock.unlock();
	}

    char getc(bool block)
	{
	    char c = 0;
	    lock.lock();

	    do {
		flush_outbuf(); // make sure the other end sees all output...
		poll();
	    } while (block && in_len == 0);

	    if (in_len)
	    {
		c = in_buf[in_head];
		in_head = (in_head + 1) % BUF_SIZE;
		in_len--;
	    }
	    lock.unlock();
	    return c;
	}

    void enqueue_char(char c)
	{ 
	    if (in_len >= BUF_SIZE)
		return; // just drop
	    in_buf[(in_head + in_len) % BUF_SIZE] = c;
	    in_len++;
	}

    void enqueue_packet(bglink_hdr_t *lnkhdr, char *buf)
	{
	    if (lnkhdr->lnk_proto != proto_id ||
		lnkhdr->dst_key != rcv_id ||
		lnkhdr->src_key == tree->node_id)
		return;

	    int len = min(lnkhdr->optional - 240 * lnkhdr->this_pkt, 240);
	    for (int i = 0; i < len; i++)
		enqueue_char(buf[i]);
	}

    bool check_breakin()
	{
            bool ret = false;
#if defined(CONFIG_KDB_BREAKIN_ESCAPE)
	    lock.lock();
	    poll();
	    ret = in_len != 0 && in_buf[in_head] == 0x1b;
	    lock.unlock();
#endif
            return ret;
	}
} tree_console;


NOINLINE void tree_console_t::poll()
{
    static char buf[240] __attribute__((aligned(16)));
    static bglink_hdr_t lnkhdr __attribute__((aligned(16)));

    while (tree->channel[channel].poll(&lnkhdr, buf))
	enqueue_packet(&lnkhdr, buf);

    while (tree->channel[channel == 0 ? 1 : 0].poll(&lnkhdr, buf)) 
    { /* deplete the other channel */ }
}

NOINLINE void tree_console_t::inject(except_regs_t *frame)
{
    static char buf[256] __attribute__((aligned(16)));
    char *c = buf;

    asm volatile(
	"stfpdx	  0, 0, %[dest]\n"
	"stfpdux  1, %[dest], %[offset]\n"
	"stfpdux  2, %[dest], %[offset]\n"
	"stfpdux  3, %[dest], %[offset]\n"
	"stfpdux  4, %[dest], %[offset]\n"
	"stfpdux  5, %[dest], %[offset]\n"
	"stfpdux  6, %[dest], %[offset]\n"
	"stfpdux  7, %[dest], %[offset]\n"
	"stfpdux  8, %[dest], %[offset]\n"
	"stfpdux  9, %[dest], %[offset]\n"
	"stfpdux 10, %[dest], %[offset]\n"
	"stfpdux 11, %[dest], %[offset]\n"
	"stfpdux 12, %[dest], %[offset]\n"
	"stfpdux 13, %[dest], %[offset]\n"
	"stfpdux 14, %[dest], %[offset]\n"
	"stfpdux 15, %[dest], %[offset]\n"
	: [dest] "+b"(c)
	: [offset] "b" (16)
	);

    enqueue_packet(reinterpret_cast<bglink_hdr_t*>(buf), &buf[16]);
}

NOINLINE void tree_console_t::flush_outbuf()
{
    static bglink_hdr_t lnkhdr __attribute__((aligned(16)));

    if (!out_len)
	return;

    tree->init_link_hdr(&lnkhdr);

    lnkhdr.dst_key = send_id;
    lnkhdr.this_pkt = 0;
    lnkhdr.total_pkt = 1;
    lnkhdr.lnk_proto = proto_id;
    lnkhdr.optional = out_len;
	
    bgtree_header_t hdr;
    if (dest_node == -1)
	hdr.set_broadcast(route);
    else
	hdr.set_p2p(route, dest_node);

    tree->channel[channel].send(hdr, lnkhdr, out_buf);

    memset(out_buf, 0, BUF_SIZE);
    out_len = 0;
}

int atoi(char* &string)
{
    int val = 0;
    while (*string >= '0' && *string <= '9')
    {
	val = val * 10 + (*string - '0');
	string++;
    }
    return val;
}

NOINLINE bool tree_console_t::init(fdt_t *fdt)
{
    fdt_property_t *prop;

    fdt_node_t *tty = fdt->find_subtree("/plb/tty");
    if (!tty)
	return false;

    /* figure the configuration of the tty first so that we can map
     * the correct tree channel */
    if (! (prop = fdt->find_property_node(tty, "tree-route")) )
	return false;
    route = prop->get_word(0);

    if (! (prop = fdt->find_property_node(tty, "link-protocol")) )
	return false;
    proto_id = prop->get_word(0);
    
    if (! (prop = fdt->find_property_node(tty, "tree-channel")) )
	return false;
    channel = prop->get_word(0);

    send_id = rcv_id = 2;
    dest_node = 0;

    fdt_node_t *l4node = fdt->find_subtree("/l4");
    if ( l4node && (prop = fdt->find_property_node(l4node, "dbgcon")) )
    {
	/* format: sndid,rcvid,dest */
	char *string = prop->get_string();
	send_id = atoi(string);
	rcv_id = atoi(++string);
	dest_node = atoi(++string);
    }

    tree = bgtree_t::get_device(fdt, 0);

#warning hard coded console
    return true;
}

void init_bgtree()
{
    static bool initialized = false;
    if (!initialized) {
	bgtree_t::tree.init(get_fdt());
	tree_console.init(get_fdt());
	initialized = true;
    }
}

void putc_bgtree(char c)
{
#if defined(CONFIG_KDB_BREAKIN)
    tree_console.poll(); /* first empty the fifos */
#endif
    tree_console.putc(c);
}

char getc_bgtree(bool block)
{
    return tree_console.getc(block);
}
#endif

kdb_console_t kdb_consoles[] = {
#if defined(CONFIG_KDB_CONS_BGP_JTAG)
    { "jtag", init_jtag, putc_jtag, getc_jtag },
#endif
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    { "tree", init_bgtree, putc_bgtree, getc_bgtree },
#endif
    KDB_NULL_CONSOLE
};

#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void)
{
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    if (tree_console.check_breakin())
	enter_kdebug("breakin");
#endif
}
#endif

void kdb_inject(except_regs_t* frame)
{
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    tree_console.inject(frame);
#endif
}

