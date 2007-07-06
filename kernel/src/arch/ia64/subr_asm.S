/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/subr_asm.S
 * Description:   Various IA-64 helper functions
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
 * $Id: subr_asm.S,v 1.15 2003/11/03 16:27:30 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(asm.h)
#include INC_ARCH(psr.h)


/*
 * PSR bits for PAL calls.
 */

#define CLR_BITS_PAL	       (PSR_INT_ENABLE				| \
				PSR_LOWER_FP_DISABLE			| \
				PSR_DEBUG_BREAKPOINT			| \
				PSR_LOWER_PRIV_TRANSFER_TRAP		| \
				PSR_TAKEN_BRANCH_TRAP			| \
				PSR_INSTRUCTION_SET			| \
				PSR_INSTRUCTION_DEBUG_DISABLE		| \
				PSR_DATA_ACCESS_FAULTS			| \
				PSR_DATA_DEBUG_FAULTS_DISABLE		| \
				PSR_SINGLE_STEP_ENABLE			| \
				PSR_EXCEPTION_DEFERRAL			| \
				PSR_INSTRUCTION_ACCESS_FAULT_DISABLE)

#define SET_BITS_PAL	       (PSR_REGISTER_BANK_1)

#define CLR_BITS_PAL_PHYS      (CLR_BITS_PAL				| \
				PSR_DATA_TRANSLATION			| \
				PSR_REGISTER_STACK_TRANSLATION		| \
				PSR_INSTRUCTION_TRANSLATION)
				
/*
 * PSR bits for EFI calls.
 */
				
#define CLR_BITS_EFI	       (PSR_INSTRUCTION_SET			| \
				PSR_DATA_ACCESS_FAULTS			| \
				PSR_DATA_DEBUG_FAULTS_DISABLE		| \
				PSR_SINGLE_STEP_ENABLE			| \
				PSR_EXCEPTION_DEFERRAL			| \
				PSR_INSTRUCTION_ACCESS_FAULT_DISABLE)

#define CLR_BITS_EFI_PHYS      (CLR_BITS_EFI				| \
				PSR_DATA_TRANSLATION			| \
				PSR_REGISTER_STACK_TRANSLATION		| \
				PSR_INSTRUCTION_TRANSLATION)

#define SET_BITS_EFI	       (PSR_REGISTER_BANK_1)


/*
 * PSR bits for determining addressing mode.
 */

#define PSR_ADDR_MODE_BITS	(PSR_DATA_TRANSLATION			| \
				 PSR_REGISTER_STACK_TRANSLATION		| \
				 PSR_INSTRUCTION_TRANSLATION)

/**
 * Switch addressing mode.  Stack pointers, ip and gp are converted so
 * that they are still valid.
 *
 * @param region	Region to switch to.  If region equals 0, also
 *			turn off address translation
 */
BEG_PROC (ia64_switch_mode)
	.regstk	1,9,0,0

region		= in0
saved_pfs	= loc0
saved_rsc	= loc1
saved_rnat	= loc2
new_ip		= loc3
new_bsp		= loc4
new_rp		= loc5
new_psr		= loc6
mode_bits	= loc7
phys_offset	= loc8

	mov	new_psr = psr
	rsm	psr.i | psr.ic
	mov	saved_rsc = ar.rsc
	cmp.eq	p6,p7 = 0,region		// determine mode
	;;
	mov	ar.rsc = 0
	flushrs					// spill all dirty regs
	srlz.i
	;;
1:	mov	new_ip  = ip
	mov	new_bsp = ar.bsp
	mov	new_rp  = rp
	mov	saved_rnat = ar.rnat
	dep	new_psr = -1,new_psr,44,1	// register bank 1
	movl	mode_bits = PSR_ADDR_MODE_BITS
	.pred.rel "mutex",p6,p7
(p6)	movl	phys_offset = -CONFIG_IA64_PHYSMEM_OFFSET
(p7)	movl	phys_offset = CONFIG_IA64_PHYSMEM_OFFSET
	;;
	dep	new_bsp = region,new_bsp,61,3	// relocate bsp
	dep	new_rp  = region, new_rp,61,3	// relocate rp
	dep	sp      = region,     sp,61,3	// relocate sp
	dep	gp      = region,     gp,61,3	// relocate gp
	dep	new_ip  = region, new_ip,61,3	// relocate ip
	.pred.rel "mutex",p6,p7
