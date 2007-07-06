/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/ultrasparc/mmu_trap.S
 * Description:  UltraSPARC MMU trap handlers.
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
 * $Id: mmu_trap.S,v 1.7 2004/05/24 03:06:10 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(ttable.h)
#include INC_ARCH(asi.h)
#include INC_CPU(asi.h)

#include <asmsyms.h>

/* glue procedures for C functions called in this file */
/* these functions are defined in glue/v4-sparc64/ultrasparc/mmu.c */
TRAP_C_GLUE(itsb_miss_handler)
TRAP_C_GLUE(dtsb_miss_handler)
TRAP_C_GLUE(data_fault_handler)

/* User-mode faults which cause an exception ipc */
EXCEPTION_TTABLE_ENTRY(instruction_access_exception)
EXCEPTION_TTABLE_ENTRY(data_access_exception)
EXCEPTION_TTABLE_ENTRY(instruction_access_error)
EXCEPTION_TTABLE_ENTRY(data_access_error)

/**
 * fast_instruction_access_MMU_miss,X: I-TLB miss in kernel mode
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_instruction_access_MMU_miss,O)
	/* check for 8kb page tsb hit */
	ldxa		[ %g0 ] ASI_IMMU_TSB_8KB_PTR, %g4
	ldxa		[ %g0 ] ASI_IMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0 /* test tag (global bit ignored) */
	bz,pt		%xcc, 2f
	or		%g3, 0x400, %g3 /* XXX */
1:	/* check for 64kb page tsb hit */
	ldxa		[ %g0 ] ASI_IMMU_TSB_64KB_PTR, %g4
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0
	bz,pt		%xcc, 2f
	or		%g3, 0x400, %g3 /* XXX */
	/* tsb miss, call the tsb miss handler */
1:	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	OTRAP_C_CALL(itsb_miss_handler)
	nop
2:	/* tsb hit. set execute access bit in the tsb and load the tlb */
	stx		%g3, [ %g4 + 8 ] 
	stxa		%g3, [ %g0 ] ASI_ITLB_DATA_IN
	retry

/**
 * fast_data_access_MMU_miss,O: D-TLB miss in user mode
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_data_access_MMU_miss,O)
	/* check for 8kb page tsb hit */
	ldxa		[ %g0 ] ASI_DMMU_TSB_8KB_PTR, %g4
	ldxa		[ %g0 ] ASI_DMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0 /* test tag (global bit ignored) */
	bz,pt		%xcc, 2f
	sethi		%hi(0x1000), %g6 /* XXX */ 
1:	/* check for 64kb page tsb hit */
	ldxa		[ %g0 ] ASI_DMMU_TSB_64KB_PTR, %g4
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0
	bz,pt		%xcc, 2f
	sethi		%hi(0x1000), %g6 /* XXX */ 
	/* tsb miss, call the tsb miss handler */
1:	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	OTRAP_C_CALL(dtsb_miss_handler)
	nop
2:	/* tsb hit. set read access bit in the tsb and load the tlb */
	or		%g3, %g6, %g3
	stx		%g3, [ %g4 + 8 ] 
	stxa		%g3, [ %g0 ] ASI_DTLB_DATA_IN
	retry

/**
 * fast_data_access_protection,O: protection violation during data access
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_data_access_protection,O)
	/* the MMU has given us a pointer to a TSB entry, but we must still
	 * check that it's the right entry - for superpage mappings it will
	 * point to the 8kb half of the TSB, while the mapping is actually in
	 * the 64kb half. Also, the TSB entry might have been replaced. */
	ldxa		[ %g0 ] ASI_DMMU_TSB_DIRECT_PTR, %g4
	ldxa		[ %g0 ] ASI_DMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	xorcc		%g1, %g2, %g0
	bnz,pn		%xcc, 1f /* not the right entry? */
	/* load the TSB data, and check the w access and valid bits */
	brgez,pn	%g3, 1f /* not a valid entry? */
	andcc		%g3, 0x100, %g0 /* mask access w bit */
	bz,pn		%xcc, 2f /* mapping not writable? */
	/* set the write-reference and tlb_writable bits, store back into TSB */
	or		%g3, 0x802, %g3 /* XXX */
	stx		%g3, [ %g4 + 8 ]
	/* now flush the entry from the TLB... */
	set		TLB_SFSR, %g5
	ldxa		[ %g5 ] ASI_DMMU, %g5
	set		TLB_SFAR, %g1
	ldxa		[ %g1 ] ASI_DMMU, %g1
	set		0x1fff, %g6
	andn		%g1, %g6, %g1 /* copy page number */
	and		%g5, 0x30, %g5 /* copy context register select bits */
	or		%g1, %g5, %g5 /* construct address for demap */
	stxa		%g0, [ %g5 ] ASI_DMMU_DEMAP
	/* ... and reload it */
	stxa		%g3, [ %g0 ] ASI_DTLB_DATA_IN
	retry
1:	/* call handler for superpage accesses and TSB misses */
	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	OTRAP_C_CALL(dtsb_miss_handler)
	nop
2:	/* call handler for protection faults */
	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	OTRAP_C_CALL(data_fault_handler)
	nop

/* USED kernel traps that need to be implemented. */
UNIMPLEMENTED_TTABLE_ENTRY(instruction_access_exception,X)
UNIMPLEMENTED_TTABLE_ENTRY(instruction_access_error,X)
UNIMPLEMENTED_TTABLE_ENTRY(data_access_exception,X)
UNIMPLEMENTED_TTABLE_ENTRY(data_access_error,X)

