/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/ultrasparc/tsb.h
 * Description:  TSB class for the UltraSPARC CPU implmenetation
 *               of SPARC v9. The Translation Storage Buffer (TSB)
 *               is basically a software TLB.
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
 * $Id: tsb.h,v 1.4 2004/05/21 02:34:52 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ULTRASPARC__TSB_H__
#define __ARCH__SPARC64__ULTRASPARC__TSB_H__

#include <debug.h>

#include INC_CPU(tlb.h)

/***************************************************************************
 * Notes: The TSB is where the MMU's 'fast' trap handlers look to handle    *
 * TLB faults. In the advent of a TSB miss, etc these inline handlers fall  *
 * back to C++ code that accesses the page tables and generate page faults, *
 * etc. The TSB field vpn_short does not contain bits vpn[21:13] as these   *
 * are implied by the TSB index.                                            *
 ***************************************************************************/

#warning awiggins (02-09-03): Should make TSB_ARRAY_BITS a configuration option.
#define TSB_ARRAY_SIZE_BITS 0                     /* [0..7] */
#define TSB_ARRAY_BITS      (9 + TSB_ARRAY_SIZE_BITS)
#define TSB_ARRAY_SIZE      (1 << TSB_ARRAY_BITS) /* # entries in each array. */

/* 4 arrays * array size * 16 bytes per entry. */
#define TSB_SIZE (4 * TSB_ARRAY_SIZE * 16)

#ifndef ASSEMBLY

#define TSB_DATA_ENTRY TLB_DATA_ENTRY

#define TSB_VPN_SHORT_SHIFT 22

/**
 *  tsb_tag_t: TSB entry tag field.
 */
struct tsb_tag_t {
    BITFIELD5(u64_t,
	      vpn_short   : 42, /* Short virtual page number, vpn[63:22]. */
	      __reserved1 : 6,  /* Reserved by the CPU implementation.    */
	      context     : 13, /* Context identifier (ASID) of tte.      */
	      __reserved2 : 2,  /* Reserved by the CPU implementation.    */
	      global      : 1   /* Is this mapping global to all spaces.  */

	     ) // BITFIELD5()

};

/**
 *  tsb_data_t: TSB entry data field.
 */
struct tsb_data_t {
    TSB_DATA_ENTRY

};

/**
 *  tsbent_t: TSB entry.
 */
class tsbent_t { 
private:
    /* The Tag field of a TSB entry. */
    union {
	u64_t     tag_raw;
	tsb_tag_t tag;

    }; // union

    /* The data field of a TSB entry. */
    union {
	u64_t      data_raw;
	tsb_data_t data;

    }; // union

public:
    /* Tag field manipulation. */

    void set_va(addr_t vaddr)
    {
	tag.vpn_short = (u64_t)vaddr >> TSB_VPN_SHORT_SHIFT;
    }
    void set_asid(hw_asid_t asid) {tag.context = (u64_t)asid;}
    addr_t get_va(void) {return (addr_t)(tag.vpn_short << TSB_VPN_SHORT_SHIFT);}
    addr_t get_va(u16_t tsb_index)
    {
	// Calculate the entire VA of this entry, including the part that
	// depends on its index in the tsb
	if(data.size == tlb_t::size_8k)
	    return (addr_t)((word_t)get_va() |
			    (tsb_index << SPARC64_PAGE_BITS));
	else
	    return (addr_t)((word_t)get_va() |
			    (tsb_index << (SPARC64_PAGE_BITS + 3)));
    }
    hw_asid_t get_asid(void) {return (hw_asid_t)tag.context;}

    /* Data field manipulation. */

