/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/trap.S
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
 * $Id: trap.S,v 1.8 2004/07/01 04:03:09 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(ttable.h)

DECLAR_STRING(unused_trap, ".rodata", "Unused trap vector!\n")
DECLAR_STRING(unimplemented_trap, ".rodata", "Unimplemented trap vector!\n")

/* USED user traps that need to be implemented. */
UNIMPLEMENTED_TTABLE_ENTRY(fp_disabled,O)         /* Think about this more.   */
UNIMPLEMENTED_TTABLE_ENTRY(internal_processor_error,O)/* Look up details on this. */
UNIMPLEMENTED_TTABLE_ENTRY(async_data_error,O)    /* Look up details on this. */

/*****************************
* Used Kernel (TL > 0) traps *
*****************************/


/**
 *  illegal_instruction,X: Used to trap to KDB from kernel.
 *  Contraints: 8 instructions or less.
 */
BEGIN_TTABLE_ENTRY(illegal_instruction,X)
	call	_Z12kdebug_entryPv	! kdebug_entry()	/* Inst 1. */
	 nop							/* Inst 2. */
	done							/* Inst 3. */
	
/* END_TTABLE_ENTRY(illegal_instruction,X) */

/* USED Kernel traps that need to be implemented. */
UNIMPLEMENTED_TTABLE_ENTRY(internal_processor_error,X)/* Look up details on this. */
UNIMPLEMENTED_TTABLE_ENTRY(async_data_error,X)    /* Look up details on this. */
	
/* Since the kernel doesn't use trap inst (trap_instruction,X) isn't used   */

/***********************
* User (TL = 0) traps. *
***********************/	

TRAP_C_GLUE(send_exception)

/* These are all turned into exception IPCs */
EXCEPTION_TTABLE_ENTRY(illegal_instruction)
EXCEPTION_TTABLE_ENTRY(privileged_opcode)
EXCEPTION_TTABLE_ENTRY(unimplemented_LDD)
EXCEPTION_TTABLE_ENTRY(unimplemented_STD)
EXCEPTION_TTABLE_ENTRY(fp_exception_ieee_754)
EXCEPTION_TTABLE_ENTRY(fp_exception_other)
EXCEPTION_TTABLE_ENTRY(tag_overflow)
EXCEPTION_TTABLE_ENTRY(division_by_zero)
EXCEPTION_TTABLE_ENTRY(mem_address_non_aligned)
EXCEPTION_TTABLE_ENTRY(LDDF_mem_address_not_aligned)
EXCEPTION_TTABLE_ENTRY(STDF_mem_address_not_aligned)
EXCEPTION_TTABLE_ENTRY(privileged_action)
EXCEPTION_TTABLE_ENTRY(LDQF_mem_address_not_aligned)
EXCEPTION_TTABLE_ENTRY(STQF_mem_address_not_aligned)

/* These shouldn't get to the kernel or shouldn't happen */	
UNUSED_TTABLE_ENTRY(power_on_reset,O)
UNUSED_TTABLE_ENTRY(watchdog_reset,O)
UNUSED_TTABLE_ENTRY(externally_initiated_reset,O)
UNUSED_TTABLE_ENTRY(software_initiated_reset,O)
UNUSED_TTABLE_ENTRY(RED_state_exception,O)

/*******************************
* Unused Kernel (TL > 0) traps *
*******************************/

/* These shouldn't get to the kernel or shouldn't happen */

UNUSED_TTABLE_ENTRY(interrupt_level_n,X)
UNUSED_TTABLE_ENTRY(power_on_reset,X)
UNUSED_TTABLE_ENTRY(watchdog_reset,X)
UNUSED_TTABLE_ENTRY(externally_initiated_reset,X)
UNUSED_TTABLE_ENTRY(software_initiated_reset,X)
UNUSED_TTABLE_ENTRY(RED_state_exception,X)
UNUSED_TTABLE_ENTRY(privileged_opcode,X)
UNUSED_TTABLE_ENTRY(unimplemented_LDD,X)
UNUSED_TTABLE_ENTRY(unimplemented_STD,X)
UNUSED_TTABLE_ENTRY(fp_disabled,X)                /* Not sure about this one */
UNUSED_TTABLE_ENTRY(fp_exception_ieee_754,X)
UNUSED_TTABLE_ENTRY(fp_exception_other,X)
UNUSED_TTABLE_ENTRY(tag_overflow,X)
UNUSED_TTABLE_ENTRY(division_by_zero,X)
UNUSED_TTABLE_ENTRY(mem_address_non_aligned,X)
UNUSED_TTABLE_ENTRY(LDDF_mem_address_not_aligned,X)
UNUSED_TTABLE_ENTRY(STDF_mem_address_not_aligned,X)
UNUSED_TTABLE_ENTRY(privileged_action,X)
UNUSED_TTABLE_ENTRY(LDQF_mem_address_not_aligned,X)
UNUSED_TTABLE_ENTRY(STQF_mem_address_not_aligned,X)


/*******************************
* Return-from-trap routines    *
*******************************/

/**
 * xtrap_retry: Restore state and retry, for TL>1 traps
 */
BEGIN_PROC(xtrap_retry,".text")
	/* Restore the out & global registers from the trap frame, %sp last */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O0 ], %o0
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O1 ], %o1
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O2 ], %o2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O3 ], %o3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ], %o4
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ], %o5
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ], %g1
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ], %g2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ], %g3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ], %g4
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ], %g5
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	/* Return to the previous trap */
	retry

/**
 * otrap_retry: Restore state and retry, for TL=1 traps
 */
BEGIN_PROC(otrap_retry,".text")
	/* Restore the trap frame. */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O0 ], %o0
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O1 ], %o1
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O2 ], %o2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O3 ], %o3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ], %o4
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ], %o5
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ], %g1
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ], %g2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ], %g3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ], %g4
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ], %g5
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	/* Switch to alternate globals. */
	rdpr    %pstate, %g6
	wrpr    %g6, PSTATE_AG, %pstate
	/* Restore the register window state. */
	WINDOW_STATE_RESTORE();
	/* Return to user mode */
	retry
