/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     arch/mips64/tlb.h
 * Description:   TLB management
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
 * $Id: tlb.h,v 1.16 2004/06/04 02:14:25 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS64__TLB_H__
#define __ARCH__MIPS64__TLB_H__

#include INC_ARCH(mipsregs.h)

/**
 * A translation specifying a mapping to a physical address, including
 * page access rights, memory attributes, etc.  Virtual address, page
 * size, and so on are specified when inserting the translation into
 * the TLB.
 */
class translation_t
{
    union {
	struct {
	    BITFIELD7( word_t,
		global		: 1,
		valid		: 1,
		dirty		: 1,
		memory_attrib	: 3,
		phys_addr	: 24, /* How big do you want to make
	         		       * this? */
		__rv1		: 32,
		rid		: 2   /* Region ID: 00b user/ 01b supervisor/ 11b kernel */
	    )
	} x;
	word_t raw;
    };

public:

    /* memory coherency attributes (map to EntryLo bits) */
    enum memattrib_e {
	write_through_noalloc	= CONFIG_CACHABLE_NO_WA,
	write_through		= CONFIG_CACHABLE_WA,
	uncached		= CONFIG_NOCACHE,
	write_back		= CONFIG_CACHABLE_NONCOHERENT,
#if CONFIG_UNCACHED
	l4default		= uncached,
#else
#if CONFIG_SMP
	coherent		= CONFIG_CACHABLE_COW,
	l4default		= coherent,
#else
	l4default		= write_back,
#endif
#endif
    };


    /*
     * Retrieval
     */

    /*
     * reset accessed and dirty bit for page
     */
    void reset_reference_bits (void)
	{ /* Do nothing */ }

    /*
     * @return memory attributes for page
     */
    memattrib_e memattrib (void)
	{ return (memattrib_e) x.memory_attrib; }

    /**
     * @return physical address of mapping
     */
    addr_t phys_addr (void)
	{ return (addr_t) (x.phys_addr << CONFIG_MIPS64_VPN_SHIFT); }

    /**
     * @return raw contents of translation
     */
    u64_t get_raw (void)
	{ return raw; }


    word_t get_valid (void)
	{ return x.valid; }

    word_t get_dirty (void)
	{ return x.dirty; }

    word_t get_global (void)
	{ return x.global; }

    bool is_kernel (void)
	{ return (x.rid != 0); } /* 0 - user, 1 or 3 is kernel */

    /*
     * Modification
     */

    void set_dirty (word_t d)
	{ x.dirty = d; }

    void set_valid (word_t v)
	{ x.valid = v; }

    void set_global (word_t g)
	{ x.global = g; }

    void set_attrib (memattrib_e memattrib)
	{ x.memory_attrib = (word_t) memattrib; }

    void set_phys_addr(addr_t phys_addr)
	{
	    x.phys_addr = (word_t) phys_addr >> CONFIG_MIPS64_VPN_SHIFT;
	}

    void set (memattrib_e memattrib,
	      bool allow_access,
	      bool allow_modify,
	      bool kernel,
	      addr_t phys_addr);


    /*
     * Creation
     */

    /**
     * Create new translation with undefined contents.
     */
    translation_t (void) {}

    /**
     * Create new translation with the given raw contents.
     * @param x		raw contenents of new translation
     */
    translation_t (word_t x) { raw = x; }

    translation_t (memattrib_e memattrib,
		   bool allow_access,
		   bool allow_modify,
		   bool kernel,
		   addr_t phys_addr);

    /*
     * Translation cache access
     */

    void put_tc (addr_t vaddr, word_t page_size, word_t asid);
};


/**
 * Create new translation and set initialize it according to input
 * parameters.
 *
 * @param memattrib		memory attribute for mapping
 * @param allow_access		allow accesses on mapped page
 * @param allow_modify		allow modifications on mapped page
 * @param phys_addr		physical address
 */
INLINE
translation_t::translation_t (memattrib_e memattrib,
			      bool allow_access,
			      bool allow_modify,
			      bool kernel,
			      addr_t phys_addr)
{
    raw = 			0;
    x.global =			kernel;
    x.valid =			allow_access;
    x.dirty =			allow_modify;
    x.memory_attrib = 		(word_t) memattrib;
    x.phys_addr =		(word_t) phys_addr >> CONFIG_MIPS64_VPN_SHIFT;
    x.rid	=		kernel ? 1 : 0;	    /* 00b - user, 01b - super, 11b - kernel */
}


/**
 * Set translation according to input parameters.
 *
 * @param present		is page present or not
 * @param memattrib		memory attribute for mapping
 * @param allow_access		allow accesses on mapped page
 * @param allow_modify		allow modifications on mapped page
 * @param access_rights		access rights for mapping
 * @param phys_addr		physical address
 * @param defer_exceptions	shoule exception in page be deferred
 */
INLINE void
translation_t::set (memattrib_e memattrib,
		    bool allow_access,
		    bool allow_modify,
		    bool kernel,
		    addr_t phys_addr)
{
    x.global =			kernel;
    x.valid =			allow_access;
    x.dirty =			allow_modify;
    x.memory_attrib = 		(word_t) memattrib;
    x.phys_addr =		(word_t) phys_addr >> CONFIG_MIPS64_VPN_SHIFT;
    x.rid =			kernel ? 1 : 0;	    /* 00b - user, 01b - super, 11b - kernel */
}