(p6)	andcm	new_psr = new_psr, mode_bits
(p7)	or	new_psr = new_psr, mode_bits
	;;
	add	new_bsp = phys_offset, new_bsp
	add	new_rp  = phys_offset, new_rp
	add	sp      = phys_offset, sp
	add	gp      = phys_offset, gp
	add	new_ip  = phys_offset, new_ip
	;;
	mov	ar.bspstore = new_bsp
	mov	rp = new_rp
	add	new_ip = 2f-1b,new_ip		// calculate rfi return
	;;
	mov	ar.rnat = saved_rnat
	mov	cr.ipsr = new_psr
	mov	cr.iip = new_ip
	mov	cr.ifs = r0
	;;
	rfi
	;;	
2:	mov	ar.rsc = saved_rsc
	br.ret.sptk.few rp

END_PROC (ia64_switch_mode)


/**
 * Switch to physical addressing mode.  Convert stack pointers, ip, and
 * gp to virtual pointers.
 */
BEG_PROC (ia64_switch_to_phys)
	alloc	loc0 = ar.pfs,1,9,0,0
	mov	in0 = 0
	br.sptk.few ia64_switch_mode
END_PROC (ia64_switch_to_phys)


/**
 * Switch to virtual addressing mode.  Convert stack pointers, ip, and
 * gp to virtual pointers.
 */
BEG_PROC (ia64_switch_to_virt)
	alloc	loc0 = ar.pfs,1,9,0,0
	mov	in0 = 7
	br.sptk.few ia64_switch_mode
END_PROC (ia64_switch_to_virt)




/**
 * Changes addressing mode according to new PSR value (passed in through
 * r14).  Relocate stack pointer and register stack pointer according to
 * the new mode.
 *
 * @param new_psr	new value for PSR (in r14)
 *
 * Clobbers:	r15-r21, p6-p7
 * Clears:	psr.i, psr.ic
 */
BEG_PROC (ia64_set_addressing_mode)

new_psr		= r14
saved_rsc	= r15	// below r16 in case of bank switching
region		= r16
new_bsp		= r17
new_rp		= r18
new_ip		= r19
saved_rnat	= r20
phys_offset	= r21

	rsm	psr.i | psr.ic
	mov	saved_rsc = ar.rsc
	tbit.nz	p6,p7 = new_psr,17		// determine mode
	;;
	mov	ar.rsc = 0
(p6)	mov	region = 7			// virtual addresses
(p7)	mov	region = 0			// physical addresses
(p6)	movl	phys_offset = CONFIG_IA64_PHYSMEM_OFFSET
(p7)	movl	phys_offset = -CONFIG_IA64_PHYSMEM_OFFSET
	flushrs					// spill all dirty regs
	srlz.i
	;;
1:	mov	new_ip = ip
	mov	new_bsp = ar.bsp
	mov	new_rp = rp
	mov	saved_rnat = ar.rnat
	;;
	dep	new_bsp = region,new_bsp,61,3	// relocate bsp
	dep	new_rp  = region, new_rp,61,3	// relocate rp
	dep	sp      = region,     sp,61,3	// relocate sp
	dep	new_ip  = region, new_ip,61,3	// relocate ip
	;;
	add	new_bsp = phys_offset, new_bsp
	add	new_rp  = phys_offset, new_rp
	add	sp      = phys_offset, sp
	add	new_ip	= phys_offset, new_ip
	;;
	mov	ar.bspstore = new_bsp
	mov	rp = new_rp
	add	new_ip = 2f-1b,new_ip		// calculate rfi return
	;;
	mov	ar.rnat = saved_rnat
	mov	cr.ipsr = new_psr
	mov	cr.iip = new_ip
	mov	cr.ifs = r0
	;;
	rfi
	;;	
2:	mov	ar.rsc = saved_rsc
	br.ret.sptk.few rp
	
END_PROC (ia64_set_addressing_mode)


/**
 * Switch to physcical mode and invoke the indicated EFI function.
 *
 * @param function	physical address of function pointer
 * @param arg1		1st arg to EFI function
 * @param arg2		2nd arg to EFI function
 * @param arg3		3rd arg to EFI function
 * @param arg4		4th arg to EFI function
 * @param arg5		5th arg to EFI function
 *
 */
