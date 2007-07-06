/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/ultrasparc/mmu.h
 * Description:  MMU specifics of the UltraSPARC CPU implmenetation
 *               of SPARC v9.
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
 * $Id: mmu.h,v 1.6 2004/02/22 23:04:20 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ULTRASPARC__MMU_H__
#define __ARCH__SPARC64__ULTRASPARC__MMU_H__

#include <config.h>

/*******************************************************************************
* Notes:                                                                       *
*                                                                              *
* - UltraSPARC I/II support a 44 bit virtual address space while UltraSPARC    *
* III supports the ful 64 bit address space. So we use a 3 and 5 level page    *
* tables respectively. All support 4 page sizes of 8KB, 64KB, 512KB and 4MB.   *
* All page table levels are 10 bits worth of VA indexing to have 1024 entries  *
* except the leaf levels which are 11 bits and twice the size to accomidate    *
* the shadow page table (SPT) for the mapping database making 4096 entires.    *
*                                                                              *
* - The UltraSPARC while using a software loaded TLB provides some support for *
* a Translation Storage Buffer (TSB) which is effectively a software TLB. This *
* support comes in the form of a precalculated hash index which can be used to *
* provide indexing into seperate I/D-TSBs, which can be split 8K/64K page size *
* support and even pseudo set-associative. Although the TLB entry doesn't      *
* support orthogonal execute/read permissions, they can be encoded by setting  *
* the I/D-TSB entries for the same page differently based on the permissions   *
* defined by software. The TSB sizes supported are 512 - 64K entries.          *
* We use seperate I/D-TSBs which are each split in to 8K/64K page size support *
* regions. The 64K region also holds 512K and 4M page mappings.                *
*                                                                              *
* - The TSB is stored in the CPU local data and a pinned mapping for CPU local *
*   data is used.                                                              *
*                                                                              *
* - Since the UltraSPARC MMU provides no way to access untranslated            *
* instruction references when the MMU is enabled and only limited access to    *
* untranslated data references (uncached or L2 cached only) certain pinning    *
* guidelines must be followed: TLB-miss handler, TSB and linked data,          *
* asyncronous trap handlers and data must all be pinned (locked) in the TLB.   *
* In addition the TSB-miss handler and data, interrupt-vector handler and data *
* must be pinned in the TSB. We handle this by using locked TLB entries for    *
* the first 4MB (BOOTPAGE) of memory in both the I/D-TLBs. All kernel code and *
* static are stored in this region and at initialisation time dynamically      *
* allocated memory (for things like page tables) is only avaliable from this   *
* area. Dynamic memory allocations after initialisation for things like page   *
* tables may allocate memory outside of the pinned region. The result is that  *
* page tables may produce TLB faults. The exception to this is the static      *
* mappings of the kernel page table which are allocated out of the 4MB pinned  *
* area.                                                                        *
*                                                                              *
*******************************************************************************/

/* 4MB BOOTPAGE is used. */

#define BOOTPAGE_PGSIZE size_4m
#define BOOTPAGE_SIZE   (1 << 22)

/* UltraSPARC page sizes and access bits */

#define SPARC64_PAGE_BITS 13          /* 8KB base page size */
#define SPARC64_PAGE_SIZE (1UL << 13)
#define SPARC64_PAGE_MASK (~(SPARC64_PAGE_SIZE - 1))

#define NUM_CONTEXTS    (1 << 13)         /* 13 bits of CONTEXT (ASID) tag. */
#define NUCLEUS_CONTEXT 0                 /* Hardcoded to 0 on ultrasparc.  */
#define INVALID_CONTEXT (NUM_CONTEXTS-1)  /* Used in invalid TSB entries.  */

#define HW_VALID_PGSIZES ((1 << 13) | /*   8KB */ \
			  (1 << 16) | /*  64KB */ \
			  (1 << 19) | /* 512KB */ \
			  (1 << 22))  /*   4MB */

#define EXECUTE_ACCESS_BIT (1 << 0)
#define WRITE_ACCESS_BIT   (1 << 1)
#define READ_ACCESS_BIT    (1 << 2)

#define HW_ACCESS_BITS (EXECUTE_ACCESS_BIT | /* eXecute independent   */ \
			WRITE_ACCESS_BIT)    /* Write independent     */
