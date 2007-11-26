/*********************************************************************
 *
 * Copyright (C) 2002-2006,  Karlsruhe University
 *
 * File path:     glue/v4-amd64/trap.S 
 * Description:   trap handlers
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
 * $Id: trap.S,v 1.13 2006/10/21 13:05:28 reichelt Exp $
 *
 *********************************************************************/
/* Stack Layout:
 * (undef)
 * usp
 * uflags
 * (undef)
 * uip
 * 
 */
#include INC_ARCH(asm.h)
#include INC_ARCH(x86.h)
#include <tcb_layout.h>
#include <asmsyms.h>
	
.macro TID_TO_TCB reg, tmp
	shr	$VALID_THREADNO_SHIFT, \reg
	andq	$(VALID_THREADNO_MASK << KTCB_BITS), \reg
	movq 	$KTCB_AREA_START, \tmp
	addq	\tmp, \reg
.endm

.macro CURRENT_TCB reg
	movq	%rsp, \reg
	andq	$(KTCB_MASK), \reg
.endm

#define UTCB_MR(x)	(OFS_UTCB_MR + ((x) * 8))

/* JS:	TODO
 * 1) Actually we'd have to dispatch, perhaps by xlat or jump table
 * 2) As soon as we have a fastpath, we cannot use callee saved registers 
 *    for UTCB, UIP, USP, UFLAGS anymore
 * 3) As soon as we have a working tcb_layout.h, we can calculate
 *    the address of user_ipc at compile time and avoid lea
 */
BEGIN_PROC(syscall_entry)
	movq %rsp, %rbp			// save USP 
	leaq tss(%rip), %rsp		// load TSS 
	movq 4(%rsp), %rsp		// load KSP (tss->rsp0)
	subq $40, %rsp			// load KSP (tss->rsp0)
	movq %rbp, 24(%rsp)		// save USP 
	movq %r11, 16(%rsp)		// save UFLAGS
	movq $0,    8(%rsp)		// set CS to SYSCALL marker (0)
	leaq user_ipc_entry(%rip), %rbp	// load user_ipc
	cmpq %rcx, %rbp			// coming from user_ipc ?
	jne   no_ipc			// no -> catch it 
	/* IPC Stub */
	movq %rdi,   (%rsp)		// save UIP (jump back to user)

#if defined(CONFIG_IPC_FASTPATH)
	/* msgtag only untyped items and no flags */
	movq	%r9, %rcx
	andl	$(0x3ff << 6), %ecx

	/* no receive timeout jsXXX: use setne */
	movq	%r8, %rbp
	or	%bp, %cx
	jne	slowpath

	/* has send phase? (catch no receive later) */
	test	%rsi, %rsi
	je	slowpath

	
	/* calculate TCB pointers of current and dest */
	mov	%rsi,   %rbp
	TID_TO_TCB	%rbp, %rcx

	/* check thread id of destination
	 * here we could also bail out!!! */
	cmpq	%rsi, OFS_TCB_MYSELF_GLOBAL(%rbp)
	jne	slowpath

	/* check if destination space is not 0 */
	movq	OFS_TCB_SPACE(%rbp), %rcx
	test	%rcx, %rcx
	jz	slowpath

	/* check if destination is not in wakeup/late wakeup queue */
	movq	OFS_TCB_QUEUE_STATE(%rbp), %rcx
	test	$(QSTATE_WAKEUP | QSTATE_LATE_WAKEUP), %rcx
	jne	slowpath
	
	CURRENT_TCB	%rcx


	
	/* check for resources of current and destination
	 * also check if dest is doing a fastpath-ipc  */
	movq    OFS_TCB_RESOURCE_BITS(%rcx), %r11 
	orq	OFS_TCB_RESOURCE_BITS(%rbp), %r11
	orq	(KTCB_SIZE + KSTACK_CS * 8)(%rbp), %r11
	jne	slowpath
	
	/* get myself */
	movq	OFS_TCB_MYSELF_GLOBAL(%rcx), %r11

	/* check that dest thread is waiting for current or any */
	cmpq	$TSTATE_WAITING_FOREVER, OFS_TCB_THREAD_STATE(%rbp)
	jne	slowpath
	/* jsXXX: try to use a register for partner */
	cmpq	OFS_TCB_PARTNER(%rbp), %r11
	je	1f
	cmpq	$-1, OFS_TCB_PARTNER(%rbp)
	jnc	slowpath


