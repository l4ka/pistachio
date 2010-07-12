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
#include <l4/arch.h>

#include "kickstart.h"
#include "fdt.h"

extern inline void dcbi(void* ptr)
{
    asm volatile ("dcbi  0,%0" : : "r" (ptr) : "memory");
}


extern inline void mtdcrx(unsigned int dcrn, unsigned int value)
{
    asm volatile("mtdcrx %0,%1": :"r" (dcrn), "r" (value) : "memory");
}



class bgp_mailbox_t 
{
public:
    volatile unsigned short command;	// comand; upper bit=ack
    unsigned short len;			// length (does not include header)
    unsigned short result;		// return code from reader
    unsigned short crc;			// 0=no CRC
    char data[0];
};

class bgp_cons_t {
public:
    bgp_mailbox_t *mb;
    unsigned size;
    unsigned dcr_set;
    unsigned dcr_clear;
    unsigned dcr_mask;
    bool verbose;

    void send_command(int command)
	{ 
	    mb->command = command;
	    asm volatile("sync");
	    mtdcrx(dcr_set, dcr_mask);
	    
	    do {
		dcbi((void*)&mb->command);
	    } while(!(mb->command & 0x8000));
	}

    bool init(fdt_t *fdt)
	{
	    fdt_property_t *prop;
	    fdt_node_t *node = fdt->find_subtree("/jtag/console0");

	    if (! (prop = fdt->find_property_node(node, "reg")) )
		return false;

	    // addr is 64 bit with upper part 0
	    mb = (bgp_mailbox_t*)prop->get_word(1);
	    size = prop->get_word(2);
	    
	    if (! (prop = fdt->find_property_node(node, "dcr-reg")) )
		return false;

	    dcr_set = prop->get_word(0);
	    dcr_clear = prop->get_word(1);
	    
	    if (! (prop = fdt->find_property_node(node, "dcr-mask")) )
		return false;
	    dcr_mask = prop->get_word(0);

	    verbose = false;
	    fdt_node_t *l4node = fdt->find_subtree("/l4");
	    if ((prop = fdt->find_property_node(l4node, "kickstart")))
	    {
		if (strstr(prop->get_string(), "verbose"))
		    verbose = true;
	    }

	    return true;
	}

    void putc(int c)
	{
	    if (!mb || !verbose)
		return;

	    mb->data[mb->len++] = c;
	
	    if (mb->len >= size || c == '\n')
	    {
		send_command(2);
		mb->len = 0;
	    }
	}
};

bgp_cons_t bgp_cons;

extern "C" void putc(int c)
{
    bgp_cons.putc(c);
#if defined(CONFIG_COMPORT)
    extern void __l4_putc(int c);
    __l4_putc(c);
#endif
}


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
#if defined(CONFIG_COMPORT)
    extern void *__l4_dtree;
    __l4_dtree = fdt_ptr;
#endif
    
    bgp_cons.init(fdt_ptr);
    loader();
}
