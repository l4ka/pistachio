/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     src/arch/powerpc/swtlb.h
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#ifndef __ARCH__POWERPC__SWTLB_H__
#define __ARCH__POWERPC__SWTLB_H__

/* TLB entry 0 */
#define PPC_TLB_VALID		(0x80000000 >> 22)
#define PPC_TLB_SPACE0		(0)
#define PPC_TLB_SPACE1		(0x80000000 >> 23)
#define PPC_TLB_SIZE(x)		((1 << (x) -1) >> 24)
#define PPC_TLB_SIZE_1K		PPC_TLB_SIZE(0)
#define PPC_TLB_SIZE_4K		PPC_TLB_SIZE(1)
#define PPC_TLB_SIZE_16K	PPC_TLB_SIZE(2)
#define PPC_TLB_SIZE_64K	PPC_TLB_SIZE(3)
#define PPC_TLB_SIZE_256K	PPC_TLB_SIZE(4)
#define PPC_TLB_SIZE_1M		PPC_TLB_SIZE(5)
#define PPC_TLB_SIZE_16M	PPC_TLB_SIZE(7)
#define PPC_TLB_SIZE_256M	PPC_TLB_SIZE(9)
#define PPC_TLB_SIZE_1G		PPC_TLB_SIZE(10)

/* TLB entry 1 */
#define PPC_TLB_RPN(x)		(x & 0xfffff300)
#define PPC_TLB_ERPN(x)		(x & 0xf)

/* TLB entry 2 */
#define PPC_TLB_WL1		(0x80000000 >> 11)
#define PPC_TLB_IL1I		(0x80000000 >> 12)
#define PPC_TLB_IL1D		(0x80000000 >> 13)
#define PPC_TLB_IL2I		(0x80000000 >> 14)
#define PPC_TLB_IL2D		(0x80000000 >> 15)
#define PPC_TLB_U0		(0x80000000 >> 16)
#define PPC_TLB_U1		(0x80000000 >> 17)
#define PPC_TLB_U2		(0x80000000 >> 18)
#define PPC_TLB_U3		(0x80000000 >> 19)
#define PPC_TLB_WRITETHROUGH	(0x80000000 >> 20)
#define PPC_TLB_CACHEINHIBIT	(0x80000000 >> 21)
#define PPC_TLB_MEMCOHERENCY	(0x80000000 >> 22)
#define PPC_TLB_GUARDED		(0x80000000 >> 23)
#define PPC_TLB_ENDIAN		(0x80000000 >> 24)
#define PPC_TLB_UX		(0x80000000 >> 26)
#define PPC_TLB_UW		(0x80000000 >> 27)
#define PPC_TLB_UR		(0x80000000 >> 28)
#define PPC_TLB_SX		(0x80000000 >> 29)
#define PPC_TLB_SW		(0x80000000 >> 30)
#define PPC_TLB_SR		(0x80000000 >> 31)

#define PPC_MAX_TLB_ENTRIES	64

#ifndef ASSEMBLY

#include INC_ARCH(ppc_registers.h)

inline void ppc_tlbwe(word_t index, const word_t field, word_t value)
{
    asm volatile ("tlbwe %[value], %[index], %[field]\n"
		  : 
		  : [value]"b"(value), [index]"b"(index), 
		    [field]"i"(field)); 
}

inline word_t ppc_tlbre(word_t index, const word_t field)
{
    word_t value;
    asm volatile ("tlbre %[value], %[index], %[field]\n"
		  : [value]"=b"(value)
		  : [index]"b"(index), [field]"i"(field));
    return value;
}

inline word_t ppc_tlbsx(word_t vaddr, word_t &index)
{
    word_t found;
    asm volatile ("tlbsx. %[index],0,%[vaddr]\n"
		  "beq	1f\n"
		  "li	%[found], 0\n"
		  "1:\n"
		  : [index] "=b"(index), [found] "=&b"(found)
		  : [vaddr] "b"(vaddr), "[found]"(true));
    return found;
}

inline word_t ppc_get_pid()
{
    return ppc_get_spr(SPR_PID);
}

inline void ppc_set_pid(word_t pid)
{
    ppc_set_spr(SPR_PID, pid);
}


class ppc_tlb0_t {
public:
    union {
	word_t raw;
	struct {
	    u32_t epn		: 22;
	    u32_t valid		: 1;
	    u32_t trans_space	: 1;
	    u32_t size		: 4;
	    u32_t parity	: 4;
	};
    };

