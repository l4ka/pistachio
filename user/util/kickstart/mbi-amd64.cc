/*********************************************************************
 *                
 * Copyright (C) 2003-2004, 2006, 2008,  Karlsruhe University
 *                
 * File path:     mbi-amd64.cc
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
 * $Id: mbi-amd64.cc,v 1.4 2006/02/21 11:43:35 stoess Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4io.h>

#include "lib.h"
#include "kickstart.h"
#include "mbi.h"
#include "kipmgr.h"

/* Override physical memory size from BIOS when non-zero.  Variable
   can be set with command line parameter maxmem=<num>[KMG] */
extern unsigned int max_phys_mem;
extern unsigned int additional_kmem_size;


// The kernel cannot use memory beyond this limit jsXXX: Not needed on AMD64
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


void install_memory(mbi_t* mbi, kip_manager_t * kip)
{
    // Mark all physical memory as shared by default to allow for
    // device access
    kip->dedicate_memory(0x0, ~0ULL, L4_SharedMemoryType, 0);

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
	    if (m->type == 1 &&
		max_phys_mem != 0 &&
		m->size > 1024UL*1024UL &&
		(L4_Word64_t) m->base + (L4_Word64_t) m->size > max_phys_mem)
	    {
		//printf("max_phys_mem = %x, base = %x, size = %x\n", 
		//       max_phys_mem, (L4_Word_t) m->base, (L4_Word_t) m->size);
		kip->dedicate_memory(
		    max_phys_mem, 
		    m->base + m->size - 1,
		    L4_ReservedMemoryType,
		    0);
		
		m->size = max_phys_mem - m->base;
	    }

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
        
        /* The standard PC's VGA memory hasn't been seen in any BIOS
         * memory map so far. So we fake an entry for it. */
        kip->dedicate_memory(0xA0000, 0xC0000 - 1, 
                             L4_SharedMemoryType, 0);

	/* Standard PC's may have VGA and Extension ROMs -- fake
	 * another entry */
        kip->dedicate_memory(0xC0000, 0xF0000 - 1, 
                             L4_SharedMemoryType, 0);

        if (additional_kmem_size)
        {
            // Second round: Find a suitable KMEM area
            m = (mmap_t*) mbi->mmap_addr;
            // Iterate over all entries
            while ((L4_Word_t) m < (mbi->mmap_addr + mbi->mmap_length))
            {
		if (m->type == 1)
		    {

			/*
			 * We want that the chunk be 2 MByte aligned
			 */ 

			
			L4_Word_t useable_base = ROUND_UP(m->base, MB(2));
			L4_Word_t useable_size = 
			    min((L4_Word_t) ROUND_DOWN(m->size - (useable_base - m->base), MB(2)),
				(L4_Word_t) ROUND_DOWN(MAX_KMEM_END, MB(2)));
			L4_Word_t useable_end = useable_base + useable_size;

			if (useable_size >= ROUND_UP(additional_kmem_size, MB(2)))
			{
			    
			    // Make sure the end is within kernel's reach
			    // Mark the memory block as in use by the kernel
			    kip->dedicate_memory(useable_end - ROUND_UP(additional_kmem_size, MB(2)), 
						 useable_end -1, 
						 L4_ReservedMemoryType, 0);
			    // Stop looking
			    break;
			}
                }
                /* Skip forward by the number of bytes specified in the
                   structure. This can be more than just the 24 bytes */
                m = (mmap_t*) ((L4_Word_t) m + 4 + m->desc_size);
            }

        }
    }
    else
    {
        printf("Ooops! No BIOS memory map.\n");
        while(1);
    }
}
