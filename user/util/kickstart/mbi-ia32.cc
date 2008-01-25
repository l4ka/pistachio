/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     mbi-ia32.cc
 * Description:   IA32 specific multiboot info stuff
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
 * $Id: mbi-ia32.cc,v 1.7 2007/06/01 11:49:22 stoess Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>
#include <l4/kip.h>

#include "kickstart.h"
#include "mbi.h"
#include "lib.h"
#include "kipmgr.h"

extern unsigned int max_phys_mem;
extern unsigned int additional_kmem_size;


// The kernel cannot use memory beyond this limit
#define MAX_KMEM_END            (240*1024*1024)

L4_Word_t grub_mbi_ptr;
L4_Word_t grub_mbi_flags;


class mmap_t {
public:
    L4_Word32_t   desc_size;
    L4_Word64_t   base;
    L4_Word64_t   size;
    L4_Word32_t   type;
};


mbi_t* mbi_t::prepare(void)
{
    if (grub_mbi_flags == 0x2BADB002)
        return (mbi_t*) grub_mbi_ptr;
    else
        return 0;
}


void install_memory(mbi_t * mbi, kip_manager_t* kip)
{
    // Mark all physical memory as shared by default to allow for
    // device access
    kip->dedicate_memory(0x0, ~0UL, L4_SharedMemoryType, 0);
    
    // Does the MBI contain a reference to the BIOS memory map?
    if (mbi->flags.mmap)
    {
        // Pointer to first entry in table
        mmap_t* m = (mmap_t*) mbi->mmap_addr;

        // Iterate over all entries
        while ((L4_Word_t) m < (mbi->mmap_addr + mbi->mmap_length))
        {
            /* Limit physical memory if necessary (max_phys_mem != 0)
               We assume that there one big chunk of main memory that
               is larger than 1MB, so we simply adjust its size. */
            if ((max_phys_mem != 0) && (m->size > 1024ULL*1024ULL))
                m->size = max_phys_mem - m->base;

            /* Mark "usable" memory (type=1) as conventional physical
               memory, everything else as architecture specific with
               the BIOS memory map type as subtype */
	    kip->dedicate_memory(m->base, m->base + m->size - 1,
                                 (m->type == 1)
                                 ? L4_ConventionalMemoryType
                                 : L4_ArchitectureSpecificMemoryType,
                                 m->type);

            /* Skip forward by the number of bytes specified in the
               structure. This can be more than just the 24 bytes */
            m = (mmap_t*) ((L4_Word_t) m + 4 + m->desc_size);
        }

        if (additional_kmem_size)
        {
	    if (additional_kmem_size >= MAX_KMEM_END)
		additional_kmem_size = MAX_KMEM_END - (8 * 1024 * 1024);
	    
            // Second round: Find a suitable KMEM area
            m = (mmap_t*) mbi->mmap_addr;
            // Iterate over all entries
            while ((L4_Word_t) m < (mbi->mmap_addr + mbi->mmap_length))
            {
                if ((m->type == 1) &&
                    (m->size >= additional_kmem_size) &&
                    (m->base <= (MAX_KMEM_END-additional_kmem_size)))
                {
                    L4_Word_t base, end;
                    
                    // Make sure the end is within kernel's reach
                    end = m->base+m->size < MAX_KMEM_END ? m->base+m->size : MAX_KMEM_END ;
                    base = end - additional_kmem_size;
		    
		    L4_Word_t mod_base = end, total_mod_size = 0;
		    
		    for (L4_Word_t i = 0; i < mbi->modcount; i++)
		    {
			if (is_intersection (base, end, mbi->mods[i].start, mbi->mods[i].end))
			{
			    L4_Word_t mod_size = mbi->mods[i].end - mbi->mods[i].start;
			    total_mod_size += mod_size;
			    
			    // Move modules that are in the way, if possible
			    if (m->size > additional_kmem_size + total_mod_size)
			    {
				printf(" relocate mod %d %x -> %x size %d\n", i, 
				       mbi->mods[i].start, mod_base, mod_size);

				memcopy(mod_base, mbi->mods[i].start, mod_size);
				
				mbi->mods[i].start = mod_base;
				mbi->mods[i].end   = mod_base + mod_size;
				

				mod_base += ROUND_UP(mod_size, 4096);
			    }
			    else
				
				base = ROUND_UP(mbi->mods[i].end, (4 * 1024 * 1024));
			}
		    }
                    // Mark the memory block as in use by the kernel
                    kip->dedicate_memory(base, end - 1, 
                                         L4_ReservedMemoryType, 0);
                    // Stop looking
                    break;
                }
                /* Skip forward by the number of bytes specified in the
                   structure. This can be more than just the 24 bytes */
                m = (mmap_t*) ((L4_Word_t) m + 4 + m->desc_size);
            }

        }
    }
    else
    {
        printf("No BIOS memory map. Using old-style MBI fields.\n");

        // mbi.mem_lower is the number of KBs below 1MB starting at 0
        kip->dedicate_memory(0x00000000ULL,
                             mbi->mem_lower * 1024ULL - 1, 
                             L4_ConventionalMemoryType, 0);

        // mbi.mem_upper is the number of KBs above 1MB
        L4_Word_t end = 0x00100000ULL + mbi->mem_upper * 1024ULL;

        // Override the end of physical memory if requested
        if (max_phys_mem != 0)
            end = max_phys_mem;

        kip->dedicate_memory(0x00100000ULL, end - 1, 
                             L4_ConventionalMemoryType, 0);

        if (additional_kmem_size)
        {
            /* Mark a memory block of appropriate size at the end of
               conventional memory as reserved for the kernel */
            kip->dedicate_memory(end - additional_kmem_size, end - 1,
                                 L4_ReservedMemoryType, 0);
        }
    }

    /* The standard PC's VGA memory hasn't been seen in any BIOS
     * memory map so far. So we fake an entry for it. */
    kip->dedicate_memory(0xA0000, 0xC0000 - 1, 
                         L4_SharedMemoryType, 0);
    
    /* Standard PC's may have VGA and Extension ROMs -- fake
     * another entry */
    kip->dedicate_memory(0xC0000, 0xF0000 - 1, 
                         L4_SharedMemoryType, 0);
}