    void set_global(bool g) {data.global = g; tag.global = g;}
    void set_privileged(bool p) {data.privileged = p;}
    void set_side_effect(bool e) {data.side_effect = e;}
    void set_lock(bool l) {data.tsb_lock = 1;}
    void set_invert_endian(bool i) {data.invert_endian = i;}
    void set_no_fault_only(bool n) {data.no_fault_only = n;}
    void set_access_bits(word_t rwx) {data.access_bits = rwx;}
    void set_ref_bits(word_t rwx) {data.ref_bits = rwx;}
    void set_size(tlb_t::pgsize_e size) {data.size = size;}
    void set_cache_attrib(tlb_t::cache_attrib_e attrib)
    {
	data.cache_attrib = attrib;
    }
    void set_pa(addr_t paddr) {data.pfn = (u64_t)paddr >> SPARC64_PAGE_BITS;}
    void set_data(tsb_data_t* newdata) { data = *newdata; }
    bool get_global(void) {return (bool)data.global;}
    bool get_privileged(void) {return (bool)data.privileged;}
    bool get_side_effect(void) {return (bool)data.side_effect;}
    bool get_lock(void) {return (bool)data.tsb_lock;}
    bool get_invert_endian(void) {return (bool)data.invert_endian;}
    bool get_no_fault_only(void) {return (bool)data.no_fault_only;}
    word_t get_access_bits(void) {return (word_t)data.access_bits;}
    word_t get_ref_bits(void) {return (word_t)data.ref_bits;}
    tlb_t::pgsize_e get_size(void) {return (tlb_t::pgsize_e)data.size;}
    tlb_t::cache_attrib_e get_cache_attrib(void)
    {
	return (tlb_t::cache_attrib_e)data.cache_attrib;
    }
    addr_t get_pa(void) {return (addr_t)(data.pfn << SPARC64_PAGE_BITS);}
    bool get_tlb_valid(void) { return data.tlb_valid; }

    /* Invalid TSB entry. */

    void clear(void)
    {
	data_raw = 0;
	tag_raw = 0;
    }

    /* Printing. */

    /* Index is used to calculate va since TSB vpn is incomplete. */
    void print(u16_t tsb_index)
    {
	printf("va: 0x%lx \tasid: 0x%x \tpa: 0x%lx \t%c%c%c%c%c%c access: %c%c%c reference: %c%c%c CA:%d %s",
	       (word_t)get_va(tsb_index),
	       (word_t)get_asid(),
	       (word_t)get_pa(),
	       get_global()        ? 'G' : 'g',
	       get_privileged()    ? 'P' : 'p',
	       get_side_effect()   ? 'E' : 'e',
	       get_lock()          ? 'L' : 'l',
	       get_invert_endian() ? 'I' : 'i',
	       get_no_fault_only() ? 'N' : 'n',
	       (get_access_bits() & READ_ACCESS_BIT)    ? 'R' : 'r',
	       (get_access_bits() & WRITE_ACCESS_BIT)   ? 'W' : 'w',
	       (get_access_bits() & EXECUTE_ACCESS_BIT) ? 'X' : 'x',
	       (get_ref_bits() & READ_ACCESS_BIT)    ? 'R' : 'r',
	       (get_ref_bits() & WRITE_ACCESS_BIT)   ? 'W' : 'w',
	       (get_ref_bits() & EXECUTE_ACCESS_BIT) ? 'X' : 'x',
	       (int)get_cache_attrib(),
	       (get_size() == tlb_t::size_8k)     ? "8KB" :
	       ((get_size() == tlb_t::size_64k)   ? "64KB" :
		((get_size() == tlb_t::size_512k) ? "512KB" : "4MB")));

    } // print()

}; // tsbent_t

/** 
 *  tsbarrays_t: TSB arrays
 *  awiggins (02-09-03): Do not touch these fields without consulting me.
 */
typedef struct {
    tsbent_t d8k[TSB_ARRAY_SIZE];  /* D-TSB array for 8KB pages only.   */
    tsbent_t d64k[TSB_ARRAY_SIZE]; /* D-TSB array for other sizes.      */
    tsbent_t i8k[TSB_ARRAY_SIZE];  /* I-TSB array for 8KB pages only.   */
    tsbent_t i64k[TSB_ARRAY_SIZE]; /* I-TSB array for other page sizes. */

} tsbarrays_t;

