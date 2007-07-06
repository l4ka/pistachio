/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006, Karlsruhe University
 *                
 * File path:     arch/arm/pgent.h
 * Description:   Generic page table manipluation for ARM
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
 * $Id: pgent.h,v 1.25 2006/11/17 17:18:25 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__ARM__PGENT_H__
#define __ARCH__ARM__PGENT_H__

#include <kmemory.h>
#include INC_ARCH(fass.h)
#include INC_ARCH(page.h)
#include INC_ARCH(ptab.h)
#include INC_GLUE(hwspace.h)	/* virt<->phys	*/
#include INC_CPU(cache.h)

#if defined(CONFIG_ARM_TINY_PAGES)
#error Tiny pages not working
#endif

EXTERN_KMEM_GROUP (kmem_pgtab);

class mapnode_t;
class space_t;

template<typename T> INLINE T phys_to_page_table(T x)
{
    return (T) ((u32_t) x + UNCACHED_OFFSET);
}

template<typename T> INLINE T virt_to_page_table(T x)
{
    return (T) ((u32_t) x - KERNEL_OFFSET + UNCACHED_OFFSET);
}

template<typename T> INLINE T page_table_to_virt(T x)
{
    return (T) ((u32_t) x - UNCACHED_OFFSET + KERNEL_OFFSET);
}

template<typename T> INLINE T page_table_to_phys(T x)
{
    return (T) ((u32_t) x - UNCACHED_OFFSET);
}

INLINE bool has_tiny_pages (space_t * space)
{
#if defined(CONFIG_ARM_TINY_PAGES)
    return true;
#else
    return false;
#endif
}

INLINE word_t arm_l2_ptab_size (space_t * space)
{
    return has_tiny_pages (space) ? ARM_L2_SIZE_TINY : ARM_L2_SIZE_NORMAL;
}


class pgent_t
{
public:
    union {
	arm_l1_desc_t	l1;
	arm_l2_desc_t	l2;
	u32_t		raw;
    };

public:
    enum pgsize_e {
#if defined(CONFIG_ARM_TINY_PAGES)
	size_1k,
#endif
	size_4k,
	size_64k,
	size_1m,
	size_4g,
	size_max = size_1m,
#if !defined(CONFIG_ARM_TINY_PAGES)
	size_1k = size_4g + 1,
#endif
    };


private:


    void sync_64k (space_t * s)
	{
	    for (word_t i = 1; i < (arm_l2_ptab_size (s) >> 6); i++)
		((u32_t *) this)[i] = raw;
	}

    void sync_4k (space_t * s)
	{
	    if (has_tiny_pages (s))
		for (word_t i = 1; i < 4; i++)
		    ((u32_t *) this)[i] = raw;
	}

    // Linknode access 

    u32_t get_linknode (space_t * s, pgsize_e pgsize)
	{
	    switch (pgsize)
	    {
	    case size_1m:
		return *page_table_to_virt((u32_t *) ((word_t) this + ARM_L1_SIZE));
	    case size_64k:
	    case size_4k:
	    case size_1k:
		return *page_table_to_virt((u32_t *) ((word_t) this + arm_l2_ptab_size (s)));
	    default:
		return 0;
	    }
	}
    
    void set_linknode (space_t * s, pgsize_e pgsize, u32_t val)
	{
	    switch (pgsize)
	    {
	    case size_1m:
		*page_table_to_virt((u32_t *) ((word_t) this + ARM_L1_SIZE)) = val;
		break;
	    case size_64k:
	    case size_4k:
	    case size_1k:
		*page_table_to_virt((u32_t *) ((word_t) this + arm_l2_ptab_size (s))) = val;
	    default:
		break;
	    }
	}

public:

    // Predicates

    bool is_valid (space_t * s, pgsize_e pgsize)
	{
	    switch (pgsize)
	    {
	    case size_1m:
	    case size_64k:
		return (raw != 0);
	    case size_4k:
		if (has_tiny_pages(s)) {
		    return (raw != 0);
		} else {
		    return (l2.small.two == 2);
		}
	    case size_1k:
		return (l2.tiny.three == 3);
	    default:
		return false;
	    }
	}
    
    bool is_writable (space_t * s, pgsize_e pgsize)
	{
	    return pgsize == size_1m ?
		    l1.section.ap == arm_l1_desc_t::rw :
		    l2.small.ap0 == arm_l1_desc_t::rw;
	}
    
    bool is_readable (space_t * s, pgsize_e pgsize)
	{ return l1.section.ap >= arm_l1_desc_t::ro; }
    
    bool is_executable (space_t * s, pgsize_e pgsize)
	{ return l1.section.ap >= arm_l1_desc_t::ro; }

    bool is_subtree (space_t * s, pgsize_e pgsize)
	{
	    switch (pgsize)
	    {
	    case size_1m:
		return (l1.section.two != 2) && (l1.fault.zero != 0);
	    case size_64k:
		return (l2.large.one != 1) && (raw != 0);
	    case size_4k:
		if (has_tiny_pages(s)) {
		    return (l2.small.two != 2) && (raw != 0);
		}
	    default:
		return false;
	    }
	}

