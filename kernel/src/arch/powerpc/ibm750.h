/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/ibm750.h
 * Description:	Types and functions specific to the IBM PowerPC 750.
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
 * $Id: ibm750.h,v 1.5 2003/12/11 12:46:36 joshua Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__IBM750_H__
#define __ARCH__POWERPC__IBM750_H__

/* Maximum address for which tlbie operates. */
#define POWERPC_TLBIE_MAX_BITS	17
#define POWERPC_TLBIE_MAX	(1 << POWERPC_TLBIE_MAX_BITS)
/* Minimum address difference for which tlbie operates. */
#define POWERPC_TLBIE_INC_BITS	12
#define POWERPC_TLBIE_INC	(1 << POWERPC_TLBIE_INC_BITS)


#if !defined(ASSEMBLY)

class ppc750_mmcr0_t 
{
public:
    enum ppc750_perf_event_t {
    	nop = 0,
	cycle_cnt = 1,
	instr_complete_cnt = 2,
	instr_dispatch_cnt = 4,
	itlb_search_cycles = 6,
	l2_hit_cnt = 7,
	ea_cnt = 8,
	l1_threshold_miss_cnt = 10,
	br_unresolved_cnt = 11,
    };

    union {
	word_t raw;
	struct {
	    word_t dis	: 1;	/* Disables counting unconditionally. */
	    word_t dp	: 1;	/* Disables counting in supervisor mode. */
	    word_t du	: 1;	/* Disables counting in user mode. */
	    word_t dms	: 1;	/* Disables counting while MSR[PM] is set. */
	    word_t dmr	: 1;	/* Disables counting while MSR[PM] is zero. */
	    word_t enint : 1;	/* Enables perf mon interrupt signalling. */
	    word_t discount : 1; /* Disables counting when a perf mon 
				    interrupt is signaled. */
	    word_t rtcselect : 2; /* 64-bit time base, bit selection enable. */
	    word_t intonbittrans : 1; /* Cause interrupt signaling on bit
					 transition. */
	    word_t threshold : 6;
	    word_t pmc1intcontrol : 1; /* Enables interrupt signaling due to a
					  PMC1 counter overflow. */
	    word_t pmcintcontrol : 1; /* Enables interrupt signaling due to any
					 PMC2-PMC4 counter overflow. */
	    word_t pmctrigger : 1; /* Can be used to trigger counting of
				      PMC2-PMC4 after PMC1 has overflowed. */
	    ppc750_perf_event_t pmc1select : 7; /* PMC1 input selector. */
	    ppc750_perf_event_t pmc2select : 6; /* PMC2 input selector. */
	} x;
    };
};

INLINE void ppc_set_mmcr0( word_t val )
{
    asm volatile("mtspr 952, %0" : : "r" (val) );
}

INLINE word_t ppc_get_mmcr0( void )
{
    word_t ret;
    asm volatile("mfspr %0, 952" : "=r" (ret) );
    return ret;
}

class ppc750_mmcr1_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t pmc3select	: 5;	/* PMC3 input selector. */
	    word_t pmc4select	: 5;	/* PMC4 input selector. */
	    word_t __reserved	: 22;	/* Reserved. */
	} x;
    };
};

INLINE void ppc_set_mmcr1( word_t val )
{
    asm volatile("mtspr 956, %0" : : "r" (val) );
}

INLINE word_t ppc_get_mmcr1( void )
{
    word_t ret;
    asm volatile("mfspr %0, 956" : "=r" (ret) );
    return ret;
}

class ppc750_pmc_t
{
public:
    union {
	word_t raw;
	struct {
	    word_t overflow : 1;
	    word_t counter : 31;
	} x;
    };
};

INLINE void ppc_set_pmc1( word_t val )
{
    asm volatile("mtspr 953, %0" : : "r" (val) );
}

INLINE word_t ppc_get_pmc1( void )
{
    word_t ret;
    asm volatile("mfspr %0, 953" : "=r" (ret) );
    return ret;
}

INLINE void ppc_set_pmc2( word_t val )
{
    asm volatile("mtspr 954, %0" : : "r" (val) );
}

INLINE word_t ppc_get_pmc2( void )
{
    word_t ret;
    asm volatile("mfspr %0, 954" : "=r" (ret) );
    return ret;
}

