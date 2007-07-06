/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/cr.h
 * Description:   IA64 control registers
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
 * $Id: cr.h,v 1.12 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__CR_H__
#define __ARCH__IA64__CR_H__

#include INC_ARCH(psr.h)

class cr_dcr_t
{
public:
    union {
	struct {
	    word_t pp			: 1;
	    word_t be			: 1;
	    word_t lc			: 1;
	    word_t __rv1		: 5;
	    word_t dm			: 1;
	    word_t dp			: 1;
	    word_t dk			: 1;
	    word_t dr			: 1;
	    word_t da			: 1;
	    word_t dd			: 1;
	    word_t __rv2		: 49;
	};
	u64_t raw;
    };
};

class cr_pta_t
{
public:
    enum format_e {
	fmt_short = 0,
	fmt_long = 1
    };

    union {
	struct {
	    word_t vhpt_enable		: 1;
	    word_t __rv1		: 1;
	    word_t vhpt_size		: 6;
	    word_t vhpt_format		: 1;
	    word_t __rv2		: 6;
	    word_t vhpt_base		: 49;
	};
	u64_t raw;
    };

    inline bool is_vhpt_enabled (void)
	{ return vhpt_enable; };

    inline word_t size (void)
	{ return (1UL << vhpt_size); };

    inline format_e format (void)
	{ return (format_e) vhpt_format; }

    inline addr_t base (void)
	{ return (addr_t) (vhpt_base << 49); }
};

class cr_isr_t
{
public:
    union {
	struct {
	    word_t code			: 16;
	    word_t vector		: 8;
	    word_t __rv1		: 8;
	    word_t rwx			: 3;
	    word_t non_access		: 1;
	    word_t speculative_load	: 1;
	    word_t register_stack	: 1;
	    word_t incomplete_reg_frame	: 1;
	    word_t nested_interruption	: 1;
	    word_t supervisor_override	: 1;
	    word_t instruction_slot	: 2;
	    word_t exception_deferral	: 1;
	    word_t __rv2		: 20;
	};
	u64_t raw;
    };
};

class cr_itir_t
{
public:
    union {
	struct {
	    word_t __rv1		: 2;
	    word_t ps			: 6;
	    word_t key			: 24;
	    word_t __rv2		: 32;
	};
	u64_t raw;
    };

    inline word_t page_size (void)
	{ return (1UL << ps); }

    inline word_t protection_key (void)
	{ return key; }
};

class cr_ifs_t
{
public:
    union {
	struct {
	    word_t sof			: 7;
	    word_t sol			: 7;
	    word_t sor			: 4;
	    word_t rrb_gr		: 7;
	    word_t rrb_fr		: 7;
	    word_t rrb_pr		: 6;
	    word_t __rv			: 25;
	    word_t valid		: 1;
	};
	u64_t raw;
    };

    inline word_t locals (void)
	{ return sol; }

    inline word_t outputs (void)
	{ return sof - sol; }

    inline word_t framesize (void)
	{ return sof; }
};

class cr_lid_t
{
public:
    union {
	struct {
	    word_t __rv			: 16;
	    word_t eid			: 8;
	    word_t id			: 8;
	    word_t __ig			: 32;
	};
	u64_t raw;
    };
};

class cr_tpr_t
{
public:
    union {
	struct {
	    word_t __ig1		: 4;
	    word_t mic			: 4;
	    word_t __rv			: 8;
	    word_t mmi			: 1;
	    word_t __ig2		: 47;
	};
	u64_t raw;
    };

    static cr_tpr_t all_enabled (void)
	{
	    cr_tpr_t tpr;
	    tpr.raw = 0;
	    return tpr;
	}

    static cr_tpr_t some_enabled (word_t int_class)
	{
	    cr_tpr_t tpr;
	    tpr.raw = 0;
	    tpr.mic = int_class;
	    return tpr;
	}
};

class cr_ivec_t
{
public:
    union {
	struct {
	    word_t vector		: 8;
	    word_t dm			: 3;
	    word_t __rv1		: 1;
	    word_t __ig1		: 1;
	    word_t ipp			: 1;
	    word_t __rv2		: 1;
	    word_t tm			: 1;
	    word_t m			: 1;
	    word_t __ig2		: 47;
	};
	u64_t raw;
    };

    cr_ivec_t (void) {}
    cr_ivec_t (word_t w) { raw = w; }
};


INLINE cr_dcr_t cr_get_dcr (void)
{
    cr_dcr_t dcr;
    __asm__ ("mov %0 = cr.dcr" :"=r" (dcr.raw));
    return dcr;
}

INLINE word_t cr_get_itm (void)
{
    word_t itm;
    __asm__ ("mov %0 = cr.itm" :"=r" (itm));
    return itm;
}

INLINE void cr_set_itm (word_t value)
{
    __asm__ ("mov cr.itm = %0" ::"r" (value));
}

