/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * Filename:      io.cc
 * Author:        Volkmar Uhlig, Jan Stoess <stoess@kit.edu>
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
 ********************************************************************/

#include <debug.h>
#include <lib.h>
#include <kdb/console.h>
#include <sync.h>
#include INC_ARCH(io.h)
#include INC_ARCH(cache.h)
#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(ppc44x.h)
#include INC_PLAT(fdt.h)

#define SEC_PPC44X_IO		".kdebug"
extern addr_t setup_console_mapping(paddr_t paddr, int log2size);
bool getc_blocked = false;


/* Section assignements */
#if defined(CONFIG_KDB_CONS_BGP_JTAG)
static void putc_jtag (char) SECTION (SEC_PPC44X_IO);
static char getc_jtag (bool) SECTION (SEC_PPC44X_IO);
static void init_jtag (void) SECTION (SEC_PPC44X_IO);
#endif

#if defined(CONFIG_KDB_CONS_BGP_TREE)
static void putc_bgtree (char) SECTION (SEC_PPC44X_IO);
static char getc_bgtree (bool) SECTION (SEC_PPC44X_IO);
static void init_bgtree (void) SECTION (SEC_PPC44X_IO);
#endif

#if defined(CONFIG_KDB_CONS_COM)
static void putc_serial (char) SECTION (SEC_PPC44X_IO);
static char getc_serial (bool) SECTION (SEC_PPC44X_IO);
static void init_serial (void) SECTION (SEC_PPC44X_IO);
#endif 

#if defined(CONFIG_KDB_BREAKIN)
void kdebug_check_breakin (void) SECTION (SEC_PPC44X_IO);
#endif

kdb_console_t kdb_consoles[] = {
#if defined(CONFIG_KDB_CONS_BGP_JTAG)
    { "jtag", init_jtag, putc_jtag, getc_jtag },
#endif
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    { "tree", init_bgtree, putc_bgtree, getc_bgtree },
#endif
#if defined(CONFIG_KDB_CONS_COM)
    { "serial", &init_serial, &putc_serial, &getc_serial },
#endif 
    KDB_NULL_CONSOLE
};

/*
**
** Serial port I/O functions.
**
*/
#if defined(CONFIG_KDB_CONS_COM)

#if !defined(CONFIG_KDB_COMPORT)
#define CONFIG_KDB_COMPORT 0
#endif
#if !defined(CONFIG_KDB_COMSPEED)
#define CONFIG_KDB_COMSPEED 115200
#endif
static u8_t *comport = CONFIG_KDB_COMPORT;


static void init_serial (void)
{
#define IER	(comport+1)
#define EIR	(comport+2)
#define LCR	(comport+3)
#define MCR	(comport+4)
#define LSR	(comport+5)
#define MSR	(comport+6)
#define DLLO	(comport+0)
#define DLHI	(comport+1)

#if CONFIG_COMPORT == 1
    UNIMPLEMENTED();
#endif
    
#if CONFIG_COMPORT == 0
    /*  FDT  */
    fdt_property_t *prop;
    fdt_node_t *node;
    fdt_t *fdt;    

    if (!(fdt = get_dtree()))
        return;
    
    if (!(node = fdt->find_subtree("/aliases")))
        return;

    if (! (prop = fdt->find_property_node(node, "serial0")) )
        return;

    if (!(node = fdt->find_subtree(prop->get_string())))
        return;
    
    if (! (prop = fdt->find_property_node(node, "reg")) )
        return;
    
    // Serial bus is beyond 4GB
    u64_t comport_phys = 0x100000000ULL | (u64_t) prop->get_word(0);
    comport = (u8_t*)setup_console_mapping(comport_phys, 12);
    
#endif /* CONFIG_COMPORT == 0 */

    if (comport)
    {
        out_8(LCR, 0x80);          /* select bank 1        */
        for (volatile int i = 10000000; i--; );
        out_8(DLLO, (((115200/CONFIG_KDB_COMSPEED) >> 0) & 0x00FF));
        out_8(DLHI, (((115200/CONFIG_KDB_COMSPEED) >> 8) & 0x00FF));
        out_8(LCR, 0x03);          /* set 8,N,1            */
        out_8(IER, 0x00);          /* disable interrupts   */
        out_8(EIR, 0x07);          /* enable FIFOs */
        in_8(IER);
        in_8(EIR);
        in_8(LCR);
        in_8(MCR);
        in_8(LSR);
        in_8(MSR);
        
    }

}


static void putc_serial (const char c)
{
    while ((in_8(LSR) & 0x20) == 0);
    out_8(comport,c);
    while ((in_8(LSR) & 0x40) == 0);
    if (c == '\n')
	putc_serial('\r');
}

static char getc_serial (bool block)
{
    if ((in_8(LSR) & 0x01) == 0)
    {
	if (!block)
	    return (char) -1;
	
        getc_blocked = true;
	while ((in_8(LSR) & 0x01) == 0);
        getc_blocked = false;
	
    }
    return in_8(comport);
}

static bool check_breakin_serial ()
{
#if defined(CONFIG_KDB_BREAKIN_BREAK) || defined(CONFIG_KDB_BREAKIN_ESCAPE)
    u8_t c = in_8(LSR);
#endif

#if defined(CONFIG_KDB_BREAKIN_ESCAPE)
    if ((c & 0x01) && (in_8(comport) == 0x1b))
        return true;
#endif
    return false;
}

#endif /* defined(CONFIG_KDB_CONS_COM) */


#if defined(CONFIG_KDB_CONS_BGP_JTAG)
/*
**
** Bluegene JTAG I/O functions.
**
*/

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
    cons.init(get_dtree());
    init_bgtree();
}

const char kbd_ret[] = "1q2q";
static char getc_jtag(bool block) 
{
    static int cnt = 0;

    if (cnt < 1)
	return kbd_ret[cnt++];

    if (block)
    {
        getc_blocked = true;
	while(1);
        getc_blocked = false;
    }
    return 0; 
}

static void putc_jtag(char c) 
{
    cons.putc(c);
}
#endif

/*
**
** Bluegene Tree I/O functions.
**
*/

#if defined(CONFIG_KDB_CONS_BGP_TREE)

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
    ppc_set_dcrx(dcr_base + 0x45, 0);
    ppc_set_dcrx(dcr_base + 0x49, 0);

    /* clear anything that may be pending */
    ppc_get_dcrx(dcr_base + 0x44);
    ppc_get_dcrx(dcr_base + 0x48);

    return true;
}

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

            getc_blocked = true;
	    do {
		flush_outbuf(); // make sure the other end sees all output...
		poll();
	    } while (block && in_len == 0);
            getc_blocked = false;

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
	bgtree_t::tree.init(get_dtree());
	tree_console.init(get_dtree());
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


void kdb_inject(except_regs_t* frame)
{
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    tree_console.inject(frame);
#endif
}


#if defined(CONFIG_KDB_BREAKIN) 
void kdebug_check_breakin (void)
{
#if defined(CONFIG_KDB_CONS_BGP_TREE)
    if (tree_console.check_breakin())
	enter_kdebug("breakin");
#endif

#if defined(CONFIG_KDB_CONS_COM)
    if (check_breakin_serial())
	enter_kdebug("breakin");
#endif 
    
    return;
}
#endif