INLINE void ppc_set_pmc3( word_t val )
{
    asm volatile("mtspr 957, %0" : : "r" (val) );
}

INLINE word_t ppc_get_pmc3( void )
{
    word_t ret;
    asm volatile("mfspr %0, 957" : "=r" (ret) );
    return ret;
}

INLINE void ppc_set_pmc4( word_t val )
{
    asm volatile("mtspr 958, %0" : : "r" (val) );
}

INLINE word_t ppc_get_pmc4( void )
{
    word_t ret;
    asm volatile("mfspr %0, 958" : "=r" (ret) );
    return ret;
}

INLINE void ppc_set_sia( word_t val )
{
    asm volatile("mtspr 955, %0" : : "r" (val) );
}

INLINE word_t ppc_get_sia( void )
{
    word_t ret;
    asm volatile("mfspr %0, 955" : "=r" (ret) );
    return ret;
}

/*****************************************************************************
 *
 * Hardware Implementation-Dependent Registers (HID0, HID1)
 *
 *****************************************************************************/

INLINE void ppc_set_hid0( u32_t val )
{
    asm volatile("mtspr 1008, %0" : : "r" (val) );
}

INLINE u32_t ppc_get_hid0( void )
{
    u32_t ret;
    asm volatile("mfspr %0, 1008" : "=r" (ret) );
    return ret;
}

class ppc750_hid0_t
{
public:
    void read() { this->raw = ppc_get_hid0(); }
    void write() { ppc_set_hid0( this->raw ); }

public:
    union {
	u32_t raw;
	struct {
	    u32_t emcp : 1;
	    u32_t dbp  : 1;
	    u32_t eba  : 1;
	    u32_t ebd  : 1;
	    u32_t bclk : 1;
	    u32_t unused0 : 1;
	    u32_t eclk : 1;
	    u32_t par  : 1;
	    u32_t doze : 1;	/* Doze enabled with MSR[POW]. */
	    u32_t nap  : 1;	/* Nap enabled with MSR[POW]. */
	    u32_t sleep : 1;	/* Sleep enabled with MSR[POW]. */
	    u32_t dpm  : 1;	/* Dynamic power management enable. */
	    u32_t unused1 : 3;
	    u32_t nhr  : 1;	/* Not hard reset. */
	    u32_t ice  : 1;	/* Instruction cache enable. */
	    u32_t dce  : 1;	/* Data cache enable. */
	    u32_t ilock : 1;	/* Instruction cache lock. */
	    u32_t dlock : 1;	/* Data cache lock. */
	    u32_t icfi : 1;	/* Insturction cache flash invalidate. */
	    u32_t dcfi : 1;	/* Data cache flash invalidate. */
	    u32_t spd  : 1;	/* Speculative cache access disable. */
	    u32_t ifem : 1;
	    u32_t sge  : 1;	/* Store gathering enable. */
	    u32_t dcfa : 1;	/* Data cache flush assist. */
	    u32_t btic : 1;	/* Branch target instruction cache enable. */
	    u32_t unused2 : 1;
	    u32_t abe  : 1;
	    u32_t bht  : 1;	/* Branch history table enable. */
	    u32_t unused3 : 1;
	    u32_t noopti : 1;	/* No-op the data cache touch instructions. */
	} x;
    };
};

INLINE void ppc750_configure( void )
{
    ppc750_hid0_t hid0;
    hid0.read();

    hid0.x.nhr = 1;     /* Detect a soft reset. */
    hid0.x.doze = 1;    /* Doze when we set MSR[POW]. */
    hid0.x.nap = 0;     /* Disable nap mode. */
    hid0.x.sleep = 0;   /* Disable sleep mode. */
    hid0.x.dpm = 1;     /* Enable dynamic power mode. */
    hid0.x.ice = 1;     /* Enable instruction cache. */
    hid0.x.dce = 1;     /* Enable data cache. */
    hid0.x.sge = 1;     /* Enable store gathering. */
    hid0.x.btic = 1;    /* Enable branch target instruction cache. */
    hid0.x.bht = 1;     /* Enable branch history table. */
    hid0.x.noopti = 0;  /* Enable data cache touch instructions. */

    hid0.write();
}

#endif	/* ASSEMBLY */

#endif /* __ARCH__POWERPC__IBM750_H__ */