BEG_PROC (call_efi_physical)
	.regstk	6,5,5,0

saved_pfs	= loc0
saved_rp	= loc1
saved_gp	= loc2
func_ip		= loc3
func_gp		= loc4

	alloc	saved_pfs = ar.pfs,6,5,5,0
	mov	saved_rp = rp
	mov	saved_gp = gp
	;;
	br.call.sptk.few rp = ia64_switch_to_phys

	ld8	func_ip = [in0], 8 ;;
	ld8	func_gp = [in0]
	;;
	dep	func_ip = 0,func_ip,61,3
	dep	func_gp = 0,func_gp,61,3
	;;
	mov	b6 = func_ip
	mov	gp = func_gp
	mov	out0 = in1
	mov	out1 = in2
	mov	out2 = in3
	mov	out3 = in4
	mov	out4 = in5

	br.call.sptk.many rp = b6
	;;
	br.call.sptk.few rp = ia64_switch_to_virt

	mov	gp = saved_gp
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	;;
	br.ret.sptk.many rp

END_PROC (call_efi_physical)


/**
 * Invoke the indicated EFI function.  Needed because kernel is
 * compiled -mno-pic and can therefore not handle function pointers
 * properly.
 *
 * @param function	virtual address of function pointer
 * @param arg1		1st arg to EFI function
 * @param arg2		2nd arg to EFI function
 * @param arg3		3rd arg to EFI function
 * @param arg4		4th arg to EFI function
 * @param arg5		5th arg to EFI function
 *
 */
BEG_PROC (call_efi)
	.regstk 6,3,5,0

saved_pfs	= loc0
saved_gp	= loc1
saved_rp	= loc2

	alloc	saved_pfs = ar.pfs,6,3,5,0
	mov	saved_gp = gp
	mov	saved_rp = rp
	ld8	r14 = [in0], 8		// EFI function entry point	
	;;
	mov	out0 = in1
	mov	out1 = in2
	mov	out2 = in3
	mov	out3 = in4
	mov	out4 = in5
	mov	b6 = r14
	ld8	gp = [in0]		// EFI function's global pointer
	;;
	br.call.sptk.many rp = b6
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	;;
	br.ret.sptk.many rp

END_PROC (call_efi)


/**
 * Makes a static PAL call in virtual mode.  Return four words (in
 * ret0-ret3) as specified by the IA-64 Software Developer's Manual.
 *
 * @param idx		index of PAL procedure
 * @param a1		1st argument to PAL
 * @param a2		2nd argument to PAL
 * @param a3		3rd argument to PAL
 *
 */
BEG_PROC (call_pal_static)
	.regstk 4,6,0,0	

saved_pfs	= loc0
saved_rp	= loc1
saved_psr	= loc2
entry		= loc3
return_ip	= loc4
phys_offset	= loc5

	{
1:	alloc	saved_pfs = ar.pfs,4,6,0,0	
	mov	return_ip = ip
	mov	saved_rp = rp
	}
	movl	entry = @gprel(ia64_pal_entry)
	mov	saved_psr = psr
	movl	phys_offset = CONFIG_IA64_PHYSMEM_OFFSET
	;;
	add	entry = entry,gp
	mov	r28 = in0		// procedure number
	;;
	ld8	entry = [entry]		// get entry point
	mov	r29 = in1
	mov	r30 = in2
	mov	r31 = in3
	;;
	dep	entry = -1,entry,61,3	// relocate to virtual address
	add	return_ip = 2f-1b,return_ip // calculate return address
	;;
	add	entry = phys_offset, entry
	;;
	mov	b6 = entry
	mov	b0 = return_ip
	rsm	psr.i			// disble interrupts
	;;
	br.cond.sptk b6			// call PAL
2:	mov	psr.l = saved_psr
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	bsw.1
	;;
	srlz.d
	br.ret.sptk rp

END_PROC (call_pal_static)


/**
 * Makes a static PAL call in physical mode.  Return four words (in
 * ret0-ret3) as specified by the IA-64 Software Developer's Manual.
 *
 * @param idx		index of PAL procedure
 * @param a1		1st argument to PAL
 * @param a2		2nd argument to PAL
 * @param a3		3rd argument to PAL
 *
 */
