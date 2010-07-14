/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/ppc_registers.h
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __ARCH__POWERPC__PPC_REGISTERS_H__
#define __ARCH__POWERPC__PPC_REGISTERS_H__

/*  SPR encodings for mfspr
 */
#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18
#define SPR_DAR		19
#define SPR_DEC		22
#define SPR_SDR1	25
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_SPRG4	276
#define SPR_SPRG5	277
#define SPR_SPRG6	278
#define SPR_SPRG7	279
#define SPR_ASR		280
#define SPR_EAR		282
#define SPR_PVR		287

#define SPR_IBAT0U	528
#define SPR_IBAT0L	529
#define SPR_IBAT1U	530
#define SPR_IBAT1L	531
#define SPR_IBAT2U	532
#define SPR_IBAT2L	533
#define SPR_IBAT3U	534
#define SPR_IBAT3L	535
#define SPR_DBAT0U	536
#define SPR_DBAT0L	537
#define SPR_DBAT1U	538
#define SPR_DBAT1L	539
#define SPR_DBAT2U	540
#define SPR_DBAT2L	541
#define SPR_DBAT3U	542
#define SPR_DBAT3L	543
#define SPR_DABR	1013

#ifdef CONFIG_PPC_BOOKE
#define SPR_PID		0x030
#define SPR_DECAR	0x036
#define SPR_CSRR0	0x03a
#define SPR_CSRR1	0x03b
#define SPR_DEAR	0x03d
#define SPR_ESR		0x03e
#define SPR_IVPR	0x03f
#define SPR_USPRG0	0x100
#define SPR_SPRG4U	0x104
#define SPR_SPRG5U	0x105
#define SPR_SPRG6U	0x106
#define SPR_SPRG7U	0x107
#define SPR_TBL		0x10c
#define SPR_TBH		0x10d
#define SPR_PIR		0x11e

#define SPR_DBSR	0x130
#define SPR_DBCR0	0x134
#define SPR_DBCR1	0x135
#define SPR_DBCR2	0x136
#define SPR_IAC1	0x138
#define SPR_IAC2	0x139
#define SPR_IAC3	0x13a
#define SPR_IAC4	0x13b
#define SPR_DAC1	0x13c
#define SPR_DAC2	0x13d
#define SPR_DVC1	0x13e
#define SPR_DVC2	0x13f
#define SPR_TSR		0x150
#define SPR_TCR		0x154

#define SPR_IVOR(x)	(0x190 + (x))
#define SPR_MCSRR0	0x23a
#define SPR_MCSRR1	0x23b
#define SPR_MCSR	0x23c
#define SPR_INV0	0x370
#define SPR_INV1	0x371
#define SPR_INV2	0x372
#define SPR_INV3	0x373
#define SPR_ITV0	0x374
#define SPR_ITV1	0x375
#define SPR_ITV2	0x376
#define SPR_ITV3	0x377
#define SPR_CCR1	0x378
#define SPR_DNV0	0x390
#define SPR_DNV1	0x391
#define SPR_DNV2	0x392
#define SPR_DNV3	0x393
#define SPR_DTV0	0x394
#define SPR_DTV1	0x395
#define SPR_DTV2	0x396
#define SPR_DTV3	0x397
#define SPR_DVLIM	0x398
#define SPR_IVLIM	0x399
#define SPR_RSTCFG	0x39b
#define SPR_DCDBTRL	0x39c
#define SPR_DCDBTRH	0x39d
#define SPR_ICDBTRL	0x39e
#define SPR_ICDBTRH	0x39f
#define SPR_MMUCR	0x3b2
#define SPR_CCR0	0x3b3
#define SPR_ICDBDR	0x3d3
#define SPR_DBDR	0x3f3
#endif /* BOOKE */



#ifndef ASSEMBLY