    ppc_tlb0_t()
	{ }
    ppc_tlb0_t(word_t vaddr, word_t log2size, bool valid=true, int space = 0)
	{ init_vaddr_size(vaddr, log2size, valid, space); }

    bool is_valid()
	{ return valid; }

    static ppc_tlb0_t invalid()
	{ 
	    ppc_tlb0_t tmp;
	    tmp.raw = 0;
	    return tmp;
	}

    void init_vaddr_size(word_t vaddr, word_t log2size, bool valid = true, int space = 0)
	{
	    this->raw = 0;
	    this->epn = vaddr >> 10;
	    this->size = (log2size - 10) / 2;
	    this->trans_space = space;
	    this->valid = valid;
	}

    void set_vaddr(word_t vaddr)
	{ epn = vaddr >> 10; }

    word_t get_size()
	{ return (1024 << (size * 2)); }

    word_t get_log2size()
	{ return (size * 2) + 10; }

    word_t get_vaddr()
	{ return epn << 10; }

    bool is_vaddr_covered(word_t vaddr)
	{ 
	    return (vaddr >= get_vaddr() && 
		    vaddr <= get_vaddr() + get_size() - 1); 
	}

    void operator += (const word_t offset)
	{ epn += (offset >> 10); }

    static bool is_valid_pagesize(word_t log2size)
	{ return (KB(1) | KB(4) | KB(16) | KB(64) | KB(256) | 
		  MB(1) | MB(16) | MB(256) | GB(1)) & (1 << log2size);
	}

    void write(int index)
	{ ppc_tlbwe(index, 0, raw); }

    void read(int index)
	{ raw = ppc_tlbre(index, 0); }
} __attribute((packed));


class ppc_tlb1_t {
public:
    union {
	u32_t raw;
	struct {
	    u32_t page		: 22;
	    u32_t parity	: 2;
	    u32_t __res		: 4;
	    u32_t extpage	: 4;
	};
    };

    ppc_tlb1_t()
	{ }
    ppc_tlb1_t(ppc_tlb1_t &tlb1)
	{ raw = tlb1.raw; }
    ppc_tlb1_t(u64_t paddr)
	{ init_paddr(paddr); }


    void write(word_t index)
	{ ppc_tlbwe(index, 1, raw); }

    void read(word_t index)
	{ raw = ppc_tlbre(index, 1); }

    void init_paddr(u64_t paddr)
	{
	    raw = 0;
	    set_paddr(paddr);
	}

    void set_paddr(u64_t paddr)
	{ 
	    page = (paddr & ~0UL) >> 10;
	    extpage = (paddr >> 32ULL);
	}

    u64_t get_paddr()
	{ return ((u64_t)extpage << 32) | ((u64_t)page << 10); }

    void operator += (const u64_t offset)
	{ set_paddr(get_paddr() + offset); }
} __attribute((packed));


class ppc_tlb2_t {
public:
    union {
	u32_t raw;
	struct {
	    u32_t parity	: 2;
	    u32_t __res1	: 9;
	    u32_t wt_l1		: 1;
	    u32_t inhibit_l1i	: 1;
	    u32_t inhibit_l1d	: 1;
	    u32_t inhibit_l2i	: 1;
	    u32_t inhibit_l2d	: 1;
	    u32_t user0		: 1;
	    u32_t user1		: 1;
	    u32_t user2		: 1;
	    u32_t user3		: 1;
	    u32_t write_through	: 1;
	    u32_t inhibit	: 1;
	    u32_t mem_coherency	: 1;
	    u32_t guarded	: 1;
	    u32_t endian	: 1;
	    u32_t __res2	: 1;
	    u32_t user_execute	: 1;
	    u32_t user_write	: 1;
	    u32_t user_read	: 1;
	    u32_t super_execute	: 1;
	    u32_t super_write	: 1;
	    u32_t super_read	: 1;
	};
    };

    void write(int index)
	{ ppc_tlbwe(index, 2, raw); }

    void read(int index)
	{ raw = ppc_tlbre(index, 2); }

    void init()
	{ raw = 0; }

    void init_shared_smp()
	{ 
	    raw = 0;
	    mem_coherency = 1;
#ifdef CONFIG_PPC_CACHE_L1_WRITETHROUGH
	    wt_l1 = 1;
	    user2 = 1;
#endif
	}

    void init_guarded()
	{
	    init_shared_smp();
	    guarded = 1;
	}

