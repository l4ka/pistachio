/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/ski/palemu.S
 * Description:   PAL emulation for HP's SKI emulator
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
 * $Id: palemu.S,v 1.2 2003/09/24 19:05:29 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(asm.h)

#define PALPROC(func, idx)			\
						;\
9:	cmp.ne	p6,p0 = idx, index		;\
(p6)	br.sptk.few 9f				;\
						;\
	.global	func				;\
func:	


BEG_PROC (pal_emulator)

index	= r28
arg1	= r29
arg2	= r30
arg3	= r31

	// Determine procedure type
	mov	r14 = 256
	mov	r15 = 512
	;;
	cmp.ltu	p6,p0 = index, r14
	cmp.ltu	p7,p0 = index, r15
(p6)	br.sptk.few	static
(p7)	br.sptk.few	stacked
	;;
	mov	r14 = 768
	mov	r15 = 1024
	;;
	cmp.ltu	p6,p0 = index, r14
	cmp.ltu	p7,p0 = index, r15
(p6)	br.sptk.few	static
(p7)	br.sptk.few	stacked

	// Reserved index
	mov	ret0 = -1
	mov	ret1 = 0
	mov	ret2 = 0
	mov	ret3 = 0
	br.sptk.many rp


	//
	// Static procedure calls
	//

static:

PALPROC (pal_ptce_info, 6)
	mov	ret0 = 0			// OK
	movl	ret1 = 0x100000000		// tc_base
	movl	ret2 = 0x0000000200000003	// count0, count1
	movl	ret3 = 0x1000000000002000	// stride0, stride1
	br.sptk.many rp

PALPROC (pal_freq_ratios, 14)
    	mov	ret0 = 0			// OK
    	movl	ret1 = 0x100000064		// proc_ratio (1/100)
    	movl	ret2 = 0x100000100		// bus_ratio  (1/256)
    	movl	ret3 = 0x100000064		// itc_ratio (1/100)
	br.sptk.many rp

PALPROC (pal_vm_summary, 8)
    	mov	ret0 = 0			// OK
    	movl	ret1 = 0x003f000000000000	// 63 bits physical
    	movl	ret2 = 0x000000000000003f	// 63 bits virtual
    	movl	ret3 = 0
	br.sptk.many rp

PALPROC (pal_rse_info, 19)
    	mov	ret0 = 0			// OK
    	movl	ret1 = 96			// Number of stacked regs
    	movl	ret2 = 0			// No hints
    	movl	ret3 = 0
	br.sptk.many rp

PALPROC (pal_cache_flush, 1)
	mov	r14 = ar.lc
	movl	r15 = 0x80000			// 512k mill cache lines (16MB)
	;;
	mov	ar.lc = r15
	movl	r15 = 0xe000000000000000
	;;
1:	fc	r15
	add	r15 = 32, r15
	br.cloop.sptk.few 1b
	sync.i
	;;
	srlz.i
	;;
	mov	ar.lc = r14
	mov	ret0 = 0			// OK
	mov	ret3 = 0
	br.sptk.many rp

PALPROC (pal_perf_mon_info, 15)
	mov	ret0 = 0			// OK
	movl	ret1 = 0x12082004		// generic=4, width=32,
						// cycles=18, retired=8
    	movl	ret2 = 0
    	movl	ret3 = 0

	mov	r14 = 0xffff			// Implelemted PMCs
	mov	r15 = 0xffff			// Implemented PMDs
	add	arg2 = 8, arg1
	;;
	// Bitmask for implemented PMCs
	st8	[arg1] = r14, 16
	st8	[arg2] = r0, 16
	;;
	st8	[arg1] = r0, 16
	st8	[arg2] = r0, 16
	;;
	// Bitmask for implemented PMDs
	st8	[arg1] = r15, 16
	st8	[arg2] = r0, 16
	;;
	st8	[arg1] = r0, 16
	st8	[arg2] = r0, 16
	mov	r14 = 0xf0			// Cycle count PMCs
	mov	r15 = 0x10			// Retired bundles PMCs
	;;
	// Bitmask for cycle count capable PMCs
	st8	[arg1] = r14, 16
	st8	[arg2] = r0, 16
	;;
	st8	[arg1] = r0, 16
	st8	[arg2] = r0, 16
	;;
	// Bitmask for retired bundles capable PMCs
	st8	[arg1] = r15, 16
	st8	[arg2] = r0, 16
	;;
	st8	[arg1] = r0
	st8	[arg2] = r0
	;;
	br.sptk.many rp

9:	// Unimplemented call
	mov	ret0 = -1
	mov	ret1 = 0
	mov	ret2 = 0
	mov	ret3 = 0
	br.sptk.many rp



	//
	// Stacked procedure calls
	//

stacked:

9:	// Unimplemented call
	mov	ret0 = -1
	mov	ret1 = 0
	mov	ret2 = 0
	mov	ret3 = 0
	br.ret.sptk.many rp

END_PROC (pal_emulator)
