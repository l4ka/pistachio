/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/tlb.h
 * Description:   TLB and TR management
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
 * $Id: tlb.h,v 1.20 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__TLB_H__
#define __ARCH__IA64__TLB_H__

/**
 * A huge page size supported by the architecture.
 */
#define HUGE_PGSIZE		(28)


/**
 * A translation specifying a mapping to a physical address, including
 * page access rights, memory attributes, etc.  Virtual address, page
 * size, and so on are specified when inserting the translation into
 * the TLB or a translation register.
 */
class translation_t
{
    union {
	struct {
	    word_t present		: 1;
	    word_t __rv1		: 1;
	    word_t memory_attrib	: 3;
	    word_t allow_access		: 1;
	    word_t allow_modification	: 1;
	    word_t privilege_level	: 2;
	    word_t access_rights	: 3;
	    word_t phys_addr		: 38;
	    word_t __rv2		: 2;
	    word_t defer_exceptions	: 1;
	    word_t __ig			: 11;
	} x;
	word_t raw;
    };

public:

    enum memattrib_e {
	write_back		= 0,
	write_coalescing	= 6,
	uncacheable		= 4,
	uncacheable_exported	= 5,
	nat_page		= 7
    };

    enum access_rights_e {
	ro	= 0,
	rx	= 1,
	rw	= 2,
	rwx	= 3,
	ro_rw	= 4,
	rx_rwx	= 5,
	rwx_rw	= 6,
	xp_rx	= 7
    };

    enum type_e {
	code	= 1,
	data	= 2,
	both	= 3
    };


    //
    // Retrieval
    //

    /**
     * @return true if page is present
     */
    bool is_present (void)
	{ return raw & (1 << 0); }

    /**
     * @return true if page has been accessed
     */
    bool is_accessed (void)
	{ return raw & (1 << 5); }

    /**
     * set accessed bit for page
     */
    void set_accessed (void)
	{ raw |= (1 << 5); }

    /**
     * @return true if page has been modified
     */
    bool is_dirty (void)
	{ return raw & (1 << 6); }

    /**
     * set dirty bit for page
     */
    void set_dirty (void)
	{ raw |= (1 << 6); }

    /**
     * reset accessed and dirty bit for page
     */
    void reset_reference_bits (void)
	{ raw &= ~((1UL << 5) | (1UL << 6)); }

    /**
     * @return true if exceptions on page is deferred
     */
    bool is_defer_exceptions (void)
	{ return raw & (1L << 52); }

    /**
     * @return memory attributes for page
     */
    memattrib_e memattrib (void)
	{ return (memattrib_e) x.memory_attrib; }

    /**
     * @return privilege level of mapping
     */
    word_t privilege_level (void)
	{ return x.privilege_level; }

    /**
     * @return access rights for mapping
     */
    access_rights_e access_rights (void)
	{ return (access_rights_e) x.access_rights; }

    /**
     * @return physical address of mapping
     */
    addr_t phys_addr (void)
	{ return (addr_t) (x.phys_addr << 12); }

    /**
     * @return raw contents of translation
     */
    u64_t get_raw (void)
	{ return raw; }


    //
    // Modification
    //

    /**
     * Set access rights of translation.
     * @param rights	new access rights
     */
    void set_access_rights (access_rights_e rights)
	{ x.access_rights = rights; }

    /**
     * Set memory attributes of translation.
     * @param attr	new memeory attributes
     */
    void set_memattrib (memattrib_e attr)
	{ x.memory_attrib = attr; }

    /**
     * Make translation writable.
     * @return true if operation succeeded, false otherwise
     */
    bool set_writable (void)
	{
	    if ((access_rights_e) x.access_rights == rx_rwx)
		return false;
	    x.access_rights |= 2;
	    return true;
	}

    /**
     * Make translation writable.
     * @return true if operation succeeded, false otherwise
     */
    bool set_execable (void)
	{
	    if ((access_rights_e) x.access_rights == rwx_rw)
		return false;
	    x.access_rights |= 1;
	    return true;
	}

    void set_phys_addr(addr_t phys_addr)
	{
	    x.phys_addr = (word_t) phys_addr >> 12;
	}

    void set (bool present,
	      memattrib_e memattrib,
	      bool allow_access,
	      bool allow_modify,
	      word_t privilege_level,
	      access_rights_e access_rights,
	      addr_t phys_addr,
	      bool defer_exceptions);


    //
    // Creation
    //

    /**
     * Create new translation with undefined contents.
     */
    translation_t (void) {}

    /**
     * Create new translation with the given raw contents.
     * @param x		raw contenents of new translation
     */
    translation_t (word_t x) { raw = x; }

