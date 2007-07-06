/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    arch/sparc64/ultrasparc/tlb.h
 * Description:  TLB class for the UltraSPARC CPU implmenetation
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
 * $Id: tlb.h,v 1.4 2004/02/22 23:07:35 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ULTRASPARC__TLB_H__
#define __ARCH__SPARC64__ULTRASPARC__TLB_H__

#ifndef ASSEMBLY

#include <types.h>
#include <asid.h>

#include INC_ARCH(asi.h)

/**************************************************************************
 * Notes: Currently only looked at the UltraSPARC I/II TLB. UltraSPARC III *
 * support will require modifications.                                     *
 * See the UltraSPARC manuals for details of fields.                       *
 **************************************************************************/

#define TLB_ENTRIES 64 /* UltraSPARC I/II have 64 entry I/D-TLBs. */

#define TLB_DATA_ENTRY                                                         \
BITFIELD17(u64_t,                                                            \
	   global        : 1,  /* Is this mapping global to all spaces. */   \
	   tlb_writable  : 1,  /* TLB write access bit.                 */   \
	   privileged    : 1,  /* Is this a kernel only mapping.        */   \
	   side_effect   : 1,  /* Is this an I/O style mapping.         */   \
	   cache_attrib  : 2,  /* See cache_attrib_e.                   */   \
	   tlb_lock      : 1,  /* TTE locked into TLB once loaded.      */   \
	   access_bits   : 3,  /* Page is r/w/x.        Ignored by TLB. */   \
	   ref_bits      : 3,  /* Page has been r/w/x.  Ignored by TLB. */   \
	   pfn           : 28, /* Physical Frame Number.                */   \
	   __rv1         : 9,  /* Reserved by the CPU implementation.   */   \
	   __rv2         : 4,  /* Defined by pgent_t in INC_CPU(pgent.h)*/   \
	   __spare       : 4,  /* Can use later.        Ignored by TLB. */   \
	   tsb_lock      : 1,  /* TTE locked into TSB.  Ignored by TLB. */   \
	   invert_endian : 1,  /* Invert endianess of mapping.          */   \
	   no_fault_only : 1,  /* Must be accessed via NO_FAULT ASIs.   */   \
	   size          : 2,  /* See pgsize_e below.                   */   \
	   tlb_valid     : 1)  /* DTLB read bit, ITLB execute bit.      */


/* Location (TLB entry) of pinned kernel mappings in the TLB. */
#define TLB_KERNEL_LOCKED 0

/**
 *  Manages I/D-TLB entries.
 */
class tlb_t {
private:
    /* The TAG/CAM field of a TLB entry. */
    union {
	u64_t tag_raw;
	struct {
	    BITFIELD2(u64_t,
		      context : 13, /* Context identifier (ASID). */
		      vpn     : 51  /* Virtual page number.       */

		     ) // BITFIELD2()
	} tag;

    }; // union

    /* The DATA/RAM field of a TLB entry. */
    union {
	u64_t data_raw;
	struct {
	    TLB_DATA_ENTRY
	} data;

    }; // union

public:  
    /* Which hardware TLB. */
    enum tlb_e {
	no_tlb  = 0, /* No TLB.   */
	d_tlb   = 1, /* D-TLB.    */
	i_tlb   = 2, /* I-TLB.    */
	all_tlb = 3  /* All TLBs. */
    };

    /* Caching attributes. */
    enum cache_attrib_e {
	uncached  = 0, /* Totally uncached.                          */
	cache_phy = 2, /* Cachable in physically tagged caches only. */
	cache_vir = 3  /* Cachable in all caches.                    */
    };

    /* Page sizes. */
    enum pgsize_e {
	size_8k   = 0,
	size_64k  = 1,
	size_512k = 2,
	size_4m   = 3
    };

public:

    /* I/D-TLB entry management. */

