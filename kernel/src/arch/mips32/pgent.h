/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/pgent.h
 * Description:   Generic page table manipulation for MIPS32
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
 * $Id: pgent.h,v 1.3 2006/11/18 11:04:53 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS32__PGENT_H__
#define __ARCH__MIPS32__PGENT_H__

#include <types.h>
#include <kmemory.h>
#include <debug.h>

#include INC_GLUE(config.h)

#define NUM_PTAB_ENTRIES 1024 

#define PTAB_SIZE NUM_PTAB_ENTRIES * sizeof( pgent_t )

#define PAGEBITS 12


EXTERN_KMEM_GROUP( kmem_pgtab );


class mapnode_t;
class space_t;


class pgent_t {
	
	
public:

    union {

        // page directory entry
        struct {
            word_t address		: 28; // must be 16byte aligned to fit into
            word_t is_subtree	: 1;
            word_t is_kernel	: 1;
            word_t reserved		: 2;
        } pdent;
		
        // page table entry
        struct {
            word_t is_global	: 1;
            word_t is_valid		: 1;
            word_t is_writable	: 1;
            word_t cattr		: 3;
            word_t pfn		: 20;
            word_t padding		: 2;
            word_t is_subtree	: 1;
            word_t is_kernel	: 1;
            word_t reserved		: 2;
        } ptent;

        word_t raw;
    };


    enum pgsize_e {
        size_4k = 0,
        size_4m = 1,		
        size_max = size_4m 
    };

    enum cattr_e {
        uncached	= 0x0,
        cacheable	= 0x3
    };

	
    // -- linknode access

    void set_linknode (pgsize_e pgsize, u32_t val) {
        ASSERT( pdent.is_subtree == 0 && "pgent_t::set_linknode() should not be called on pdir" );
        *(u32_t*)((word_t)this + PTAB_SIZE ) = val;
    }
	
    void set_linknode (space_t * s, pgsize_e pgsize, mapnode_t * map, addr_t vaddr) {
        set_linknode( pgsize,  (u32_t)map^(u32_t)vaddr ); 
    }

    u32_t get_linknode (pgsize_e pgsize) { 
        ASSERT( pdent.is_subtree == 0 && "pgent_t::get_linknode() should not be called on pdir" );
        return( *(u32_t *)((word_t)this + PTAB_SIZE) ); 
    }
   

    // -- predicated
	
    bool is_valid (space_t * s, pgsize_e pgsize) {
        return( raw != 0 );
    }

    bool is_readable (space_t * s, pgsize_e pgsize) {
        return( ptent.is_valid == 1 );
    }

    bool is_writable (space_t * s, pgsize_e pgsize) {
        return( ptent.is_writable == 1 );
    }

    bool is_executable (space_t * s, pgsize_e pgsize) {
        return( ptent.is_valid == 1 );
    }

    bool is_subtree (space_t * s, pgsize_e pgsize) {
        return( pdent.is_subtree == 1 );	
    }

    bool is_kernel( space_t * s, pgsize_e pgsize ) {
        return( pdent.is_kernel == 1 );	
    }
	

    // -- retrieval
	
    addr_t address (space_t * s, pgsize_e pgsize) {
        return( (addr_t)(ptent.pfn << 12) );
    }
	
    pgent_t* subtree (space_t * s, pgsize_e pgsize) {
        ASSERT( pdent.is_subtree == 1 && "pgent_t::subtree() called on ptab entry" );
        return( (pgent_t*)(pdent.address << 4) );
    }

    mapnode_t* mapnode (space_t * s, pgsize_e pgsize, addr_t vaddr) {
        return( (mapnode_t *) (get_linknode(pgsize) ^ (u32_t)vaddr) ); 
    }
	
    addr_t vaddr( space_t* s, pgsize_e pgsize, mapnode_t* map ) {
        return( (addr_t)(get_linknode(pgsize) ^ (u32_t) map) );
    }

