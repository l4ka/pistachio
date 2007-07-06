/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, University of New South Wales
 *                
 * File path:   glue/v4-sparc64/ultrasparc/pgent.h
 * Description: Page table manipulation for the UltraSPARC CPU
 *              implementation of SPARC v9.
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
 * $Id: pgent.h,v 1.4 2006/11/17 17:00:38 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_SPARC64__ULTRASPARC__PGENT_H__
#define __GLUE__V4_SPARC64__ULTRASPARC__PGENT_H__

#include <debug.h> /* for UNIMPLEMENTED() */

#include INC_GLUE_API_ARCH(hwspace.h)
#include INC_CPU(mmu.h)
#include INC_CPU(tsb.h)

/**
 *  Note: MDB_SHIFTS and the page table layout are defined in INC_CPU(mmu.h)
 *  as they depend on the mmu.
 */

/***********************
 * Forward declarations *
 ***********************/

class mapnode_t;
class space_t;

struct tsb_data_t;

class pgent_t
{

private:
    union {
	word_t raw;
	struct {
	    BITFIELD5(u64_t,
		      subtree     : 50, /* Pointer to subtree.   */
		      is_subtree  : 1,  /* 1 for valid subtree.  */
		      is_valid    : 1,  /* 0 for subtree.        */
		      __spare     : 9,  /* Can use later.        */		
		      pgsize      : 3   /* Size this entry maps. */

		     ) // BITFIELD5()
	} tree; 

	struct {
	    BITFIELD13(u64_t,
		__rv1       : 7,  /* Defined by tlb_data_t in INC_CPU(tlb.h) */
		access_bits : 3,  /* Page is r/w/x.          Ignored by TLB. */
		ref_bits    : 3,  /* Page has been r/w/x.    Ignored by TLB. */
		__rv2       : 37, /* Defined by tlb_data_t.                  */
		is_subtree  : 1,  /* 0 for leaf entry.       Ignored by TLB. */
		is_valid    : 1,  /* 1 for valid leaf entry. Ignored by TLB. */
		in_itsb     : 1,  /* Entry exists in I-TSB.  Ignored by TLB. */
		in_dtsb     : 1,  /* Entry exists in D-TSB.  Ignored by TLB. */
		__spare     : 4,  /* Can use later.          Ignored by TLB. */
		__rv3       : 1,  /* Defined by tsb_data_t.  Ignored by TLB. */
		__rv4       : 2,  /* Defined by tlb_data_t.                  */
		pgsize      : 2,  /* See pgsize_e below (4m & smaller only). */
		__rv5       : 1   /* Defined by tlb_data_t.                  */

		      ) // BITFIELD13()
	} leaf;

    }; // union

public:

    enum pgsize_e {
	size_8k = 0,
	size_64k,  // 1, HW supported
	size_512k, // 2, HW supported
	size_4m,   // 3, HW supported
	size_16m,  // 4, Level 3/5, 24 bits
	size_16g,  // 5, Level 2/4, 34 bits

#if (SPARC64_VIRTUAL_ADDRESS_BITS == 44)

	size_16t,  // 6, Level 1, 44 bits
	size_max = size_16g

#elif (SPARC64_VIRTUAL_ADDRESS_BITS == 64)

	size_16t,  // 6, Level 3, 44 bits
	size_16p,  // 7, Level 2, 54 bits
	size_16e,  // 8, Level 1, 64 bits
	size_max = size_16p

#else

#warning Currently only support 44 or 64 bit virtual address spaces!

#endif /* SPARC64_VIRTUAL_ADDRESS_BITS == * */

    }; // pgsize_e

    /**********
     * Methods *
     **********/

public:

    tsb_data_t* tsb_data(void);

    // Predicates

    bool is_valid(space_t * s, pgsize_e pgsize);
    bool is_subtree(space_t * s, pgsize_e pgsize);
    bool is_readable(space_t * s, pgsize_e pgsize);
    bool is_writable(space_t * s, pgsize_e pgsize);
    bool is_executable(space_t * s, pgsize_e pgsize);
    bool is_kernel(space_t * s, pgsize_e pgsize);

    // Retrieval

    addr_t address(space_t * s, pgsize_e pgsize);
    pgent_t * subtree(space_t * s, pgsize_e pgsize);
    mapnode_t * mapnode(space_t * s, pgsize_e pgsize, addr_t vaddr);
    addr_t vaddr(space_t * s, pgsize_e pgsize, mapnode_t * map);
    word_t reference_bits(space_t * s, pgsize_e pgsize, addr_t vaddr);
    word_t attributes(space_t * s, pgsize_e pgsize);

    // Modification

    void clear(space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    void insert(space_t * s, pgent_t::pgsize_e pgsize,
		addr_t vaddr, tsb_t::tsb_e tsb);
    void flush(space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr);
    void make_subtree(space_t * s, pgsize_e pgsize, bool kernel);
    void remove_subtree(space_t * s, pgsize_e pgsize, bool kernel);
    void set_entry(space_t * s, pgsize_e pgsize, addr_t paddr,
		   word_t rwx, word_t attrib, bool kernel);
    void revoke_rights(space_t * s, pgsize_e pgsize, word_t rwx);
    void update_rights(space_t * s, pgsize_e pgsize, word_t rwx);
    void reset_reference_bits(space_t * s, pgsize_e pgsize);
    void update_reference_bits(space_t *s, pgsize_e pgsize, word_t rwx);
    void set_attributes(space_t * s, pgsize_e pgsize, word_t attrib);
    void set_linknode(space_t * s, pgsize_e pgsize, 
		      mapnode_t * map, addr_t vaddr);

    // Movement

    pgent_t * next(space_t * s, pgsize_e pgsize, word_t num);

private:
    // Linknode access

    u64_t get_linknode(void);
    void set_linknode(u64_t val);

}; // pgent_t


#endif /* !__GLUE__V4_SPARC64__ULTRASPARC__PGENT_H__ */

