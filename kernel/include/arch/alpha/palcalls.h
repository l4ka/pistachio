/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/palcalls.h
 * Created:       15/07/2002 17:29:59 by Simon Winwood (sjw)
 * Description:   Kernel PAL calls 
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
 * $Id: palcalls.h,v 1.12 2004/11/08 23:54:19 awiggins Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__ALPHA__PALCALLS_H__
#define __ARCH__ALPHA__PALCALLS_H__

#include INC_ARCH(pal.h)

/* return value is in v0 */
/* sjw (15/07/2002): Not all pal will use v0 ... */
#define syscall0(name, scratch...)						\
static inline u64_t name (void) {						\
	u64_t register v0 __asm__("$0");					\
										\
	asm volatile ("call_pal %1" : "=r" (v0) : "i" (PAL_##name) : scratch);	\
										\
	return v0;								\
}

#define syscall1(name, arg1, scratch...)							\
static inline u64_t name (u64_t arg1) {								\
	u64_t register v0 __asm__("$0");							\
	u64_t register a0 __asm__("$16") = arg1;						\
												\
	asm volatile ("call_pal %1" : "=r" (v0) : "i" (PAL_##name), "r"(a0) : scratch);	\
												\
	return v0;										\
}

#define syscall2(name, arg1, arg2, scratch...)			\
static inline u64_t name (u64_t arg1, u64_t arg2) {		\
	u64_t register v0 __asm__("$0");			\
	u64_t register a0 __asm__("$16") = arg1;		\
	u64_t register a1 __asm__("$17") = arg2;		\
								\
	asm volatile ("call_pal %1" : "=r" (v0) 		\
		      : "i" (PAL_##name), "r"(a0), "r"(a1) 	\
		      : scratch);				\
								\
	return v0;						\
}

/* This class holds all the PAL specific stuff that we require */
class PAL {
 public:
    /* Interrupt stuff */
    enum entry_addresses_e {
	entInt = 0,
	entArith = 1,
	entMM = 2,
	entIF = 3,
	entUna = 4,
	entSys = 5
    };

    /* TLB stuff */
    enum tbi_param_e {
	flush_tbisi = 1,
	flush_tbisd = 2,
	flush_tbis = 3,
	flush_tbiap = -1,
	flush_tbia = -2
    };

    enum IPL_levels_e {
	IPL_all = 0,
	IPL_highest = 7
    };

#define pal_scratch "$1", "$22", "$23", "$24", "$25"

    /* Unprivileged */

    syscall0(bpt, "memory");
    syscall0(bugchk, "memory");
    /* Ignore callsys */
    /* Ignore clrfen */
    syscall2(gentrap, arg0, arg1, "memory");
    syscall0(imb, "memory");
    syscall0(rdunique, "memory");
    /* Ignore urti */
    syscall1(wrunique, unique, "memory");

    /* Priveleged */
    syscall1(cflush, pfn, "memory");
    syscall0(cserve, "memory");
    syscall0(draina, "memory");
    syscall0(halt, "memory");
    syscall0(rdmces, pal_scratch);
    syscall0(rdps, pal_scratch);
    syscall0(rdusp, pal_scratch);
    syscall0(rdval, pal_scratch);
    /* These two will probably be called from asm anyway */ 
    syscall0(retsys, pal_scratch);
    syscall0(rti, pal_scratch);

    syscall1(swpctx, new_pcbb, pal_scratch);
    syscall1(swpipl, ipl, pal_scratch);
    /* Ignore swappal */
    syscall2(tbi, type, addr, pal_scratch);
    syscall0(whami, pal_scratch);
    syscall2(wrent, addr, type, pal_scratch);
    syscall1(wrfen, fen, pal_scratch);
    syscall1(wripir, ipir, pal_scratch);
    syscall1(wrkgp, kgp, pal_scratch);
    syscall1(wrmces, mces, pal_scratch);
    syscall2(wrperfmon, data1, data2, pal_scratch);
    syscall1(wrusp, usp, pal_scratch);
    syscall1(wrval, val, pal_scratch);
    syscall1(wrvptptr, ptr, pal_scratch);
    /* Ignore wtint */

#undef pal_scratch
    
    /* wrapper functions */
    static void tbisi(word_t va) 
	{ tbi(flush_tbisi, va); }

    static void tbisd(word_t va) 
	{ tbi(flush_tbisd, va); }
	
    static void tbis(word_t va) 
	{ tbi(flush_tbis, va); }

    static void tbiap() 
	{ tbi((word_t) flush_tbiap, 0); }

    static void tbia() 
	{ tbi((word_t) flush_tbia, 0); }

};

#endif /* __ARCH__ALPHA__PALCALLS_H__ */