BEG_PROC (call_pal_static_phys)
	.regstk 4,6,0,0	

saved_pfs	= loc0
saved_rp	= loc1
saved_psr	= loc2
entry		= loc3
return_ip	= loc4
phys_offset	= loc5

	alloc	saved_pfs = ar.pfs,4,6,0,0	
	mov	return_ip = ip
	mov	saved_rp = rp

	movl	entry = @gprel(ia64_pal_entry)
	mov	saved_psr = psr
	movl	phys_offset = CONFIG_IA64_PHYSMEM_OFFSET
	;;
	add	entry = entry,gp
	mov	r28 = in0		// procedure number
	;;
	ld8	entry = [entry]		// get entry point
	mov	r29 = in1
	mov	r30 = in2
	mov	r31 = in3
	br.call.sptk rp = ia64_switch_to_phys
	;;
1:	mov	return_ip = ip
	;;
	add	return_ip = 2f-1b,return_ip // calculate return address
	;;
	mov	b6 = entry
	mov	b0 = return_ip
	rsm	psr.i			// disble interrupts
	;;
	br.cond.sptk b6			// call PAL
	;;
2:	br.call.sptk.few rp = ia64_switch_to_virt
	;;
	mov	psr.l = saved_psr
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	bsw.1
	;;
	srlz.d
	br.ret.sptk rp

END_PROC (call_pal_static_phys)


/**
 * Makes a stacked PAL call in virtual mode.  Return four words (in
 * ret0-ret3) as specified by the IA-64 Software Developer's Manual.
 *
 * @param idx		index of PAL procedure
 * @param a1		1st argument to PAL
 * @param a2		2nd argument to PAL
 * @param a3		3rd argument to PAL
 *
 */
BEG_PROC (call_pal_stacked)
	.regstk 4,5,4,0	

saved_pfs	= loc0
saved_rp	= loc1
saved_psr	= loc2
entry		= loc3
phys_offset	= loc4

	alloc	saved_pfs = ar.pfs,4,5,4,0	
	mov	saved_rp = rp
	mov	saved_psr = psr
	movl	entry = @gprel(ia64_pal_entry)
	movl	phys_offset = CONFIG_IA64_PHYSMEM_OFFSET
	;;
	add	entry = entry,gp
	mov	out0 = in0		// procedure number
	mov	r28 = in0
	;;
	ld8	entry = [entry]		// get entry point
	mov	out1 = in1
	mov	out2 = in2
	mov	out3 = in3
	;;
	dep	entry = -1,entry,61,3	// relocate to virtual address
	;;
	add	entry = phys_offset, entry
	;;
	mov	b6 = entry
	rsm	psr.i			// disble interrupts
	;;
	br.call.sptk.many rp = b6	// call PAL
	mov	psr.l = saved_psr
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	bsw.1
	;;
	srlz.d
	br.ret.sptk rp

END_PROC (call_pal_stacked)


/**
 * Makes a stacked PAL call in physical mode.  Return four words (in
 * ret0-ret3) as specified by the IA-64 Software Developer's Manual.
 *
 * @param idx		index of PAL procedure
 * @param a1		1st argument to PAL
 * @param a2		2nd argument to PAL
 * @param a3		3rd argument to PAL
 *
 */
BEG_PROC (call_pal_stacked_phys)
	.regstk 4,5,4,0	

saved_pfs	= loc0
saved_rp	= loc1
saved_psr	= loc2
entry		= loc3
return_ip	= loc4

	alloc	saved_pfs = ar.pfs,4,5,4,0	
	mov	saved_psr = psr
	movl	r14 = CLR_BITS_PAL_PHYS
	mov	saved_rp = rp
	movl	entry = @gprel(ia64_pal_entry)
	;;
	add	entry = entry,gp
	movl	r15 = SET_BITS_PAL
	andcm	r14 = saved_psr, r14
	;;
	or	r14 = r14, r15
	ld8	entry = [entry]		// get entry point
	;;
	br.call.sptk.many rp = ia64_set_addressing_mode
	;;
	mov	r28 = in0		// copy arguments
	mov	out0 = in0
	mov	out1 = in1
	mov	out2 = in2
	mov	out3 = in3
	mov	b6 = entry
	;;
	br.call.sptk.many rp = b6	// call PAL
	mov	r14 = saved_psr
	;;
	br.call.sptk.many rp = ia64_set_addressing_mode
	bsw.1
	;;
	mov	rp = saved_rp
	mov	ar.pfs = saved_pfs
	;;
	srlz.d
	br.ret.sptk.many rp

