/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     arch/amd64/pgent.h
 * Description:   Generic page table manipluation for X86-64
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
 * $Id: pgent.h,v 1.14 2006/11/18 10:15:29 stoess Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__PGENT_H__
#define __ARCH__AMD64__PGENT_H__

#include <kmemory.h>
#include <debug.h>
#include INC_GLUE(config.h)
#include INC_GLUE(hwspace.h)
#include INC_ARCH(ptab.h)
#include INC_ARCHX(x86,mmu.h)

#define HW_PGSHIFTS             { 12, 21, 30, 39, 48, 64 }
#define HW_VALID_PGSIZES        ((1 << 12) | (1 << 21))

#define MDB_PGSHIFTS            { 12, 21, 30, 39, 48 }
#define MDB_NUM_PGSIZES         (4)

EXTERN_KMEM_GROUP(kmem_pgtab);

#if defined(CONFIG_NEW_MDB)
#define mapnode_t mdb_node_t
#endif

class mapnode_t;
class space_t;


class pgent_t
{
public:
    union {
	amd64_pgent_t    pgent;
	u64_t            raw;
    };

    enum pgsize_e {
	size_4k   = 0,
	size_2m   = 1,
	size_1g   = 2,
	size_512g = 3,
	size_max = size_512g
    };
private:

    /* Shadow pagetable */
#if !defined(CONFIG_SMP)
    u64_t get_linknode (pgsize_e pgsize)
	{ return *(u64_t *) ((word_t) this + AMD64_PTAB_BYTES); }
    
    void set_linknode (pgsize_e pgsize, u64_t val)
	{ *(u64_t *) ((word_t) this + AMD64_PTAB_BYTES) = val; }

    void sync (space_t * s, pgsize_e pgsize) { }
#else

#define PTAB_LINKNODE_OFFSET (AMD64_PTAB_BYTES)
    u64_t get_linknode (pgsize_e pgsize)
	{
	    // only PDIRs and PTABs are shadowed on AMD64
	    // TODO: it seems that it is not necessary to make this SMP-specific
	    ASSERT(pgsize == size_2m || pgsize == size_4k);
	    return *(u64_t*) ( (word_t)this + PTAB_LINKNODE_OFFSET );
	}
    
    void set_linknode (pgsize_e pgsize, u64_t val)
	{ 	 
	    ASSERT(pgsize == size_2m || pgsize == size_4k);
	    *(u64_t *)( (word_t)this + PTAB_LINKNODE_OFFSET ) = val;
	}

    pgent_t * get_pgent_cpu(unsigned cpu)
	{ return (pgent_t*) ((word_t) this + AMD64_PTAB_BYTES * cpu); }


