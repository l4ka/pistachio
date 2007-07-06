/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   MIPS CPU Registers
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
 * $Id: mipsregs.h,v 1.9 2004/12/02 00:02:53 cvansch Exp $
 *                
 ********************************************************************/

#ifndef _ARCH_MIPS64_MIPSREGS_H_
#define _ARCH_MIPS64_MIPSREGS_H_

/* MIPS CoProcessor-0 Registers*/
#define CP0_INDEX $0	/* selects TLB entry for r/w ops & shows probe success */ 
#define CP0_RANDOM $1	/* counter - random number generator */
#define CP0_ENTRYLO $2	/* low word of a TLB entry */
#define CP0_ENTRYLO0 $2	/* R4k uses this for even-numbered virtual pages */
#define CP0_ENTRYLO1 $3	/* R4k uses this for odd-numbered virtual pages */
#define CP0_CONTEXT $4	/* TLB refill handler's kernel PTE entry pointer */
#define CP0_PAGEMASK $5	/* R4k page number bit mask (impl. variable page sizes) */
#define CP0_WIRED $6	/* R4k lower bnd for Random (controls randomness of TLB) */
#define CP0_ERROR $7	/* R6k status/control register for parity checking */
#define CP0_BADVADDR $8	/* "bad" virt. addr (VA of last failed v->p translation) */
#define CP0_COUNT $9	/* R4k r/w reg - continuously incrementing counter */
#define CP0_ENTRYHI $10	/* High word of a TLB entry */
#define CP0_COMPARE $11	/* R4k traps when this register equals Count */
#define CP0_STATUS $12	/* Kernel/User mode, interrupt enb., & diagnostic states */
#define CP0_CAUSE $13	/* Cause of last exception */
#define CP0_EPC $14	/* Address to return to after processing this exception */
#define CP0_PRID $15	/* Processor revision identifier */
#define CP0_CONFIG $16	/* R4k config options for caches, etc. */
#define CP0_LLADR $17	/* R4k last instruction read by a Load Linked */
#define CP0_LLADDR $17	/* Inconsistencies in naming... sigh. */
#define CP0_WATCHLO $18	/* R4k hardware watchpoint data */
#define CP0_WATCHHI $19	/* R4k hardware watchpoint data */
/* 20-21,23-24 - RESERVED */
#define CP0_PTR $22	/* MIPS64 Performance Trace Register */
#define CP0_PERF $25	/* MIPS64 Performance Counter Register Mapping */
#define CP0_ECC $26	/* R4k cache Error Correction Code */
#define CP0_CACHEERR $27	/* R4k read-only cache error codes */
#define CP0_TAGLO $28	/* R4k primary or secondary cache tag and parity */
#define CP0_TAGHI $29	/* R4k primary or secondary cache tag and parity */
#define CP0_ERROREPC $30	/* R4k cache error EPC */

/* MIPS CoProcessor-0 Registers*/
#define CP1_REVISION $0	/* FPU Revision */
#define CP1_STATUS $31 /* FPU STatus */

/* Config Register - Cacheability Codes */
#define CONFIG_CACHABLE_NO_WA 0
#define CONFIG_CACHABLE_WA 1
#define CONFIG_NOCACHE 2
#define CONFIG_CACHABLE_NONCOHERENT 3
#define CONFIG_CACHABLE_CE 4
#define CONFIG_CACHABLE_COW 5
#define CONFIG_CACHABLE_CUW 6
#define CONFIG_CACHABLE_ACCEL 7
#define CONFIG_CACHE_MASK 7

#define _INS_(x) #x
#define STR(x) _INS_(x)

/* Read from CP0 register */
#define read_32bit_cp0_register(reg)	\
({ unsigned int _rd_data;		\
        __asm__ __volatile__(		\
        "mfc0 %0,"STR(reg)		\
        : "=r" (_rd_data));		\
        _rd_data;})

