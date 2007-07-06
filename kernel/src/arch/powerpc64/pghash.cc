/****************************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/page.cc
 * Description:	Manipulates the ppc64 page hash table.
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
 * $Id: pghash.cc,v 1.7 2005/01/19 13:52:06 cvansch Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include INC_ARCH(page.h)
#include INC_ARCH(pghash.h)
#include INC_PLAT(prom.h)
#include INC_ARCH(string.h)
#include INC_GLUE(space.h)
#include INC_ARCH(segment.h)
#include INC_ARCH(msr.h)


static word_t pte_replace_next;


ppc64_pte_t* ppc64_htab_t::find_insertion( word_t virt, word_t vsid, 
	word_t *slot, word_t *is_second_hash, bool large )
{
    ppc64_pte_t *group[2];
    word_t hash;
    int cnt, evict_hash, evict_slot;

    /* Locate the primary PTEGs. */
    hash = this->primary_hash( virt, vsid, large );
    group[0] = this->get_pteg( hash );

    /* Search for an invalid pte, and while searching, keep track of 
     * unreferenced pages.
     */
    for( cnt = 0; cnt < HTAB_PTEG_SIZE; cnt++ )
    {
	if (group[0][cnt].x.bolted == 0)
	{
	    if( group[0][cnt].x.v == 0 )
	    {
		*slot = cnt;
		*is_second_hash = 0;
		return &group[0][cnt];
	    }
	}
    }

    /* Locate the secondary PTEGs. */
    hash = this->secondary_hash( hash );
    group[1] = this->get_pteg( hash );

    for( cnt = 0; cnt < HTAB_PTEG_SIZE; cnt++ )
    {
	if (group[1][cnt].x.bolted == 0)
	{
	    if( group[1][cnt].x.v == 0 )
	    {
		*slot = cnt;
		*is_second_hash = 1;
		return &group[1][cnt];
	    }
	}
    }

    /* We must evict a pte. */
    cnt = 0;
    do {
	// choose a random, unbolted slot.
	evict_hash = (pte_replace_next >> 3) & 1;
	evict_slot = pte_replace_next & 7;
	pte_replace_next ++;

	if ( group[ evict_hash ][ evict_slot ].x.bolted == 0 )
	{
	    *slot = evict_slot;
	    *is_second_hash = evict_hash;
	    return &group[ evict_hash ][ evict_slot ];
	}

	cnt ++;
	if ( cnt >= HTAB_PTEG_SIZE*2 )
	    enter_kdebug( "No unbolted entries found" );
    } while (1);
}

SECTION(".init") void ppc64_htab_t::init( word_t phys_base, 
	word_t virt_start, word_t size )
{
    word_t i;
    this->base = (ppc64_pte_t *)(virt_start | phys_base);
    this->size = size;
    this->phys_base = phys_base;
    for (i = 0; i < 64; i ++)
	if ((1ul<<i) == size)
	    break;
    if ((i > 46) || (i < 18))
	prom_exit("Invalid hash page table size");
    if (phys_base & ((1ul<<i)-1))
	prom_exit("Invalid hash page table alignment");

    this->htab_size = (i-18);
    this->hash_mask = (1ul << (this->htab_size + HTAB_PTEGs_BITS)) - 1;

    prom_puts("Zero hash table memory...");
    of1275_client_interface_t *of = PTRRELOC(get_of1275());
    of->claim((addr_t)phys_base, size, 0);

    // zero_block( (word_t *)virt_start, size );
    for (word_t i=phys_base; i < (phys_base + size); i+=8)
	*(word_t*)i = 0;

    prom_puts(" done.\n\r");
    pte_replace_next = 0;
}

extern word_t realmode_call;
extern word_t virtmode_call;
extern word_t backmode_call;

SECTION(".init") void ppc64_htab_t::activate()
{
    ppc64_sdr1_t sdr1;

    sdr1.create(this->phys_base, this->htab_size);

    /* Install the HASH page table and jump into virtual mode */
    prom_print_hex( "Init MMU, sdr1", sdr1.raw );
    prom_puts( "\n\r" );

    /* Jump into real mode */
    asm volatile (
	"   mtsrr0	%0		\n"	/* Set the jump address */
	"   mtsrr1	%1		\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	"realmode_call:			\n"	/* Jump target		*/
	:: "r" (PTRRELOC(&realmode_call)), "r" (MSR_REAL_MODE)
	: "memory"
    );

    /* Flush the MMU */
    isync();
    ppc64_invalidate_tlb();

    /* Init Segmentation and Insert the entry for the kernel (first 256MB) */
    segment_t::init_cpu( get_kernel_space(), (addr_t)KERNEL_OFFSET );

    /* Setup the Hash Table Pointer */
    ppc64_set_sdr1(sdr1.raw);

    /* Jump into virtual mode */
    asm volatile (
	"   ld	    %%r1,   0(%%r1)	\n"	/* Load stack		*/
	"   add	    %%r1,   %%r1,   %2	\n"	/* Fixup stack		*/
	"   ld	    %%r2,   8(%0)	\n"	/* Load toc		*/
	"   ld	    %0,	    0(%0)	\n"	/* Load dest		*/
	"   mtsrr0  %0			\n"	/* Set the jump address */
	"   mtsrr1  %1			\n"	/* Set the target msr	*/
	"   rfid			\n"	/* Jump			*/

	:: "r" (PTRRELOC(&virtmode_call)), "r" (MSR_INIT_KERNEL_MODE),
	   "r" (KERNEL_OFFSET)
	: "memory"
    );
}

