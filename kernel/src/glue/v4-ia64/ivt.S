/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/ivt.S
 * Description:   IA-64 interrupt vector table
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
 * $Id: ivt.S,v 1.34 2004/04/01 14:49:20 skoglund Exp $
 *                
 ********************************************************************/
#include <asmsyms.h>
#include INC_ARCH(asm.h)
#include INC_GLUE(context_handle.h)
#include INC_GLUE(space.h)

#define BEG_IVT_ENTRY(num, offset, name)		;\
	.org	ia64_vector_table + offset		;\
	.globl	ivt_##name				;\
ivt_##name:
#define END_IVT_ENTRY(num, name)


#define RESERVED(num, offset)				\
	BEG_IVT_ENTRY(num, offset, reserved_##num)	;\
	EXCEPTION (num)					;\
	END_IVT_ENTRY(num, reserved_##num)


#define xxENABLE_INTS()	(p2) ssm psr.i
#define ENABLE_INTS()


#define EXCEPTION(num)					\
							;\
arg_ip		= r31					;\
arg_num		= r30					;\
							;\
1:	mov	arg_ip = ip				;\
	mov	arg_num = num				;\
	;;						;\
	add	arg_ip = 2f-1b, arg_ip			;\
	br.sptk.many save_context			;\
							;\
2:	alloc	r14 = ar.pfs,0,0,2,0			;\
	mov	out0 = num				;\
	mov	out1 = sp				;\
	add	sp = -16,sp ;;				;\
	br.call.sptk.many rp = handle_exception		;\
	add	sp = 16,sp				;\
	ENABLE_INTS()					;\
	br.sptk.many load_context			;\
	/* NOTREACHED */				;\
	rfi

#define SAVE_EXCEPTION_CONTEXT_CALL(num)		\
							;\
arg_ip		= r31					;\
arg_num		= r30					;\
							;\
1:	mov	arg_ip = ip				;\
	mov	arg_num = num				;\
	;;						;\
	add	arg_ip = 2f-1b, arg_ip			;\
	br.sptk.many save_context			;\
2:	


#define HANDLE_EXCEPTION_SHORT(num, func)		\
							;\
arg_ip		= r31					;\
arg_num		= r30					;\
							;\
 1:	mov	arg_ip = ip				;\
	mov	arg_num = num				;\
	;;						;\
	add	arg_ip = 2f-1b, arg_ip			;\
	br.sptk.many save_context			;\
							;\
2:	alloc	r14 = ar.pfs,0,0,1,0			;\
	mov	out0 = sp				;\
	add	sp = -16,sp ;;				;\
	br.call.sptk.many rp = func			;\
	add	sp = 16,sp				;\
	ENABLE_INTS()					;\
	br.sptk.many load_context			;\
	/* NOTREACHED */				;\
	rfi



	.section	.ivt, "ax"
	.p2align	15
	.global		ia64_vector_table
ia64_vector_table:


/*
**
**	Interrupt Vector Table
**
*/



	//
	// VHPT Translation
	//
BEG_IVT_ENTRY (0, 0x0000, vhpt_translation)
	EXCEPTION (0)
END_IVT_ENTRY (0, vhpt_translation)


	//
	// Intsruction TLB
	//
BEG_IVT_ENTRY (1, 0x0400, instruction_tlb)
	EXCEPTION (1)
END_IVT_ENTRY (1, instruction_tlb)


	//
	// Data TLB
	//
BEG_IVT_ENTRY (2, 0x0800, data_tlb)
	EXCEPTION (2)
END_IVT_ENTRY (2, data_tlb)


	//
	// Alternate Instruction TLB
	//
BEG_IVT_ENTRY (3, 0x0c00, alternate_instruction)
	SAVE_EXCEPTION_CONTEXT_CALL (3)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_vhpt_miss
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (3, alternate_instruction)


	//
	// Alternate Data TLB
	//
BEG_IVT_ENTRY (4, 0x1000, alternate_data)

	// Check if we faulted on page containing our own TCB
	mov	r16 = cr.ifa
	mov	r17 = r_KERNEL_SP
	mov	r18 = pr
	mov	r19 = cr.ipsr
	;;
	add	r17 = -1, r17
	;;
	dep	r16 = 0, r16, 0, KTCB_ALLOC_SIZE_SHIFT
	dep	r17 = 0, r17, 0, KTCB_ALLOC_SIZE_SHIFT
	tbit.nz	p6,p7 = r19,33		// Is user-level?
	;;
(p7)	cmp.eq	p7,p0 = r16,r17		// Is current TCB?
	;;
(p7)	br.spnt handle_tcb_fault
	mov	pr = r18, 0x1ffff
	;;
	SAVE_EXCEPTION_CONTEXT_CALL (4)

	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_vhpt_miss
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */

#define TCB_PAGE_TRANSLATION_BITS		\
	((1 << 0) |	/* Present */		\
	 (0 << 2) |	/* Write-back */	\
	 (1 << 5) |	/* Accessed */		\
	 (1 << 6) |	/* Dirty */		\
	 (0 << 7) |	/* Privilege level 0 */	\
	 (2 << 9))	/* Read-write */

handle_tcb_fault:
	mov	pr = r18, 0x1ffff
	dep.z	r16 = KTCB_ALLOC_SIZE_SHIFT, 2, 7
	mov	r17 = r_PHYS_TCB_ADDR
	movl	r18 = TCB_PAGE_TRANSLATION_BITS
	;;
	mov	cr.itir = r16
	or	r17 = r17, r18
	;;
	itc.d	r17
	rfi
END_IVT_ENTRY (4, alternate_data)


	//
	// Data Nested TLB
	//
BEG_IVT_ENTRY (5, 0x1400, data_nested_tlb)
	EXCEPTION (5)
END_IVT_ENTRY (5, data_nested_tlb)


	//
	// Instruction Key Miss
	//
BEG_IVT_ENTRY (6, 0x1800, instruction_key_miss)
	EXCEPTION (6)
END_IVT_ENTRY (6, instruction_key_miss)


	//
	// Data Key Miss
	//
BEG_IVT_ENTRY (7, 0x1c00, data_key_miss)
	EXCEPTION (7)
END_IVT_ENTRY (7, data_key_miss)


	//
	// Dirty-Bit
	//
BEG_IVT_ENTRY (8, 0x2000, dirty_bit)
	SAVE_EXCEPTION_CONTEXT_CALL (8)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_reference_bits
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (8, dirty_bit)


	//
	// Instruction Access-Bit
	//
BEG_IVT_ENTRY (9, 0x2400, instruction_access_bit)
	SAVE_EXCEPTION_CONTEXT_CALL (9)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_reference_bits
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (9, instruction_access_bit)


	//
	// Data Access-Bit
	//
BEG_IVT_ENTRY (10, 0x2800, data_access_bit)
	SAVE_EXCEPTION_CONTEXT_CALL (10)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_reference_bits
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (10, data_access_bit)


	//
	// Break Instruction
	//
BEG_IVT_ENTRY (11, 0x2c00, break_instruction)
	HANDLE_EXCEPTION_SHORT (11, handle_break)
END_IVT_ENTRY (11, break_instruction)


	//
	// External Interrupt
	//
BEG_IVT_ENTRY (12, 0x3000, external_interrupt)
//	SAVE_EXCEPTION_CONTEXT_CALL (12)

arg_ip		= r31
arg_ivr		= r30

	mov	arg_ivr = cr.ivr
1:	mov	arg_ip = ip
	;;
	add	arg_ip = 3f-1b, arg_ip
	br.sptk.many save_context_extint

2:	mov	ret0 = cr.ivr
	;;
3:	alloc	r14 = ar.pfs,0,2,2,0
	mov	out0 = ret0
	mov	out1 = sp

#if 0
	;;
	mov	loc0 = out0
	mov	loc1 = out1
	;;
	add	out0 = 80*4, loc0
	add	sp = -16, sp
	br.call.sptk.few rp = spin_wheel
	add	sp = 16, sp
	mov	out0 = loc0
	mov	out1 = loc1
#endif

	add	r15 = @gprel(interrupt_vector), gp
	;;
	srlz.d
	cmp.eq	p6,p0 = 15, out0	// Check for spurious interrupt
	shladd	r15 = out0, 3, r15
	;;
(p6)	br.sptk.many load_context
	mov	cr.eoi = r0
	ld8	r14 = [r15]
	;;
	srlz.d
	;;
	mov	b6 = r14
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = b6
	add	sp = 16, sp
	br.sptk.few 2b
	/* NOTREACHED */
END_IVT_ENTRY (12, external_interrupt)

	RESERVED (13, 0x3400)
	RESERVED (14, 0x3800)
	RESERVED (15, 0x3c00)
	RESERVED (16, 0x4000)
	RESERVED (17, 0x4400)
	RESERVED (18, 0x4800)
	RESERVED (19, 0x4c00)


	//
	// Page Not Present
	//
BEG_IVT_ENTRY (20, 0x5000, page_not_present)
	EXCEPTION (20)
END_IVT_ENTRY (20, page_not_present)


	//
	// Key Permission
	//
BEG_IVT_ENTRY (21, 0x5100, key_permission)
	EXCEPTION (21)
END_IVT_ENTRY (21, key_permission)


	//
	// Instruction Access Rights
	//
BEG_IVT_ENTRY (22, 0x5200, instuction_access_rights)
	SAVE_EXCEPTION_CONTEXT_CALL (22)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_access_rights_fault
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (22, instuction_access_rights)


	//
	// Data Acess Rights
	//
BEG_IVT_ENTRY (23, 0x5300, data_access_rights)
	SAVE_EXCEPTION_CONTEXT_CALL (23)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_access_rights_fault
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (23, data_access_rights)


	//
	// General Exception
	//
BEG_IVT_ENTRY (24, 0x5400, general_exception)
	EXCEPTION (24)
END_IVT_ENTRY (24, general_exception)


	//
	// Disabled FP-Register
	//
BEG_IVT_ENTRY (25, 0x5500, disabled_fp_register)
	SAVE_EXCEPTION_CONTEXT_CALL (25)
	alloc	r14 = ar.pfs,0,0,1,0
	mov	out0 = sp
	add	sp = -16, sp
	;;
	br.call.sptk.many rp = handle_disabled_fp
	add	sp = 16, sp
	ENABLE_INTS()
	br.sptk.many load_context
	/* NOTREACHED */
END_IVT_ENTRY (25, disabled_fp_register)


	//
	// NaT Consumption
	//
BEG_IVT_ENTRY (26, 0x5600, nat_consumption)
	EXCEPTION (26)
END_IVT_ENTRY (26, nat_consumption)


	//
	// Speculation
	//
BEG_IVT_ENTRY (27, 0x5700, speculation)
	EXCEPTION (27)
END_IVT_ENTRY (27, speculation)

	RESERVED (28, 0x5800)

	//
	// Debug
	//
BEG_IVT_ENTRY (29, 0x5900, debug)
	HANDLE_EXCEPTION_SHORT (29, handle_debug_event)
END_IVT_ENTRY (29, debug)


	//
	// Unaligned Reference
	//
BEG_IVT_ENTRY (30, 0x5a00, unaligned_reference)
	EXCEPTION (30)
END_IVT_ENTRY (30, unaligned_reference)


	//
	// Unsupported Data Reference
	//
BEG_IVT_ENTRY (31, 0x5b00, unsupported_data_reference)
	EXCEPTION (31)
END_IVT_ENTRY (31, unsupported_data_reference)


	//
	// Floating-point Fault
	//
BEG_IVT_ENTRY (32, 0x5c00, floating_point_fault)
	EXCEPTION (32)
END_IVT_ENTRY (32, floating_point_fault)


	//
	// Floating-point Trap
	//
BEG_IVT_ENTRY (33, 0x5d00, floating_point_trap)
	EXCEPTION (33)
END_IVT_ENTRY (33, floating_point_trap)


	//
	// Lower-Privilege Transfer Trap
	//
BEG_IVT_ENTRY (34, 0x5e00, lower_privilege_xfer_trap)
	EXCEPTION (34)
END_IVT_ENTRY (34, lower_privilege_xfer_trap)


	//
	// Taken Branch Trap
	//
BEG_IVT_ENTRY (35, 0x5f00, taken_branch)
	HANDLE_EXCEPTION_SHORT (35, handle_debug_event)
END_IVT_ENTRY (35, taken_branch)


	//
	// Single Step Trap
	//
BEG_IVT_ENTRY (36, 0x6000, single_step)
	HANDLE_EXCEPTION_SHORT (36, handle_debug_event)
END_IVT_ENTRY (36, single_step)

	RESERVED (37, 0x6100)
	RESERVED (38, 0x6200)
	RESERVED (39, 0x6300)
	RESERVED (40, 0x6400)
	RESERVED (41, 0x6500)
	RESERVED (42, 0x6600)
	RESERVED (43, 0x6700)
	RESERVED (44, 0x6800)


	//
	// IA-32 Exception
	//
BEG_IVT_ENTRY (45, 0x6900, ia32_exception)
	EXCEPTION (45)
END_IVT_ENTRY (45, ia32_exception)


	//
	// IA-32 Intercept
	//
BEG_IVT_ENTRY (46, 0x6a00, ia32_intercept)
	EXCEPTION (46)
END_IVT_ENTRY (46, ia32_intercept)


	//
	// IA-32 Interrupt
	//
BEG_IVT_ENTRY (47, 0x6b00, ia32_interrupt)
	EXCEPTION (47)
END_IVT_ENTRY (47, ia32_interrupt)

	RESERVED (48, 0x6c00)
	RESERVED (49, 0x6d00)
	RESERVED (50, 0x6e00)
	RESERVED (51, 0x6f00)
	RESERVED (52, 0x7000)
	RESERVED (53, 0x7100)
	RESERVED (54, 0x7200)
	RESERVED (55, 0x7300)
	RESERVED (56, 0x7400)
	RESERVED (57, 0x7500)
	RESERVED (58, 0x7600)
	RESERVED (59, 0x7700)
	RESERVED (60, 0x7800)
	RESERVED (61, 0x7900)
	RESERVED (62, 0x7a00)
	RESERVED (63, 0x7b00)
	RESERVED (64, 0x7c00)
	RESERVED (65, 0x7d00)
	RESERVED (66, 0x7e00)
	RESERVED (67, 0x7f00)
