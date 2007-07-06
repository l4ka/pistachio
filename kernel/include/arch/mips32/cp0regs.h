/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     arch/mips32/cp0regs.h
 * Description:   MIPS32 CP0 macros
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
 * $Id: cp0regs.h,v 1.1 2006/02/23 21:07:39 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS32__CP0REGS_H__
#define __ARCH__MIPS32__CP0REGS_H__

/*
 * Coprocessor 0 register names
 */
#define CP0_INDEX       $0
#define CP0_RANDOM      $1
#define CP0_ENTRYLO0    $2
#define CP0_ENTRYLO1    $3
#define CP0_CONF        $3
#define CP0_CONTEXT     $4
#define CP0_PAGEMASK    $5
#define CP0_WIRED       $6
#define CP0_INFO        $7
#define CP0_BADVADDR    $8
#define CP0_COUNT       $9
#define CP0_ENTRYHI     $10
#define CP0_COMPARE     $11
#define CP0_STATUS      $12
#define CP0_CAUSE       $13
#define CP0_EPC         $14
#define CP0_PRID        $15
#define CP0_CONFIG      $16
#define CP0_LLADDR      $17
#define CP0_WATCHLO     $18
#define CP0_WATCHHI     $19
#define CP0_XCONTEXT    $20
#define CP0_FRAMEMASK   $21
#define CP0_DIAGNOSTIC  $22
#define CP0_DEBUG       $23
#define CP0_DEPC        $24
#define CP0_PERFORMANCE $25
#define CP0_ECC         $26
#define CP0_CACHEERR    $27
#define CP0_TAGLO       $28
#define CP0_TAGHI       $29
#define CP0_ERROREPC    $30
#define CP0_DESAVE      $31


/* Interupt Enable/Cause Bits */
//#define INT_SW0     (1<<8)
//#define INT_SW1     (1<<9)
//#define INT_IRQ0    (1<<10)
//#define INT_IRQ1    (1<<11)
//#define INT_IRQ2    (1<<12)
//#define INT_IRQ3    (1<<13)
//#define INT_IRQ4    (1<<14)
//#define INT_IRQ5    (1<<15)

/* Status register bits */
#define ST_IE		(1<<0)
#define ST_EXL		(1<<1)
#define ST_ERL		(1<<2)	
#define ST_UM		(1<<4)
#define ST_IM		(0xff<<8)
#define ST_NMI		(1<<19)
#define ST_SR		(1<<20)
#define ST_TS		(1<<21)
#define ST_BEV		(1<<22)
#define ST_RE		(1<<25)
#define ST_R		(1<<26)
#define ST_RP		(1<<27)
#define ST_CU		(0xf<<28)
#define ST_CU0		(0x1<<28)
#define ST_CU1		(0x2<<28)
#define ST_CU2		(0x4<<28)
#define ST_CU3		(0x8<<28)
#define ST_XX		(0x8<<28)

/* Cause register */
#define CAUSE_EXCCODE	(31<<2)
#define CAUSE_EXCCODE_NUM(x)	((x>>2) & 31)
//#define CAUSE_IP	(255<<8)
//#define CAUSE_IP0	(1<<8)
//#define CAUSE_IP1	(1<<9)
//#define CAUSE_IP2	(1<<10)
//#define CAUSE_IP3	(1<<11)
//#define CAUSE_IP4	(1<<12)
//#define CAUSE_IP5	(1<<13)
//#define CAUSE_IP6	(1<<14)
//#define CAUSE_IP7	(1<<15)
//#define CAUSE_IV	(1<<23)
//#define CAUSE_CE	(3<<28)
//#define CAUSE_CE_NUM(x)	((x>>28) & 3)
//#define CAUSE_BD	(1<<31)





/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 *  Configure language
 */
#ifdef __ASSEMBLY__
#define _ULCAST_
#else
#define _ULCAST_ (unsigned long)
#endif

/* Read from CP0 register */
#define read_32bit_cp0_register(reg)	\
({ unsigned int _rd_data;		\
        __asm__ __volatile__(		\
        "mfc0 %0,"STR(reg)		\
        : "=r" (_rd_data));		\
        _rd_data;})


#define write_32bit_cp0_register(reg,value) \
        __asm__ __volatile__(		\
        "mtc0\t%0,"STR(reg)		\
        : : "r" (value));

