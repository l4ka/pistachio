/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/tlb.cc
 * Description:   MIPS32 TLB management
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
 * $Id: tlb.cc,v 1.1 2006/02/23 21:07:45 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(tlb.h)
#include INC_ARCH(cp0regs.h)
#include INC_ARCH(mips_cpu.h)
#include <debug.h>

tlb_t tlb;

static word_t tlb_size;

void SECTION(".init") tlb_t::init() {

    tlb_size = get_mips_cpu()->get_tlb_size();
    ASSERT( tlb_size == 16  && "set mips_cpu_t::tlb_size properly before calling any tlb functions" );
	
    unsigned pagemask = 0x0; // 4k pages
    unsigned wired   = 0;  // kernel stack

    __asm__ __volatile__("mtc0 %0, "STR(CP0_PAGEMASK)"\n\t"
                         "mtc0 %1, "STR(CP0_WIRED)""
                         :
                         : "r"(pagemask), "r"(wired)
	);
	
    for( unsigned i = 0; i < tlb_size; i++ ) {
		
        __asm__ __volatile__(
            "mtc0	%0, "STR(CP0_INDEX)"	\n\t"
            "mtc0	%1, "STR(CP0_ENTRYHI)"	\n\t"
            "mtc0	$0, "STR(CP0_ENTRYLO0)"	\n\t"
            "mtc0	$0, "STR(CP0_ENTRYLO1)"	\n\t"
            "nop;nop						\n\t"
            "tlbwi"
            :
            : "r"( i ), "r"( get_invalid() )
            );

    }
}


word_t tlb_t::get_invalid() {

    static word_t invalid = KSEG0_BASE;
    invalid += 0x2000;
    if( EXPECT_FALSE(invalid >= KSEG1_BASE) ) invalid = KSEG0_BASE;
    return( invalid );
}


/* flush all asid-tagged tlb entries */
void tlb_t::flush( word_t asid, word_t vaddr, word_t page_size ) {
    
    ASSERT(page_size == 12);
    
    word_t entryhi, entryhi_bak;
    word_t index;
    word_t global = 0x1;
    
    __asm__ __volatile__ (
        "mfc0	%0,"STR(CP0_ENTRYHI)"\n\t"
        : "=r" (entryhi_bak)
        );

    entryhi = ( vaddr & 0xffffe000 ) | asid;

    __asm__ __volatile__ (
        "mtc0	%1,"STR(CP0_ENTRYHI)"	\n\t"
        "nop;nop;						\n\t"
        "tlbp;							\n\t"
        "nop;nop;						\n\t"
        "mfc0	%0,"STR(CP0_INDEX)"		\n\t"
        : "=r" (index) : "r" (entryhi)
        );

    if( !(index & 0x80000000) )  { 
        // -- match
	
        __asm__ __volatile__ (
            "tlbr			\n\t"
            "nop;nop;nop;	\n\t"
            );

        if( vaddr & ( 1 << 12 ) ) {	    /* Odd entry */
            __asm__ __volatile__ (
                "mtc0	%0,"STR(CP0_ENTRYLO1)"\n\t"
                : 
                : "r" (global)
                );
        } 
        else {  /* Even entry */
            __asm__ __volatile__ (
                "mtc0	%0,"STR(CP0_ENTRYLO0)"\n\t"
                : 
                : "r" (global)
                );
        }
		
        __asm__ __volatile__ (
            "nop;nop\n\t"
            "tlbwi\n\t"
            "nop;nop;nop;\n\t"
            );

    }
	
    __asm__ __volatile__ (
        "mtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
        : 
        : "r" (entryhi_bak)
        );
}


void tlb_t::flush( word_t asid ) {
	
    unsigned entryhi_bak;
    unsigned wired;

    __asm__ __volatile__ (
        "mfc0 %0, "STR(CP0_ENTRYHI)"\n\t"
        : "=r" (entryhi_bak)
	);

    // don't need this...
    __asm__ __volatile__ (
        "mtc0	$0, "STR(CP0_ENTRYLO0)";nop;nop;nop\n\t"
        "mtc0	$0, "STR(CP0_ENTRYLO1)";nop;nop;nop\n\t"
        "mfc0	%[wired], "STR(CP0_WIRED)";nop;nop;nop\n\t"
        : [wired] "=r" (wired)
	);		
	
    for( unsigned i = wired; i < tlb_size; i++ ) {
        __asm__ __volatile__(
            "mtc0 %0, "STR(CP0_INDEX)"\n\t"	
            "nop;nop;nop\n\t"
            "tlbr\n\t"
            "mfc0 $8, "STR(CP0_ENTRYHI)"\n\t"
            "nop;nop;nop\n\t"
            "and  $8, $8, 0xff\n\t"
            "bne  $8, %1, 1f\n\t"
            "nop\n\t"
            "mtc0	%2, "STR(CP0_ENTRYHI)";nop;nop;nop\n\t"
            "tlbwi;nop;nop;nop\n\t"
            "1:"
            : 
            : "r"(i), "r"(asid), "r"(get_invalid())
            : "$8"
            );
    }

    __asm__ __volatile__ (
        "mtc0 %0, "STR(CP0_ENTRYHI)"\n\t"
        :
        : "r" (entryhi_bak)
	);
}