END_PROC (call_pal_stacked_phys)


/**
 * Stub invoked when there is contention on a spinlock.
 *
 * r30:		IP of spinlock code
 * r31:		Location of spinlock variable
 * p15, b7:	Scratch registers
 *
 */
BEG_PROC (ia64_spinlock_contention)
	mov	b7 = r30
1:	ld8	r30 = [r31]
	;;
	cmp.ne	p15,p0 = r30, r0
	;;
(p15)	br.spnt.few 1b	
	br.sptk.many b7
END_PROC (ia64_spinlock_contention)


/**
 * Acquire spinlock.
 * @param lock		location of spinlock
 */
BEG_PROC (ia64_acquire_spinlock)
	alloc	r14 = ar.pfs,1,0,0,0
	mov	r31 = in0
1:	mov	r30 = ip
	mov	r29 = 1
	mov	ar.ccv = r0
	;;
	cmpxchg8.acq r29 = [r31], r29, ar.ccv
	;;
	cmp.ne	p15,p0 = r29, r0
	;;
(p15)	br.spnt.few ia64_spinlock_contention
	br.ret.sptk.many b0
END_PROC (ia64_acquire_spinlock)


/**
 * Release spinlock.
 * @param lock		location of spinlock
 */
BEG_PROC (ia64_release_spinlock)
	alloc	r14 = ar.pfs,1,0,0,0
	st8.rel	[in0] = r0
	;;
	br.ret.sptk.many b0
END_PROC (ia64_release_spinlock)


/**
 * Spin wheel.
 * @param pos		position of spin wheel
 */
BEG_PROC (spin_wheel)
	alloc	loc0 = ar.pfs,1,3,0,0
	movl	loc1 = (6 << 61) + (0xb8000) + CONFIG_IA64_PHYSMEM_OFFSET
	;;
	shladd	loc1 =  in0, 1, loc1
	;;
	ld1	loc2 = [loc1]
	;;
	cmp.eq	p6,p0 = 0x2d, loc2
	cmp.eq	p7,p0 = 0x5c, loc2
	cmp.eq	p8,p0 = 0x7c, loc2
	mov	loc2 = 0x2d
	;;
	.pred.rel "mutex",p6,p7
	.pred.rel "mutex",p6,p8
	.pred.rel "mutex",p7,p8
(p6)	mov	loc2 = 0x5c
(p7)	mov	loc2 = 0x7c
(p8)	mov	loc2 = 0x2f
	;;
	st1	[loc1] = loc2, 1
	mov	loc2 = 7
	;;
	st1	[loc1] = loc2
	br.ret.sptk.many rp
END_PROC (spin_wheel)


BEG_PROC (__udivdi3)
	.regstk 2,0,0,0
	// Transfer inputs to FP registers.
	setf.sig f8 = in0
	setf.sig f9 = in1
	;;
	// Convert the inputs to FP, to avoid FP software-assist faults.
	fcvt.xuf.s1 f8 = f8
	fcvt.xuf.s1 f9 = f9
	;;
	// Compute the reciprocal approximation.
	frcpa.s1 f10, p6 = f8, f9
	;;
	// 3 Newton-Raphson iterations.
(p6)	fnma.s1 f11 = f9, f10, f1
(p6)	fmpy.s1 f12 = f8, f10
	;;
(p6)	fmpy.s1 f13 = f11, f11
(p6)	fma.s1 f12 = f11, f12, f12
	;;
(p6)	fma.s1 f10 = f11, f10, f10
(p6)	fma.s1 f11 = f13, f12, f12
	;;
(p6)	fma.s1 f10 = f13, f10, f10
(p6)	fnma.s1 f12 = f9, f11, f8
	;;
(p6)	fma.s1 f10 = f12, f10, f11
	;;
	// Round quotient to an unsigned integer.
	fcvt.fxu.trunc.s1 f10 = f10
	;;
	// Transfer result to GP registers.
	getf.sig ret0 = f10
	br.ret.sptk rp
	;;