INLINE word_t ppc_get_sprg( const int which )
{
    word_t val;
    asm volatile( "mfsprg %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc_set_sprg( const int which, word_t val )
{
    asm volatile( "mtsprg %0, %1" : : "i" (which), "r" (val) );
}

INLINE void ppc_set_srr0( word_t val )
	{ asm volatile( "mtsrr0 %0" : : "r" (val) ); }

INLINE void ppc_set_srr1( word_t val )
	{ asm volatile( "mtsrr1 %0" : : "r" (val) ); }

INLINE word_t ppc_get_srr0( void )
{
	word_t val;
	asm volatile( "mfsrr0 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_srr1( void )
{
	word_t val;
	asm volatile( "mfsrr1 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_sdr1( void )
{
    word_t val;
    asm volatile( "mfsdr1 %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_sdr1( word_t val )
{
    asm volatile( "mtsdr1 %0" : : "r" (val) );
}

INLINE word_t ppc_get_dar( void )
{
    word_t val;
    asm volatile( "mfspr %0, 19" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dsisr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 18" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_tbl( void )
{
    word_t val;
    asm volatile( "mftbl %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_tbl( word_t val )
{
    asm volatile( "mttbl %0" : : "r" (val) );
}

INLINE word_t ppc_get_tbu( void )
{
    word_t val;
    asm volatile( "mftbu %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_tbu( word_t val )
{
    asm volatile( "mttbu %0" : : "r" (val) );
}

INLINE u64_t ppc_get_timebase()
{
    return (static_cast<u64_t>(ppc_get_tbu()) << 32 | 
	    static_cast<u64_t>(ppc_get_tbl()));
}

INLINE word_t ppc_get_dabr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 1013" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dec( void )
{
    word_t val;
    asm volatile( "mfdec %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_dec( word_t val )
{
    asm volatile( "mtdec %0" : : "r" (val) );
}

INLINE void ppc_set_decar( word_t val )
{
    asm volatile( "mtdecar %0" : : "r" (val) );
}

INLINE word_t ppc_get_ear( void )
{
    word_t val;
    asm volatile( "mfspr %0, 282" : "=r" (val) );
    return val;
}


INLINE void ppc_set_spr(const word_t spr, word_t val)
{
    asm volatile("mtspr %[spr], %[val]; sync" 
		 : : [spr] "i"(spr), [val] "r"(val));
}

INLINE word_t ppc_get_spr(const word_t spr)
{
    word_t val;
    asm volatile("mfspr %[val], %[spr]" 
		 : [val] "=r"(val) 
		 : [spr] "i" (spr));
    return val;
}

INLINE word_t ppc_get_dcr( word_t dcrn)
{
    word_t value;
    asm volatile ("mfdcrx %0,%1": "=r" (value) : "r" (dcrn) : "memory");
    return value;
}

INLINE void ppc_set_dcr( word_t dcrn, word_t value )
{
    asm volatile("mtdcrx %0,%1": :"r" (dcrn), "r" (value) : "memory");
}

class ppc_esr_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t machine_check	: 1;
	    word_t __res0		: 3;
	    word_t illegal_instr	: 1;
	    word_t privileged_instr	: 1;
	    word_t trap			: 1;
	    word_t floating_point	: 1;
	    word_t store		: 1;
	    word_t __res1		: 1;
	    word_t locking		: 2;
	    word_t aux			: 1;
	    word_t unimplemented_op	: 1;
	    word_t byte_ordering	: 1;
	    word_t imprecise		: 1;
	    word_t __res2		: 11;
	    word_t cond_reg		: 1;
	    word_t compare		: 1;
	    word_t cond_reg_field	: 3;
	} x;
    };

    word_t read()
	{ raw = ppc_get_spr(SPR_ESR); return raw; }
};

class ppc_tcr_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t watchdog_period	: 2;
	    word_t watchdog_reset_ctrl	: 2;
	    word_t watchdog_irq_enable	: 1;
	    word_t dec_irq_enable	: 1;
	    word_t fixed_interval_period: 2;
	    word_t fixed_interval_irq_enable	: 1;
	    word_t auto_reload		: 1;
	    word_t			: 22;
	};
    };
    ppc_tcr_t()
	{ raw = 0; }
    void write()
	{ ppc_set_spr(SPR_TCR, raw); }
    void read()
	{ raw = ppc_get_spr(SPR_TCR); }

    u64_t get_watchdog_period()
	{ return 1ULL << (21 + (watchdog_period * 4)); }
    u64_t get_fixed_interval_period()
	{ return 1ULL << (13 + (fixed_interval_period * 4)); }
};

class ppc_tsr_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t enable_next_watchdog		: 1;
	    word_t watchdog_irq_status		: 1;
	    word_t watchdog_timer_reset_status	: 2;
	    word_t decrementer_irq_status	: 1;
	    word_t fixed_interval_irq_status	: 1;
	    word_t				: 26;
	};
    };

    static ppc_tsr_t dec_irq()
	{ 
	    ppc_tsr_t tsr;
	    tsr.raw = 0;
	    tsr.decrementer_irq_status = 1;
	    return tsr;
	}

    void write()
	{ ppc_set_spr(SPR_TSR, raw); }

    bool pending_irqs()
	{ 
	    return watchdog_irq_status ||
		   decrementer_irq_status ||
		   fixed_interval_irq_status; 
	}

} __attribute__((packed));


INLINE u64_t ppc_get_fpscr()
{
    u64_t value;
    asm volatile (
	    "mffs %%f0 ;"
	    "stfd %%f0, 0(%0) ;"
	    : /* ouputs */
	    : /* inputs */
	      "b" (&value)
	    );
    return value;
}

INLINE void ppc_set_fpscr( u64_t value )
{
    asm volatile (
        "lfd %%f0, 0(%0) ;"
        "mtfsf 0xff, %%f0 ;"
        : /* ouputs */
        : /* inputs */
          "b" (&value)
        );

}


#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC__PPC_REGISTERS_H__ */

