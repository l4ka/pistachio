/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/mips_cpu.h
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
 * $Id: mips_cpu.h,v 1.1 2006/02/23 21:07:39 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS32__MIPS_CPU_H__
#define __ARCH__MIPS32__MIPS_CPU_H__

#ifdef CONFIG_SMP
# error "SMP is not supported" 
#endif

class mips_cpu_t {
	
public:
    void cli (void);
    void restore_mask (unsigned int mask);
    void sti (void);
    void sleep (void);
    unsigned int save_flags (void);
    void restore_flags (unsigned int flags);

public:
    unsigned int set_cp0_status (unsigned int set);
    unsigned int clear_cp0_status (unsigned int clear);
    unsigned int change_cp0_status (unsigned int mask, unsigned int newbits);
    
    unsigned int set_cp0_cause (unsigned int set);
    unsigned int clear_cp0_cause (unsigned int clear);
    unsigned int change_cp0_cause (unsigned int mask, unsigned int newbits);
    
    unsigned int set_cp0_config (unsigned int set);
    unsigned int clear_cp0_config (unsigned int clear);
    unsigned int change_cp0_config (unsigned int mask, unsigned int newbits);

public:
    void enable_fpu (void);
    void disable_fpu (void);

public:

    // -- processor id
    enum procid_e {
        mips_unknown = 0x0,
        mips_4Kc	 = 0x80,
        mips_4Kp	 = 0x83,
        mips_4Km	 = 0x83
    };

    // -- cache associativity
    enum cache_assoc_e {
        assoc_dm      = 0x0,
        assoc_2_way   = 0x1,
        assoc_3_way   = 0x2,
        assoc_4_way   = 0x3,
        assoc_unknown = 0xffff
    };

    // -- cache line size
    enum cache_ls_e {
        size_not_present = 0x0,
        size_16_bytes    = 0x3,
        size_unknown     = 0xffff
    };

    // -- cache sets per way
    enum cache_spw_e {
        spw_64	= 0x0,
        spw_128 = 0x1,
        spw_256 = 0x2
    };
	
private:
	
    word_t tlb_size;
	
    procid_e procid;
	
    cache_spw_e icache_spw;
    cache_spw_e dcache_spw;
	
    cache_ls_e icache_ls;
    cache_ls_e dcache_ls;
	
    cache_assoc_e icache_assoc;
    cache_assoc_e dcache_assoc;
	
public:

    void set_tlb_size( word_t size );
    word_t get_tlb_size();

    void set_procid( procid_e id );
    procid_e get_procid();

    void set_icache_spw( cache_spw_e sets );
    cache_spw_e get_icache_spw();
    void set_dcache_spw( cache_spw_e sets );
    cache_spw_e get_dcache_spw();

    void set_icache_ls( cache_ls_e size );
    cache_ls_e get_icache_ls();
    void set_dcache_ls( cache_ls_e size );
    cache_ls_e get_dcache_ls();


    void set_icache_assoc( cache_assoc_e assoc );
    cache_assoc_e get_icache_assoc();
    void set_dcache_assoc( cache_assoc_e assoc );
    cache_assoc_e get_dcache_assoc();


};


INLINE mips_cpu_t* get_mips_cpu() {
    extern mips_cpu_t mips_cpu;
    return( &mips_cpu );
}


/*
 * send the current processor to sleep
 */
INLINE void processor_sleep() {

#if 1

#warning workaround for wait instruction that does not work on SIMICS
    extern unsigned timer_interrupt;
	
    // enable interrupts
    __asm__ __volatile__(
        "mfc0 $8, $12	\n\t"
        "nop;nop;nop	\n\t"
        "ori  $8, 1		\n\t"
        "mtc0 $8, $12   \n\t"
        ::: "$8"
	);
	
    while(timer_interrupt == 0)
        ;
    timer_interrupt = 0;
	
    // disable interrupts	
    __asm__ __volatile__(
        "mfc0 $8, $12				\n\t"
        "nop;nop;nop				\n\t"
        "and  $8, 0xfffffffe		\n\t"
        "mtc0 $8, $12				\n\t"
        ::: "$8"
	);

#else

    get_mips_cpu()->sti();
    get_mips_cpu()->sleep();
    get_mips_cpu()->cli();
    get_mips_cpu()->restore_mask(get_idle_tcb()->arch.int_mask);

#endif
}

#endif /* !__ARCH__MIPS32__MIPS_CPU_H__ */