    translation_t (bool present,
		   memattrib_e memattrib,
		   bool allow_access,
		   bool allow_modify,
		   word_t privilege_level,
		   access_rights_e access_rights,
		   addr_t phys_addr,
		   bool defer_exeptions);

    //
    // Translation register access
    //

    void put_itr (word_t num, addr_t vaddr, word_t page_size, word_t key);
    void put_dtr (word_t num, addr_t vaddr, word_t page_size, word_t key);

    //
    // Translation cache access
    //

    void put_tc (type_e type, addr_t vaddr, word_t page_size, word_t key);
};


/**
 * Create new translation and set initialize it according to input
 * parameters.
 *
 * @param present		is page present or not
 * @param memattrib		memory attribute for mapping
 * @param allow_access		allow accesses on mapped page
 * @param allow_modify		allow modifications on mapped page
 * @param privilege_level	privilege level of mapping
 * @param access_rights		access rights for mapping
 * @param phys_addr		physical address
 * @param defer_exceptions	shoule exception in page be deferred
 */
INLINE
translation_t::translation_t (bool present,
			      memattrib_e memattrib,
			      bool allow_access,
			      bool allow_modify,
			      word_t privilege_level,
			      access_rights_e access_rights,
			      addr_t phys_addr,
			      bool defer_exceptions)
{
    raw = 			0;
    x.present = 		present;
    x.memory_attrib = 		(word_t) memattrib;
    x.allow_access =		allow_access;
    x.allow_modification =	allow_modify;
    x.privilege_level =		privilege_level;
    x.access_rights =		(word_t) access_rights;
    x.phys_addr =		(word_t) phys_addr >> 12;
    x.defer_exceptions =	defer_exceptions;
}


/**
 * Set translation according to input parameters.
 *
 * @param present		is page present or not
 * @param memattrib		memory attribute for mapping
 * @param allow_access		allow accesses on mapped page
 * @param allow_modify		allow modifications on mapped page
 * @param privilege_level	privilege level of mapping
 * @param access_rights		access rights for mapping
 * @param phys_addr		physical address
 * @param defer_exceptions	shoule exception in page be deferred
 */
INLINE void
translation_t::set (bool present,
		    memattrib_e memattrib,
		    bool allow_access,
		    bool allow_modify,
		    word_t privilege_level,
		    access_rights_e access_rights,
		    addr_t phys_addr,
		    bool defer_exceptions)
{
    x.__rv1 = x.__rv2 =		0;
    x.present = 		present;
    x.memory_attrib = 		(word_t) memattrib;
    x.allow_access =		allow_access;
    x.allow_modification =	allow_modify;
    x.privilege_level =		privilege_level;
    x.access_rights =		(word_t) access_rights;
    x.phys_addr =		(word_t) phys_addr >> 12;
    x.defer_exceptions =	defer_exceptions;
}


/**
 * Insert a new instruction translation into the inidcated translation
 * register.
 *
 * @param num		registers number
 * @param vaddr		virtual address
 * @param page_size	page size (log2)
 * @param key		protection key
 */
INLINE void
translation_t::put_itr (word_t num,
			addr_t vaddr,
			word_t page_size,
			word_t key)
{
    __asm__ __volatile__ (
	"	mov	r14 = psr ;;		\n"
	"	rsm	psr.ic|psr.i ;;		\n"
	"	srlz.i ;;			\n"
	"	mov	cr.ifa = %[vaddr]	\n"
	"	mov	cr.itir = %[itir]	\n"
	"	srlz.i ;;			\n"
	"	itr.i	itr[%[num]] = %[tr] ;;	\n"
	"	srlz.i ;;			\n"
	"	mov	psr.l = r14 ;;		\n"
	"	srlz.i ;;			\n"
	:
	:
	[num]	"r" (num),
	[tr]	"r" (raw),
	[vaddr]	"r" (vaddr),
	[itir]	"r" ((key << 8) + (page_size << 2))
	:
	"r14");
}


/**
 * Insert a new data translation into the inidcated translation
 * register.
 *
 * @param num		registers number
 * @param vaddr		virtual address
 * @param page_size	page size (log2)
 * @param key		protection key
 */
INLINE void
translation_t::put_dtr (word_t num,
			addr_t vaddr,
			word_t page_size,
			word_t key)
{
    __asm__ __volatile__ (
	"	mov	r14 = psr ;;		\n"
	"	rsm	psr.ic|psr.i ;;		\n"
	"	srlz.i ;;			\n"
	"	mov	cr.ifa = %2		\n"
	"	mov	cr.itir = %3		\n"
	"	srlz.i ;;			\n"
	"	itr.d	dtr[%0] = %1 ;;		\n"
	"	srlz.d				\n"
	"	mov	psr.l = r14 ;;		\n"
	"	srlz.i ;;			\n"
	:
	:
	"r" (num),
	"r" (raw),
	"r" (vaddr),
	"r" ((key << 8) + (page_size << 2))
	:
	"r14");
}


