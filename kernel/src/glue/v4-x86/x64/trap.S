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

.macro SAVE_UTCB1 reg
	movq	%rax,  UTCB_MR(1)(\reg)
	movq	%rbx,  UTCB_MR(2)(\reg)
	movq	%r10,  UTCB_MR(3)(\reg)
	movq	%r12,  UTCB_MR(4)(\reg)
	movq	%r13,  UTCB_MR(5)(\reg)
	movq	%r14,  UTCB_MR(6)(\reg)	
	movq	%r15,  UTCB_MR(7)(\reg)	
.endm

.macro RESTORE_UTCB1 reg
	movq UTCB_MR(1)(\reg), %rax 		
	movq UTCB_MR(2)(\reg), %rbx 		
	movq UTCB_MR(3)(\reg), %r10 		
	movq UTCB_MR(4)(\reg), %r12 		
	movq UTCB_MR(5)(\reg), %r13 		
	movq UTCB_MR(6)(\reg), %r14 		
    	movq UTCB_MR(7)(\reg), %r15	
.endm

/* jsXXX:
 * - dispatch by xlat or jump table
 * - calculate the address of user_ipc at compile time and avoid lea
 */
BEGIN_PROC(syscall_entry)
	movq %rsp, %rbp			// save USP 
	leaq tss(%rip), %rsp		// load TSS 
	movq 4(%rsp), %rsp		// load KSP (tss->rsp0)
	subq $40, %rsp			// load KSP (tss->rsp0)
	movq %rbp, 24(%rsp)		// save USP 
	movq %r11, 16(%rsp)		// save UFLAGS
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

#if defined(CONFIG_SMP)
	/* check that both threads are on the same CPU */	
	mov	OFS_TCB_CPU(%rbp), %r11 
	cmp	%r11, OFS_TCB_CPU(%rcx) 
	jne	slowpath
#endif

	/* get myself */
	movq	OFS_TCB_MYSELF_GLOBAL(%rcx), %r11
	

	/* check that dest thread is waiting for current or any */
	cmpq	$TSTATE_WAITING_FOREVER, OFS_TCB_THREAD_STATE(%rbp)
	jne	slowpath
	
	cmpq	OFS_TCB_PARTNER(%rbp), %r11
	je	1f
	cmpq	$-1, OFS_TCB_PARTNER(%rbp)
	jnc	slowpath

1:	
	/* make sure that current really blocks
	 * (FromSpec == To) || (SendHead == NULL) */
	cmpq	%rsi, %rdx
	je	2f

	/* FromSpec == any && current->sendhead == 0 */
	cmpq	$-1, %rdx
	jne	slowpath
	cmpq	$0, OFS_TCB_SEND_HEAD(%rcx)
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
	 *    rdi = UIP (free)
	 *    rbp = destination TCB
	 *    r8  = timeout (free)
	 *    r11 = current->myself_global 
	 */

	/* calculate number of untyped words */
	movq    %r9, %rdx
	testb	$~0x7, %dl
	jnz	fp_copy_loop
fp_copy_loop_done:
	cmpq	$0, OFS_TCB_RESOURCE_BITS(%rcx)		// any resources?
	jnz	fp_save_resources
fp_save_resources_done:
	movq	%r11, %rsi				// from tid = current tid
	movq	%rbp, %r11				// current = to
	subq	$16, %rsp					
	pushq	$fp_ipc_done
	movq	OFS_TCB_MYSELF_LOCAL(%r11), %rdi	// local id
	movq	OFS_TCB_PDIR_CACHE(%r11), %rbp		// new PDIR
	/* jsXXX: set gs via virtual address, without selector */ 
	movq	%rdi, %gs:0
	movq	%rsp, OFS_TCB_STACK(%rcx)		// store stack ptr
	movq	OFS_TCB_STACK(%r11), %rsp		// load stack ptr

	cmpq	%rbp, OFS_TCB_PDIR_CACHE(%rcx)
	je	3f					// same ptab
	movq	%rbp, %cr3				// switch ptab
3:
	leaq	KTCB_SIZE(%r11), %rdx
	movq	%rdx, (tss + 4)				// save stack ptr to tss.esp0
	popq	%rdx
	cmpq	$fp_ipc_done, %rdx
	jne	4f
	cmpq	$0, OFS_TCB_RESOURCE_BITS(%r11)
  	jnz	fp_load_resources
	jmp	fp_ipc_ret
4:
	movq    %rdi, %rbx				// local ID
	movq    %r11, %rdi				// dest
	movq	OFS_TCB_UTCB(%r11), %rsi		// load UTCB from TCB
	movq	%r9,  UTCB_MR(0)(%rsi)			// store MR0 into UTCB
	testq	$0x7, %r9
	jnz	fp_reg_copy_loop
fp_reg_copy_loop_done:
	jmpq	*%rdx

fp_ipc_done:
	/* from tcb.h: r11 = dtcb */
	movq	$TSTATE_RUNNING, OFS_TCB_THREAD_STATE(%r11)
	cmpq	$0, OFS_TCB_RESOURCE_BITS(%r11)
	jnz	fp_load_resources
fp_load_resources_done:
	movq	OFS_TCB_UTCB(%r11), %rcx		// current UTCB
	movq	UTCB_MR(0)(%rcx), %r9			// restore MR0 from UTCB
	jmp	fp_ipc_slow_ret	
#endif