END_PROC (__udivdi3)

BEG_PROC (__divdi3)

	.regstk 2,0,0,0
	// Transfer inputs to FP registers.
	setf.sig f8 = in0
	setf.sig f9 = in1
	;;
	// Convert the inputs to FP, so that they won't be treated as unsigned.
	fcvt.xf f8 = f8
	fcvt.xf f9 = f9
	;;
	// Compute the reciprocal approximation.
	frcpa.s1 f10, p6 = f8, f9
	;;
	// 3 Newton-Raphson iterations.
(p6)	fnma.s1 f11 = f9, f10, f1
(p6)	fmpy.s1 f12 = f8, f10
	;;
(p6)	fmpy.s1 f13 = f11, f11
(p6)	fma.s1 f12 = f11, f12, f12
	;;
(p6)	fma.s1 f10 = f11, f10, f10
(p6)	fma.s1 f11 = f13, f12, f12
	;;
(p6)	fma.s1 f10 = f13, f10, f10
(p6)	fnma.s1 f12 = f9, f11, f8
	;;
(p6)	fma.s1 f10 = f12, f10, f11
	;;
	// Round quotient to an integer.
	fcvt.fx.trunc.s1 f10 = f10
	;;
	// Transfer result to GP registers.
	getf.sig ret0 = f10
	br.ret.sptk rp
	;;

END_PROC (__divdi3)


/**
 * Save upper floating-point registers (f32--f127).
 *
 * @param buf		location to store registers (1536 bytes)
 */
BEG_PROC (ia64_save_highfp)

	.regstk 1,0,0,0