    /* Sets an entry in the D-TLB and/or I-TLB. */
    void set(u16_t entry, tlb_e tlb)
    {
	//ASSERT(tlb != no_tlb);
	//ASSERT(entry < TLB_ENTRIES);

	if(tlb & d_tlb) { // D-TLB entry.
	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 "stxa\t%3, [%4] %5\n"
				 : /* no outputs */
				 : "r" (tag_raw),               // %0
				   "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_DMMU),              // %2 
				   "r" (data_raw),              // %3
				   "r" (entry << 3),            // %4 (3 LSB are zero)
				   "i" (ASI_DTLB_DATA_ACCESS)); // %5
	}
	if(tlb & i_tlb) { // I-TLB entry.
	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 "stxa\t%3, [%4] %5\n"
				 : /* no outputs */
				 : "r" (tag_raw),               // %0
				   "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_IMMU),              // %2 
				   "r" (data_raw),              // %3
				   "r" (entry << 3),            // %4 (3 LSB are zero)
				   "i" (ASI_ITLB_DATA_ACCESS)); // %5
	}

    } // set()

    /* Sets an entry in the D-TLB and/or I-TLB using the CPU's replacement
     * algorithm.
     */
    void set(tlb_e tlb)
    {
	// ASSERT(tlb != no_tlb);
	if(tlb & d_tlb) { // D-TLB entry.
	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 "stxa\t%3, [%%g0] %4\n"
				 : /* no outputs */
				 : "r" (tag_raw),               // %0
				   "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_DMMU),              // %2 
				   "r" (data_raw),              // %3
				   "i" (ASI_DTLB_DATA_IN));     // %4
	}
	if(tlb & i_tlb) { // I-TLB entry.
	    __asm__ __volatile__("stxa\t%0, [%1] %2\n"
				 "stxa\t%3, [%%g0] %4\n"
				 : /* no outputs */
				 : "r" (tag_raw),               // %0
				   "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_IMMU),              // %2 
				   "r" (data_raw),              // %3
				   "i" (ASI_ITLB_DATA_IN));     // %4
	}
    }

    /* Get an entry from either D-TLB or I-TLB. */
    void get(u16_t entry, tlb_e tlb)
    {
	ASSERT(entry < TLB_ENTRIES);

	switch (tlb) {
	case d_tlb:
	__asm__ __volatile__("ldxa\t[%2] %3, %0\n"
			     "ldxa\t[%2] %4, %1\n"
			     : "=r" (data_raw),            // %0
			       "=r" (tag_raw)              // %1
			     : "r" (entry << 3),           // %2 (3 LSB are zero)
			       "i" (ASI_DTLB_DATA_ACCESS), // %3
			       "i" (ASI_DTLB_TAG_READ));   // %4
	break;
	case i_tlb:
	__asm__ __volatile__("ldxa\t[%2] %3, %0\n"
			     "ldxa\t[%2] %4, %1\n"
			     : "=r" (data_raw),            // %0
			       "=r" (tag_raw)              // %1
			     : "r" (entry << 3),           // %2 (3 LSB are zero)
			       "i" (ASI_ITLB_DATA_ACCESS), // %3
			       "i" (ASI_ITLB_TAG_READ));   // %4
	break;
	default:
	ASSERT(false);
	}

    } // get()

    static word_t get_tag_access(tlb_e tlb) {
	word_t access_reg = 0;

	if(tlb == d_tlb) { // D-TLB access register.
	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (access_reg)            // %0
				 : "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_DMMU));             // %2 
	} else if(tlb == i_tlb) { // I-TLB access register.
	    __asm__ __volatile__("ldxa\t[%1] %2, %0\n"
				 : "=r" (access_reg)            // %0
				 : "r" (TLB_TAG_ACCESS),        // %1
				   "i" (ASI_IMMU));             // %2 
	}

	return access_reg;
    }

    /* Tag field manipulation. */

    void set_va(addr_t vaddr) {tag.vpn = (u64_t)vaddr >> SPARC64_PAGE_BITS;}
    void set_asid(hw_asid_t asid) {tag.context = (u64_t)asid;}
    addr_t get_va(void) {return (addr_t)(tag.vpn << SPARC64_PAGE_BITS);}
    hw_asid_t get_asid(void) {return (hw_asid_t)tag.context;}

    /* Data field manipulation. */

    void set_global(bool g) {data.global = g;}
    void set_writable(bool w) {data.tlb_writable = w;}
    void set_privileged(bool p) {data.privileged = p;}
    void set_side_effect(bool e) {data.side_effect = e;}
    void set_lock(bool l) {data.tlb_lock = l;}
    void set_invert_endian(bool i) {data.invert_endian = i;}
    void set_no_fault_only(bool n) {data.no_fault_only = n;}
    void set_valid(bool v) {data.tlb_valid = v;}
    void set_size(pgsize_e size) {data.size = size;}
    void set_cache_attrib(cache_attrib_e attrib)
    {
	data.cache_attrib = attrib;
    }
    void set_pa(addr_t paddr) {data.pfn = (u64_t)paddr >> SPARC64_PAGE_BITS;}
    bool get_global(void) {return (bool)data.global;} 
    bool get_writable(void) {return (bool)data.tlb_writable;}
    bool get_privileged(void) {return (bool)data.privileged;}
    bool get_side_effect(void) {return (bool)data.side_effect;}
    bool get_lock(void) {return (bool)data.tlb_lock;}
    bool get_invert_endian(void) {return (bool)data.invert_endian;}
    bool get_no_fault_only(void) {return (bool)data.no_fault_only;}
    bool get_valid(void) {return (bool)data.tlb_valid;}
    pgsize_e get_size(void) {return (pgsize_e)data.size;}
    cache_attrib_e get_cache_attrib(void)
    {
	return (cache_attrib_e)data.cache_attrib;
    }
    addr_t get_pa(void) {return (addr_t)(data.pfn << SPARC64_PAGE_BITS);}

    /* Invalid TLB entry. */

    void clear(void)
    {
	data_raw = 0;
	tag_raw = 0;
    }

    /* Printing. */

    void print(void)
    {
	printf("va: 0x%lx \tasid: 0x%x \tpa: 0x%lx \t%c%c%c%c%c%c%c%c CA:%d %s",
	       (word_t)get_va(),
	       (word_t)get_asid(),
	       (word_t)get_pa(),
	       get_global()        ? 'G' : 'g',
	       get_writable()      ? 'W' : 'w',
	       get_privileged()    ? 'P' : 'p',
	       get_side_effect()   ? 'E' : 'e',
	       get_lock()          ? 'L' : 'l',
	       get_invert_endian() ? 'I' : 'i',
	       get_no_fault_only() ? 'N' : 'n',
	       get_valid()         ? 'V' : 'v',
	       (int)get_cache_attrib(),
	       (get_size() == size_8k)     ? "8KB" :
	       ((get_size() == size_64k)   ? "64KB" :
		((get_size() == size_512k) ? "512KB" : "4MB")));

    } // print()

}; // tlb_t

#endif /* !ASSEMBLY */


#endif /* !__ARCH__SPARC64__ULTRASPARC__TLB_H__ */
