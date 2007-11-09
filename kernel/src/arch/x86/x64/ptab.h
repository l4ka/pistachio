/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     arch/x86/x64/ptab.h
 * Description:   X86-64 pagetable management (4KByte long mode)
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
 * $Id: ptab.h,v 1.9 2006/11/18 10:15:04 stoess Exp $
 *                
 ********************************************************************/


#ifndef __ARCH__X86__X64__PTAB_H__
#define __ARCH__X86__X64__PTAB_H__

#include INC_ARCH_SA(pagebits.h)

#if !defined(ASSEMBLY)

class amd64_pgent_t 
{
public:
    enum pagesize_e {
	size_4k = 0,
	size_2m = 1     
    };

    // predicates
    bool is_valid() 
	{ return pg4k.present == 1; }

    bool is_writable() 
	{ return pg4k.rw == 1; }

    bool is_executable() 
	{ return pg4k.nx == 0; }

    bool is_accessed()
	{ return pg4k.accessed == 1; }

    bool is_dirty()
	{ return pg4k.dirty == 1; }
    
    /** 
     * JS: This is only a correct superpage predicate,
     * if PAT Tables aren't enabled 
     * 
     */
    bool is_superpage()
	{ return pg2m.super == 1; }

    bool is_kernel()
	{ return pg4k.privilege == 0; }

    bool is_write_through()
	{ return pg4k.write_through == 1; }

    bool is_cache_disabled()
	{ return pg4k.cache_disabled == 1; }
    
    word_t is_pat(pagesize_e size)
	{ return (size == size_4k ? pg4k.pat : pg2m.pat); }
    
    addr_t get_address(const pagesize_e size = size_4k)
	{ 
	    if (size == size_4k)
		return (addr_t) (raw & AMD64_4KPAGE_MASK);
	    else 
		return (addr_t) (raw & AMD64_2MPAGE_MASK);
	}

    amd64_pgent_t * get_ptab()
	{ return (amd64_pgent_t*)(raw & AMD64_PTE_MASK); }

    u64_t get_raw()
	{ return raw; }

    // modification
    void clear()
	{ raw = 0; }

    /* used to set an entry pointing to a physical page */
    void set_entry(addr_t addr, pagesize_e size, u64_t attrib)
	{ 
	    if (size == size_4k){
		raw = ((u64_t)(addr) & AMD64_4KPAGE_MASK);
		raw |= (attrib & AMD64_4KPAGE_FLAGS_MASK);
	    }
	    else{
		raw = ((u64_t)(addr) & AMD64_2MPAGE_MASK) | AMD64_2MPAGE_SUPER;
		raw |= (attrib & AMD64_2MPAGE_FLAGS_MASK);
		
	    }

	}

    void set_cacheability (bool cacheable, pagesize_e size)
    {
	this->pg4k.cache_disabled = !cacheable;
	if (size == size_4k) 
	    pg4k.pat = 0;
	else
	    pg2m.pat = 0;
    }

    void set_pat (word_t pat, pagesize_e size)
	{
	    pg4k.write_through  = (pat & 1) ? 1 : 0;
	    pg4k.cache_disabled = (pat & 2) ? 1 : 0;
	    if (size == size_4k)
		pg4k.pat = (pat & 4) ? 1 : 0;
	    else
		pg2m.pat = (pat & 4) ? 1 : 0;
	}

    void set_global (bool global)
    {
	this->pg4k.global = global;
    }

    /* used to set an entry pointing to the next table in hierarchy */
    void set_ptab_entry(addr_t addr, u32_t attrib)
	{
	    raw = ((u64_t)(addr) & AMD64_PTE_MASK) | AMD64_PAGE_VALID | (attrib & AMD64_PTE_FLAGS_MASK);
	}
		
    void copy(const amd64_pgent_t pgent)
	{
	    raw = pgent.raw;
	}

    /* SMP specific synchronization flag */
    bool must_sync()
    {
	return (pg4k.sync == 1);
    }

    /* SMP specific synchronization flag */
    void set_sync()
    {
	pg4k.sync = 1;
    }


    /* SMP specific cpu-local flag */
    bool is_cpulocal()
    {
	return (pg4k.cpu_local == 1);
    }


    /* SMP specific cpu-local flag */
    void set_cpulocal()
    {
	pg4k.cpu_local = 1;
    }


private:
    union {
	struct {
	    u64_t present		:1;
	    u64_t rw			:1;
	    u64_t privilege		:1;
	    u64_t write_through		:1;

	    u64_t cache_disabled	:1;
	    u64_t accessed		:1;
	    u64_t dirty			:1;
	    u64_t pat			:1;  

	    u64_t global		:1;
	    u64_t sync                  :1;
	    u64_t cpu_local             :1;
	    u64_t avl			:1;
	    
	    u64_t base			:40;
	    u64_t available		:11;
	    u64_t nx		        :1;

	} pg4k;
	struct {
	    u64_t present		:1;
	    u64_t rw			:1;
	    u64_t privilege		:1;
	    u64_t write_through		:1;

	    u64_t cache_disabled	:1;
	    u64_t accessed		:1;
	    u64_t dirty			:1;

	    u64_t super			:1;
	    u64_t global		:1;
	    u64_t sync                  :1;
	    u64_t cpu_local             :1;
	    u64_t avl			:1;
	    
	    u64_t pat			:1;  
	    u64_t mbz			:8;
	    u64_t base			:31;
	    u64_t available		:11;
	    u64_t nx		        :1;

	} pg2m;
	u64_t raw;
    };

    friend class pgent_t;
};

#endif /* !ASSEMBLY */


#endif /* !__ARCH__X86__X64__PTAB_H__ */
