/*********************************************************************
 *                
 * Copyright (C) 2002-2004,   University of New South Wales
 *                
 * File path:     arch/alpha/pgent.h
 * Description:   Generic page table manipluation for Alpha
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
 * $Id: pgent.h,v 1.21 2004/04/26 17:40:17 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__ALPHA__PGENT_H__
#define __ARCH__ALPHA__PGENT_H__

#include <kmemory.h>
#include <debug.h>

#include INC_GLUE(config.h)
#include INC_GLUE(hwspace.h)
#include INC_ARCH(page.h)

#define MDB_PGSHIFTS           HW_PGSHIFTS
/* sjw (12/09/2002): ????  */
#define MDB_NUM_PGSIZES        (NUM_HW_PGSHIFTS - 1)

EXTERN_KMEM_GROUP (kmem_pgtab);

class space_t;
class mapnode_t;

extern word_t hw_pgshifts[];

class pgent_t
{
public:
    enum pgsize_e {
	/* PAL supported (in L3) */
	size_8k = 0,
	size_64k = 1,
	size_512k = 2,
	size_4m = 3,

	size_8m, /* 23 bits */
	size_8g, /* 33 bits */
	size_8t, /* 43 bits */

	size_bottomlevel = size_8m,
/* sjw (30/07/2002): Yuck */
#if CONFIG_ALPHA_ADDRESS_BITS == 43
	size_all = size_8t, 
	size_max = size_8g,
#else
	size_256t,
	size_all = size_256t,
	size_max = size_8t,
#endif /* CONFIG_ALPHA_ADDRESS_BITS */

	size_base = size_8k,
    };

private:
    union {
	word_t raw;
	struct {
	    unsigned valid           :1;
	    unsigned fault_on_read   :1;
	    unsigned fault_on_write  :1;
	    unsigned fault_on_exec   :1;
	    unsigned as_match        :1; /* Doubles as the kernel flag */
	    unsigned granularity     :2; /* pgsize_e, <= size_4m */
	    unsigned rsv2            :1;
	    unsigned kre             :1; /* ure <=> kre */
	    unsigned ure             :1;
	    unsigned rsv1            :2;
	    unsigned kwe             :1; /* uwe <=> kwe */
	    unsigned uwe             :1;
	    unsigned rsv0            :2;
	    /* 16 bits reserved for software */
	    unsigned subtree         :8; /* Makes lookups faster --- can use extb commands */
	    unsigned unused          :8;
	    unsigned pfn             :32;
	} pgent;
    };

private:
    // Linknode access 

    /* sjw (30/07/2002):  This will _really_ break if we dont use kseg pte addresses! */
    u64_t get_linknode (void)
	{ return *(u64_t *) ((word_t) this + ALPHA_PAGE_SIZE); }

    void set_linknode (u64_t val)
	{ *(u64_t *) ((word_t) this + ALPHA_PAGE_SIZE) = val; }

    addr_t get_page(void) 
	{ return (addr_t) ((word_t) pgent.pfn << ALPHA_PAGE_BITS); }

public:

    // Predicates
    /* sjw (02/08/2002): I think */
    bool is_valid (space_t * s, pgsize_e pgsize)
	{ return pgent.valid || (pgent.subtree && pgent.granularity < (word_t) pgsize); }
    
    bool is_writable (space_t * s, pgsize_e pgsize)
	{ return pgent.kwe; }
    
    bool is_readable (space_t * s, pgsize_e pgsize)
	{ return pgent.kre; }
    
    bool is_executable (space_t * s, pgsize_e pgsize)
	{ return pgent.kre; }

    bool is_subtree (space_t * s, pgsize_e pgsize)
	{ 
	    /* sjw (02/08/2002): ??? */
	    if(pgsize >= size_bottomlevel) 
		return pgent.subtree; 
	    else
		return pgent.granularity < (word_t) pgsize;
	}

    bool is_kernel (space_t * s, pgsize_e pgsize)
	{ return pgent.as_match; }

    // Retrieval
    addr_t address (space_t * s, pgsize_e pgsize)
	{ return addr_mask(get_page(),
			   ~((1UL << hw_pgshifts[pgsize]) - 1));}
	
    pgent_t * subtree (space_t * s, pgsize_e pgsize)
	{ 
	    if(pgsize >= size_bottomlevel) 
		return (pgent_t *) phys_to_virt(get_page()); 
	    else
		return (pgent_t *) this;
	}