/**
 * fast_instruction_access_MMU_miss,X: I-TLB miss in kernel mode
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_instruction_access_MMU_miss,X)
	/* check for 8kb page tsb hit */
	ldxa		[ %g0 ] ASI_IMMU_TSB_8KB_PTR, %g4
	ldxa		[ %g0 ] ASI_IMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0 /* test tag (global bit ignored) */
	bz,pt		%xcc, 2f
	or		%g3, 0x400, %g3 /* XXX */
1:	/* check for 64kb page tsb hit */
	ldxa		[ %g0 ] ASI_IMMU_TSB_64KB_PTR, %g4
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0
	bz,pt		%xcc, 2f
	or		%g3, 0x400, %g3 /* XXX */
	/* tsb miss, call the tsb miss handler */
1:	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	XTRAP_C_CALL(itsb_miss_handler)
	nop
2:	/* tsb hit. set execute access bit in the tsb and load the tlb */
	stx		%g3, [ %g4 + 8 ] 
	stxa		%g3, [ %g0 ] ASI_ITLB_DATA_IN
	retry

/**
 * fast_data_access_MMU_miss,X: D-TLB miss in kernel mode
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_data_access_MMU_miss,X)
	/* check for 8kb page tsb hit */
	ldxa		[ %g0 ] ASI_DMMU_TSB_8KB_PTR, %g4
	ldxa		[ %g0 ] ASI_DMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0 /* test tag (global bit ignored) */
	bz,pt		%xcc, 2f
	sethi		%hi(0x1000), %g6 /* XXX */ 
1:	/* check for 64kb page tsb hit */
	ldxa		[ %g0 ] ASI_DMMU_TSB_64KB_PTR, %g4
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	brgez,pn	%g3, 1f
	xorcc		%g1, %g2, %g0
	bz,pt		%xcc, 2f
	sethi		%hi(0x1000), %g6 /* XXX */ 
	/* tsb miss, call the tsb miss handler */
1:	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	XTRAP_C_CALL(dtsb_miss_handler)
	nop
2:	/* tsb hit. set read access bit in the tsb and load the tlb */
	or		%g3, %g6, %g3
	stx		%g3, [ %g4 + 8 ] 
	stxa		%g3, [ %g0 ] ASI_DTLB_DATA_IN
	retry

/**
 * fast_data_access_protection,X: protection violation during data access
 * Constraints: 32 instructions or less
 */
BEGIN_TTABLE_ENTRY(fast_data_access_protection,X)
	/* the MMU has given us a pointer to a TSB entry, but we must still
	 * check that it's the right entry - for superpage mappings it will
	 * point to the 8kb half of the TSB, while the mapping is actually in
	 * the 64kb half. Also, the TSB entry might have been replaced. */
	ldxa		[ %g0 ] ASI_DMMU_TSB_DIRECT_PTR, %g4
	ldxa		[ %g0 ] ASI_DMMU, %g1
	ldda		[ %g4 ] ASI_NUCLEUS_QUAD_LDD, %g2
	xorcc		%g1, %g2, %g0
	bnz,pn		%xcc, 1f /* not the right entry? */
	/* load the TSB data, and check the w access and valid bits */
	brgez,pn	%g3, 1f /* not a valid entry? */
	andcc		%g3, 0x100, %g0 /* mask access w bit */
	bz,pn		%xcc, 2f /* mapping not writable? */
	/* set the write-reference and tlb_writable bits, store back into TSB */
	or		%g3, 0x802, %g3 /* XXX */
	stx		%g3, [ %g4 + 8 ]
	/* now flush the entry from the TLB... */
	set		TLB_SFSR, %g5
	ldxa		[ %g5 ] ASI_DMMU, %g5
	set		TLB_SFAR, %g1
	ldxa		[ %g1 ] ASI_DMMU, %g1
	set		0x1fff, %g6
	andn		%g1, %g6, %g1 /* copy page number */
	and		%g5, 0x30, %g5 /* copy context register select bits */
	or		%g1, %g5, %g5 /* construct address for demap */
	stxa		%g0, [ %g5 ] ASI_DMMU_DEMAP
	/* ... and reload it */
	stxa		%g3, [ %g0 ] ASI_DTLB_DATA_IN
	retry
1:	/* call handler for superpage accesses and TSB misses */
	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	XTRAP_C_CALL(dtsb_miss_handler)
	nop
2:	/* call handler for protection faults */
	rdpr		%pstate, %g5
	wrpr		%g5, (PSTATE_MG | PSTATE_AG), %pstate
	XTRAP_C_CALL(data_fault_handler)
	nop

#ifdef CONFIG_KDB
/* Kernel PA and VA watchpoints */
BEGIN_TTABLE_ENTRY(VA_watchpoint,X)
	/* Disable the watchpoint */
	setx		0x600000, %g3, %g4
	ldxa		[ %g0 ] ASI_LSU_CONTROL_REG, %g3
	andn		%g3, %g4, %g3
	stxa		%g3, [ %g0 ] ASI_LSU_CONTROL_REG
	/* Jump to the debugger */
	call		_Z12kdebug_entryPv
	 nop

BEGIN_TTABLE_ENTRY(PA_watchpoint,X)
	/* Disable the watchpoint */
	setx		0x1800000, %g3, %g4
	ldxa		[ %g0 ] ASI_LSU_CONTROL_REG, %g3
	andn		%g3, %g4, %g3
	stxa		%g3, [ %g0 ] ASI_LSU_CONTROL_REG
	/* Jump to the debugger */
	call		_Z12kdebug_entryPv
	 nop
#endif

/**
 *  The following MMU traps are not supported by UltraSPARC CPUs:
 *
 *  - instruction_access_MMU_miss, replaced by fast_instruction_access_MMU_miss.
 *  - data_access_MMU_miss, replaced by fast_data_access_MMU_miss.
 *  - data_access_protection, replaced by fast_data_access_protection.
 */
