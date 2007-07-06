/*********************************************************************
 *                
 * Copyright (C) 2004-2006,  Karlsruhe University
 *                
 * File path:     platform/pc99/io_space.cc
 * Description:   IO-Fpage implementation for IA-32
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
 * $Id: io_space.cc,v 1.4 2006/12/05 15:23:16 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(ptab.h)
#include INC_ARCH(tss.h)
#include INC_API(fpage.h)
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(thread.h)
#include INC_GLUE(space.h)
#include INC_PLAT(io_space.h)
#include <linear_ptab.h>


DECLARE_KMEM_GROUP(kmem_iofp);


/* 
 * void zero_io_bitmap()
 * 
 */

void zero_io_bitmap(space_t *space, word_t port, word_t log2size)
{
    word_t bits;
    word_t size = (1UL << log2size);
       
    //TRACEF("zero_io_bitmap(): port=%x, size=%x, space=%p\n", port, size, space);

    /* 
     * If the task had no mappings before, it has the default IOPBM (which is
     * set to ~0). Create a new one, set it to 0 between port and port + size,
     * and to ~0 else
     */

    if (space->get_io_bitmap() == tss.get_io_bitmap())
    {
	
	    
	word_t *new_iopbm = (word_t*) kmem.alloc(kmem_iofp, IOPERMBITMAP_SIZE);
	    
	if (new_iopbm == NULL)
	{
	    TRACEF("zero_io_bitmap(): out of memory\n");
	    return;
	}

	space->install_io_bitmap((addr_t) new_iopbm);

	word_t *w = new_iopbm;

	/* 
	 * until we reach the port, all bits are set to 1 (word-wise) 
	 */	    
	for (; w < new_iopbm + (port / BITS_WORD); w++)
	    *w = ~(0UL); 

	/* 
	 * Set the bits before port, zero the others 
	 */	    
	word_t bits = min(size , BITS_WORD - (port & (BITS_WORD - 1) ) );
	*w++ =  ~(( ~(0UL) >> (BITS_WORD - bits)) << (port & (BITS_WORD - 1))); 

	/* 
	 * Wordwise zero between bit port and bit port+size 
	 */
	for (size -= bits; size >= BITS_WORD; size -= BITS_WORD)
	    *w++ = 0;
	    
	/* 
	 * Zero the word hosting bit port+size 
	 */
	if (size)
	    *w++ = ~( ~(0UL) >> (BITS_WORD - size));
	    
	/* 
	 * The remaining words are set to 1 
	 */
	for (; w < new_iopbm + IOPERMBITMAP_SIZE / sizeof(word_t) ; w++)
	    *w = ~0UL;

	return;
    } 

    
    word_t *w = (word_t *) space->get_io_bitmap();
    
    /* 
     * Jump to the right word 
     */
    w += (port / BITS_WORD);

    /* 
     * Zero the bits after bit port 
     */	    
    bits = min(size , BITS_WORD - (port & (BITS_WORD - 1)) );

    *w++ &=  ~(( ~(0U) >> (BITS_WORD - bits)) << (port & (BITS_WORD -1))); 

    /* 
     * Wordwise zero between bit port and bit port+size 
     */
    for (size -= bits; size >= BITS_WORD; size -= BITS_WORD)
	*w++ = 0;
	    
    /* 
     * Zero the bits before bit port+size 
     */
    if (size)
	*w++ &= ~(~(0U) >> (BITS_WORD - size));

}

/* 
 * void set_io_bitmap()
 * 
 */

void set_io_bitmap(space_t *space, word_t port, word_t log2size)
{
    word_t bits;
    word_t size = (1UL << log2size);
    word_t *w = (word_t *) space->get_io_bitmap();
    
    //TRACEF("set_io_bitmap(): port=%x, size=%x, space=%p\n", port, size, space);
    
    /* 
     * Jump to the right word 
     */
    w += port / BITS_WORD;
	
    /* 
     * Set the bits after bit port 
     */	    
  
    bits = min(size , BITS_WORD - (port & (BITS_WORD - 1)) );
    *w++ |=   ((~(0U) >> (BITS_WORD - bits)) << (port & (BITS_WORD - 1))); 
	    
    
    /* 
     * Wordwise set between bit port and bit port+size 
     */
    for (size -= bits; size >= BITS_WORD; size -= BITS_WORD)
	*w++ = ~0UL;
	
    /* 
     * Set the bits before bit port+size 
     */
    if (size)
	*w++ |= ( ~(0UL) >> (BITS_WORD - size));

   
}