/**
 * Number of virtual address bits implemented by the CPU. 
 */
extern word_t mips64_num_vaddr_bits;


/**
 * Insert code and/or data translation into translation cache.
 *
 * @param vaddr		virtual address
 * @param page_size	page size (log2)
 * @param asid		address space ID
 */
INLINE void
translation_t::put_tc (addr_t vaddr, word_t page_size, word_t asid)
{
    word_t entry_hi, save;
    word_t entry_lo0, entry_lo1;
    long int temp;

    ASSERT(page_size == 12);
    ASSERT(asid < CONFIG_MAX_NUM_ASIDS);

    __asm__ __volatile__ (
	"dmfc0	%0,"STR(CP0_ENTRYHI)"\n\t"
	: "=r" (save)
    );

    if (x.global) asid = 0;    /* Global? */

    entry_hi = (x.rid<<62) | (asid) |
		((((word_t)vaddr>>(page_size+1))&((1<<27)-1))<<(13));
    /* TRACEF("entry_hi = %p, raw = %p, phy = %8x\n", entry_hi, raw, (raw>>6)<<12); */

    __asm__ __volatile__ (
	"dmtc0	%1,"STR(CP0_ENTRYHI)"\n\t"
	"nop;nop;nop;tlbp;nop;nop;nop;"
	"mfc0	%0,"STR(CP0_INDEX)"\n\t"
	: "=r" (temp) : "r" (entry_hi)
    );

    if (temp < 0)   /* is top bit set? */
	goto none;

    __asm__ __volatile__ (
	"tlbr\n\t"
	"nop;nop;nop;\n\t"
    );

    if ((word_t)vaddr & (1<<page_size))	    /* Odd entry */
    {
	__asm__ __volatile__ (
	    "dmtc0	%0,"STR(CP0_ENTRYLO1)"\n\t"
	    : : "r" (raw)
	);
    } else  /* Even entry */
    {
	__asm__ __volatile__ (
	    "dmtc0	%0,"STR(CP0_ENTRYLO0)"\n\t"
	    : : "r" (raw)
	);
    }
    __asm__ __volatile__ (
	"nop;nop;nop;\n\t"
	"tlbwi\n\t"
"nop;nop;nop;\n\t"
    );
    goto exit;

none:
    if ((word_t)vaddr & (1<<page_size))	    /* Odd entry */
    {
	entry_lo0 = 1;	/* Set global bit (both must be global for kernel) */
	entry_lo1 = raw;
    } else  /* Even entry */
    {
	entry_lo0 = raw;
	entry_lo1 = 1;
    }
    __asm__ __volatile__ (
	"dmtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
	"dmtc0	%1,"STR(CP0_ENTRYLO0)"\n\t"
	"dmtc0	%2,"STR(CP0_ENTRYLO1)"\n\t"
	"nop;nop;nop;\n\t"
	"tlbwr\n\t"
"nop;nop;nop;\n\t"
	: : "r" (entry_hi), "r" (entry_lo0), "r" (entry_lo1)
    );

exit:
    __asm__ __volatile__ (
	"dmtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
	: : "r" (save)
    );

}

/**
 * Purge all data and instruction translations in the translation
 * with the indicated virtual memory region.
 *
 * @param vaddr		virtual address to purge
 * @param page_size	size of region to purge (log2)
 * @param asid		address space ID of mapping to purge
 */
INLINE void purge_tc (addr_t vaddr, word_t page_size, word_t asid)
{
    word_t entry_hi, save;
    s64_t temp;

    ASSERT(page_size == 12);
    ASSERT(asid < CONFIG_MAX_NUM_ASIDS);

    __asm__ __volatile__ (
	"dmfc0	%0,"STR(CP0_ENTRYHI)"\n\t"
	: "=r" (save)
    );

    entry_hi = (word_t)vaddr & (~((1UL<<(page_size+1)) - 1)) | asid;

    __asm__ __volatile__ (
	"dmtc0	%1,"STR(CP0_ENTRYHI)"\n\t"
	"nop;nop;nop;tlbp;nop;nop;nop;"
	"mfc0	%0,"STR(CP0_INDEX)"\n\t"
	: "=r" (temp) : "r" (entry_hi)
    );

    if (temp < 0)   /* is top bit set? */
	goto exit;

    __asm__ __volatile__ (
	"tlbr\n\t"
	"nop;nop;nop;\n\t"
    );

    if ((word_t)vaddr & (1UL<<page_size))	    /* Odd entry */
    {
	__asm__ __volatile__ (
	    "dmtc0	$0,"STR(CP0_ENTRYLO1)"\n\t"
	);
    } else  /* Even entry */
    {
	__asm__ __volatile__ (
	    "dmtc0	$0,"STR(CP0_ENTRYLO0)"\n\t"
	);
    }
    __asm__ __volatile__ (
	"nop;nop;nop;\n\t"
	"tlbwi\n\t"
"nop;nop;nop;\n\t"
    );

exit:
    __asm__ __volatile__ (
	"dmtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
	: : "r" (save)
    );
}

/**
 * Flush TLB and page cache for asid specified
 * 
 * @param asid		address space ID of space to purge
 */
INLINE void flush_tc (word_t asid)
{
    UNIMPLEMENTED();
}

/**
 * Initialize the TLB
 */
void init_tlb (void);

#endif /* !__ARCH__MIPS64__TLB_H__ */