    void sync (space_t * s, pgsize_e pgsize)
    {	
	/* PML4-entries are always synced. PDP and PDIR entries are synced if
	 * sync bit is set.
	 */
	if ( (pgsize == size_512g) || (this->pgent.must_sync() ) ) 
	    for (unsigned cpu = 1; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	    {
		pgent_t * cpu_pgent = get_pgent_cpu(cpu);
		*cpu_pgent = *this;
	    }
    }
public:

    void assign_cpu_subtree(space_t * s, pgsize_e pgsize, addr_t addr, bool kernel)
    {	
	ASSERT(kernel);
	pgent.set_ptab_entry
	    (virt_to_phys(addr), 
	     AMD64_PAGE_USER | AMD64_PAGE_WRITABLE | AMD64_PAGE_CPULOCAL);
    }
    
    
    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    if ( pgsize == size_512g && idx() >= CPULOCAL_PML4  )
	    {
		// cpu-local PML4 entry must point to a cpu-local PDP
		//TRACEF("allocating per-cpu PDP for cpu-local data\n"); 

		// allocate pdps for cpu-local data (one per cpu)
		amd64_pgent_t *cpu_local_pdp = (amd64_pgent_t *)
		    kmem.alloc(kmem_pgtab, AMD64_PTAB_BYTES * CONFIG_SMP_MAX_CPUS);
		amd64_pgent_t *pdpent = cpu_local_pdp;
		for (word_t cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
		{
		    for( word_t i = 0; i < CPULOCAL_PDP; i++ )
			pdpent++->set_sync();
		    for( word_t i = CPULOCAL_PDP; i < 512; i++ )
			pdpent++->set_cpulocal();
		}
		
		// set PML4-entry to point to newly allocated PDP
		pgent.set_ptab_entry(virt_to_phys(cpu_local_pdp), 
		     AMD64_PAGE_USER | AMD64_PAGE_WRITABLE | AMD64_PAGE_CPULOCAL);
	    }
	    else if ( pgsize == size_1g && pgent.is_cpulocal() && idx() >= CPULOCAL_PDP )
	    {
		// cpu-local PDP must point to a cpu-local PDIR
		//TRACEF("allocating pdirs for cpu-local data\n"); 
		// allocate pdirs for cpu-local data (one per cpu)
		amd64_pgent_t *cpu_local_pdir = (amd64_pgent_t *)
		    kmem.alloc(kmem_pgtab, AMD64_PTAB_BYTES * CONFIG_SMP_MAX_CPUS);

		
		amd64_pgent_t *pdirent = cpu_local_pdir;
		// initialize sync-bit of empty pdir.
		for (word_t idx = 0; idx < 512 * CONFIG_SMP_MAX_CPUS; idx++)
		    pdirent++->set_sync();
		// set PDP-entry to point to newly allocated pdir
		pgent.set_ptab_entry(virt_to_phys(cpu_local_pdir), 
				     AMD64_PAGE_USER | AMD64_PAGE_WRITABLE | AMD64_PAGE_CPULOCAL );
	    }
	    else
	    {
		// double size for shadow pagetable
		word_t size = 
		    (!kernel && (pgsize == size_4k || pgsize == size_2m))
		    ? 2 * AMD64_PTAB_BYTES : AMD64_PTAB_BYTES;

		pgent.set_ptab_entry
		    (virt_to_phys
		     (kmem.alloc (kmem_pgtab, size)), AMD64_PAGE_USER | AMD64_PAGE_WRITABLE |
		     (pgent.must_sync() ? AMD64_PAGE_SYNC : 0) );
	    }
	    sync(s, pgsize);

	}

    // sets an entry but does not sync and does not preserve sync bit.
    void set_cpu_entry (space_t * s, pgsize_e pgsize,
		    addr_t paddr, word_t rwx, bool kernel)
	{
	    pgent.set_entry (paddr, (amd64_pgent_t::pagesize_e) pgsize,
			     (kernel ? (AMD64_PAGE_KERNEL | AMD64_PAGE_GLOBAL) : AMD64_PAGE_USER) |
			     (rwx & 2 ? AMD64_PAGE_WRITABLE : 0) |
			     AMD64_PAGE_VALID | AMD64_PAGE_CPULOCAL);	    
	}
    
    
#endif /* CONFIG_SMP */

public:
    // Predicates
    
    bool is_valid (space_t * s, pgsize_e pgsize)
	{ return pgent.is_valid (); }
    
    bool is_writable (space_t * s, pgsize_e pgsize)
	{ return pgent.is_writable (); }
    
    bool is_readable (space_t * s, pgsize_e pgsize)
	{ return pgent.is_valid(); }
    
    bool is_executable (space_t * s, pgsize_e pgsize)
	{ return pgent.is_executable(); }

    bool is_subtree (space_t * s, pgsize_e pgsize)
	{ return !(pgsize == size_4k || (pgsize == size_2m && pgent.is_superpage())); }

    bool is_kernel (space_t * s, pgsize_e pgsize)
	{ return pgent.is_kernel(); }

    // Retrieval

    addr_t address (space_t * s, pgsize_e pgsize)
	{ return pgent.get_address (); }

    pgent_t * subtree (space_t * s, pgsize_e pgsize)
	{ return (pgent_t *) phys_to_virt (pgent.get_ptab ()); }

 
    mapnode_t * mapnode (space_t * s, pgsize_e pgsize, addr_t vaddr) 
	{ return (mapnode_t *) (get_linknode (pgsize) ^ (u64_t) vaddr); }

    addr_t vaddr (space_t * s, pgsize_e pgsize, mapnode_t * map) 
	{ return (addr_t) (get_linknode(pgsize) ^ (u64_t) map); }

    word_t rights (space_t * s, pgsize_e pgsize)
	{ return pgent.is_writable () ? 7 : 5; }
    
    word_t reference_bits (space_t * s, pgsize_e pgsize, addr_t vaddr)
	{
	    word_t rwx = 0;
	    if (pgent.is_accessed ()) { rwx = 5; }
	    if (pgent.is_dirty ()) { rwx |= 6; }
	    return rwx;
	}    

    word_t attributes (space_t * s, pgsize_e pgsize)
	{
	    return (pgent.is_write_through() ? 1 : 0) |
		(pgent.is_cache_disabled() ? 2 : 0) |
		(pgent.is_pat((amd64_pgent_t::pagesize_e) pgsize) ? 4 : 0);
	}

    word_t idx()
        { return (((word_t)this & (word_t)(AMD64_PTAB_BYTES - 1)) / sizeof(pgent_t)); }

    // Modification

    void clear (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
	{
	    pgent.clear ();
	    sync (s, pgsize);
	    
	    if (! kernel)
		set_linknode (pgsize, 0);
	}
    
    void flush (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr)
	{
	}
    
    
#if !defined(CONFIG_SMP)        
    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    word_t size = 
		(!kernel && (pgsize == size_4k || pgsize == size_2m))
		? 2* AMD64_PTAB_BYTES : AMD64_PTAB_BYTES;
	
	    pgent.set_ptab_entry
		(virt_to_phys
		 (kmem.alloc (kmem_pgtab, size)), AMD64_PAGE_USER | AMD64_PAGE_WRITABLE);
	    
	    sync(s, pgsize);
	}
#endif
    void remove_subtree (space_t * s, pgsize_e pgsize, bool kernel)
	{
	    word_t size = 
		(!kernel && (pgsize == size_4k || pgsize == size_2m)) 
		? 2 * AMD64_PTAB_BYTES : AMD64_PTAB_BYTES;
	    
	    addr_t ptab = pgent.get_ptab ();
	    pgent.clear();
	    sync(s, pgsize);
	    kmem.free (kmem_pgtab, phys_to_virt (ptab), size);
	}

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr,
		    word_t rwx, word_t attrib, bool kernel)
	{
	    pgent.set_entry (paddr,
			     (amd64_pgent_t::pagesize_e) pgsize,
			     (kernel ? (AMD64_PAGE_KERNEL | AMD64_PAGE_GLOBAL)
			      : AMD64_PAGE_USER) |
			     (attrib & 1 ? AMD64_PAGE_WRITE_THROUGH : 0) |
			     (attrib & 2 ? AMD64_PAGE_CACHE_DISABLE : 0) |
			     (attrib & 4 ? (pgsize == size_4k ?
					    AMD64_4KPAGE_PAT :
					    AMD64_2MPAGE_PAT) : 0) |
#if defined(CONFIG_SMP)
			     // Preserve sync bit
			     (pgent.must_sync() ? AMD64_PAGE_SYNC : 0) |
#endif
			     (rwx & 2 ? AMD64_PAGE_WRITABLE : 0) |
			     AMD64_PAGE_VALID);

	    sync(s, pgsize);	    
	}