ptr0	= in0
ptr1	= r14

	add	ptr1 = 16, in0
	;;
	stf.spill [ptr0] = f32, 32
	stf.spill [ptr1] = f33, 32
	;; 
	stf.spill [ptr0] = f34, 32
	stf.spill [ptr1] = f35, 32
	;; 
	stf.spill [ptr0] = f36, 32
	stf.spill [ptr1] = f37, 32
	;;
	stf.spill [ptr0] = f38, 32
	stf.spill [ptr1] = f39, 32
	;;
	stf.spill [ptr0] = f40, 32
	stf.spill [ptr1] = f41, 32
	;;
	stf.spill [ptr0] = f42, 32
	stf.spill [ptr1] = f43, 32
	;; 
	stf.spill [ptr0] = f44, 32
	stf.spill [ptr1] = f45, 32
	;; 
	stf.spill [ptr0] = f46, 32
	stf.spill [ptr1] = f47, 32
	;; 
	stf.spill [ptr0] = f48, 32
	stf.spill [ptr1] = f49, 32
	;; 
	stf.spill [ptr0] = f50, 32
	stf.spill [ptr1] = f51, 32
	;; 
	stf.spill [ptr0] = f52, 32
	stf.spill [ptr1] = f53, 32
	;; 
	stf.spill [ptr0] = f54, 32
	stf.spill [ptr1] = f55, 32
	;; 
	stf.spill [ptr0] = f56, 32
	stf.spill [ptr1] = f57, 32
	;; 
	stf.spill [ptr0] = f58, 32
	stf.spill [ptr1] = f59, 32
	;; 
	stf.spill [ptr0] = f60, 32
	stf.spill [ptr1] = f61, 32
	;; 
	stf.spill [ptr0] = f62, 32
	stf.spill [ptr1] = f63, 32
	;; 
	stf.spill [ptr0] = f64, 32
	stf.spill [ptr1] = f65, 32
	;; 
	stf.spill [ptr0] = f66, 32
	stf.spill [ptr1] = f67, 32
	;; 
	stf.spill [ptr0] = f68, 32
	stf.spill [ptr1] = f69, 32
	;; 
	stf.spill [ptr0] = f70, 32
	stf.spill [ptr1] = f71, 32
	;; 
	stf.spill [ptr0] = f72, 32
	stf.spill [ptr1] = f73, 32
	;; 
	stf.spill [ptr0] = f74, 32
	stf.spill [ptr1] = f75, 32
	;; 
	stf.spill [ptr0] = f76, 32
	stf.spill [ptr1] = f77, 32
	;; 
	stf.spill [ptr0] = f78, 32
	stf.spill [ptr1] = f79, 32
	;; 
	stf.spill [ptr0] = f80, 32
	stf.spill [ptr1] = f81, 32
	;; 
	stf.spill [ptr0] = f82, 32
	stf.spill [ptr1] = f83, 32
	;; 
	stf.spill [ptr0] = f84, 32
	stf.spill [ptr1] = f85, 32
	;; 
	stf.spill [ptr0] = f86, 32
	stf.spill [ptr1] = f87, 32
	;; 
	stf.spill [ptr0] = f88, 32
	stf.spill [ptr1] = f89, 32
	;; 
	stf.spill [ptr0] = f90, 32
	stf.spill [ptr1] = f91, 32
	;; 
	stf.spill [ptr0] = f92, 32
	stf.spill [ptr1] = f93, 32
	;; 
	stf.spill [ptr0] = f94, 32
	stf.spill [ptr1] = f95, 32
	;; 
	stf.spill [ptr0] = f96, 32
	stf.spill [ptr1] = f97, 32
	;; 
	stf.spill [ptr0] = f98, 32
	stf.spill [ptr1] = f99, 32
	;; 
	stf.spill [ptr0] = f100, 32
	stf.spill [ptr1] = f101, 32
	;; 
	stf.spill [ptr0] = f102, 32
	stf.spill [ptr1] = f103, 32
	;; 
	stf.spill [ptr0] = f104, 32
	stf.spill [ptr1] = f105, 32
	;; 
	stf.spill [ptr0] = f106, 32
	stf.spill [ptr1] = f107, 32
	;; 
	stf.spill [ptr0] = f108, 32
	stf.spill [ptr1] = f109, 32
	;; 
	stf.spill [ptr0] = f110, 32
	stf.spill [ptr1] = f111, 32
	;; 
	stf.spill [ptr0] = f112, 32
	stf.spill [ptr1] = f113, 32
	;; 
	stf.spill [ptr0] = f114, 32
	stf.spill [ptr1] = f115, 32
	;; 
	stf.spill [ptr0] = f116, 32
	stf.spill [ptr1] = f117, 32
	;; 
	stf.spill [ptr0] = f118, 32
	stf.spill [ptr1] = f119, 32
	;; 
	stf.spill [ptr0] = f120, 32
	stf.spill [ptr1] = f121, 32
	;; 
	stf.spill [ptr0] = f122, 32
	stf.spill [ptr1] = f123, 32
	;; 
	stf.spill [ptr0] = f124, 32
	stf.spill [ptr1] = f125, 32
	;; 
	stf.spill [ptr0] = f126
	stf.spill [ptr1] = f127
	;; 
	br.ret.sptk.few rp

END_PROC (ia64_save_highfp)


/**
 * Restore upper floating-point registers (f32--f127).
 *
 * @param buf		location where registers are stored (1536 bytes)
 */
BEG_PROC (ia64_restore_highfp)
	
	.regstk 1,0,0,0