    word_t reference_bits (space_t * s, pgsize_e pgsize, addr_t vaddr) {
		
        ASSERT( pdent.is_subtree == 0 && "pgent_t::reference_bits() called on pdir entry" );
        // XXX this does not always return the correct result (there is no hardware reference bit)	
        if( !ptent.is_valid ) {
            return( 0 );
        }
        else if( ptent.is_writable ) {
            return( 6 );
        }
        else {
            return( 4 );
        }
    }

    word_t attributes (space_t * s, pgsize_e pgsize) {
	return 0;
    }

    // -- modification

    void clear (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr) {
        ASSERT( pdent.is_subtree == 0 && "pgent_t::clear() should not be called on pdir" );
        this->raw = 0;
        if( !kernel ) set_linknode( pgsize, 0 );
    }
	
    void flush (space_t * s, pgsize_e pgsize, bool kernel, addr_t vaddr) {
        ASSERT(!"pgent_t::flush is not implemented");
        // XXX
    }

    void make_subtree (space_t * s, pgsize_e pgsize, bool kernel) {
		
        void* mem = (void*)kmem.alloc( kmem_pgtab, kernel ? PTAB_SIZE : 2 * PTAB_SIZE );
        ASSERT( ((word_t)mem & 0xf) == 0 && "pgent_t::make_subtree(): memory is not 16 byte aligned" );

        pdent.address = ((word_t)mem) >> 4;
        pdent.is_subtree = 1;
    }

    void remove_subtree (space_t * s, pgsize_e pgsize, bool kernel) {
        ASSERT( pdent.is_subtree == 1 && "pgent_t::remove_subtree() should not be called on ptab entry" );

        kmem.free( kmem_pgtab, (void*)(pdent.address << 4), kernel ? PTAB_SIZE : 2 * PTAB_SIZE );
        raw = 0;
    }

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr, word_t rwx, word_t attrib, bool kernel) {
        ptent.is_valid = 1;
        ptent.is_writable = writable;
        ptent.pfn = ((word_t)paddr >> PAGEBITS);
        ptent.is_subtree = 0;
        ptent.is_kernel = kernel;
        ptent.cattr = attrib;
    }

    void set_entry (space_t * s, pgsize_e pgsize, addr_t paddr, word_t rwx, bool kernel) {
	set_entry( s, pgsize, paddr, rwx, cattr_e::uncached, kernel);
    }

    void set_global (space_t * s, pgsize_e pgsize, bool value) {
        ASSERT( pdent.is_subtree == 0 && "pgent_t::set_global() should not be called on pdir" );
        ptent.is_global = ( value ? 1 : 0 );
    }	

    void set_entry (space_t * s, pgsize_e pgsize, pgent_t pgent) {
        raw = pgent.raw;
    }

    void set_attributes (space_t * s, pgsize_e pgsize, word_t attrib) {
        ptent.cattr = attrib;
    }

    void revoke_rights (space_t * s, pgsize_e pgsize, word_t rwx) {
        ASSERT( !"pgent_t::revoke_rights not implemented" ); 
        // XXX
    }

    void update_rights (space_t * s, pgsize_e pgsize, word_t rwx) {
        ptent.is_writable = (rwx & 2) ? 1 : 0;
        ptent.is_valid = (rwx & 4 ) ? 1 : 0;
    }

    void reset_reference_bits (space_t * s, pgsize_e pgsize) {
        // there is not real reference bit
    }
   	
    void update_reference_bits (space_t * s, pgsize_e pgsize, word_t rwx) {
        // there is not real reference bit
    }


    // -- movement

    pgent_t* next (space_t* s, pgsize_e pgsize, word_t num) {
        return( this + num );
    }

    // -- debug

    void dump_misc (space_t * s, pgsize_e pgsize)
	{
	}
};

#endif /* !__ARCH__MIPS32__PGENT_H__ */