/* Read implied by write */
/*  It seems you can't specify what other access bits a particular
 *  bit is dependent on. This seems a bit limiting.
 */

/* UltraSPARC virtual address space sizes */

#if (CONFIG_SPARC64_ULTRASPARC1 || CONFIG_SPARC64_ULTRASPARC2)

#define SPARC64_VIRTUAL_ADDRESS_BITS  44
#define SPARC64_PHYSICAL_ADDRESS_BITS 41

/* Define the structure of the linear_ptab page table */
#define HW_PGSHIFTS     {13, 16, 19, 22, 24, 34, 44}

#define PT_LEVEL1ENTRIES (1 << 10)
#define PT_LEVEL2ENTRIES (1 << 10)
#define PT_LEVEL3ENTRIES (1 << (11 + 1 /* For shadow page tables. */))

#define MDB_PGSHIFTS    {13, 16, 19, 22, 32, 41}
#define MDB_NUM_PGSIZES 5

/* Define the alignment required to avoid cache aliasing problems */
#define SPARC64_ALIAS_BOUNDARY (1 << (SPARC64_PAGE_BITS+1))
#define SPARC64_CACHE_ALIGN (((SPARC64_ALIAS_BOUNDARY/SPARC64_PAGE_SIZE)-1) \
			     << SPARC64_PAGE_BITS)

#elif (CONFIG_SPARC64_ULTRASPARC3)

#define SPARC64_VIRTUAL_ADDRESS_BITS  64
#define SPARC64_PHYSICAL_ADDRESS_BITS 43

/* Define the structure of the linear_ptab page table */
#define HW_PGSHIFTS     {13, 16, 19, 22, 24, 34, 44, 54, 64}

#define PT_LEVEL1ENTRIES (1 << 10)
#define PT_LEVEL2ENTRIES (1 << 10)
#define PT_LEVEL3ENTRIES (1 << 10)
#define PT_LEVEL4ENTRIES (1 << 10)
#define PT_LEVEL5ENTRIES (1 << (11 + 1 /* For shadow page tables. */))

#define MDB_PGSHIFTS    {13, 16, 19, 22, 32, 43} 
#define MDB_NUM_PGSIZES 6

/* Define the alignment required to avoid cache aliasing problems */
#define SPARC64_ALIAS_BOUNDARY (1 << (SPARC64_PAGE_BITS+1))
#define SPARC64_CACHE_ALIGN (((SPARC64_ALIAS_BOUNDARY/SPARC64_PAGE_SIZE)-1) \
			     << SPARC64_PAGE_BITS)

#else

#error Unknown UltraSPARC CPU defined!

#endif /* CONFIG_SPARC64_ULTRASPARC* */

/* UltraSPARCs ASI specifics */

#define SPARC64_HAS_ASI_NUCLEUS 1 /* UltraSPARC supports a NUCLEUS context */

#ifndef ASSEMBLY /* We don't want the TTE stuff in non C/C++ files */

#include <types.h>
#include <asid.h>

#include INC_CPU(tlb.h)
#include INC_ARCH(asi.h)
#include INC_ARCH(types.h)

/**
 *  Context Register (PRIMARY/SECONDARY/NUCLEUS)
 */
class context_t {
public:
    hw_asid_t context;

public:
    /* Context regiters. */
    enum context_e {
	primary   = 0,
	secondary = 1,
	nucleus   = 2, /* Doubles to indicate neither primary/secondary contexts. */
	reserved  = 3  /* We use it to indicate both primary/secondary contexts.  */
    };

public:
    /* Context management. */
    void set(context_e context_register)
    {
	if(context_register == primary || context_register == reserved) {

	    __asm__ __volatile__("stxa\t%0, [%1] %2\n\t"
				 "membar    #Sync"
				 : /* No outputs */
				 : "r" (context),         // %0
				 "r" (PRIMARY_CONTEXT), // %1
				 "i" (ASI_DMMU));       // %2

	} else if(context_register == secondary || context_register == reserved) {

	    __asm__ __volatile__("stxa\t%0, [%1] %2\n\t"
				 "membar    #Sync"
				 : /* No outputs */
				 : "r" (context),           // %0
				 "r" (SECONDARY_CONTEXT), // %1
				 "i" (ASI_DMMU));         // %2

	} else {ASSERT(false);} // Nucleus context is always 0.

    } // set()

