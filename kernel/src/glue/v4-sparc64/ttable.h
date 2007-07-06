/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/ttable.h
 * Description:  Assembler macros for L4 v4, SPARC v9 Trap Table.
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
 * $Id: ttable.h,v 1.8 2004/07/01 04:02:17 philipd Exp $
 *                
 ********************************************************************/

#ifndef __GLUE_V4_SPARC64__TTABLE_H__
#define __GLUE_V4_SPARC64__TTABLE_H__

#include INC_ARCH(frame.h)
#include INC_ARCH(asm.h)

#include <asmsyms.h>

#ifdef __ASSEMBLER__
.register %g2,#ignore
.register %g3,#ignore
.register %g6,#scratch
.register %g7,kstack
#endif

/**
 *  UNIMPLEMENTED_O_TRAP: Unimplmented user trap handler.
 *  Trap to KDB. 
 *  Contraints: 8 instructions or less.
 */
#define UNIMPLEMENTED_O_TRAP()                                  \
1:	setx	unimplemented_trap, %o1, %o0; /* Inst 1 - 6. */ \
	call	trap_kdebug;                  /* Inst 7.     */ \
	 rd	%pc, %o1;                     /* Inst 8.     */

/**
 *  UNIMPLEMENTED_X_TRAP: Unimplmented kernel trap handler.
 *  Trap to KDB.
 *  Contraints: 8 instructions or less.
 */
#define UNIMPLEMENTED_X_TRAP()                                  \
1:	setx	unimplemented_trap, %o1, %o0; /* Inst 1 - 6. */ \
	call	trap_kdebug;                  /* Inst 7.     */ \
	 rd	%pc, %o1;                     /* Inst 8.     */

/**
 *  UNUSED_O_TRAP: Unused user (TL = 0) trap handler.
 *  Forward exception to the users exception handler via IPC.
 *  Contraints: 8 instructions or less.
 */
#warning IMPLEMENTME!
#define UNUSED_O_TRAP() UNIMPLEMENTED_O_TRAP()


/**
 *  UNUSED_X_TRAP: Unused kernel (TL > 0) trap handler.
 *  Shouldn't happen, trap to the debugger.
 *  Contraints: 8 instructions or less.
 */
#warning IMPLEMENTME!
#define UNUSED_X_TRAP()                                  \
1:	setx	unused_trap, %o1, %o0; /* Inst 1 - 6. */ \
	call	trap_kdebug;           /* Inst 7.     */ \
	 rd	%pc, %o1;              /* Inst 8.     */

/**
 * WINDOW_STATE_SAVE: Before entering the kernel, save the register window state
 * so that save / restore / flushw instructions will behave correctly and not
 * touch user space
 */
#define WINDOW_STATE_SAVE() \
	/* save the number of windows in the UTCB in %g1 */		\
	rdpr    %otherwin, %g1;						\
	/* set OTHERWIN to the number of windows currently in use */	\
	rdpr	%canrestore, %g5;					\
	wrpr	%g5, %otherwin;						\
	/* clear CANRESTORE */						\
	wrpr	%g0, %canrestore;					\
	/* set CANSAVE to the number of windows remaining */		\
	set	NWINDOWS-2, %g6;					\
	sub	%g6, %g5, %g6;						\
	wrpr	%g6, %cansave;						\
	/* no need to clean windows for the kernel */			\
	set	NWINDOWS-1, %g5;					\
	wrpr	%g5, %cleanwin

/**
 * WINDOW_STATE_RESTORE: On leaving the kernel, set the register window state
 * so any windows saved by the kernel will be correctly restored.
 */
#define WINDOW_STATE_RESTORE() \
	/* Find the number of user windows and move it to CANRESTORE */	\
	rdpr	%otherwin, %g5;						\
	wrpr	%g5, %canrestore;					\
	/* Set OTHERWIN to the number of UTCB windows */		\
	wrpr	%g1, %otherwin;						\
	/* Adjust CANSAVE */						\
	set	NWINDOWS-2, %g6;					\
	sub     %g6, %g1, %g6;						\
	sub     %g6, %g5, %g6;						\
	wrpr    %g6, %cansave;						\
	/* make sure that windows the kernel has used get cleaned */	\
	wrpr	%g0, %cleanwin

	
/**
 * DEFINE_OTRAP_GLUE: Define an assembler routine that saves state and jumps
 * to a given C function, to be called from inside a TL=0 (O) trap.
 */