1:	
#ifdef CONFIG_SMP
	/* check that both threads are on the same CPU */	
	mov	OFS_TCB_CPU(%rbp), %r11 /* %ebp, %ecx */
	cmp	%r11, OFS_TCB_CPU(%rcx) /* %ebx */
	jne	slowpath
#endif
	/* make sure that current really blocks
	 * (FromSpec == To) || (SendHead == NULL) */
	cmpq	%rsi, %rdx
	je	2f

	/* FromSpec == any && current->sendhead == 0 */
	cmpq	$-1, %rdx
	jne	slowpath
	cmpl	$0, OFS_TCB_SEND_HEAD(%rbp)
	jne	slowpath

2:	
	/**************************************************
	 * if we reach this point the transfer can 
	 * take place
	 **************************************************/

	/* set current thread state to waiting */
	movq	%rdx, OFS_TCB_PARTNER(%rcx)
	movq	$TSTATE_WAITING_FOREVER, OFS_TCB_THREAD_STATE(%rcx)

	/* set destination thread state to running */
	movq	$TSTATE_RUNNING, OFS_TCB_THREAD_STATE(%rbp)
	
	/* 
	 * Registers:
	 *    rcx = current TCB
	 *    rdx = from specifier (free)
	 *    rsi = to specifier (free)
	 *    rdi = UTCB (free)
	 *    rbp = destination TCB
	 *    r8  = timeout (free)
	 *    r11 = current->myself_global
	 */

	/* set partner field */
	/* movq	%r11, OFS_TCB_PARTNER(%rbp) */

	/* calculate number of untyped words */ 
	movq    %r9, %rdx
	testb	$~0x7, %dl
	jnz	fp_copy_loop
fp_copy_loop_done:

	
5:	movq	%rbp, %rsi
	
	pushq	$fp_slow_ipc_done
	movq	OFS_TCB_MYSELF_LOCAL(%rsi), %rdi	// local id
	movq	OFS_TCB_PDIR_CACHE(%rsi), %rbp		// new PDIR
	/* jsXXX: set gs via virtual address, without selector */ 
	movq	%rdi, %gs:0
	movq	%rsp, OFS_TCB_STACK(%rcx)		// store stack ptr
	// set tss.esp0
	lea	KTCB_SIZE(%rsi), %rsp			// load stack ptr
	movq	%rsp, (tss + 4)				// save stack ptr to tss.esp0

	cmpq	%rbp, OFS_TCB_PDIR_CACHE(%rcx)
	je	6f					// same ptab
	movq	%rbp, %cr3				// switch ptab
6:	
	movq	%r11, %rsi				// from specifier

	
	movq (KSTACK_UIP * 8)(%rsp), %rcx		// restore UIP
	movq (KSTACK_UFLAGS * 8)(%rsp), %r11		// restore UFLAGS
	movq (KSTACK_USP * 8)(%rsp), %rsp		// restore USP
	sysretq

fp_copy_loop:
 	movq	OFS_TCB_UTCB(%rbp), %rdi		// dst UTCB
	movq	OFS_TCB_UTCB(%rcx), %rsi		// source UTCB
	addq	$(OFS_UTCB_MR+64),  %rdi		// start at MR8
	addq	$(OFS_UTCB_MR+64),  %rsi		// start at MR8
	/* jsXXX: change rcx and rdx immediately, at the top of the path */
	xchgq	%rcx, %rdx
	andl	$0x3f, %ecx
	subl	$7, %ecx
	cld
	rep	movsq	(%rsi), (%rdi)
	movq	%rdx, %rcx
	jmp	fp_copy_loop_done