    void set_entry (space_t * s, pgsize_e pgsize, pgent_t pgent)
	{	 
	    this->pgent = pgent.pgent;	
	    sync(s, pgsize);
	}
    
    void set_cacheability (space_t * s, pgsize_e pgsize, bool cacheable)
	{
	    this->pgent.set_cacheability(cacheable, 
					 (amd64_pgent_t::pagesize_e) pgsize);
	    sync(s, pgsize);
	}

    void set_attributes (space_t * s, pgsize_e pgsize, word_t attrib)
	{
	    this->pgent.set_pat(attrib, (amd64_pgent_t::pagesize_e) pgsize);
	    sync(s, pgsize);
	}

    void set_global (space_t * s, pgsize_e pgsize, bool global)
	{
	    pgent.set_global (global);
	    sync (s, pgsize);
	}

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{ if (rwx & 2) raw &= ~AMD64_PAGE_WRITABLE; sync(s, pgsize); }

    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{ if (rwx & 2) raw |= AMD64_PAGE_WRITABLE; sync(s, pgsize); }
    
    void set_rights (space_t * s, pgsize_e pgsize, word_t rwx)
	{
	    if ((rwx & 2) && ! (raw & AMD64_PAGE_WRITABLE))
	    {
		raw |= AMD64_PAGE_WRITABLE;
		sync (s, pgsize);
	    }
	    else if (! (rwx & 2) && (raw & AMD64_PAGE_WRITABLE))
	    {
		raw &= ~AMD64_PAGE_WRITABLE;
		sync (s, pgsize);
	    }
	}

    void reset_reference_bits (space_t * s, pgsize_e pgsize)
	{ raw &= ~(AMD64_PAGE_ACCESSED | AMD64_PAGE_DIRTY); sync(s, pgsize); }

    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx)
	{ raw |= ((rwx >> 1) & 0x3) << 5; }

    void set_linknode (space_t * s, pgsize_e pgsize,
		       mapnode_t * map, addr_t vaddr)
	{ set_linknode (pgsize, (u64_t) map ^ (u64_t) vaddr); }

    // Movement

    pgent_t * next (space_t * s, pgsize_e pgsize, word_t num)
	{ return this + num; }

    // Debug

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	    if (pgent.pg4k.global)
		printf ("global ");
	}
};


#if defined(CONFIG_NEW_MDB)
#undef mapnode_t
#endif


#endif /* !__ARCH__AMD64__PGENT_H__ */