INLINE addr_t cr_get_iva (void)
{
    addr_t iva;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.iva		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (iva));
    return iva;
}

INLINE cr_pta_t cr_get_pta (void)
{
    cr_pta_t pta;
    __asm__ ("mov %0 = cr.pta" :"=r" (pta.raw));
    return pta;
}

INLINE psr_t cr_get_ipsr (void)
{
    psr_t ipsr;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.ipsr	\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (ipsr.raw));
    return ipsr;
}

INLINE cr_isr_t cr_get_isr (void)
{
    cr_isr_t isr;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.isr		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (isr.raw));
    return isr;
}

INLINE addr_t cr_get_iip (void)
{
    addr_t iip;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.iip		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (iip));
    return iip;
}

INLINE addr_t cr_get_ifa (void)
{
    addr_t ifa;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.ifa		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (ifa));
    return ifa;
}

INLINE cr_itir_t cr_get_itir (void)
{
    cr_itir_t itir;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.itir	\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (itir.raw));
    return itir;
}

INLINE addr_t cr_get_iipa (void)
{
    addr_t iipa;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.iipa	\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (iipa));
    return iipa;
}

INLINE cr_ifs_t cr_get_ifs (void)
{
    cr_ifs_t ifs;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.ifs		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (ifs.raw));
    return ifs;
}

INLINE word_t cr_get_iim (void)
{
    word_t iim;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.iim		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (iim));
    return iim;
}

INLINE addr_t cr_get_iha (void)
{
    addr_t iha;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.iha		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (iha));
    return iha;
}

INLINE cr_lid_t cr_get_lid (void)
{
    cr_lid_t lid;
    __asm__ ("	rsm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     "	mov %0 = cr.lid		\n"
	     "	ssm psr.ic ;;		\n"
	     "	srlz.i ;; 		\n"
	     :"=r" (lid));
    return lid;
}

INLINE word_t cr_get_ivr (void)
{
    word_t ivr;
    __asm__ ("mov %0 = cr.ivr" :"=r" (ivr));
    return ivr;
}

INLINE cr_tpr_t cr_get_tpr (void)
{
    cr_tpr_t tpr;
    __asm__ ("mov %0 = cr.tpr" :"=r" (tpr));
    return tpr;
}

INLINE void cr_set_tpr (cr_tpr_t tpr)
{
    __asm__ ("mov cr.tpr = %0" ::"r" (tpr));
}

INLINE word_t cr_get_irr (word_t n)
{
    word_t irr;
    switch (n)
    {
    case 0: __asm__ ("mov %0 = cr.irr0" :"=r" (irr)); break;
    case 1: __asm__ ("mov %0 = cr.irr1" :"=r" (irr)); break;
    case 2: __asm__ ("mov %0 = cr.irr2" :"=r" (irr)); break;
    case 3: __asm__ ("mov %0 = cr.irr3" :"=r" (irr)); break;
    default: irr = 0;
    }
    return irr;
}

INLINE bool is_interrupt_pending (word_t n)
{
    word_t irr = cr_get_irr (n / 64);
    return (irr & (1UL << (n & 63)));
}

INLINE cr_ivec_t cr_get_itv (void)
{
    cr_ivec_t itv;
    __asm__ ("mov %0 = cr.itv" :"=r" (itv));
    return itv;
}

INLINE void cr_set_itv (cr_ivec_t itv)
{
    __asm__ ("mov cr.itv = %0" ::"r" (itv));
}

INLINE cr_ivec_t cr_get_pmv (void)
{
    cr_ivec_t pmv;
    __asm__ ("mov %0 = cr.pmv" :"=r" (pmv));
    return pmv;
}

INLINE void cr_set_pmv (cr_ivec_t pmv)
{
    __asm__ ("mov cr.pmv = %0" ::"r" (pmv));
}

INLINE cr_ivec_t cr_get_cmcv (void)
{
    cr_ivec_t cmcv;
    __asm__ ("mov %0 = cr.cmcv" :"=r" (cmcv));
    return cmcv;
}

INLINE void cr_set_cmcv (cr_ivec_t cmcv)
{
    __asm__ ("mov cr.cmcv = %0" ::"r" (cmcv));
}

INLINE cr_ivec_t cr_get_lrr0 (void)
{
    cr_ivec_t lrr;
    __asm__ ("mov %0 = cr.lrr0" :"=r" (lrr));
    return lrr;
}

INLINE void cr_set_lrr0 (cr_ivec_t lrr)
{
    __asm__ ("mov cr.lrr0 = %0" ::"r" (lrr));
}

INLINE cr_ivec_t cr_get_lrr1 (void)
{
    cr_ivec_t lrr;
    __asm__ ("mov %0 = cr.lrr1" :"=r" (lrr));
    return lrr;
}

INLINE void cr_set_lrr1 (cr_ivec_t lrr)
{
    __asm__ ("mov cr.lrr1 = %0" ::"r" (lrr));
}

#endif /* !__ARCH__IA64__CR_H__ */
