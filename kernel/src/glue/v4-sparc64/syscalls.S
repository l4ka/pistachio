/*********************************************************************
 *                
 * Copyright (C) 2004, University of New South Wales
 *                
 * File path:    glue/v4-sparc64/syscall.S
 * Description:  Kernel entry points for system calls
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
 * $Id: syscalls.S,v 1.5 2004/07/01 00:49:51 philipd Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(asi.h)
#include INC_ARCH(ttable.h)
#include INC_GLUE(syscalls.h)

#include <asmsyms.h>

/**
 * L4_SYSCALL_TTABLE_ENTRY: trap table entry for all L4 syscalls and KDB traps,
 * except for kernel_interface, ipc, lipc and exchange_registers.
 */
#define L4_SYSCALL_TTABLE_ENTRY(label)					\
BEGIN_TTABLE_ENTRY(trap_##label,O)					\
	ba,pt	%xcc, _enter_##label;					\
	 nop;								\
BEGIN_PROC(_enter_##label,".text")					\
	WINDOW_STATE_SAVE();						\
	/* Save registers and set up the kernel stack */		\
	mov	%sp, %g5;						\
	sub	%g7, TRAP_FRAME_SIZE + STACK_BIAS_64BIT, %sp;		\
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ];	\
	stx	%o7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ];	\
	/* Switch to main globals */					\
	rdpr	%pstate, %g5;						\
	wrpr	%g5, PSTATE_AG, %pstate;				\
	/* Save globals */						\
	stx	%g2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ];	\
	stx	%g3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ];	\
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ];	\
	/* Jump to the syscall handler */				\
	setx	normal_syscall_done - 8, %g5, %o7;			\
	ba,pt	%xcc, label;						\
	 nop

L4_SYSCALL_TTABLE_ENTRY(sys_thread_control);
L4_SYSCALL_TTABLE_ENTRY(sys_thread_switch);
L4_SYSCALL_TTABLE_ENTRY(sys_schedule);
L4_SYSCALL_TTABLE_ENTRY(sys_unmap);
L4_SYSCALL_TTABLE_ENTRY(sys_space_control);
L4_SYSCALL_TTABLE_ENTRY(sys_processor_control);

UNIMPLEMENTED_TTABLE_ENTRY(trap_sys_system_clock,O);
UNIMPLEMENTED_TTABLE_ENTRY(trap_sys_memory_control,O);
UNIMPLEMENTED_TTABLE_ENTRY(trap_sys_lipc,O)

/* 
 * ExchangeRegisters needs special treatment, because it has two more arguments
 * than can be passed in o registers. These are passed in two volatile g
 * registers, and have to be moved to the right place on the stack once we
 * get into kernel mode.
 */
BEGIN_TTABLE_ENTRY(trap_sys_exchange_registers,O)
	ba,pt	%xcc, _enter_sys_exchange_registers
	 nop

BEGIN_PROC(_enter_sys_exchange_registers,".text")
	WINDOW_STATE_SAVE()
	/* Save registers and set up the kernel stack */
	mov	%sp, %g5
	sub	%g7, TRAP_FRAME_SIZE + STACK_BIAS_64BIT, %sp
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ]
	stx	%o7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ]
	/* Switch to main globals */
	rdpr	%pstate, %g5
	wrpr	%g5, PSTATE_AG, %pstate
	/* Save globals */
	stx	%g2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ]
	stx	%g3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ]
	/* Put is_local and flags in the extra-argument area on the stack and
	 * call sys_exchange_registers */
	stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_ARG6 ] /* flags */
	call	sys_exchange_registers
	 stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_ARG7 ] /* is_local */
	/* Load globals */
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ], %g3
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ], %g4
	/* Switch to alternate globals. */
	rdpr    %pstate, %o7
	wrpr    %o7, PSTATE_AG, %pstate
	/* Restore registers. */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	WINDOW_STATE_RESTORE()
	/* Return to user mode */
	done
	

BEGIN_TTABLE_ENTRY(trap_sys_ipc,O)
	ba,pt	%xcc, sys_ipc_slowpath
	 nop