    void get(context_e context_register)
    {
	if(context_register == primary || context_register == reserved) {

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (context)         // %0
				 : "r" (PRIMARY_CONTEXT), // %1
				 "i" (ASI_DMMU));       // %2

	} else if(context_register == secondary || context_register == reserved) {

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (context)           // %0
				 : "r" (SECONDARY_CONTEXT), // %1
				 "i" (ASI_DMMU));         // %2

	} else {
	    context = 0; // Nucleus context is always 0.
	}

    } // get()

    /* Printing. */

    void print(context_e context_register)
    {
	ASSERT(context_register != nucleus && context_register != reserved);

	printf("%s: 0x%x",
	       context_register == primary ? "Primary" : "Secondary", context);

    } // print()

}; // context_t

/**
 *  Synchronous Fault Status Register (SFSR)
 */
class sfsr_t {
public:
    union {
	u32_t raw;
	struct {
	    BITFIELD10(u32_t,
		       fv    : 1, /* Fault Valid.              */
		       ow    : 1, /* Fault status overwritten. */
		       w     : 1, /* Write fault.              */
		       pr    : 1, /* Fault privileged mode.    */
		       ct    : 2, /* Faulting context.         */
		       e     : 1, /* Side effect bits status.  */
		       ft    : 7, /* Fault type.               */
		       __rv1 : 2, /* Unused.                   */
		       asi   : 8, /* Fault ASI.                */
		       __rv2 : 8  /* Unused.                   */

		      ) // BITFIELD10()
	} sfsr;

    }; // union

public:	
    /* Fault status. */
    enum status_e {
	none = 0, /* Neither.                  */
	data = 1, /* Data fault status.        */
	inst = 2, /* Instruction fault status. */
	both = 3  /* Both fault status.        */
    };

public:    
    /* SFSR management. */

    void get(status_e status)
    {
	if(status & data) { // D-SFSR

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (sfsr)       // %0
				 : "r" (TLB_SFSR),   // %1
				 "i" (ASI_DMMU));  // %2

	} else if(status & inst) { // I-SFSR

	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (sfsr)       // %0
				 : "r" (TLB_SFSR),   // %1
				 "i" (ASI_IMMU));  // %2

	} else {ASSERT(false);}

    } // get()

    /* Printing. */

    void print(status_e status)
    {
	ASSERT(status != none && status != both);

	printf("%s: %c%c%c%c%c %s FT 0x%x ASI 0x%x\n",
	       status == data ? "D-sfsr" : "I-sfsr",
	       sfsr.fv ? 'V' : 'v',
	       sfsr.ow ? 'O' : 'o',
	       sfsr.w  ? 'W' : 'w',
	       sfsr.pr ? 'P' : 'p',
	       sfsr.e  ? 'E' : 'e',
	       (sfsr.ct == context_t::primary) ? "Prim" :
	       ((sfsr.ct == context_t::secondary) ? "Sec" : "Nucl"),
	       sfsr.ft,
	       sfsr.asi);

    } // print()

}; // sfsr_t

/**
 *  Memory Management Unit (MMU)
 */
class mmu_t {
public:
    context_t primary;
    context_t secondary;
    sfsr_t d_sfsr;
    sfsr_t i_sfsr;

public:
    /* MMU general management. */

    static void init(void)
    {
	tlb_t tlb_entry;

	tlb_entry.clear();
	for(int i = 0; i < TLB_ENTRIES; i++) {
	    if(i != TLB_KERNEL_LOCKED) {tlb_entry.set(i, tlb_t::all_tlb);}
	}

	// Initialise TSB?

    } // init()

    /* Return the contents of the fault address register */
    static addr_t get_d_sfar(void)
    {
	addr_t sfar;
	__asm__ __volatile__("ldxa\t[%1] %2, %0\n"
			     : "=r" (sfar)       // %0
			     : "r" (TLB_SFAR),   // %1
			     "i" (ASI_DMMU));  // %2
	return sfar;
    }