void tlb_t::put( word_t vaddr, word_t asid, pgent_t* pg ) {
	
    word_t entryhi_bak;
    word_t entryhi, entrylo;
    word_t index;

    entryhi = ( vaddr & 0xffffe000 ) | asid;
    entrylo = pg->raw;

    __asm__ __volatile__ (
        "mfc0	%0,"STR(CP0_ENTRYHI)"\n\t"
        : "=r" (entryhi_bak)
	);

    __asm__ __volatile__ (
        "mtc0	%1,"STR(CP0_ENTRYHI)"\n\t"
        "nop;nop;nop;\n\t"
        "tlbp;\n\t"
        "nop;nop;nop;\n\t"
        "mfc0	%0,"STR(CP0_INDEX)"\n\t"
        : "=r" (index) 
        : "r" (entryhi)
	);

    if( index & 0x80000000)  { 

        // -- no match
        word_t ck21_0, ck21_1;

        if( vaddr & ( 1 << 12 ) ) {
	    /* Odd entry */
            ck21_0 = 0x1;	/* Set global bit (both must be global for kernel) */
            ck21_1 = entrylo;
        } 
        else {
            /* Even entry */
            ck21_0 = entrylo;
            ck21_1 = 0x1;
        }

        __asm__ __volatile__ (
            "mtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
            "mtc0	%1,"STR(CP0_ENTRYLO0)"\n\t"
            "mtc0	%2,"STR(CP0_ENTRYLO1)"\n\t"
            "nop;nop;nop;\n\t"
            "tlbwr\n\t"
            "nop;nop;nop;\n\t"
            : 
            : "r" (entryhi), "r" (ck21_0), "r" (ck21_1)
            );
    }
    else {
        
        // -- match 
        __asm__ __volatile__ (
            "tlbr\n\t"
            "nop;nop;nop;\n\t"
            );
        
        if( vaddr & ( 1 << 12 ) ) {	/* Odd entry */
            
            __asm__ __volatile__ (
                "mtc0	%0,"STR(CP0_ENTRYLO1)"\n\t"
                : 
                : "r" (entrylo)
                );
        }   
        else { /* Even entry */
            
            __asm__ __volatile__ (
                "mtc0	%0,"STR(CP0_ENTRYLO0)"\n\t"
                : 
                : "r" (entrylo)
                );
        }
        
        __asm__ __volatile__ (
            "nop;nop;nop;\n\t"
            "tlbwi\n\t"
            "nop;nop;nop;\n\t"
            );
    }

    __asm__ __volatile__ (
        "mtc0	%0,"STR(CP0_ENTRYHI)"\n\t"
        : 
        : "r" (entryhi_bak)
	);
}




#if 0
void tlb_t::print( ) {
    
    unsigned entryhi_bak;
    unsigned entryhi, entrylo0, entrylo1;
    
    printf("========================= CURRENT TLB =========================\n");
    
    __asm__ __volatile__ (
        "mfc0 %0, "STR(CP0_ENTRYHI)"\n\t"
        : "=r" (entryhi_bak)
	);
    
    for( unsigned i = 0; i < 16; i++ ) {
        __asm__ __volatile__(
            "mtc0 %3, "STR(CP0_INDEX)"\n\t"	
            "nop;nop;nop\n\t"
            "tlbr\n\t"
            "mfc0 %0, "STR(CP0_ENTRYHI)"\n\t"
            "nop;nop;nop\n\t"
            "mfc0 %1, "STR(CP0_ENTRYLO0)"\n\t"
            "nop;nop;nop\n\t"
            "mfc0 %2, "STR(CP0_ENTRYLO1)"\n\t"
            "nop;nop;nop\n\t"
            : "=r"(entryhi), "=r"(entrylo0), "=r"(entrylo1)
            : "r"(i)
            );
        printf("    %x: 0x%x -> 0x%x (%c%c%c), 0x%x (%c%c%c), ASID = 0x%x, \n",
               i, entryhi & 0xffffe000, 
               (entrylo0 & 0x03ffffc0) << 6, entrylo0 & 4 ? 'D' : '-', entrylo0 & 2 ? 'V' : '-', entrylo0 & 1 ? 'G' : '-', 
               (entrylo1 & 0x03ffffc0) << 6, entrylo1 & 4 ? 'D' : '-', entrylo1 & 2 ? 'V' : '-', entrylo1 & 1 ? 'G' : '-', 
               entryhi & 0xff );  
    }
    
    __asm__ __volatile__ (
        "mtc0 %0, "STR(CP0_ENTRYHI)"\n\t"
        :
        : "r" (entryhi_bak)
	);
    
    printf("===============================================================\n");
}
#endif 