BEGIN_PROC(sys_ipc_slowpath,".text")
	WINDOW_STATE_SAVE();
	/* Find the kernel-space address of the UTCB and save MRs. %g2 currently
	 * contains a pointer to an empty register window save location in the
	 * UTCB. */
	setx	UTCB_MASK, %g5, %g4
	and	%g2, %g4, %g4
	stx	%l0, [ %g4 + UTCB_MR_BASE + 0  ]
	stx	%l1, [ %g4 + UTCB_MR_BASE + 8  ]
	stx	%l2, [ %g4 + UTCB_MR_BASE + 16 ]
	stx	%l3, [ %g4 + UTCB_MR_BASE + 24 ]
	stx	%l4, [ %g4 + UTCB_MR_BASE + 32 ]
	stx	%l5, [ %g4 + UTCB_MR_BASE + 40 ]
	stx	%l6, [ %g4 + UTCB_MR_BASE + 48 ]
	stx	%l7, [ %g4 + UTCB_MR_BASE + 56 ]

	/* Save registers and set up the kernel stack */
	mov	%sp, %g5
	sub	%g7, TRAP_FRAME_SIZE + STACK_BIAS_64BIT, %sp
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ]
	stx	%o7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ]
	stx	%i6, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_I6 ]
	stx	%i7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_I7 ]

	/* Switch to main globals */
	rdpr	%pstate, %g5
	wrpr	%g5, PSTATE_AG, %pstate

	/* Save main globals & call sys_ipc */
	stx	%g2, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ]
	stx	%g3, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ]
	call	sys_ipc
	 stx	%g4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ]

.globl return_from_ipc
return_from_ipc:
	/* Restore main globals */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ], %g2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ], %g3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ], %g4

	/* Switch back to alternate globals. */
	rdpr    %pstate, %g5
	wrpr    %g5, PSTATE_AG, %pstate

	/* Recalculate the address of the UTCB (because sys_ipc may have
	 * switched threads and trashed %g4) and load the new MRs */
	setx	UTCB_MASK, %g5, %g4
	and	%g2, %g4, %g4
	ldx	[ %g4 + UTCB_MR_BASE + 0  ], %l0
	ldx	[ %g4 + UTCB_MR_BASE + 8  ], %l1
	ldx	[ %g4 + UTCB_MR_BASE + 16 ], %l2
	ldx	[ %g4 + UTCB_MR_BASE + 24 ], %l3
	ldx	[ %g4 + UTCB_MR_BASE + 32 ], %l4
	ldx	[ %g4 + UTCB_MR_BASE + 40 ], %l5
	ldx	[ %g4 + UTCB_MR_BASE + 48 ], %l6
	ldx	[ %g4 + UTCB_MR_BASE + 56 ], %l7
	/* Restore the trap frame. */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_I6 ], %i6
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_I7 ], %i7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	WINDOW_STATE_RESTORE()
	/* Return to user mode */
	done

/**
 * normal_syscall_done: restore state and return from a normal syscall.
 */
normal_syscall_done:
	/* Restore registers. */
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G2 ], %g2
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G3 ], %g3
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G4 ], %g4
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx     [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	/* Switch to alternate globals and restore window state */
	rdpr    %pstate, %g6
	wrpr    %g6, PSTATE_AG, %pstate
	WINDOW_STATE_RESTORE()
	/* Return to user mode */
	done
 
/*
 * KernelInterface must preserve all registers other than the outputs. However,
 * it cannot switch threads or pagefault, so there's no need to mess with
 * register windows.
 */
BEGIN_TTABLE_ENTRY(trap_sys_kernel_interface,O)
	ba,pt	%xcc, _enter_sys_kernel_interface
	 nop
BEGIN_PROC(_enter_sys_kernel_interface,".text")
	/* Save registers and set up the kernel stack */
	mov	%sp, %g5
	sub	%g7, TRAP_FRAME_SIZE + STACK_BIAS_64BIT, %sp
	/* Save preserved out registers. */
	stx	%o4, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ]
	stx	%o5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ]
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ]
	stx	%o7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ]
	/* Switch to main globals and save preserved globals */
	rdpr	%pstate, %g5
	wrpr	%g5, PSTATE_AG, %pstate
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ]
	call	sys_kernel_interface
	 stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ]
	
	/* Restore registers and return */
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O4 ], %o4
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O5 ], %o5
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ], %o7
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G1 ], %g1
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_G5 ], %g5
	ldx	[ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ], %sp
	done


