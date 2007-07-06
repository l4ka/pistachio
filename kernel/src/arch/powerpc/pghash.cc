/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/page.cc
 * Description:	Manipulates the ppc page hash table.
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
 * $Id: pghash.cc,v 1.8 2003/09/24 19:05:30 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include INC_ARCH(bat.h)
#include INC_ARCH(page.h)
#include INC_ARCH(pghash.h)
#include INC_ARCH(string.h)
#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(msr.h)

#include INC_GLUE(hwspace.h)
#include INC_GLUE(bat.h)


ppc_translation_t * ppc_htab_t::find_insertion( word_t virt, word_t vsid, 
	word_t *slot, word_t *is_second_hash )
{
    ppc_translation_t *groups[2];
    word_t hash;
    int cnt, group, clean_slot, clean_hash;

    /* If we find an unreferenced page, keep track of it for eviction. */
    clean_hash = -1;
    clean_slot = -1;

    /* Locate the primary and secondary PTEGs. */
    hash = this->primary_hash( virt, vsid );
    groups[0] = this->get_pteg( hash );
    hash = this->secondary_hash( hash );
    groups[1] = this->get_pteg( hash );

    /* Search for an invalid pte, and while searching, keep track of 
     * unreferenced pages.
     */
    for( group = 0; group < 2; group++ )
	for( cnt = 0; cnt < HTAB_PTEG_SIZE; cnt++ )
	    if( groups[group][cnt].x.v == 0 ) {
		*slot = cnt;
		*is_second_hash = group;
		return &groups[group][cnt];
	    }
	    else if( !groups[group][cnt].x.r && !groups[group][cnt].x.c ) {
		clean_slot = cnt;
		clean_hash = group;
	    }

    /* We must evict a pte. */
    if( clean_slot == -1 ) {
	// Unable to find an unreferenced pte, so choose a random slot.
	clean_hash = 0;
	clean_slot = (virt >> POWERPC_PAGE_BITS) & 7;
    }
    *slot = clean_slot;
    *is_second_hash = clean_hash;
    return &groups[ clean_hash ][ clean_slot ];
}

SECTION(".init.memory") void ppc_htab_t::bat_map( void )
{
    ppc_bat_t bat;
    
    /*  Map with a bat register. */
    bat.raw.upper = bat.raw.lower = 0;
    bat.x.bepi = (word_t)this->base >> BAT_BEPI;
    bat.x.bl = (this->size-1) >> 17;
    bat.x.vs = 1;
    bat.x.brpn = this->phys_base >> BAT_BRPN;
    bat.x.m = 1;
    bat.x.pp = BAT_PP_READ_WRITE;

    ppc_set_pghash_dbat( l, bat.raw.lower );
    ppc_set_pghash_dbat( u, bat.raw.upper );
    isync();
}

SECTION(".init.memory") void ppc_htab_t::init( word_t phys_base, 
	word_t virt_start, word_t size )
{
    this->base = (ppc_translation_t *)virt_start;
    this->size = size;
    this->phys_base = phys_base;
    this->htab_mask = (size-1) >> 16;
    this->hash_mask = (this->htab_mask << 10) | ((1 << 10) - 1);

    /* Activate the bat register, and zero the memory region (which invalidates
     * all PTE's.
     */
    this->bat_map();
    zero_block( (word_t *)virt_start, size );
}


/* Locations in the inlined assembler in ppc_htab_install() */
extern "C" void ppc_htab_install_real( void );
extern "C" void ppc_htab_install_real_exit( void );

SECTION(".init.memory") void ppc_htab_install( ppc_sdr1_t sdr1, ppc_segment_t segment_val )
{
    word_t real_entry;
    word_t msr_off_mask, msr_on_mask;

    /* Invalidate the tlb!!! */
    asm volatile ("isync");
    ppc_invalidate_tlb();

    /* While in real mode (important!), install the sdr1 register and 
     * set the segment IDs.
     */
    real_entry = virt_to_phys((word_t)ppc_htab_install_real);
    msr_on_mask = (MSR_IR_ENABLED << MSR_IR) | (MSR_DR_ENABLED << MSR_DR);
    msr_off_mask = ~msr_on_mask;
    asm volatile (
	    "mfmsr %%r10 ;"		// Get the current msr.
	    "and %%r10, %%r10, %4 ;"	// Disable address translation.
	    "mtsrr1 %%r10 ;"		// Prepare to activate the new msr.
	    "mtsrr0 %0 ;"	// Prepare to jump to ppc_htab_install_real
	    "rfi ;"			// Jump to real mode.

	    "ppc_htab_install_real: ;"

	    "sync ;"			// Prepare to change the page hash.
	    "mtspr 25, %1 ;"		// Install the page hash.
	    "isync ;"			// Activate the page hash.

	    /* Set the segment ID. */
	    "mr %%r10, %2 ;"		// Prepare to set the segment ID.
	    "mtsr 0, %%r10 ; addi %%r10, %%r10, 1 ;"	
	    "mtsr 1, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 2, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 3, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 4, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 5, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 6, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 7, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 8, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 9, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 10, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 11, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 12, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 13, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 14, %%r10 ; addi %%r10, %%r10, 1 ;"
	    "mtsr 15, %%r10 ;"
	    "isync ;"			// Activate the new segment ID.

	    "mfmsr %%r10 ;"		// Get the current msr.
	    "or %%r10, %%r10, %5 ;"	// Enable address translation.
	    "mtsrr1 %%r10 ;"		// Prepare to activate the new msr.
	    "mtsrr0 %3 ;"		// Prepare to jump to virtual mode.
	    "rfi ;"			// Jump to virtual mode.
	    "ppc_htab_install_real_exit:"
	    : 
	    : "b" (real_entry), "b" (sdr1.raw),
	      "b" (segment_val.raw), "b" (ppc_htab_install_real_exit), 
	      "b" (msr_off_mask), "b" (msr_on_mask)
	    : "ctr", "10" );
}

SECTION(".init.memory") void ppc_htab_t::activate( ppc_segment_t segment_val )
{
    ppc_sdr1_t sdr1;

    sdr1.x.htaborg = (word_t)this->phys_base >> POWERPC_HTABORG_SHIFT;
    sdr1.x.htabmask = this->htab_mask;

    ppc_htab_install( sdr1, segment_val );
}