    mapnode_t * mapnode (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{ return (mapnode_t *) (get_linknode() ^ (word_t) vaddr); }

    addr_t vaddr (space_t * s, pgsize_e pgsize, mapnode_t * map)
	{ return (addr_t) (get_linknode() ^ (word_t) map); }

    word_t reference_bits (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{
	    /* sjw (28/07/2002): Note that PAL doesn't touch the page table, so just improvise.  I will
	     *  use the fault-on-read etc. to implement these later.
	     */
	    word_t rwx = 0;
	    if(pgent.kwe) 
		rwx = 6;
	    else if(pgent.kre) 
		rwx = 4;
		
	    return rwx;
	}

    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    // XXX: Implement me
	}
    
    // Modification
    void clear (space_t * s, pgsize_e pgsize, bool kernel,  addr_t vaddr)
	{
	    raw = 0UL;
	    if (!kernel)
		set_linknode(0);
	}

    void flush (space_t * s, pgsize_e pgsize, bool kernel,  addr_t vaddr)
	{
	}

    /* Creates a private PTE */
    pgent_t create_entry(space_t *s, pgsize_e pgsize, addr_t paddr, bool readable, 
			 bool writable, bool executable, bool kernel, bool leaf)
	{
	    pgent_t tmp;
	    tmp.raw = 0;

#if defined(CONFIG_SWIZZLE_IO_ADDR)
	    // Allow bit 40 to select I/O space
	    if (((word_t)paddr >> 40) & 1)
		paddr = (addr_t)((word_t)paddr | (1UL << 43));
#endif

	    tmp.pgent.pfn = (word_t) paddr >> ALPHA_PAGE_BITS;
	    if(readable) 
		tmp.pgent.kre = 1;

	    if(writable) 
		tmp.pgent.kwe = 1;

	    /* Ignore executable */
	    if(!kernel) {
		if(leaf) {
		    tmp.pgent.uwe = tmp.pgent.kwe;
		    tmp.pgent.ure = tmp.pgent.kre;
		}
	    } else {
		tmp.pgent.as_match = 1;
	    }

	    /* sjw (02/08/2002): Ugly */
	    if(leaf) {
		tmp.pgent.granularity = (word_t) pgsize;
		tmp.pgent.valid = 1;
	    } else {
		if(pgsize < size_bottomlevel)
		    tmp.pgent.granularity = (word_t) pgsize - 1;
		else
		    tmp.pgent.valid = 1;

		tmp.pgent.subtree = true;
	    }

	    return tmp;
	}

    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    addr_t page = 0;

	    if(pgsize >= size_bottomlevel) {
		page = kmem.alloc(kmem_pgtab, kernel ? ALPHA_PAGE_SIZE :
					 ALPHA_PAGE_SIZE * 2);
		page = virt_to_phys(page);
	    }
	    
	    raw = create_entry(s, pgsize, page, true, true, false, kernel, false).raw;
	}

    void remove_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    if(pgsize >= size_bottomlevel) {
		addr_t ptab = get_page();
		kmem.free (kmem_pgtab, phys_to_virt(ptab),
			   kernel ? ALPHA_PAGE_SIZE : ALPHA_PAGE_SIZE * 2);
	    }
	    raw = 0;
	}

    /* This is technically an assignment operator */
    void set_entry(space_t *space, pgsize_e pgsize, pgent_t pgent)
	{
	    raw = pgent.raw;
	}

    void set_entry (space_t * s, pgsize_e pgsize,
			   addr_t paddr, bool readable, 
			   bool writable, bool executable, bool kernel)
	{
	    pgent_t tmp, *ptr;
	    int count;

	    tmp = create_entry(s, pgsize, paddr, readable, writable, executable, kernel, true);
	    
	    count = 1 << (pgsize * 3);
	    
	    /* Set all entries in the superpage */
	    for(ptr = this; count > 0; --count) 
		ptr->raw = tmp.raw;
	}

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{ if (rwx & 2) { pgent.uwe = pgent.kwe = 0; } }

    /* sjw (28/07/2002):  Do we only update user mappings? */
    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{ if (rwx & 2) { pgent.kwe = 1; pgent.uwe = 1;} }

    void reset_reference_bits (space_t * s, pgsize_e pgsize)
	{ /* Do nothing */ }

    void set_linknode (space_t * s, pgsize_e pgsize,
		       mapnode_t * map, addr_t vaddr)
	{ set_linknode ((word_t) map ^ (word_t) vaddr); }

    // Movement

    pgent_t * next (space_t * s, pgsize_e pgsize, word_t num)
	{ 
	    switch(pgsize) {
	    case size_64k:
		return this + 8 * num;
	    case size_512k:
		return this + 64 * num;
	    case size_4m:
		return this + 512 * num;
	    case size_8k:
	    default:
		return this + num;
	    }
	}

    // Debug
    
    void print(space_t *s, pgsize_e pgsize, word_t va) 
	{
	    printf("0x%lx -> 0x%lx (%d): %d %c%c %s %s %s\n", va, get_page(), pgsize, pgent.granularity, 
		   pgent.kre ? 'r' : '-',
		   pgent.kwe ? 'w' : '-',
		   pgent.valid ? "valid" : "invalid", 
		   pgent.as_match ? "kernel" : "user", 
		   pgent.subtree ? "subtree" : "leaf");
   

	}

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	}
};


#endif /* !__ARCH__ALPHA__PGENT_H__ */