#define OTRAP_C_ENTRY(label) otrap_##label##_glue
#define DEFINE_OTRAP_GLUE(func) \
BEGIN_PROC(OTRAP_C_ENTRY(func),".text")					\
	WINDOW_STATE_SAVE();						\
	/* Save the in and local regs in a register window and set up   \
	 * the kernel stack */						\
	mov	%sp, %g6;						\
	add	%g7, - TRAP_FRAME_SIZE - STACK_BIAS_64BIT, %sp;		\
	/* spill the out regs */					\
	stx	%o0, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O0 ];	\
	stx	%o1, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O1 ];	\
	stx	%o2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O2 ];	\
	stx	%o3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O3 ];	\
	stx	%o4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ];	\
	stx	%o5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ];	\
	stx	%g6, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ];	\
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ];	\
	/* switch to the main globals and save %g1 and %g5 */		\
	rdpr	%pstate, %o0;						\
	wrpr	%o0, PSTATE_AG, %pstate;				\
	stx	%g1, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ];	\
	stx	%g2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ];	\
	stx	%g3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ];	\
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ];	\
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ];	\
	/* call the kernel function, and then return to TL=0 */		\
	call	func;							\
	nop;								\
	call	otrap_retry;						\
	nop

/**
 * DEFINE_XTRAP_GLUE: Define an assembler routine that saves state and jumps
 * to a given C function, to be called from inside a TL>0 (X) trap.
 */
#define XTRAP_C_ENTRY(label) xtrap_##label##_glue
#define DEFINE_XTRAP_GLUE(func) \
BEGIN_PROC(XTRAP_C_ENTRY(func),".text")					\
	/* Check to see if this trap was caused by a TL=0 spill/fill    \
	 * handler. If so, pretend that the spill/fill never happened.*/\
	rdpr    %tl, %g5;						\
	cmp     %g5, 2;							\
	bne,a   %xcc, 1f;						\
	 nop;								\
	set     1, %g5;							\
	wrpr    %g5, %tl;						\
	rdpr    %tt, %g3;						\
	set     2, %g5;							\
	cmp     %g3, 0x80;						\
	bl,a    %xcc, 1f;						\
	 wrpr    %g5, %tl;						\
	cmp     %g3, 0xff;						\
	bg,a    %xcc, 1f;						\
	 wrpr    %g5, %tl;						\
	/* A window trap occurred. Restore the cwp at the time of the	\
	 * original fault, as the trap will have changed it. */		\
	rdpr	%tstate, %g5;						\
	and	%g5, 0x1f, %g5;						\
	wrpr	%g5, %cwp;						\
	/* Jump to the O-trap handler. */				\
	ba,pt   %xcc, OTRAP_C_ENTRY(func);				\
	 wrpr    %g0, %tt; /* prevent this test succeeding again */     \
1:      /* Now save the out registers to the trap frame. */		\
    	sub	%sp, TRAP_FRAME_SIZE, %g6;				\
	stx	%sp, [ %g6 + STACK_BIAS_64BIT + TRAP_FRAME_O6 ];	\
	mov	%g6, %sp;						\
	stx	%o0, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O0 ];	\
	stx	%o1, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O1 ];	\
	stx	%o2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O2 ];	\
	stx	%o3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O3 ];	\
	stx	%o4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ];	\
	stx	%o5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ];	\
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ];/* saved o7 */\
	/* switch to the main globals and save %g1 and %g5 */		\
	rdpr	%pstate, %o0;						\
	wrpr	%o0, PSTATE_AG, %pstate;				\
	stx	%g1, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ];	\
	stx	%g2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ];	\
	stx	%g3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ];	\
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ];	\
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ];	\
	/* call the kernel function, and then return from the trap */   \
	call	func;							\
	nop;								\
	call	xtrap_retry;						\
	nop

/**
 * OTRAP_C_CALL: jump to the given C function, via glue code which fixes
 * up the stack and saves registers assuming that the trap was taken from TL=0
 */
#define OTRAP_C_CALL(label) \
	mov		%o7, %g4; \
	call		OTRAP_C_ENTRY(label)

/**
 * XTRAP_C_CALL: jump to the given C function, via glue code which fixes
 * up the stack and saves registers assuming that the trap was taken from TL>0
 */
#define XTRAP_C_CALL(label) \
	mov		%o7, %g4; \
	call		XTRAP_C_ENTRY(label)

/**
 * TRAP_C_GLUE: given a function name, define assembler procedures to be used
 * by [OX]TRAP_C_ENTRY to call that function.
 */
#define TRAP_C_GLUE(func) \
	DEFINE_OTRAP_GLUE(func); \
	DEFINE_XTRAP_GLUE(func)

/**
 * EXCEPTION_TTABLE_ENTRY: Create a trap table entry that calls send_exception
 * to handle a user-level trap.
 */
#define EXCEPTION_TTABLE_ENTRY(label)							\
BEGIN_TTABLE_ENTRY(label,O)								\
	OTRAP_C_CALL(send_exception);							\
	nop

#endif /* !__GLUE_V4_SPARC64__TTABLE_H__ */
