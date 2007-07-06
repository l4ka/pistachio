/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/rr.h
 * Description:   Handling of region registers.
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
 * $Id: rr.h,v 1.8 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__RR_H__
#define __ARCH__IA64__RR_H__



#define IA64_RR_BASE(rr)	(((word_t) (rr)) << 61)
#define IA64_RR_MASK(addr)	((word_t) (addr) & ((1L << 61) - 1))
#define IA64_RR_NUM(addr)	(((word_t) (addr)) >> 61)

/**
 * Turn physical address into region specific address.
 *
 * @param rr		region number
 * @param addr		physical address
 *
 * @return address translated into indicated region.
 */
INLINE addr_t ia64_phys_to_rr (int rr, addr_t addr)
{
    return (addr_t) (IA64_RR_BASE (rr) + (word_t) IA64_RR_MASK (addr));
}


/**
 * Layout for region registers
 */
class rr_t
{
    union {
	struct {
	    word_t _vhpt_enable		: 1;
	    word_t __rv1		: 1;
	    word_t _page_size		: 6;
	    word_t _region_id		: 24;
	    word_t __rv			: 32;
	};
	word_t raw;
    };

public:

    //
    // Retrieval
    //

    /**
     * @return true if VHPT is enabled; false otherwise.
     */
    inline bool is_vhpt_enabled (void)
	{ return _vhpt_enable; }

    /**
     * @return preferred page size (log2) for region.
     */
    inline word_t page_size_log2 (void)
	{ return _page_size; }

    /**
     * @return preferred page size (in bytes) for region.
     */
    inline word_t page_size (void)
	{ return 1UL << _page_size; }

    /**
     * @return region id.
     */
    inline word_t region_id (void)
	{ return _region_id; }

    /**
     * @return actual bit-contents of region register.
     */
    operator word_t (void)
	{ return raw; }


    //
    // Modification
    //

    /**
     * Enable VHPT walker for region.
     */
    inline void enable_vhpt (void)	
	{ _vhpt_enable = 1; }

    /**
     * Disable VHPT walker for region.
     */
    inline void disable_vhpt (void)	
	{ _vhpt_enable = 0; }

    /**
     * Set preferred page size for region.
     * @param size_log2		new preferred page size (log2)
     */
    inline void set_page_size (word_t size_log2)
	{ _page_size = size_log2; }

    /**
     * Set region id.
     * @param rid		new region id
     */
    inline void set_region_id (word_t rid)
	{ _region_id = rid; }

    /**
     * Initialize region register contents.
     * @param vhpt_enabled	enable VHPT walker for region
     * @param rid		region id
     * @param size_log2		preferred page size (log2)
     */
    void set (bool vhpt_enabled, word_t rid, word_t size_log2)
	{ 
	    raw = 0;
	    _region_id = rid;
	    _vhpt_enable = vhpt_enabled ? 1 : 0;
	    _page_size = size_log2;
	}


    //
    // Creation
    //

    /**
     * Create new region register with undefined contents.
     */
    rr_t (void) {}

    /**
     * Create an initialized region register.
     * @param vhpt_enabled	enable VHPT walker for region
     * @param rid		region id
     * @param size_log2		preferred page size (log2)
     */
    rr_t (bool vhpt_enabled, word_t rid, word_t size_log2)
	{ 
	    raw = 0;
	    _region_id = rid;
	    _vhpt_enable = vhpt_enabled ? 1 : 0;
	    _page_size = size_log2;
	}


    //
    // Register access
    //

    inline void get (word_t rr_num);
    inline void put (word_t rr_num);
};

/**
 * Load values from indicated region register.
 * @param rr_num	region register to read
 */
INLINE void rr_t::get (word_t rr_num)
{
    __asm__ __volatile__ (
	"	mov	%0 = rr[%1]	\n"
	:
	"=r" (raw)
	:
	"r" (IA64_RR_BASE (rr_num)));
}

/**
 * Store values into indicated region register.
 * @param rr_num	region register to modify
 */
INLINE void rr_t::put (word_t rr_num)
{
    __asm__ __volatile__ (
	"	mov	rr[%1] = %0	\n"
	:
	:
	"r" (raw),
	"r" (IA64_RR_BASE (rr_num)));
}

/**
 * Read values from indicated region register.
 * @param rr_num	region register to read
 * @return value of indicated region register.
 */
INLINE rr_t get_rr (word_t rr_num)
{
    rr_t ret;

    __asm__ __volatile__ (
	"	mov	%0 = rr[%1]	\n"
	:
	"=r" (ret)
	:
	"r" (IA64_RR_BASE (rr_num)));

    return ret;
}


#endif /* !__ARCH__IA64__RR_H__ */