/* Interupt Enable/Cause Bits */
#define INT_SW0     (1<<8)
#define INT_SW1     (1<<9)
#define INT_IRQ0    (1<<10)
#define INT_IRQ1    (1<<11)
#define INT_IRQ2    (1<<12)
#define INT_IRQ3    (1<<13)
#define INT_IRQ4    (1<<14)
#define INT_IRQ5    (1<<15)


/*
 * Macros to access the system control coprocessor
 */

#define __read_32bit_c0_register(source, sel)                           \
({ int __res;                                                           \
        if (sel == 0)                                                   \
                __asm__ __volatile__(                                   \
                        "mfc0\t%0, " #source "\n\t"                     \
                        : "=r" (__res));                                \
        else                                                            \
                __asm__ __volatile__(                                   \
                        ".set\tmips32\n\t"                              \
                        "mfc0\t%0, " #source ", " #sel "\n\t"           \
                        ".set\tmips0\n\t"                               \
                        : "=r" (__res));                                \
        __res;                                                          \
})

#define __write_32bit_c0_register(register, sel, value)                 \
do {                                                                    \
        if (sel == 0)                                                   \
                __asm__ __volatile__(                                   \
                        "mtc0\t%z0, " #register "\n\t"                  \
                        : : "Jr" ((unsigned int)value));                \
        else                                                            \
                __asm__ __volatile__(                                   \
                        ".set\tmips32\n\t"                              \
                        "mtc0\t%z0, " #register ", " #sel "\n\t"        \
                        ".set\tmips0"                                   \
                        : : "Jr" ((unsigned int)value));                \
} while (0)


#define read_c0_index()         __read_32bit_c0_register($0, 0)
#define write_c0_index(val)     __write_32bit_c0_register($0, 0, val)

#define read_c0_entrylo0()      __read_ulong_c0_register($2, 0)
#define write_c0_entrylo0(val)  __write_ulong_c0_register($2, 0, val)

#define read_c0_entrylo1()      __read_ulong_c0_register($3, 0)
#define write_c0_entrylo1(val)  __write_ulong_c0_register($3, 0, val)

#define read_c0_conf()          __read_32bit_c0_register($3, 0)
#define write_c0_conf(val)      __write_32bit_c0_register($3, 0, val)

#define read_c0_context()       __read_ulong_c0_register($4, 0)
#define write_c0_context(val)   __write_ulong_c0_register($4, 0, val)

#define read_c0_pagemask()      __read_32bit_c0_register($5, 0)
#define write_c0_pagemask(val)  __write_32bit_c0_register($5, 0, val)

#define read_c0_wired()         __read_32bit_c0_register($6, 0)
#define write_c0_wired(val)     __write_32bit_c0_register($6, 0, val)

#define read_c0_info()          __read_32bit_c0_register($7, 0)

#define read_c0_cache()         __read_32bit_c0_register($7, 0) /* TX39xx */
#define write_c0_cache(val)     __write_32bit_c0_register($7, 0, val)

#define read_c0_count()         __read_32bit_c0_register($9, 0)
#define write_c0_count(val)     __write_32bit_c0_register($9, 0, val)

#define read_c0_entryhi()       __read_ulong_c0_register($10, 0)
#define write_c0_entryhi(val)   __write_ulong_c0_register($10, 0, val)

#define read_c0_compare()       __read_32bit_c0_register($11, 0)
#define write_c0_compare(val)   __write_32bit_c0_register($11, 0, val)

#define read_c0_status()        __read_32bit_c0_register($12, 0)
#define write_c0_status(val)    __write_32bit_c0_register($12, 0, val)

#define read_c0_cause()         __read_32bit_c0_register($13, 0)
#define write_c0_cause(val)     __write_32bit_c0_register($13, 0, val)

#define read_c0_epc()           __read_ulong_c0_register($14, 0)
#define write_c0_epc(val)       __write_ulong_c0_register($14, 0, val)

#define read_c0_prid()          __read_32bit_c0_register($15, 0)

#define read_c0_config()        __read_32bit_c0_register($16, 0)
#define read_c0_config1()       __read_32bit_c0_register($16, 1)

#define write_c0_config(val)    __write_32bit_c0_register($16, 0, val)
#define write_c0_config1(val)   __write_32bit_c0_register($16, 1, val)



#endif /* !__ARCH__MIPS32__CP0REGS_H__ */