    /* TLB unmapping. */
    /* Unmap a whole context from TLB(s). */
    static void unmap(hw_asid_t context, tlb_t::tlb_e tlb)
    {
	context_t old_secondary, new_secondary;

	/* Save the secondary context and set it to the asid to be unmapped. */
	old_secondary.get(context_t::secondary);
	new_secondary.context = context;
	new_secondary.set(context_t::secondary);

	/* Do the demap operations. */
	word_t demap_op = (1 << 6) | (context_t::secondary << 4);
	if(tlb & tlb_t::d_tlb) {
	    __asm__ __volatile__("stxa	%%g0, [ %0 ] %1"
				 :: "r" (demap_op), "i" (ASI_DMMU_DEMAP));
	}
	if(tlb & tlb_t::i_tlb) {
	    __asm__ __volatile__("stxa	%%g0, [ %0 ] %1"
				 :: "r" (demap_op), "i" (ASI_IMMU_DEMAP));
	}

	/* Restore the secondary context. */
	old_secondary.set(context_t::secondary);
    } // unmap(context, type, tlb)

    /* Unmap a single entry from TLB(s). */
    static void unmap(hw_asid_t context, addr_t page, tlb_t::tlb_e tlb)
    {
	context_t old_secondary, new_secondary;

	/* Save the secondary context and set it to the asid to be unmapped. */
	old_secondary.get(context_t::secondary);
	new_secondary.context = context;
	new_secondary.set(context_t::secondary);

	/* Do the demap operations. */
	word_t demap_op = ((word_t)page & SPARC64_PAGE_MASK) | (context_t::secondary << 4);
	if(tlb & tlb_t::d_tlb) {
	    __asm__ __volatile__("stxa	%%g0, [ %0 ] %1"
				 :: "r" (demap_op), "i" (ASI_DMMU_DEMAP));
	}
	if(tlb & tlb_t::i_tlb) {
	    __asm__ __volatile__("stxa	%%g0, [ %0 ] %1"
				 :: "r" (demap_op), "i" (ASI_IMMU_DEMAP));
	}

	/* Restore the secondary context. */
	old_secondary.set(context_t::secondary);
    } // unmap(context, type, page, tlb);

    /* Printing. */

    /* Dump MMU state. */
    void print(sfsr_t::status_e status, context_t::context_e context)
    {
	/* Print I/D-SFSRs */
	if(status & sfsr_t::data) {
	    d_sfsr.get(sfsr_t::data);

	    d_sfsr.print(sfsr_t::data);
	}
	if(status & sfsr_t::inst) {
	    i_sfsr.get(sfsr_t::inst);

	    i_sfsr.print(sfsr_t::inst);
	}

	/* Print primary/secondary contexts. */
	if(context == context_t::primary || context == context_t::reserved) {
	    primary.get(context_t::primary);

	    primary.print(context_t::primary), printf(" ");
	}
	if(context == context_t::secondary || context == context_t::reserved) {
	    secondary.get(context_t::secondary);

	    secondary.print(context_t::secondary), printf(" ");
	}

	printf("\n");

	/* Print data fault address register */
	printf("D-SFAR: %p\n", get_d_sfar());

    } // print(status, context)

    /* Dump all TLB(s) entries. */
    static void print(tlb_t::tlb_e tlb)
    {
	ASSERT(tlb != tlb_t::no_tlb);

	if(tlb & tlb_t::d_tlb) {
	    for(int i = 0; i < TLB_ENTRIES; i++) {
		print(i, tlb_t::d_tlb);
	    }
	}
	if(tlb & tlb_t::i_tlb) {
	    for(int i = 0; i < TLB_ENTRIES; i++) {
		print(i, tlb_t::i_tlb);
	    }
	}

    } // print(tlb)

    /* Dump a single TLB entry. */
    static void print(u16_t entry, tlb_t::tlb_e tlb)
    {
	tlb_t tlb_entry;

	tlb_entry.get(entry, tlb);

	printf("%c-TLB[%d] ", (tlb & tlb_t::d_tlb) ? 'D' : 'I', entry);
	tlb_entry.print();
	printf("\n");

    } // print(entry, tlb)
}; // mmu_t

#endif /* !ASSEMBLY */


#endif /* !__ARCH__SPARC64__ULTRASPARC__MMU_H__ */
