/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     arch/mips64/mips_cpu.h
 * Created:       22/08/2002 by Carl van Schaik
 * Description:   MIPS CPU control functions
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
 * $Id: mips_cpu.h,v 1.13 2004/06/04 02:14:25 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__MIPS64__MIPS_CPU_H__
#define __ARCH__MIPS64__MIPS_CPU_H__

#include INC_ARCH(mipsregs.h)

/**
 * MIPS CPU control functions
 */

class mips_cpu
{
public:
    static inline void cli (void);
    static inline void restore_mask (unsigned int mask);
    static inline void sti (void);
    static inline void sleep (void);
    static inline unsigned int save_flags (void);
    static inline void restore_flags (unsigned int flags);

public:
    static inline unsigned int set_cp0_status (unsigned int set);
    static inline unsigned int clear_cp0_status (unsigned int clear);
    static inline unsigned int change_cp0_status (unsigned int mask, unsigned int newbits);
    
    static inline unsigned int set_cp0_cause (unsigned int set);
    static inline unsigned int clear_cp0_cause (unsigned int clear);
    static inline unsigned int change_cp0_cause (unsigned int mask, unsigned int newbits);
    
    static inline unsigned int set_cp0_config (unsigned int set);
    static inline unsigned int clear_cp0_config (unsigned int clear);
    static inline unsigned int change_cp0_config (unsigned int mask, unsigned int newbits);

#ifdef CONFIG_CPU_MIPS64_SB1
    static inline unsigned long get_cp0_perf_counter0 (void);
    static inline unsigned long get_cp0_perf_control0 (void);
    static inline unsigned long get_cp0_perf_counter1 (void);
    static inline unsigned long get_cp0_perf_control1 (void);
    static inline unsigned long get_cp0_perf_counter2 (void);
    static inline unsigned long get_cp0_perf_control2 (void);
    static inline unsigned long get_cp0_perf_counter3 (void);
    static inline unsigned long get_cp0_perf_control3 (void);

    static inline unsigned long set_cp0_perf_counter0 (unsigned long set);
    static inline unsigned long set_cp0_perf_control0 (unsigned long set);
    static inline unsigned long set_cp0_perf_counter1 (unsigned long set);
    static inline unsigned long set_cp0_perf_control1 (unsigned long set);
    static inline unsigned long set_cp0_perf_counter2 (unsigned long set);
    static inline unsigned long set_cp0_perf_control2 (unsigned long set);
    static inline unsigned long set_cp0_perf_counter3 (unsigned long set);
    static inline unsigned long set_cp0_perf_control3 (unsigned long set);
#endif

public:
    static inline void enable_fpu (void);
    static inline void disable_fpu (void);
};

#ifdef CONFIG_SMP
# warning "Is this SMP safe?"
#endif

INLINE void mips_cpu::cli (void)
{
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

INLINE void mips_cpu::restore_mask (unsigned int mask)
{
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

INLINE void mips_cpu::sti (void)
{
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

INLINE void mips_cpu::sleep (void)
{
#if !defined(CONFIG_CPU_MIPS64_R4X00) && !defined(CONFIG_PLAT_VR41XX)
    __asm__ __volatile__ (
	".set push	\n\t"
	".set noreorder	\n\t"
	"nop		\n\t"
	"wait		\n\t"
	"nop		\n\t"
	".set reorder	\n\t"
	".set pop	\n\t"
	);
#elif CONFIG_PLAT_VR41XX
    __asm__ __volatile__ (
//	"standby;	"
	".word	0x42000021;	"
    );
#else
    for (word_t i = 0; i < 1000; i++);
#endif
}

INLINE unsigned int mips_cpu::save_flags (void)
{
    unsigned int temp;
    __asm__ __volatile__ (
        ".set\tpush\n\t"
        ".set\treorder\n\t"
        "mfc0\t\\temp, $12\n\t"
        ".set\tpop\n\t"
	);
    return temp;
}

INLINE void mips_cpu::restore_flags (unsigned int flags)
{
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
}

#define __BUILD_SET_CP0(name,register)                          \
INLINE unsigned int mips_cpu::set_cp0_##name (unsigned int set)	\
{								\
    unsigned int res;						\
								\
    res = read_32bit_cp0_register(register);			\
    res |= set;							\
    write_32bit_cp0_register(register, res);			\
								\
    return res;							\
}								\
								\
INLINE unsigned int mips_cpu::clear_cp0_##name (unsigned int clear)	\
{								\
    unsigned int res;						\
								\
    res = read_32bit_cp0_register(register);			\
    res &= ~clear;						\
    write_32bit_cp0_register(register, res);			\
								\
    return res;							\
}								\
								\
INLINE unsigned int mips_cpu::change_cp0_##name (unsigned int mask, unsigned int newbits)	\
{								\
    unsigned int res;						\
								\
    res = read_32bit_cp0_register(register);			\
    res &= ~mask;						\
    res |= (mask & newbits);					\
    write_32bit_cp0_register(register, res);			\
								\
    return res;							\
}

__BUILD_SET_CP0(status,CP0_STATUS)
__BUILD_SET_CP0(cause,CP0_CAUSE)
__BUILD_SET_CP0(config,CP0_CONFIG)

INLINE void mips_cpu::enable_fpu (void)
{
    set_cp0_status(ST_CU1);
    asm("nop;nop;nop;nop");         /* max. hazard */
}

INLINE void mips_cpu::disable_fpu (void)
{
    clear_cp0_status(ST_CU1);
    /* We don't care about the cp0 hazard here  */ 
}

#ifdef CONFIG_CPU_MIPS64_SB1

#define __BUILD_SET_PERF(name,sel)                          \
INLINE unsigned long mips_cpu::get_cp0_perf_##name (void)	\
{								\
    unsigned long res;						\
								\
    res = read_64bit_cp0_register_sel(CP0_PERF,sel);		\
								\
    return res;							\
}								\
								\
INLINE unsigned long mips_cpu::set_cp0_perf_##name (unsigned long set)	\
{								\
    write_64bit_cp0_register_sel(CP0_PERF, set, sel);		\
								\
    return set;							\
}								\

__BUILD_SET_PERF(control0,0);
__BUILD_SET_PERF(counter0,1);
__BUILD_SET_PERF(control1,2);
__BUILD_SET_PERF(counter1,3);
__BUILD_SET_PERF(control2,4);
__BUILD_SET_PERF(counter2,5);
__BUILD_SET_PERF(control3,6);
__BUILD_SET_PERF(counter3,7);

#endif

#endif /* __ARCH__MIPS64__MIPS_CPU_H__ */
