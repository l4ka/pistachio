/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/cache.cc
 * Description:   Cache controller implementation for MIPS32
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
 * $Id: cache.cc,v 1.1 2006/02/23 21:07:45 ud3 Exp $
 *                
 ********************************************************************/

#include INC_ARCH(cache.h)
#include INC_ARCH(cp0regs.h)
#include INC_ARCH(mips_cpu.h)

#include INC_GLUE(config.h)

#include <debug.h>

static word_t icache_ls;
static word_t dcache_ls;
static word_t dcache_size;
static word_t icache_size;

#warning The cache functionality has never been tested as Simics/Mips does not implement caches as of yet


static void init_dcache() {
    unsigned long start = KSEG0_BASE;
    unsigned long end = (start + dcache_size);

    while( start < end ) {
        __asm__ __volatile__ (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Index_Invalidate_I)
            );
        start += dcache_ls;
    }
}


static void init_icache() {

    unsigned long start = KSEG0_BASE;
    unsigned long end = (start + icache_size);

    while( start < end ) {
        __asm__ __volatile__ (
            "cache  %1, 0(%0)"
            : : "r" (start), "i" (Index_Invalidate_D)
            );
        start += icache_ls;
    }
}


void init_cache() {

    word_t icache_spw;
    word_t dcache_spw;

    word_t icache_assoc;
    word_t dcache_assoc;	

    // get cache line size
    switch( get_mips_cpu()->get_icache_ls() ) {
    case mips_cpu_t::size_16_bytes:
        icache_ls = 16;
        break;
    default:
        ASSERT( !"init_cache: Cound not determine cache line size" );
        break;
    }
    switch( get_mips_cpu()->get_dcache_ls() ) {
    case mips_cpu_t::size_16_bytes:
        dcache_ls = 16;
        break;
    default:
        ASSERT( !"init_cache: Cound not determine cache line size" );
        break;
    }
	  
    // get cache sets per way
    switch( get_mips_cpu()->get_icache_spw() ) {
    case mips_cpu_t::spw_64:
        icache_spw = 64;
        break;
    case mips_cpu_t::spw_128:
        icache_spw = 128;
        break;
    case mips_cpu_t::spw_256:
        icache_spw = 256;
        break;
    default:
        ASSERT( !"init_cache: Cound not determine number of sets per way" );
        break;
    }
    switch( get_mips_cpu()->get_dcache_spw() ) {
    case mips_cpu_t::spw_64:
        dcache_spw = 64;
        break;
    case mips_cpu_t::spw_128:
        dcache_spw = 128;
        break;
    case mips_cpu_t::spw_256:
        dcache_spw = 256;
        break;
    default:
        ASSERT( !"init_cache: Cound not determine number of sets per way" );
        break;
    }

    // get cache assocciativity
    switch( get_mips_cpu()->get_icache_assoc() ) {
    case mips_cpu_t::assoc_dm:
        icache_assoc = 1;		
        break;
    case mips_cpu_t::assoc_2_way:
        icache_assoc = 2;		
        break;
    case mips_cpu_t::assoc_3_way:
        icache_assoc = 3;		
        break;
    case mips_cpu_t::assoc_4_way:
        icache_assoc = 4;		
        break;
    default:
        ASSERT( !"init_cache: Cound not determine cache assoc" );
        break;
    }
    switch( get_mips_cpu()->get_dcache_assoc() ) {
    case mips_cpu_t::assoc_dm:
        dcache_assoc = 1;		
        break;
    case mips_cpu_t::assoc_2_way:
        dcache_assoc = 2;		
        break;
    case mips_cpu_t::assoc_3_way:
        dcache_assoc = 3;		
        break;
    case mips_cpu_t::assoc_4_way:
        dcache_assoc = 4;		
        break;
    default:
        ASSERT( !"init_cache: Cound not determine cache assoc" );
        break;
    }


    icache_size = icache_ls * icache_spw * icache_spw;
    icache_size = dcache_ls * dcache_spw * dcache_spw;
	
	
    /* Important that these inline! */
    init_dcache();
    init_icache();
}