slowpath:
	CURRENT_TCB	%rdi
	movq	OFS_TCB_UTCB(%rdi), %rdi		// load UTCB from TCB
	movq	%r9,   UTCB_MR(0)(%rdi)			// store MR0 into UTCB
	SAVE_UTCB1	%rdi				// store MRs into UTCB
	movq	%r8, %rdi				// timeout (1st argument)
	subq	$16, %rsp				// make room for from and to
	movq	%rdx,  8(%rsp)				// save from
	movq	%rsi,   (%rsp)				// save to
	call	sys_ipc					
							
	CURRENT_TCB %r11				
	movq	%rdx, %r9				// restore MR0 from result
	movq	OFS_TCB_UTCB(%r11), %rcx		// dst UTCB
fp_ipc_slow_ret:
	movq	OFS_TCB_MYSELF_LOCAL(%r11), %rdi	// restore local id from TCB
	RESTORE_UTCB1 %rcx				// restore MRs from UTCB
fp_ipc_ret:	
	addq	$16, %rsp				// remove from, to
	movq	OFS_TCB_PARTNER(%r11), %rsi		// from specifier
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
	movq	%rsi, %rdx
	TID_TO_TCB %rdx, %rcx
	testl	$4, OFS_TCB_RESOURCE_BITS(%rdx)		// test Compatibility Mode resource
	jz	5f
	movl	$0, OFS_UTCB_WORD_SIZE_MASK+4(%r11)	// clear upper word
5:	
#endif
	movq	(%rsp), %rcx				// restore UIP
	movq	16(%rsp), %r11				// restore UFLAGS
	movq	24(%rsp), %rsp				// restore USP
	sysretq
	/* Non IPC Stub */
no_ipc:
	movq 	%rcx,   (%rsp)				// save UIP (after syscall)
	leaq 	user_nop_entry(%rip), %rbp		// load user_nop_entry
 	cmpq 	%rcx, %rbp				// coming from user_nop ?
	jne  	no_nop			
	movq 	24(%rsp),%rsp				// restore USP
	sysretq						// bye
no_nop:	
	pushq	 %r11					// push UFLAGS
	pushq	 %r10					// push arg7
	pushq	 %rax					// push arg6
	call 	syscall_dispatcher
	addq 	$24, %rsp				// remove arg6, arg7, UFLAGS
	popq 	%rcx					// restore UIP
	addq	$8, %rsp				// pop (undef)
	popq	%r11					// restore UFLAGS
	popq	%rsp					// restore USP
	sysretq


#ifdef CONFIG_IPC_FASTPATH
	/* 
	 * Registers:
	 *    rcx = current TCB
	 *    rdx = from specifier (free)
	 *    rsi = to specifier (free)
	 *    rdi = UIP (free)
	 *    rbp = destination TCB
	 *    r8  = timeout (free)
	 *    r11 = current->myself_global 
	 */
fp_save_resources:
	movq	OFS_TCB_UTCB(%rcx), %rdi		// load UTCB from TCB
	movq	%r9,   UTCB_MR(0)(%rdi)			// store MR0 into UTCB
	SAVE_UTCB1	%rdi				// store MRs into UTCB
	movq	%rcx, %rsi				// tcb 
	leaq	OFS_TCB_RESOURCES(%rcx), %rdi		// tcb->resources == this
	movq	%rcx, %r12				// save current
	movq	%rbp, %r13				// save dest
	movq	%r11, %r14				// save current_tid
	call	tcb_resources_save
	movq	%r14, %r11				// restore current_tid
	movq	%r13, %rbp				// restore dest
	movq	%r12, %rcx				// restore current
	movq	OFS_TCB_UTCB(%rcx), %rdi		// restore UTCB
	movq	UTCB_MR(0)(%rdi), %r9			// store MR0 into UTCB
	RESTORE_UTCB1 %rdi				// restore MRs from UTCB
	jmp	fp_save_resources_done

	/* Registers:
	 *    r11 = current TCB
	 */

fp_load_resources:
	movq	OFS_TCB_UTCB(%rcx), %rdi		// load UTCB from TCB
	movq	%r9,   UTCB_MR(0)(%rdi)			// store MR0 into UTCB
	movq	%rcx, %rsi				// tcb 
	leaq	OFS_TCB_RESOURCES(%rcx), %rdi		// tcb->resources == this
	movq	%r11, %r14				// save current
	call	tcb_resources_load
	movq	%r14, %r11				// restore current
	jmp	fp_load_resources_done

	/* Registers:
	 *    rcx = current TCB
	 *    rbp = dest    TCB
	 */

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

	/* Registers:
	 *    rcx = UTCB
	 *    r9  = tag
	 */

fp_reg_copy_loop:
	mov	%r9,	%r8
	and	$0x7,	%r8
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%rax,  UTCB_MR(1)(%rcx)
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%rbx,  UTCB_MR(2)(%rcx)
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%r10,  UTCB_MR(3)(%rcx)
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%r12,  UTCB_MR(4)(%rcx)
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%r13,  UTCB_MR(5)(%rcx)
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%r14,  UTCB_MR(6)(%rcx)	
	sub	$1,	%r8
	jz	fp_reg_copy_loop_done	
	movq	%r15,  UTCB_MR(7)(%rcx)	
	jmp	fp_reg_copy_loop_done	
#endif
	
END_PROC(syscall_entry)

.end