    bool is_kernel (space_t * s, pgsize_e pgsize)
	{ return l1.section.ap <= arm_l1_desc_t::none; }

    // Retrieval

    arm_domain_t get_domain(void)
        {
            return l1.section.domain;
        } 

    addr_t address (space_t * s, pgsize_e pgsize)
	{
	    switch (pgsize)
	    {
	    case size_1m:  return l1.address_section();
	    case size_64k: return l2.address_large();
	    case size_4k:  return l2.address_small();
	    case size_1k:  return l2.address_tiny();
	    default: return 0;
	    }
	}
	
    pgent_t * subtree (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_1m)
	    {
		if (has_tiny_pages (s))
		    return (pgent_t *) phys_to_page_table(l1.address_finetable());
		else
		    return (pgent_t *) phys_to_page_table(l1.address_coarsetable());
	    }
	    else
		return this;
	}

    mapnode_t * mapnode (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{ return (mapnode_t *) (get_linknode (s, pgsize) ^ (u32_t) vaddr); }

    addr_t vaddr (space_t * s, pgsize_e pgsize, mapnode_t * map)
	{ return (addr_t) (get_linknode (s, pgsize) ^ (u32_t) map); }

    word_t reference_bits (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{ return 7; }

    // Modification

private:

    void cpd_sync (space_t * s, pgsize_e pgsize)
	{
	    if (pgsize == size_1m) 
	    {
 		word_t offset = ((word_t)this & 0xffff);
		pgent_t *cpd_entry = (pgent_t *)((word_t)cpd + offset);
		arm_domain_t n = get_domain();
	
		if (arm_fass.get_space(n) == s && cpd_entry->get_domain() == n) 
		    cpd_entry->raw = raw;
	    }
	}

public:

    void set_domain(arm_domain_t domain)
        {
            l1.section.domain = domain;
        } 

    void clear (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
        {
            clear(s, pgsize, kernel); 
        }

    void clear (space_t * s, pgsize_e pgsize, bool kernel )
	{
	    raw = 0;
	    switch (pgsize)
	    {
	    case size_64k:
		sync_64k (s);
		break;
	    case size_4k:
		sync_4k (s);
		break;
	    case size_1k:
	    case size_1m:
	    default:
		break;
	    }

	    if (! kernel)
		set_linknode (s, pgsize, 0);

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    void flush (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
	{
	}

    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    switch (pgsize)
	    {
	    case size_1m:
		if (has_tiny_pages (s))
		{
		    addr_t base = kmem.alloc (kmem_pgtab, ARM_L2_SIZE_TINY * 2);
		    l1.raw = 0;
		    l1.fine_table.three = 3;
		    l1.fine_table.base_address = (word_t)
			virt_to_phys (base) >> 12;
		    arm_cache::flush_dcache_ent(base, 12);
		}
		else
		{
		    addr_t base = kmem.alloc (kmem_pgtab, ARM_L2_SIZE_NORMAL * 2);
		    l1.raw = 0;
		    l1.coarse_table.one = 1;
		    l1.coarse_table.base_address = (word_t)
			virt_to_phys (base) >> 10;
		    arm_cache::flush_dcache_ent(base, 10);
		}
		break;

	    case size_64k:
	    case size_4k:
		if (!l2.is_valid())
		    raw = (pgsize << 16);

	    default:
		break;
	    }

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    void remove_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    switch (pgsize)
	    {
	    case size_1m:
		if (has_tiny_pages (s))
		    kmem.free (kmem_pgtab, phys_to_virt (
			       (addr_t) (l1.fine_table.base_address << 12)),
			       ARM_L2_SIZE_TINY * 2);
		else
		    kmem.free (kmem_pgtab, phys_to_virt (
			       (addr_t) (l1.coarse_table.base_address << 10)),
			       ARM_L2_SIZE_NORMAL * 2);
		break;
	    case size_64k:
	    case size_4k:
	    default:
		break;
	    }

	    clear (s, pgsize, kernel);
	}



    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
		    word_t rwx, word_t attrib, bool kernel)
	{
	    word_t ap;

	    if (kernel)
		ap = rwx & 2 ? arm_l1_desc_t::none : arm_l1_desc_t::special;
	    else
		ap = rwx & 2 ? arm_l1_desc_t::rw : arm_l1_desc_t::ro;

	    switch (pgsize)
	    {
	    case size_1m:
		l1.raw = 0;
		l1.section.two = 2;
		l1.section.b = l1.section.c = (attrib == 0);
		l1.section.domain = 0;
		l1.section.ap = ap;
		l1.section.base_address = (word_t) paddr >> 20;
		break;

	    case size_64k:
		l2.raw = 0;
		l2.large.one = 1;
		l2.large.b = l2.large.c = (attrib == 0);
		l2.large.ap0 = l2.large.ap1 = l2.large.ap2 = l2.large.ap3 = ap;
		l2.large.base_address = (word_t) paddr >> 16;
		sync_64k (s);
		break;

	    case size_4k:
		l2.raw = 0;
		l2.small.two = 2;
		l2.small.b = l2.small.c = (attrib == 0);
		l2.small.ap0 = l2.small.ap1 = l2.small.ap2 = l2.small.ap3 = ap;
		l2.small.base_address = (word_t) paddr >> 12;
		sync_4k (s);
		break;

	    case size_1k:
		l2.raw = 0;
		l2.tiny.three = 3;
		l2.tiny.b = l2.tiny.c = (attrib == 0);
		l2.tiny.ap = ap;
		l2.tiny.base_address = (word_t) paddr >> 10;
		break;

	    default:
		break;
	    }

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    /* For init code, must be inline. Sets kernel and cached */
    inline void set_entry_1m (space_t * s, addr_t paddr, bool readable,
                           bool writable, bool executable, bool attrib )
	{
	    word_t ap;

	    ap = writable ? arm_l1_desc_t::none : arm_l1_desc_t::special;

	    l1.raw = 0;
	    l1.section.two = 2;
	    l1.section.b = l1.section.c = (attrib == 0);
	    l1.section.domain = 0;
	    l1.section.ap = ap;
	    l1.section.base_address = (word_t) paddr >> 20;
	}

    inline void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
			   word_t rwx, bool kernel)
       {
           set_entry(s, pgsize, paddr, readable, rwx, 0, kernel);
       }

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    word_t ap = pgsize == size_1m ? l1.section.ap : l2.tiny.ap;

	    if ((rwx & 2) && ap == arm_l1_desc_t::rw)
	    {
		ap = arm_l1_desc_t::ro;

		switch (pgsize)
		{
		case size_1m:
		    l1.section.ap = ap;
		    break;
		case size_64k:
		    l2.large.ap0 = l2.large.ap1 = l2.large.ap2 =
			l2.large.ap3 = ap;
		    sync_64k (s);
		    break;
		case size_4k:
		    l2.small.ap0 = l2.small.ap1 = l2.small.ap2 =
			l2.small.ap3 = ap;
		    sync_4k (s);
		    break;
		case size_1k:
		    l2.tiny.ap = ap;
		    break;
		default:
		    break;
		}
	    }

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    word_t attributes (space_t * s, pgsize_e pgsize)
        {
	    switch (pgsize) {
	    case size_1m:
		return !l1.section.c;
	    case size_64k:
		return !l2.large.c;
	    case size_4k:
		return !l2.small.c;
	    case size_1k:
		return !l2.tiny.c;
	    default:
		return 0;
	    }
        }


    void update_cacheable(space_t * s, pgsize_e pgsize, bool cacheable)
	{
	    switch (pgsize) {
	    case size_1m:
		l1.section.b = l1.section.c = cacheable ? 1 : 0;
		break;
	    case size_64k:
		l2.large.b = l2.large.c = cacheable ? 1 : 0;
		sync_64k (s);
		break;
	    case size_4k:
		l2.small.b = l2.small.c = cacheable ? 1 : 0;
		sync_4k (s);
		break;
	    case size_1k:
		l2.tiny.b = l2.tiny.c = cacheable ? 1 : 0;
		break;
	    default:
		break;
	    }

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    void set_attributes (space_t * s, pgsize_e pgsize, word_t attrib)
	{ update_cacheable (s, pgsize, attrib); }

    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    word_t ap = pgsize == size_1m ? l1.section.ap : l2.tiny.ap;

	    if ((rwx & 2) && ap == arm_l1_desc_t::ro)
	    {
		ap = arm_l1_desc_t::rw;

		switch (pgsize)
		{
		case size_1m:
		    l1.section.ap = ap;
		    break;
		case size_64k:
		    l2.large.ap0 = l2.large.ap1 = l2.large.ap2 =
			l2.large.ap3 = ap;
		    sync_64k (s);
		    break;
		case size_4k:
		    l2.small.ap0 = l2.small.ap1 = l2.small.ap2 =
			l2.small.ap3 = ap;
		    sync_4k (s);
		    break;
		case size_1k:
		    l2.tiny.ap = ap;
		    break;
		default:
		    break;
		}
	    }

#ifdef CONFIG_ENABLE_FASS
	    cpd_sync(s, pgsize);
#endif
	}

    void reset_reference_bits (space_t * s, pgsize_e pgsize)
	{ }

    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx)
	{ }

    void set_linknode (space_t * s, pgsize_e pgsize,
		       mapnode_t * map, addr_t vaddr)
	{ set_linknode (s, pgsize, (u32_t) map ^ (u32_t) vaddr); }

    // Movement

    pgent_t * next (space_t * s, pgsize_e pgsize, word_t num)
	{
	    switch (pgsize)
	    {
	    case size_64k:
		return this + (num * (arm_l2_ptab_size (s) >> 6));
	    case size_4k:
		if (has_tiny_pages (s))
		    return this + (num * 4);
	    case size_1m:
	    case size_1k:
		return this + num;
	    default:
		return this;
	    }
	}

    // Operators

    bool operator != (const pgent_t rhs)
	{
	    return this->raw != rhs.raw;
	}

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	}
};

#endif /* !__ARCH__ARM__PGENT_H__ */
