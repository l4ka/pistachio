/*********************************************************************
 *                
 * Copyright (C) 2005-2006, 2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/io_fpage.h
 * Description:   IO fpage declaration
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
 * $Id: io_fpage.h,v 1.5 2006/02/21 08:43:57 stoess Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__IO_FPAGE_H__
#define __GLUE__V4_X86__IO_FPAGE_H__
#if !defined(CONFIG_X86_IO_FLEXPAGES)

#include INC_API(generic-archfpage.h)

#else

#include INC_API(config.h)

class space_t;
class fpage_t;
class tcb_t;

/**
 * Flexpages are size-aligned memory objects and can cover 
 * multiple hardware pages. fpage_t implements the V4 specific
 * flexpage type, having read, write and execute bits.
 */
class arch_fpage_t
{
    /* data members */
public:
    union{
	struct {
	    BITFIELD5(word_t,
		      reserved          : 4,
		      two               : 6,
		      size              : 6,
		      base              :16,
		      : BITS_WORD - 32
		);
	} io __attribute__((packed));
	word_t raw;
    } x;
    /* member functions */
public:

    /**
     * sets the flexpage
     */
    void set(word_t base, word_t log2size, bool read, bool write, bool exec)
	{
	    x.raw = 0;
	    x.io.two = 2;
	    x.io.base = (base & (~0UL << log2size));
	    x.io.size = log2size;
	}

    /**
     * @return true if the flexpage is a nil fpage
     */
    bool is_valid_page (void)
	{ return x.io.two == 2; }

    /**
     * @return true if flexpage covers the whole I/O address space
     */
    bool is_complete_page() 
	{ return (x.io.size == 16 && x.io.base == 0);}

    /**
     * @return port of the IO fpage
     */
    u16_t get_port() 
	{ return  (x.io.base); }

    /**
     * @return base address of the fpage
     * get_base does not size-align the address
     */
    addr_t get_base() 
	{ return (addr_t) (x.io.base); }

    /**
     * @return size aligned address of the fpage
     */
    addr_t get_address()
	{ 
	    return (addr_t)(x.io.base & (~0UL << x.io.size));
	}
    
    /**
     * @return size of the flexpage
     */
    word_t get_size() 
	{ return (1UL << x.io.size); }

    /**
     * @return log2 size of the fpage
     */
    word_t get_size_log2() 
	{ return x.io.size; }
    
    /**
     * @return true if the read bit is set
     */
    bool is_read() 
	{ return true; }

    /** 
     * @return true if the write bit is set
     */
    bool is_write() 
	{ return true; }

    /**
     * @return true if the execute bit is set
     */
    bool is_execute() 
	{ return true; }

    /**
     * @return true if read, write and execute bits are set
     */
    bool is_rwx()
	{ return true; }

    /**
     * sets all permission bits in the fpage
     */
    void set_rwx() { }

    void set_rwx(word_t rwx) { }

    /**
     * @return access rights of fpage
     */
    word_t get_rwx() 	{ return true; }

    /**
     * @return delivers an fpage covering the complete IO address space
     */
    static arch_fpage_t complete()
	{
	    arch_fpage_t ret;
	    ret.x.raw = 0;
	    ret.x.io.two = 2;
	    ret.x.io.size = 16;
	    return ret;
	}

};
#endif /* !defined(CONFIG_X86_IO_FLEXPAGES) */


#endif /* !__GLUE__V4_X86__IO_FPAGE_H__ */