/**
 * Insert code and/or data translation into translation cache.
 *
 * @param type		type (code and/or data)
 * @param vaddr		virtual address
 * @param page_size	page size (log2)
 * @param key		protection key
 */
INLINE void
translation_t::put_tc (type_e type, addr_t vaddr,
		       word_t page_size, word_t key)
{
    __asm__ __volatile__ (
	"	mov	pr = %[type], (3 << 6)	\n"
	"	mov	r14 = psr ;;		\n"
	"	rsm	psr.ic|psr.i ;;		\n"
	"	srlz.d ;;			\n"
	"	mov	cr.ifa = %[vaddr]	\n"
	"	mov	cr.itir = %[itir]	\n"
	"	srlz.i ;;			\n"
	"(p6)	itc.i	%[tr] ;;		\n"
	"(p7)	itc.d	%[tr] ;;		\n"
	"	srlz.i ;;			\n"
	"	mov	psr.l = r14 ;;		\n"
	"	srlz.d ;;			\n"
	:
	:
	[type]	"r" ((word_t) type << 6),
	[tr]	"r" (raw),
	[vaddr]	"r" (vaddr),
	[itir]	"r" ((key << 8) + (page_size << 2))
	:
	"p6", "p7", "r14");
}


/**
 * Purge all instruction translation registers containing a
 * translation overlapping (partly or completely) with the indicated
 * virtual memory region.
 *
 * @param vaddr		virtual address to purge
 * @param page_size	size of region to purge (log2)
 */
INLINE void purge_itr (addr_t vaddr, word_t page_size)
{
    __asm__ __volatile__ (
	"	ptr.i	%0, %1 ;;		\n"
	"	srlz.i ;;			\n"
	:
	:
	"r" (vaddr),
	"r" (page_size << 2));
}


/**
 * Purge all data translation registers containing a translation
 * overlapping (partly or completely) with the indicated virtual
 * memory region.
 *
 * @param vaddr		virtual address to purge
 * @param page_size	size of region to purge (log2)
 */
INLINE void purge_dtr (addr_t vaddr, word_t page_size)
{
    __asm__ __volatile__ (
	"	ptr.d	%0, %1 ;;		\n"
	"	srlz.d ;;			\n"
	:
	:
	"r" (vaddr),
	"r" (page_size << 2));
}


/**
 * Purge all data and instruction translations in the translation
 * cache containing a translation overlapping (partly or completely)
 * with the indicated virtual memory region.
 *
 * @param vaddr		virtual address to purge
 * @param page_size	size of region to purge (log2)
 * @param rid		region id to purge
 */
INLINE void purge_tc (addr_t vaddr, word_t page_size, word_t rid)
{
    __asm__ __volatile__ (
	"	mov	r14 = rr[%[addr]]		\n"
	"	;;					\n"
	"	mov	rr[%[addr]] = %[rid]		\n"
	"	;;					\n"
	"	srlz.d					\n"
	"	;;					\n"
	"	ptc.g	%[addr], %[pgsize]		\n"
	"	;;					\n"
	"	mov	rr[%[addr]] = r14		\n"
	"	;;					\n"
	"	srlz.i					\n"
	"	srlz.d					\n"
	"	;;					\n"
	:
	:
	[addr]   "r" (vaddr),
	[pgsize] "r" (page_size << 2),
	[rid]	 "r" ((rid << 8) + (12 << 2))
	:
	"r14");
}


/**
 * Number of virtual address bits implemented by the CPU.  This number
 * is initialized by a PAL_VM_SUMMARY call.
 */
extern word_t ia64_num_vaddr_bits;


/**
 * Round up to nearest valid page size.
 * @param size		size to match against
 * @return size of next page size (log2) which is larger or equal than
 * given size
 */
INLINE word_t matching_pgsize (word_t size)
{
    word_t valid_sizes = (1 << 12) | (1 << 13) | (1 << 14) | (1 << 16) |
	(1 << 18) | (1 << 20) | (1 << 22) | (1 << 24) | (1 << 28);
    word_t pgsize = 12;

    for (word_t mask = 1 << 12;
	 (mask < size) || ! (mask & valid_sizes);
	 mask <<= 1, pgsize++)
    {
	if (pgsize > 28)
	    return 0;
    }

    return pgsize;
}


#endif /* !__ARCH__IA64__TLB_H__ */
