/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     arch/arm/ptab.h
 * Description:   ARM page table structures
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
 * $Id: ptab.h,v 1.6 2004/06/04 02:14:22 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__ARM__PTAB_H__
#define __ARCH__ARM__PTAB_H__

class arm_l2_desc_t
{
public:
    enum ap_e {
	special	= 0,
	none	= 1,
	ro	= 2,
	rw	= 3,
    };

    union {
	struct {
	    BITFIELD2(word_t,
		zero		: 2,
		ign		: 30
	    )
	} fault;

	struct {
	    BITFIELD9(word_t,
		one		: 2,
		b		: 1,
		c		: 1,
		ap0		: 2,
		ap1		: 2,
		ap2		: 2,
		ap3		: 2,
		sbz		: 4,
		base_address	: 16
	    )
	} large;

	struct {
	    BITFIELD8(word_t,
		two		: 2,
		b		: 1,
		c		: 1,
		ap0		: 2,
		ap1		: 2,
		ap2		: 2,
		ap3		: 2,
		base_address	: 20
	    )
	} small;

	struct {
	    BITFIELD6(word_t,
		three		: 2,
		b		: 1,
		c		: 1,
		ap		: 2,
		sbz		: 4,
		base_address	: 22
	    )
	} tiny;

	u32_t raw;
    };

    // Predicates

    bool is_valid (void)
	{ return fault.zero != 0; }

    bool is_cacheable (void)
	{ return large.c; }

    bool is_bufferable (void)
	{ return large.b; }

    void clear()
	{ raw = 0; }

    addr_t address_large()
	{ return (addr_t)(large.base_address << 16); }

    addr_t address_small()
	{ return (addr_t)(small.base_address << 12); }

    addr_t address_tiny()
	{ return (addr_t)(tiny.base_address << 10); }
};


class arm_l1_desc_t
{
public:
    enum ap_e {
	special	= 0,
	none	= 1,
	ro	= 2,
	rw	= 3,
    };

    union {
	struct {
	    BITFIELD2(word_t,
		zero		: 2,
		ign		: 30
	    )
	} fault;

	struct {
	    BITFIELD5(word_t,
		one		: 2,
		imp		: 3,
		domain		: 4,
		sbz		: 1,
		base_address	: 22
	    )
	} coarse_table;

	struct {
	    BITFIELD9(word_t,
		two		: 2,
		b		: 1,
		c		: 1,
		imp		: 1,
		domain		: 4,
		sbz1		: 1,
		ap		: 2,
		sbz2		: 8,
		base_address	: 12
	    )
	} section;

	struct {
	    BITFIELD5(word_t,
		three		: 2,
		imp		: 3,
		domain		: 4,
		sbz		: 3,
		base_address	: 20
	    )
	} fine_table;

	u32_t raw;
    };
   
    // Predicates

    bool is_valid (void)
	{ return fault.zero != 0; }

    bool is_cacheable (void)
	{ return section.c; }

    bool is_bufferable (void)
	{ return section.b; }

    void clear()
	{ raw = 0; }

    addr_t address_section()
	{ return (addr_t)(section.base_address << 20); }

    word_t address_finetable()
	{ return (fine_table.base_address << 12); }

    word_t address_coarsetable()
	{ return (coarse_table.base_address << 10); }
};

#define ARM_L1_SIZE		((1 << ARM_SECTION_BITS) << 2)
#define ARM_L2_SIZE_NORMAL	((1 << (32 - ARM_SECTION_BITS - PAGE_BITS_4K)) << 2)
#define ARM_L2_SIZE_TINY	((1 << (32 - ARM_SECTION_BITS - PAGE_BITS_1K)) << 2)

#endif /* !__ARCH__ARM__PTAB_H__ */