/*
 * get_io_pbm_phys
 *
 * returns the physical address of the IO Bitmap 
 */

addr_t space_t::get_io_bitmap()
{

    pgent_t *pgent;
    pgent_t::pgsize_e pgsize;

    if (this->lookup_mapping(addr_offset((addr_t) TSS_MAPPING, tss.get_io_bitmap_offset()), &pgent, &pgsize)){
	return phys_to_virt(pgent->address(this, pgsize));
    }
    
    TRACEF("BUG: get_io_bitmap_phys returns NULL\n");
    enter_kdebug("IO-Fpage BUG");
    return NULL;
}

/*
 * init_io_space()
 *
 * initializes the default IO bitmap
 */

void init_io_space(void)
{
    /* 
     * Set the default IOPBM Bits to 1 
     */
    word_t *p = (word_t *) tss.get_io_bitmap();
    
    for (u32_t i=0; i < IOPERMBITMAP_SIZE / sizeof(word_t); i++)
	*(p + i) = ~0UL;
    
#if defined(CONFIG_IA32_PVI)
    /* Enable PVI Bit */
#warning Setting PVI bit in CR4 will not work with vmware
    ia32_cr4_set(IA32_CR4_PVI);
#endif
    

}

/*
 * arch_map_fpage()
 * 
 * maps an IO-Fpage
 * 
 */

void arch_map_fpage (tcb_t * src, fpage_t snd_fpage,
		     word_t snd_base,
		     tcb_t * dst, fpage_t rcv_fpage,
		     bool grant)
{

    //TRACEF("raw = %x, port = %x, size %x, base = %x, from = %p, to = %p\n", 
    //   snd_fpage.raw, snd_fpage.arch.get_base(), snd_fpage.arch.get_size(), snd_base,
    //   src->get_space(), dst->get_space());
    
    if (src->get_space()->get_io_space())
    {
	if (!dst->get_space()->get_io_space())
	    dst->get_space()->set_io_space(new vrt_io_t);
	
	src->get_space()->get_io_space()->map_fpage
	    (snd_fpage, (word_t) snd_fpage.arch.get_base(), dst->get_space()->get_io_space (), rcv_fpage, grant);
    }
}


/*
 * map_io_fpage()
 * 
 * revokes an IO-Fpage
 */

void arch_unmap_fpage (tcb_t * from, fpage_t fpage, bool flush)
{
    mdb_t::ctrl_t ctrl (0);
    ctrl.unmap = true;
    ctrl.mapctrl_self = flush;
    from->get_space()->get_io_space()->mapctrl (fpage, ctrl, 0, 0);
}



/*
 * handle_io_pagefault()
 *
 * handle an IO pagefault exception 
 */


void handle_io_pagefault(tcb_t *tcb, u16_t port, u16_t log2size, addr_t ip)
{
    space_t *space = tcb->get_space();
    
    if (space == sigma0_space)
    {
	//TRACEF("sigma0 IO Pagefault %x [%x] @ %p\n", port, log2size, ip); 
	zero_io_bitmap(space, port, log2size);
	return;
    }
	
    tcb->save_state ();

    /* generate pagefault message (rw) */
    msg_tag_t tag;
    tag.set(0, 2, IPC_MR0_IO_PAGEFAULT | (1 << 2) | (1 << 1));

    /* create acceptor for whole address space */
    acceptor_t acceptor;
    acceptor.clear();
    acceptor.set_rcv_window(fpage_t::complete_arch());
    
    fpage_t iofp;
    iofp.arch.set(port, log2size, 0, 0, 0);
    
    tcb->set_tag(tag);
    tcb->set_mr(1, iofp.raw);
    tcb->set_mr(2, (word_t)ip);
    tcb->set_br(0, acceptor.raw);

    //TRACEF("send IO-pagefault IPC (%t %x)\n", TID(tcb->get_pager()), iofp.raw);
    tag = tcb->do_ipc(tcb->get_pager(), tcb->get_pager(), timeout_t::never());
    if (tag.is_error())
    {
	printf("result tag = %p, ip = %p, port = %x, size = %x\n", tag.raw, ip, port, log2size);
	enter_kdebug("IO-pagefault IPC error");
    }

    tcb->restore_state ();

}

