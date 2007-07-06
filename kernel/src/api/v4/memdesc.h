/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     api/v4/memdesc.h
 * Description:   Memory descriptors for kernel interface page
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
 * $Id: memdesc.h,v 1.8 2004/03/15 21:33:59 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__MEMDESC_H__
#define __API__V4__MEMDESC_H__


/**
 * Descriptor for a memory region described in the kernel interface
 * page.  Regions have a type, an upper, a lower limit (multiple of
 * 1K), and are valid for either virtual memory or physical memory.
 */
class memdesc_t
{
    BITFIELD5(word_t,
	_type	: 4,
	_t	: 4,
		: 1,
	_v	: 1,
	_low	: BITS_WORD - 10
	);
    BITFIELD2(word_t,
		: 10,
	_high	: BITS_WORD - 10
	);

public:

    enum type_e {
	undefined	= 0x0,
	conventional	= 0x1,
	reserved	= 0x2,
	dedicated	= 0x3,
	shared		= 0x4,
	max_type
    };

    /**
     * @return type of memory region
     */
    type_e type (void)
	{ return (type_e) _type; }


    /**
     * @return subtype of memory region
     */
    word_t subtype (void)
	{ return (word_t) _t; }


    /**
     * @return true if region is in virtual memory, false otherwise
     */
    bool is_virtual (void)
	{ return _v; }


    /**
     * @return lower address of memory region
     */
    addr_t low (void)
	{ return (addr_t) (_low << 10); }


    /**
     * @return upper address of memory region
     */
    addr_t high (void)
	{ return (addr_t) ((_high << 10) + 0x3ff); }


    /**
     * @return size of memory region in bytes
     */
    word_t size (void)
	{ return ((_high-_low+1) << 10); }


    /**
     * Modify memory descriptor.
     * @param type	new type of descriptor
     * @param t		new subtype of descriptor
     * @param virt	is new descriptor for vitual memory
     * @param low	new lower address of descriptor
     * @param high	new upper address of descriptor
     */
    void set (type_e type, word_t t, bool virt, addr_t low, addr_t high)
	{
	    _type = type;
	    _t    = t;
	    _v    = virt;
	    _low  = ((word_t) low) >> 10;
	    _high = ((word_t) high) >> 10;
	}

    void set (memdesc_t & memdesc)
	{
	    *this = memdesc;
	}
};



#endif /* !__API__V4__MEMDESC_H__ */