    void init_cpu_local()
	{ raw = 0; }

    void init_device()
	{
	    raw = 0;
	    inhibit = 1;
	    guarded = 1;
	}

    void set_user_perms(bool read, bool write, bool execute)
	{ 
	    this->user_read = read;
	    this->user_write = write;
	    this->user_execute = execute;
	}

    void set_kernel_perms(bool read, bool write, bool execute)
	{ 
	    this->super_read = read;
	    this->super_write = write;
	    this->super_execute = execute;
	}

    void set_cache(bool inhibit, bool write_through, bool guarded)
	{
	    this->inhibit = inhibit;
	    this->write_through = write_through;
	    this->guarded = guarded;
	}

    void set_l1_cache(bool inhibit_l1i, bool inhibit_l1d)
	{
	    this->inhibit_l1i = inhibit_l1i;
	    this->inhibit_l1d = inhibit_l1d;
	}

    void set_l2_cache(bool inhibit_l2i, bool inhibit_l2d)
	{
	    this->inhibit_l2i = inhibit_l2i;
	    this->inhibit_l2d = inhibit_l2d;
	}

    void set_user0(bool u0)
	{ this->user0 = u0; }

    void set_user1(bool u1)
	{ this->user1 = u1; }

    void set_user2(bool u2)
	{ this->user2 = u2; }

    void set_user3(bool u3)
	{ this->user3 = u3; }

    void set_endian(bool endian)
	{ this->endian = endian; }

    bool is_user_accessible()
	{ return raw & (7 << 3); }

    bool is_kernel_accessible()
	{ return raw & 7; }

    bool is_accessible()
	{ return raw & 0x3f; }

} __attribute((packed));

class ppc_mmucr_t {
public:
    union {
	word_t raw;
	struct {
	    word_t __res0			: 6;
	    word_t l2_store_without_allocate	: 1;
	    word_t store_without_allocate	: 1;
	    word_t __res1			: 1;
	    word_t u1_transient_enable		: 1;
	    word_t u2_store_without_allocate	: 1;
	    word_t u3_l2_store_without_allocate	: 1;
	    word_t dcache_unlock_exception	: 1;
	    word_t icache_unlock_exception	: 1;
	    word_t __res2			: 1;
	    word_t search_translation_space	: 1;
	    word_t __res3			: 8;
	    word_t search_id			: 8;
	};
    };

    void set_search_id(word_t id)
	{ search_id = id; }
    word_t get_search_id()
	{ return search_id; }

    static void write_search_id(word_t id, int space = 0)
	{ 
	    ppc_mmucr_t mmucr;
	    mmucr.read();
	    mmucr.set_search_id(id);
	    mmucr.search_translation_space = space;
	    mmucr.write();
	}

    ppc_mmucr_t read()
	{ 
	    raw = ppc_get_spr(SPR_MMUCR); 
	    return *this;
	}
    void write()
	{ ppc_set_spr(SPR_MMUCR, raw); }
};


class ppc_swtlb_t
{
public:
    word_t current_index;
    word_t high_water;
    word_t mask[2]; // use hard-coded size for better code below
    
    void init(word_t high_water)
	{ 
	    for (word_t idx = 0; idx < high_water; idx++)
		set_free(idx);
	    for (word_t idx = high_water; idx < sizeof(mask) * 8; idx++)
		set_used(idx);
	    current_index = 0;
	    this->high_water = high_water;
	}

    void set_used(word_t index)
	{ mask[index / BITS_WORD] &= ~(1 << (BITS_WORD - 1 - (index % BITS_WORD))); }

    void set_free(word_t index)
	{ mask[index / BITS_WORD] |=  (1 << (BITS_WORD - 1 - (index % BITS_WORD))); }

    word_t allocate()
	{
	    word_t idx;
	    if ((idx = count_leading_zeros( mask[0] )) < sizeof(word_t) * 8)
	    {
		set_used(idx);
		return idx;
	    }
	    else if ( (idx += count_leading_zeros( mask[1] ) ) < 2 * sizeof(word_t) * 8)
	    {
		set_used(idx);
		return idx;
	    }
	    else
		return get_replacement();
	}

    word_t allocate_pinned()
	{
	    high_water--;
	    return high_water + 1;
	}

    word_t get_replacement()
	{
	    current_index = (current_index + 1) % high_water;
	    return current_index;
	}
};

#endif

#endif /* !__ARCH__POWERPC__SWTLB_H__ */