ptr0	= in0
ptr1	= r14

	add	ptr1 = 16, in0
	;;
	ldf.fill f32 = [ptr0], 32
	ldf.fill f33 = [ptr1], 32
	;; 
	ldf.fill f34 = [ptr0], 32
	ldf.fill f35 = [ptr1], 32
	;; 
	ldf.fill f36 = [ptr0], 32
	ldf.fill f37 = [ptr1], 32
	;; 
	ldf.fill f38 = [ptr0], 32
	ldf.fill f39 = [ptr1], 32
	;; 
	ldf.fill f40 = [ptr0], 32
	ldf.fill f41 = [ptr1], 32
	;; 
	ldf.fill f42 = [ptr0], 32
	ldf.fill f43 = [ptr1], 32
	;; 
	ldf.fill f44 = [ptr0], 32
	ldf.fill f45 = [ptr1], 32
	;; 
	ldf.fill f46 = [ptr0], 32
	ldf.fill f47 = [ptr1], 32
	;; 
	ldf.fill f48 = [ptr0], 32
	ldf.fill f49 = [ptr1], 32
	;; 
	ldf.fill f50 = [ptr0], 32
	ldf.fill f51 = [ptr1], 32
	;; 
	ldf.fill f52 = [ptr0], 32
	ldf.fill f53 = [ptr1], 32
	;; 
	ldf.fill f54 = [ptr0], 32
	ldf.fill f55 = [ptr1], 32
	;; 
	ldf.fill f56 = [ptr0], 32
	ldf.fill f57 = [ptr1], 32
	;; 
	ldf.fill f58 = [ptr0], 32
	ldf.fill f59 = [ptr1], 32
	;; 
	ldf.fill f60 = [ptr0], 32
	ldf.fill f61 = [ptr1], 32
	;; 
	ldf.fill f62 = [ptr0], 32
	ldf.fill f63 = [ptr1], 32
	;; 
	ldf.fill f64 = [ptr0], 32
	ldf.fill f65 = [ptr1], 32
	;; 
	ldf.fill f66 = [ptr0], 32
	ldf.fill f67 = [ptr1], 32
	;; 
	ldf.fill f68 = [ptr0], 32
	ldf.fill f69 = [ptr1], 32
	;; 
	ldf.fill f70 = [ptr0], 32
	ldf.fill f71 = [ptr1], 32
	;; 
	ldf.fill f72 = [ptr0], 32
	ldf.fill f73 = [ptr1], 32
	;; 
	ldf.fill f74 = [ptr0], 32
	ldf.fill f75 = [ptr1], 32
	;; 
	ldf.fill f76 = [ptr0], 32
	ldf.fill f77 = [ptr1], 32
	;; 
	ldf.fill f78 = [ptr0], 32
	ldf.fill f79 = [ptr1], 32
	;; 
	ldf.fill f80 = [ptr0], 32
	ldf.fill f81 = [ptr1], 32
	;; 
	ldf.fill f82 = [ptr0], 32
	ldf.fill f83 = [ptr1], 32
	;; 
	ldf.fill f84 = [ptr0], 32
	ldf.fill f85 = [ptr1], 32
	;; 
	ldf.fill f86 = [ptr0], 32
	ldf.fill f87 = [ptr1], 32
	;; 
	ldf.fill f88 = [ptr0], 32
	ldf.fill f89 = [ptr1], 32
	;; 
	ldf.fill f90 = [ptr0], 32
	ldf.fill f91 = [ptr1], 32
	;; 
	ldf.fill f92 = [ptr0], 32
	ldf.fill f93 = [ptr1], 32
	;; 
	ldf.fill f94 = [ptr0], 32
	ldf.fill f95 = [ptr1], 32
	;; 
	ldf.fill f96 = [ptr0], 32
	ldf.fill f97 = [ptr1], 32
	;; 
	ldf.fill f98 = [ptr0], 32
	ldf.fill f99 = [ptr1], 32
	;; 
	ldf.fill f100 = [ptr0], 32
	ldf.fill f101 = [ptr1], 32
	;; 
	ldf.fill f102 = [ptr0], 32
	ldf.fill f103 = [ptr1], 32
	;; 
	ldf.fill f104 = [ptr0], 32
	ldf.fill f105 = [ptr1], 32
	;; 
	ldf.fill f106 = [ptr0], 32
	ldf.fill f107 = [ptr1], 32
	;; 
	ldf.fill f108 = [ptr0], 32
	ldf.fill f109 = [ptr1], 32
	;; 
	ldf.fill f110 = [ptr0], 32
	ldf.fill f111 = [ptr1], 32
	;; 
	ldf.fill f112 = [ptr0], 32
	ldf.fill f113 = [ptr1], 32
	;; 
	ldf.fill f114 = [ptr0], 32
	ldf.fill f115 = [ptr1], 32
	;; 
	ldf.fill f116 = [ptr0], 32
	ldf.fill f117 = [ptr1], 32
	;; 
	ldf.fill f118 = [ptr0], 32
	ldf.fill f119 = [ptr1], 32
	;; 
	ldf.fill f120 = [ptr0], 32
	ldf.fill f121 = [ptr1], 32
	;; 
	ldf.fill f122 = [ptr0], 32
	ldf.fill f123 = [ptr1], 32
	;; 
	ldf.fill f124 = [ptr0], 32
	ldf.fill f125 = [ptr1], 32
	;; 
	ldf.fill f126 = [ptr0]
	ldf.fill f127 = [ptr1]
	;; 
	br.ret.sptk.few rp

END_PROC (ia64_restore_highfp)