fp_slow_ipc_done:
	/* 
	 * see tcb.h: 
	 *	r12 = dtcb
	 */ 
	
	movq	OFS_TCB_UTCB(%r12), %rcx		// dst UTCB
	/* srXXX: local id is stored in rbx as well. */
	movq    OFS_TCB_MYSELF_LOCAL(%r12), %rdi	// restore local id from TCB
	movq	OFS_TCB_PARTNER(%r12), %rsi		// from specifier
	movq	UTCB_MR(0)(%rcx), %r9			// restore MR0
	jmp	fp_slow_ipc_ret	
#endif

slowpath:
	CURRENT_TCB	%rdi
	movq	OFS_TCB_UTCB(%rdi), %rdi		// load UTCB from TCB
	movq	%r9,   UTCB_MR(0)(%rdi)			// store MR into UTCB
	movq	%rax,  UTCB_MR(1)(%rdi)
	movq	%rbx,  UTCB_MR(2)(%rdi)
	movq	%r10,  UTCB_MR(3)(%rdi)
	movq	%r12,  UTCB_MR(4)(%rdi)
	movq	%r13,  UTCB_MR(5)(%rdi)
	movq	%r14,  UTCB_MR(6)(%rdi)	
	movq	%r15,  UTCB_MR(7)(%rdi)	
	subq $16, %rsp					// make room for from and to
	movq %rdx,  8(%rsp)				// save from
	movq %rsi,   (%rsp)				// save to
	call sys_ipc					
	addq $16, %rsp					// remove from, to
							
	movq %rax, %rsi					// move from 
	movq %rdx, %r9					// move mr0   
	CURRENT_TCB %rdi				
	movq OFS_TCB_UTCB(%rdi), %rcx			// restore UTCB from TCB
	movq OFS_TCB_MYSELF_LOCAL(%rdi), %rdi		// restore local id from TCB
fp_slow_ipc_ret:
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
	CURRENT_TCB %rax				// get current TCB
	movq OFS_TCB_PARTNER(%rax), %rax		// get partner
	TID_TO_TCB %rax, %rbx				// get partner TCB
	testl $4, OFS_TCB_RESOURCE_BITS(%rax)		// test Compatibility Mode resource
	jz cm_done
	movl $0, OFS_UTCB_WORD_SIZE_MASK+4(%rcx)	// clear upper word
cm_done:
#endif
	movq UTCB_MR(1)(%rcx), %rax 		
	movq UTCB_MR(2)(%rcx), %rbx 		
	movq UTCB_MR(3)(%rcx), %r10 		
	movq UTCB_MR(4)(%rcx), %r12 		
	movq UTCB_MR(5)(%rcx), %r13 		
	movq UTCB_MR(6)(%rcx), %r14 		
    	movq UTCB_MR(7)(%rcx), %r15	 		// restore MRs from UTCB 
	movq   (%rsp), %rcx				// restore UIP
	movq 16(%rsp), %r11				// restore UFLAGS
	movq 24(%rsp), %rsp				// restore USP
	sysretq
	/* Non IPC Stub */
no_ipc:
	movq %rcx,   (%rsp)		// save UIP (after syscall)
	leaq user_nop_entry(%rip), %rbp	// load user_nop_entry
 	cmpq %rcx, %rbp			// coming from user_nop ?
	jne  no_nop			
	movq 24(%rsp),%rsp		// restore USP
	sysretq				// bye
no_nop:	
	pushq %r11			// push UFLAGS
	pushq %r10			// push arg7
	pushq %rax			// push arg6
	call syscall_dispatcher
	addq $24, %rsp			// remove arg6, arg7, UFLAGS
	popq %rcx			// restore UIP
	addq $8, %rsp			// pop (undef)
	popq %r11			// restore UFLAGS
	popq %rsp			// restore USP
	sysretq
END_PROC(syscall_entry)

.end