/**
 *  I/D-TSB Registers, point to TSB arrays for index calculation.
 */
class tsb_reg_t {
private:
    union {
	u64_t raw;
	struct {
	    BITFIELD4(u64_t,
		      size  : 3,  /* TSB arrays size.                       */
		      __rv  : 9,  /* unused.                                */
		      split : 1,  /* Is array split into seperate 8KB/64KB. */
		      base  : 51  /* TSB array base pointer.                */

		     ) // BITFIELD4()
	} reg;

    }; // union 

public:
    /* I/D-TSB register management. */

    void set(tlb_t::tlb_e tsb)
    {
	ASSERT(tsb != tlb_t::no_tlb);

	if(tsb & tlb_t::d_tlb) { // D-TSB register.

	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 : /* no outputs */
				 : "r" (raw),          // %0
				 "r" (TSB_REGISTER), // %1
				 "i" (ASI_DMMU));    // %2

	} else if(tsb & tlb_t::i_tlb) { // I-TSB register.

	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 : /* no outputs */
				 : "r" (raw),          // %0
				 "r" (TSB_REGISTER), // %1
				 "i" (ASI_IMMU));    // %2

	} else {ASSERT(false);}

    } // set()

    void get(tlb_t::tlb_e tsb)
    {
	ASSERT(tsb != tlb_t::no_tlb);

	if(tsb & tlb_t::d_tlb) { // D-TSB register.

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (raw)          // %0
				 : "r" (TSB_REGISTER), // %1
				 "i" (ASI_DMMU));    // %2

	} else if(tsb & tlb_t::i_tlb) { // I-TSB register.

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (raw)          // %0
				 : "r" (TSB_REGISTER), // %1
				 "i" (ASI_IMMU));    // %2

	} else {ASSERT(false);}

    } // get()

    /* Field manipulation. */

    void set_base(addr_t base) {reg.base = (u64_t)base >> SPARC64_PAGE_BITS;}
    void set_split(bool split) {reg.split = split;}
    void set_size(u8_t size) {reg.size = size;}

    /* Printing. */

    void print(void)
    {
	printf("base: 0x%lx %s size %d",
	       reg.base << SPARC64_PAGE_BITS,
	       reg.split ? "split" : "unified",
	       1 << 9 + reg.size);
    }

}; // tsb_reg_t

/**
 *  Manages I/D-TSB entries.
 */
class tsb_t {
private:
    static tsbarrays_t arrays;
    tsb_reg_t d_reg; // D-TSB Register.
    tsb_reg_t i_reg; // I-TSB REgister.


public:
    /* Which TSB array. */
    enum tsb_e {
	d8k_tsb  = 1, /* 8KB page D-TSB.     */
	d64k_tsb = 2, /* Other D-TSB.        */
	i8k_tsb  = 3, /* 8KB page I-TSB.     */
	i64k_tsb = 4  /* Other I-TSB.        */
    };

public:
    /* I/D-TSB management. */

    void init(void)
    {
	d_reg.set_base((addr_t)arrays.d8k);
	d_reg.set_split(true);
	d_reg.set_size(TSB_ARRAY_SIZE_BITS);
	d_reg.set(tlb_t::d_tlb);
	i_reg.set_base((addr_t)arrays.i8k);
	i_reg.set_split(true);
	i_reg.set_size(TSB_ARRAY_SIZE_BITS);
	i_reg.set(tlb_t::i_tlb);

	for(int i = 0; i < TSB_ARRAY_SIZE; i++) {
	    arrays.d8k[i].clear();
	    arrays.d64k[i].clear();
	    arrays.i8k[i].clear();
	    arrays.i64k[i].clear();
	}
    }