/**
 * SYSCALL_TRAP_TTABLE_ENTRY: send a syscall trap IPC to the thread's exception
 * handler, with the trap number in g1. Software traps that are not used by L4
 * have trap table entries defined by this macro.
 */
#define SYSCALL_TRAP_TTABLE_ENTRY(trap_number)\
BEGIN_TTABLE_ENTRY(trap_##trap_number,O)				\
	b	_enter_syscall_ipc;					\
	 set	trap_number, %g1

BEGIN_PROC(_enter_syscall_ipc,".text")
	WINDOW_STATE_SAVE()
	/* Save registers and set up the kernel stack */
	mov	%sp, %g5
	sub	%g7, TRAP_FRAME_SIZE + STACK_BIAS_64BIT, %sp
	stx	%g5, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O6 ]
	stx	%o7, [ %sp + STACK_BIAS_64BIT + TRAP_FRAME_O7 ]
	/* Switch to main globals */
	rdpr	%pstate, %g5
	wrpr	%g5, PSTATE_AG, %pstate
	setx	normal_syscall_done - 8, %g5, %o7
	ba,pt	%xcc, send_syscall_ipc
	 nop

#define SYSCALL_TRAP_16_TTABLE_ENTRIES(x) \
	SYSCALL_TRAP_TTABLE_ENTRY(x##0); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##1); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##2); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##3); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##4); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##5); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##6); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##7); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##8); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##9); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##a); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##b); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##c); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##d); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##e); \
	SYSCALL_TRAP_TTABLE_ENTRY(x##f)

SYSCALL_TRAP_16_TTABLE_ENTRIES(0x0)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x1)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x2)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x3)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x4)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x5)
SYSCALL_TRAP_16_TTABLE_ENTRIES(0x6)

#if CONFIG_KDB
L4_SYSCALL_TTABLE_ENTRY(sys_enter_kdebug);
L4_SYSCALL_TTABLE_ENTRY(sys_kdebug_putc);
L4_SYSCALL_TTABLE_ENTRY(sys_kdebug_getc);
L4_SYSCALL_TTABLE_ENTRY(sys_kdebug_getc_nonblock);
#else /* !CONFIG_KDB */
SYSCALL_TRAP_TTABLE_ENTRY(0x7c);
SYSCALL_TRAP_TTABLE_ENTRY(0x7d);
SYSCALL_TRAP_TTABLE_ENTRY(0x7e);
SYSCALL_TRAP_TTABLE_ENTRY(0x7f);
#endif /* CONFIG_KDB */

#if CONFIG_KDB
/**
 * sys_enter_kdebug: print the user-space string that %o0 is pointing to, then
 * enter kdb.
 */
#define ENTER_KDB_STRING_MAX	256
BEGIN_PROC(sys_enter_kdebug,".text")
	/* Copy string from user-space */
	setx	2f, %g5, %l0
	mov	%l0, %o1
	set	ENTER_KDB_STRING_MAX, %l1
3:
	lduba	[ %o0 ] ASI_AIUP, %l2
	subcc	%l1, 1, %l1
	bz,pn	%xcc, 4f
	 add	%o0, 1, %o0
	brz,pn	%l2, 4f
	 stb	%l2, [ %l0 ]
	b	3b
	 add	%l0, 1, %l0
4:
	stb	%g0, [ %l0 ]
	setx	1f, %g5, %o0
	call	printf
	 nop
	call	_Z12kdebug_entryPv
	 nop
	ba,pt	%xcc, normal_syscall_done
	 nop
.data
1:	.string "%s\n"
2:	.space	ENTER_KDB_STRING_MAX
.previous

BEGIN_PROC(sys_kdebug_putc,".text")
	ba,pt	%xcc, _Z4putcc
	 nop

BEGIN_PROC(sys_kdebug_getc,".text")
	ba,pt	%xcc, _Z4getcb
	 set	1, %o0

BEGIN_PROC(sys_kdebug_getc_nonblock,".text")
	ba,pt	%xcc, _Z4getcb
	 set	0, %o0

#endif /* CONFIG_KDB */
