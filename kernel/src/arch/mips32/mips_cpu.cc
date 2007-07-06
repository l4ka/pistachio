/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/mips_cpu.cc
 * Description:   Basic MIPS32 CPU management
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
 * $Id: mips_cpu.cc,v 1.1 2006/02/23 21:07:45 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(mips_cpu.h)
#include INC_ARCH(cp0regs.h)

#include <debug.h>

mips_cpu_t mips_cpu;


void mips_cpu_t::cli (void) {
	
    __asm__ __volatile__ (
        ".set	push		\n"
        ".set	reorder		\n"
        ".set	noat		\n"
        "mfc0	$1,$12		\n"
        "ori	$1,1		\n"
        "xori	$1,1		\n"
        ".set	noreorder	\n"
        "mtc0	$1,$12		\n"
        "sll	$0, $0, 1;	# nop\n"
        "sll	$0, $0, 1;	# nop\n"
        "sll	$0, $0, 1;	# nop\n"
        ".set	pop;\n\t"
        ::: "$1", "memory"
	);

}


void mips_cpu_t::restore_mask (unsigned int mask) {
	
    word_t temp = ~(0x00ff00ul);
    __asm__ __volatile__ (
        ".set	push		\n"
        ".set	reorder		\n"
        ".set	noat		\n"
        "mfc0	$1,$12		\n"
        "and	$1, $1, %[temp]		\n"
        "sll	%0, %0, 8		\n"
        "or	$1, $1, %0		\n"
        "mtc0	$1, $12			\n"
        "sll	$0, $0, 1;	# nop\n"
        "sll	$0, $0, 1;	# nop\n"
        "sll	$0, $0, 1;	# nop\n"
        ".set	pop;\n\t"
        :: "r" (mask), [temp] "r" (temp) : "$1", "memory"
        );

}


void mips_cpu_t::sti (void) {
	
    __asm__ __volatile__ (
        ".set	push	    \n"
        ".set	reorder	    \n"
        ".set	noat	    \n"
        "mfc0	$1,$12	    \n"
        "ori	$1,0x01	    \n"
        "mtc0	$1,$12	    \n"
        ".set	pop	    \n"
        ::: "$1"
	);

}


void mips_cpu_t::sleep (void) {

    __asm__ __volatile__ (
        ".set push		\n\t"
        ".set noreorder	\n\t"
        "nop			\n\t"
        "wait			\n\t"
        "nop			\n\t"
        ".set reorder	\n\t"
        ".set pop		\n\t"
	);
}


unsigned int mips_cpu_t::save_flags (void) {
	
    ASSERT( !"mips_cpu_t::save_flags() not implemented" );
    return 0;
	
#if 0
    unsigned int temp;
    
    __asm__ __volatile__ (
        ".set\tpush\n\t"
        ".set\treorder\n\t"
        "mfc0\t\\temp, $12\n\t"
        ".set\tpop\n\t"
        );
    return temp;
#endif
}

void mips_cpu_t::restore_flags (unsigned int flags) {

    ASSERT( !"mips_cpu_t::restore_flags() not implemented" );

#if 0
    __asm__ __volatile__ (
    	".set\tnoreorder\n\t"
    	".set\tnoat\n\t"
    	"mfc0\t$1, $12\n\t"
    	"andi\t\\flags, 1\n\t"
    	"ori\t$1, 1\n\t"
    	"xori\t$1, 1\n\t"
    	"or\t\\flags, $1\n\t"
    	"mtc0\t\\flags, $12\n\t"
    	"sll\t$0, $0, 1\t\t\t# nop\n\t"
    	"sll\t$0, $0, 1\t\t\t# nop\n\t"
    	"sll\t$0, $0, 1\t\t\t# nop\n\t"
    	".set\tat\n\t"
    	".set\treorder\n\t"
        );
#endif
}


#define __BUILD_SET_CP0(name,register)					                \
unsigned int mips_cpu_t::set_cp0_##name (unsigned int set)		                \
{									                \
    unsigned int res;							                \
									                \
    res = read_32bit_cp0_register(register);				                \
    res |= set;								                \
    write_32bit_cp0_register(register, res);				                \
									                \
    return res;								                \
}									                \
									                \
unsigned int mips_cpu_t::clear_cp0_##name (unsigned int clear)		                \
{									                \
    unsigned int res;							                \
									                \
    res = read_32bit_cp0_register(register);				                \
    res &= ~clear;							                \
    write_32bit_cp0_register(register, res);                                            \
                                                                                        \
    return res;										\
}											\
											\
unsigned int mips_cpu_t::change_cp0_##name (unsigned int mask, unsigned int newbits)	\
{											\
    unsigned int res;									\
											\
    res = read_32bit_cp0_register(register);						\
    res &= ~mask;									\
    res |= (mask & newbits);								\
    write_32bit_cp0_register(register, res);						\
											\
    return res;										\
}


__BUILD_SET_CP0(status,CP0_STATUS)
__BUILD_SET_CP0(cause,CP0_CAUSE)
__BUILD_SET_CP0(config,CP0_CONFIG)



void mips_cpu_t::enable_fpu (void) {

    set_cp0_status(ST_CU1);
    asm("nop;nop;nop;nop"); 
}


void mips_cpu_t::disable_fpu (void) {

    clear_cp0_status(ST_CU1);
}



void mips_cpu_t::set_tlb_size( word_t size ) {

    this->tlb_size = size;
}


word_t mips_cpu_t::get_tlb_size() {

    return( this->tlb_size );
}



void mips_cpu_t::set_procid( procid_e id ) {

    this->procid = id;
}

mips_cpu_t::procid_e mips_cpu_t::get_procid() {
    
    return( this->procid );
}




void mips_cpu_t::set_icache_spw( mips_cpu_t::cache_spw_e sets ) {

    this->icache_spw = sets;
}


mips_cpu_t::cache_spw_e mips_cpu_t::get_icache_spw() {

    return( this->icache_spw );
}


void mips_cpu_t::set_dcache_spw( mips_cpu_t::cache_spw_e sets ) {

    this->dcache_spw = sets;
}


mips_cpu_t::cache_spw_e mips_cpu_t::get_dcache_spw() {

    return( this->dcache_spw );
}



void mips_cpu_t::set_icache_ls( mips_cpu_t::cache_ls_e size ) {

    this->icache_ls = size;
}


mips_cpu_t::cache_ls_e mips_cpu_t::get_icache_ls() {

    return( this->icache_ls );
}


void mips_cpu_t::set_dcache_ls( mips_cpu_t::cache_ls_e size ) {

    this->dcache_ls = size;
}


mips_cpu_t::cache_ls_e mips_cpu_t::get_dcache_ls() {

    return( this->dcache_ls );
}



void mips_cpu_t::set_icache_assoc( mips_cpu_t::cache_assoc_e assoc ) {
    this->icache_assoc = assoc;
}


mips_cpu_t::cache_assoc_e mips_cpu_t::get_icache_assoc() {

    return( this->icache_assoc );
}


void mips_cpu_t::set_dcache_assoc( mips_cpu_t::cache_assoc_e assoc ) {

    this->dcache_assoc = assoc;
}


mips_cpu_t::cache_assoc_e mips_cpu_t::get_dcache_assoc() {

    return( this->dcache_assoc );
}