    /* Calculate index into TSB arrays. */
    static u16_t index(addr_t vaddr, tlb_t::pgsize_e size)
    {
	// index for 8k and 64k sizes is calculated by the MMU hardware. This
	// function emulates that calculation. 
	if(size == tlb_t::size_8k) {
	    return ((word_t)vaddr >> SPARC64_PAGE_BITS) % TSB_ARRAY_SIZE;
	} else {
	    // Larger page sizes are treated as if they are 64kb (so there might
	    // be multiple TSB entries in the 64kb TSB referring to one >64kb
	    // superpage)
	    return ((word_t)vaddr >> (SPARC64_PAGE_BITS + 3)) % TSB_ARRAY_SIZE;
	}

	ASSERT(false);
	return 0;
    }

    /* Get a pointer to an entry in either D-TSB or I-TSB. */
    static void get(u16_t index, tsbent_t** entry, tsb_e tsb)
    {
	ASSERT(index < TSB_ARRAY_SIZE);

	switch(tsb) {
	case d8k_tsb:
	    *entry = &arrays.d8k[index];
	    break;
	case d64k_tsb:
	    *entry = &arrays.d64k[index];
	    break;
	case i8k_tsb:
	    *entry = &arrays.i8k[index];
	    break;
	case i64k_tsb:
	    *entry = &arrays.i64k[index];
	    break;
	default:
	    ASSERT(false);
	}
    }

    /* Printing. */

    /* Dump TSB state. */
    void print(void)
    {
	d_reg.get(tlb_t::d_tlb);
	i_reg.get(tlb_t::i_tlb);

	printf("D-TSB - "), d_reg.print(), printf("\nI-TSB - "), i_reg.print(),
	printf("\n");

    } // print()

    /* Dump all TSB(s) entries. */
    static void print(tsb_e tsb)
    {
	if(tsb == d8k_tsb) {
	    for(int i = 0; i < TSB_ARRAY_SIZE; i++) {print(i, d8k_tsb);}
	}
	if(tsb == d64k_tsb) {
	    for(int i = 0; i < TSB_ARRAY_SIZE; i++) {print(i, d64k_tsb);}
	}
	if(tsb == i8k_tsb) {
	    for(int i = 0; i < TSB_ARRAY_SIZE; i++) {print(i, i8k_tsb);}
	}
	if(tsb == i64k_tsb) {
	    for(int i = 0; i < TSB_ARRAY_SIZE; i++) {print(i, i64k_tsb);}
	}

    } // print(tsb)

    /* Dump a single TSB entry. */
    static void print(u16_t entry, tsb_e tsb)
    {
	ASSERT(entry < TSB_ARRAY_SIZE);

	if(tsb == d8k_tsb) {
	    if(arrays.d8k[entry].get_access_bits()) {
		printf("D-TSB-8K[%d] ", entry), 
		arrays.d8k[entry].print(entry), printf("\n");
	    }
	} else if(tsb == d64k_tsb) {
	    if(arrays.d64k[entry].get_access_bits()) {
		printf("D-TSB-64K[%d] ", entry), 
		arrays.d64k[entry].print(entry), printf("\n");
	    }
	} else if(tsb == i8k_tsb) {
	    if(arrays.i8k[entry].get_access_bits()) {
		printf("I-TSB-8K[%d] ", entry), 
		arrays.i8k[entry].print(entry), printf("\n");
	    }
	} else if(tsb == i64k_tsb) {
	    if(arrays.i64k[entry].get_access_bits()) {
		printf("I-TSB-64K[%d] ", entry), 
		arrays.i64k[entry].print(entry), printf("\n");
	    }
	} else {

	    ASSERT(false);
	}

    } // print(entry, tsb)

}; // tsb_t

#endif /* !ASSEMBLY */


#endif /* !__ARCH__SPARC64__ULTRASPARC__TSB_H__ */
