/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     l4/ia64/runconv.h
 * Description:   IA-64 runtime convention definitions
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
 * $Id: runconv.h,v 1.7 2004/04/15 14:20:38 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__RUNCONV_H__
#define __L4__IA64__RUNCONV_H__

#define __L4_MOST_CALLER_SAVED_GENERAL_REGS 				\
    "r2",  "r3", "r21", "r22",						\
    "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",	\
    "out0",  "out1",  "out2",  "out3",  "out4",				\
    "out5",  "out6",  "out7"

#define __L4_SOME_CALLER_SAVED_GENERAL_REGS 				\
    "r2",  "r3", "r21", "r22",						\
    "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"

#define __L4_CALLER_SAVED_GENERAL_REGS 					\
    "r2",  "r3",							\
    "r8",  "r9",  "r10", "r11",						\
    "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22",	\
    "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",	\
    "out0",  "out1",  "out2",  "out3",  "out4",				\
    "out5",  "out6",  "out7"

#define __L4_CALLER_SAVED_FP_REGS					  \
    "f6",  "f7",  "f8",  "f9",  "f11", "f12", "f13", "f14", "f15",	  \
    "f32", "f33", "f34", "f35", "f36", "f37", "f38", "f39",		  \
    "f40", "f41", "f42", "f43", "f44", "f45", "f46", "f47", "f48", "f49", \
    "f50", "f51", "f52", "f53", "f54", "f55", "f56", "f57", "f58", "f59", \
    "f60", "f61", "f62", "f63", "f64", "f65", "f66", "f67", "f68", "f69", \
    "f70", "f71", "f72", "f73", "f74", "f75", "f76", "f77", "f78", "f79", \
    "f80", "f81", "f82", "f83", "f84", "f85", "f86", "f87", "f88", "f89", \
    "f90", "f91", "f92", "f93", "f94", "f95", "f96", "f97", "f98", "f99", \
    "f100", "f101", "f102", "f103", "f104",				  \
    "f105", "f106", "f107", "f108", "f109",				  \
    "f110", "f111", "f112", "f113", "f114",				  \
    "f115", "f116", "f117", "f118", "f119",				  \
    "f120", "f121", "f122", "f123", "f124",				  \
    "f125", "f126", "f127"

#define __L4_CALLER_SAVED_PREDICATE_REGS \
    "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15"

#define __L4_CALLER_SAVED_BRANCH_REGS \
    "b0", "b6", "b7"

#define __L4_CALLER_SAVED_REGS						\
    __L4_CALLER_SAVED_GENERAL_REGS, __L4_CALLER_SAVED_FP_REGS,		\
    __L4_CALLER_SAVED_PREDICATE_REGS, __L4_CALLER_SAVED_BRANCH_REGS,	\
    "ar.pfs"

#define __L4_CLOBBER_CALLER_REGS(regs...)				\
    __L4_MOST_CALLER_SAVED_GENERAL_REGS, __L4_CALLER_SAVED_FP_REGS,	\
    __L4_CALLER_SAVED_PREDICATE_REGS, __L4_CALLER_SAVED_BRANCH_REGS,	\
    "ar.pfs" , ## regs

#define __L4_CLOBBER_CALLER_REGS_NOOUT(regs...)				\
    __L4_SOME_CALLER_SAVED_GENERAL_REGS, __L4_CALLER_SAVED_FP_REGS,	\
    __L4_CALLER_SAVED_PREDICATE_REGS, __L4_CALLER_SAVED_BRANCH_REGS,	\
    "ar.pfs" , ## regs

#define __L4_CALLEE_SAVED_GENERAL_REGS \
    "r4", "r5", "r6", "r7"

#define __L4_CALLEE_SAVED_FP_REGS				\
    "f2", "f3", "f4", "f5",					\
    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",	\
    "f23", "f24", "f25", "f26", "f27", "f29", "f30", "f31"

#define __L4_CALLEE_SAVED_PREDICATE_REGS				  \
    "p16", "p17", "p18", "p19", 					  \
    "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29", \
    "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", \
    "p40", "p41", "p42", "p43", "p44", "p45", "p46", "p47", "p48", "p49", \
    "p50", "p51", "p52", "p53", "p54", "p55", "p56", "p57", "p58", "p59", \
    "p60", "p61", "p62", "p63"

#define __L4_CALLEE_SAVED_BRANCH_REGS \
    "b1", "b2", "b3", "b4", "b5"

#define __L4_CALLEE_SAVED_REGS						\
    __L4_CALLEE_SAVED_GENERAL_REGS, __L4_CALLEE_SAVED_FP_REGS,		\
    __L4_CALLEE_SAVED_PREDICATE_REGS, __L4_CALLEE_SAVED_BRANCH_REGS


#endif /* !__L4__IA64__RUNCONV_H__ */