#define read_64bit_cp0_register(reg)	\
({ unsigned long _rd_data;		\
        __asm__ __volatile__(		\
        "dmfc0 %0,"STR(reg)		\
        : "=r" (_rd_data));		\
        _rd_data;})

#define read_64bit_cp0_register_sel(reg, sel)	\
({ unsigned long _rd_data;		\
        __asm__ __volatile__(		\
        "dmfc0 %0,"STR(reg)","STR(sel)		\
        : "=r" (_rd_data));		\
        _rd_data;})

#define write_32bit_cp0_register(reg,value) \
        __asm__ __volatile__(		\
        "mtc0\t%0,"STR(reg)		\
        : : "r" (value));

#define write_64bit_cp0_register(reg,value) \
        __asm__ __volatile__(		\
        "dmtc0\t%0,"STR(reg)		\
        : : "r" (value))

#define write_64bit_cp0_register_sel(reg,value,sel) \
        __asm__ __volatile__(		\
        "dmtc0\t%0,"STR(reg)","STR(sel)		\
        : : "r" (value))


/* Interupt Enable/Cause Bits */
#define INT_SW0     (1<<8)
#define INT_SW1     (1<<9)
#define INT_IRQ0    (1<<10)
#define INT_IRQ1    (1<<11)
#define INT_IRQ2    (1<<12)
#define INT_IRQ3    (1<<13)
#define INT_IRQ4    (1<<14)
#define INT_IRQ5    (1<<15)

/* Status register bits */
#define ST_IE		(1<<0)
#define ST_EXL		(1<<1)
#define ST_ERL		(1<<2)	
#define ST_KSU		(3<<3)
#define ST_U		(2<<3)
#define ST_S		(1<<3)
#define ST_K		(0<<3)
#define ST_UX		(1<<5)
#define ST_SX		(1<<6)
#define ST_KX 		(1<<7)
#define ST_DE		(1<<16)
#define ST_CE		(1<<17)
#define ST_NMI		(1<<19)
#define ST_SR		(1<<20)
#define ST_TS		(1<<21)
#define ST_BEV		(1<<22)
#define ST_PX		(1<<23)
#define ST_MX		(1<<24)
#define ST_RE		(1<<25)
#define ST_FR		(1<<26)
#define ST_RP		(1<<27)

#define ST_IM		(0xff<<8)
#define ST_CH		(1<<18)
#define ST_SR		(1<<20)
#define ST_TS		(1<<21)
#define ST_BEV		(1<<22)
#define ST_PX		(1<<23)
#define ST_MX		(1<<24)
#define ST_CU		(0xf<<28)
#define ST_CU0		(0x1<<28)
#define ST_CU1		(0x2<<28)
#define ST_CU2		(0x4<<28)
#define ST_CU3		(0x8<<28)
#define ST_XX		(0x8<<28)

#define USER_FLAG_READ_MASK	(ST_CU | ST_RP | ST_FR | ST_RE | ST_MX | ST_PX | ST_UX)
#define USER_FLAG_WRITE_MASK	(ST_XX | ST_RP | ST_FR | ST_RE | ST_PX | ST_PX | ST_UX)

/* Cause register */
#define CAUSE_EXCCODE	(31<<2)
#define CAUSE_EXCCODE_NUM(x)	((x>>2) & 31)
#define CAUSE_IP	(255<<8)
#define CAUSE_IP0	(1<<8)
#define CAUSE_IP1	(1<<9)
#define CAUSE_IP2	(1<<10)
#define CAUSE_IP3	(1<<11)
#define CAUSE_IP4	(1<<12)
#define CAUSE_IP5	(1<<13)
#define CAUSE_IP6	(1<<14)
#define CAUSE_IP7	(1<<15)
#define CAUSE_IV	(1<<23)
#define CAUSE_CE	(3<<28)
#define CAUSE_CE_NUM(x)	((x>>28) & 3)
#define CAUSE_BD	(1<<31)

#endif /* _ARCH_MIPS64_MIPSREGS_H_ */
