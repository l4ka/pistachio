/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     api/v4/fpage.h
 * Description:   V4 flexpages
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
 * $Id: fpage.h,v 1.26 2006/11/14 18:46:31 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__FPAGE_H__
#define __API__V4__FPAGE_H__

#include INC_API(config.h)
#include INC_GLUE(fpage.h)

class mempage_t 
{
public:
    union {
	struct {
	    BITFIELD7(word_t,
		      execute		: 1,
		      write		: 1,
		      read		: 1,
		      reserved		: 1,
		      size		: 6,
		      base		: L4_FPAGE_BASE_BITS,
		      : BITS_WORD - L4_FPAGE_BASE_BITS - 10
		);
	} x __attribute__((packed));
	word_t raw;
    };
};

/**
 * Flexpages are size-aligned memory objects and can cover 
 * multiple hardware pages. fpage_t implements the V4 specific
 * flexpage type, having read, write and execute bits.
 */
class fpage_t
{
    /* data members */
public:
    union {
	mempage_t mem;
	arch_fpage_t arch;
	word_t raw;
    };
    /* member functions */
public:
    /**
     * sets the flexpage
     */
    void set(word_t base, word_t size, bool read, bool write, bool exec)
	{
	    if (EXPECT_FALSE(arch.is_valid_page() == false))
	    {
		raw = 0;
		mem.x.base = (base & (~0UL << size)) >> 10;
		mem.x.size = size;
		mem.x.read = read;
		mem.x.write = write;
		mem.x.execute = exec;
	    }
	    else 
	    {
		arch.set(base, size, read, write, exec);
	    }
	}
    
    /**
     * @return true if the flexpage is a nil fpage
     */
    bool is_nil_fpage() 
	{ return raw == 0; }
    
    /**
     * @return true if the flexpage is a memory page
     */
    bool is_mempage (void)
	{ return arch.is_valid_page() == false; }


    /**
     * @return true if the flexpage is an architecture specific page
     */
    bool is_archpage (void)
	{ return arch.is_valid_page() == true; }


    /**
     * @return true if flexpage covers the whole address space
     * the complete fpage is a special value defined in the V4 API
     */
    bool is_complete_mempage() 
	{ return is_mempage () && mem.x.size == 1 && mem.x.base == 0; }

    bool is_complete_fpage() 
	{ return (is_mempage () && is_complete_mempage()) ||
		(arch.is_valid_page () && arch.is_complete_page()); }
    
    /**
     * checks if a given address range is contained completely within
     * the fpage
     * 
     * @param start start address of range
     * @param end end address of range
     * @return true if range is contained within fpage, false otherwise */
    bool is_range_in_fpage(addr_t start, addr_t end)
	{
	    /* VU: should we make sure, that base is valid? */
	    return (is_complete_fpage()) ||
		(get_address() <= start && addr_offset(get_address(), get_size()) >= end);
	}
    
    /**
     * @param addr returns true if addr is within the range of 
     * the flexpage
     */
    bool is_addr_in_fpage(addr_t addr)
	{ return is_range_in_fpage (addr, (addr_t)((word_t)addr + sizeof(addr_t))); }

    /**
     * checks if the current and the given fpage overlap
     * @return true if fpages overlap
     */
    bool is_overlapping(fpage_t fpage)
	{
	    if (is_complete_fpage ())
		return true;
	    else if (fpage.get_address() < this->get_address())
		return (addr_offset(fpage.get_address(), fpage.get_size()) >
			this->get_address());
	    else 
		return (addr_offset(this->get_address(), this->get_size()) >
			fpage.get_address());
	}

    bool is_range_overlapping(addr_t start, addr_t end)
	{
	    if (is_complete_fpage ())
		return true;
	    else if (start < get_address())
		return end > get_address();
	    else
		return addr_offset(get_address(), get_size()) > start;
	}

    /**
     * @return base address of the fpage
     * get_base does not size-align the address
     */
    addr_t get_base() 
	{ return is_mempage() ? (addr_t)(mem.x.base << 10) : arch.get_base(); }

    /**
     * @return size aligned address of the fpage
     */
    addr_t get_address()
	{ return is_mempage() 
		? (addr_t)((mem.x.base << 10) & (~0UL << mem.x.size))
		: (addr_t)(arch.get_address());
	}
    
    
    /**
     * @return size of the flexpage
     */
    word_t get_size() 
	{ return is_mempage() ? (1UL << mem.x.size) : arch.get_size(); }

    /**
     * @return log2 size of the fpage
     */
    word_t get_size_log2() 
	{ return is_mempage() 
		? is_complete_mempage() ? sizeof(word_t) * 8 : mem.x.size
		: arch.get_size_log2(); 
	}

    /**
     * @return access rights of fpage
     */
    word_t get_access (void)
	   { return get_rwx(); }

    /**
     * @return true if the read bit is set
     */  
    bool is_read() 
	{ return mem.x.read; }

    /** 
     * @return true if the write bit is set
     */
    bool is_write() 
	{ return mem.x.write; }

    /**
     * @return true if the execute bit is set
     */
    bool is_execute() 
	{ return mem.x.execute; }

    /**
     * @return true if read, write and execute bits are set
     */
    bool is_rwx()
	{ return mem.x.read && mem.x.write && mem.x.execute; }

    /**
     * sets all permission bits in the fpage
     */
    void set_rwx()
	{ mem.x.read = 1; mem.x.write = 1; mem.x.execute = 1; }

    void set_rwx(word_t rwx)
	{ raw = (raw & ~7) | (rwx & 7); }

    /**
     * @return access rights of fpage
     */
    word_t get_rwx()
	{ return raw & 7; }

    /**
     * @return delivers an fpage covering the complete address space
     */
    static fpage_t complete_mem()
	{
	    fpage_t ret;
	    ret.raw = 0;
	    ret.mem.x.size = 1;
	    return ret;
	}


   /**
     * @return delivers an fpage covering the complete architecture-specific
     * address space
     */
    static fpage_t complete_arch ()
	{
	    fpage_t ret;
	    ret.arch = arch_fpage_t::complete();
	    return ret;
	}

    /**
     * @return delivers an nil fpage
     */
    static fpage_t nilpage()
	{
	    fpage_t ret;
	    ret.raw = 0;
	    return ret;
	}
    

};


/*
 * Helper functions used in conjunction with mapping.
 */

INLINE word_t base_mask (fpage_t fp, word_t size)
{
    return ((~0UL) >> ((sizeof (word_t) * 8) - fp.get_size_log2 ())) &
	((size == 0 ? (~0UL) : ~((~0UL) >> ((sizeof (word_t) * 8) - size))));
}

INLINE addr_t address (fpage_t fp, word_t size)
{
    return (addr_t) ((word_t) fp.get_base () & ~((1UL << size) - 1));
}


#endif /* !__API__V4__FPAGE_H__ */
