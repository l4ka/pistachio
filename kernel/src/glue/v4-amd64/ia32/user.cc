/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/user.cc
 * Description:   user-mode syscall stubs for 32-bit programs
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
 * $Id: user.cc,v 1.2 2006/10/21 02:02:44 reichelt Exp $
 *                
 ********************************************************************/
#include INC_GLUE(ia32/user.h)

#define BEGIN(name, section)				\
	"	.section " section ",\"ax\",@progbits\n"\
	"	.global " #name "		\n"	\
	#name ":				\n"

#if defined(CONFIG_CPU_AMD64_EM64T)

/* Possible optimization:
   In the cases where END directly follows SYSCALL, the additional
   call instruction would not be necessary, except that the current
   implementation uses the UIP address to select the syscall. */

#define SYSCALL						\
	"	movl	%esp,	%ebp		\n"	\
	"	call	0f			\n"

#define END						\
	"	ret				\n"	\
	"0:					\n"	\
	"	popl	%ecx			\n"	\
	"	sysenter			\n"

#else /* !defined(CONFIG_CPU_AMD64_EM64T) */

#define SYSCALL						\
	"	syscall				\n"

#define END						\
	"	ret				\n"

#endif /* !defined(CONFIG_CPU_AMD64_EM64T) */


__asm__ __volatile__ (
	"	.global user_lipc_32		\n"
	"	.set user_lipc_32, user_ipc_32	\n"

	BEGIN(user_ipc_32,		  ".user.syscall_32.ipc")
	"	movl	%esi,	(%edi)		\n"	/* save MR0 */
	"	movl	%ecx,	%ebx		\n"	/* save ECX (clobbered by syscall) */
	"	test	$0x3f,	%al		\n"	/* check if to=local/nilthread */
	"	je	3f			\n"	/* yes: jump */
	"1:					\n"	/* to=global */
	"	test	$0x3f,	%dl		\n"	/* check if from=local/nilthread/anylocal */
	"	je	4f			\n"	/* from=global */
	"2:					\n"
	SYSCALL
	"	movl	%edx,	%esi		\n"	/* load result */
	"	movl	%gs:0,	%edi		\n"	/* load UTCB */
	"	movl	4(%edi), %ebx		\n"	/* load MR1 */
	"	movl	8(%edi), %ebp		\n"	/* load MR2 */
#if defined(CONFIG_CPU_AMD64_K8)
	/* Reload the stack segment, otherwise we sometimes
	   get a stack exception on the first attempt to
	   use the stack pointer. I have not been able to
	   find out why this happens. It only occurs on
	   real hardware, depends on whether threads are
	   executed in parallel, and disappears when
	   enter_kdebug is called after sys_ipc. */
	"	movl	%ss,	%edx		\n"
	"	movl	%edx,	%ss		\n"
#endif /* defined(CONFIG_CPU_AMD64_K8) */
	END
	"3:					\n"	/* to=local/nilthread */
	"	test	%eax,	%eax		\n"	/* check if to=nilthread */
	"	je	1b			\n"	/* yes: continue */
	"	movl	-60(%eax), %eax		\n"	/* no: load global id (to) */
	"	test	$0x3f,	%dl		\n"	/* check if from=local/nilthread */
	"	jne	2b			\n"	/* no: continue */
	"4:					\n"	/* from=local/nilthread */
	"	test	%edx,	%edx		\n"	/* check if from=nilthread */
	"	je	2b			\n"	/* yes: continue */
	"	movl	%edx,	%ebp		\n"	/* check if from=anylocal */
	"	addl	$0x40,	%ebp		\n"
	"	jz	2b			\n"	/* yes: continue */
	"	movl	-60(%edx), %edx		\n"	/* no: load global id (from) */
	"	jmp	2b			\n"	/* continue */

	BEGIN(user_exchange_registers_32, ".user.syscall_32.exregs")
	"	pushl	%ecx			\n"	/* save ECX (control) */
	"	movl	%gs:0,	%ecx		\n"	/* load UTCB */
	"	movl	$0,	-504(%ecx)	\n"	/* clear is_local in UTCB */
	"	test	$0x3f,	%al		\n"	/* local destination id? */
	"	jne	1f			\n"
	"	movl	-60(%eax), %eax		\n"	/* yes: load global id from UTCB */
	"	movl	$1,	-504(%ecx)	\n"	/* set is_local in UTCB */
	"1:					\n"
	"	movl	%ebp,	-500(%ecx)	\n"	/* save pager in UTCB */
	"	popl	-496(%ecx)		\n"	/* save control in UTCB */
	SYSCALL
	"	movl	%gs:0,	%ecx		\n"	/* load UTCB */
	"	movl	-496(%ecx), %ecx	\n"	/* load control from UTCB */
	END

	BEGIN(user_system_clock_32,	  ".user.syscall_32.sysclock")
	"	pushl	%ebx			\n"	/* save EBX */
	"	pushl	%ebp			\n"	/* save EBP */
	SYSCALL
	"	popl	%ebp			\n"	/* restore EBP */
	"	popl	%ebx			\n"	/* restore EBX */
	END

	BEGIN(user_thread_switch_32,	  ".user.syscall_32.threadswtch")
	"	pushl	%eax			\n"	/* save EAX */
	"	pushl	%ebx			\n"	/* save EBX */
	"	pushl	%ecx			\n"	/* save ECX */
	"	pushl	%edx			\n"	/* save EDX */
	"	pushl	%esi			\n"	/* save ESI */
	"	pushl	%edi			\n"	/* save EDI */
	"	pushl	%ebp			\n"	/* save EBP */
	"	test	%eax,	%eax		\n"
	"	je	1f			\n"
	"	test	$0x3f,	%al		\n"
	"	jne	1f			\n"
	"	movl	-60(%eax), %eax	\n"
	"1:					\n"
	SYSCALL
	"	popl	%ebp			\n"	/* restore EBP */
	"	popl	%edi			\n"	/* restore EDI */
	"	popl	%esi			\n"	/* restore ESI */
	"	popl	%edx			\n"	/* restore EDX */
	"	popl	%ecx			\n"	/* restore ECX */
	"	popl	%ebx			\n"	/* restore EBX */
	"	popl	%eax			\n"	/* restore EAX */
	END

	BEGIN(user_schedule_32,		  ".user.syscall_32.schedule")
	"	test	$0x3f,	%al		\n"
	"	jne	1f			\n"
	"	movl	-60(%eax), %eax		\n"
	"1:					\n"
	"	movl	%ecx,	%ebx		\n"	/* save ECX (clobbered by syscall) */
	SYSCALL
	END

	BEGIN(user_unmap_32,		  ".user.syscall_32.unmap")
	SYSCALL
	"	movl	%eax,	%edi		\n"	/* load UTCB */
	"	movl	(%edi),	%esi		\n"	/* load MR0 */
	END

	BEGIN(user_thread_control_32,	  ".user.syscall_32.threadctrl")
	"	test	%ecx,	%ecx		\n"
	"	je	1f			\n"
	"	test	$0x3f,	%cl		\n"
	"	jne	1f			\n"
	"	movl	-60(%ecx), %ecx		\n"
	"1:	test	%edx,	%edx		\n"
	"	je	1f			\n"
	"	test	$0x3f,	%dl		\n"
	"	jne	1f			\n"
	"	movl	-60(%edx), %edx		\n"
	"1:	test	%esi,	%esi		\n"
	"	je	1f			\n"
	"	test	$0x3f,	%esi		\n"
	"	jne	1f			\n"
	"	movl	-60(%esi), %esi		\n"
	"1:					\n"
	"	movl	%ecx,	%ebx		\n"	/* save ECX (clobbered by syscall) */
	SYSCALL
	END

	BEGIN(user_space_control_32,	  ".user.syscall_32.spacectrl")
	"	test	%edi,	%edi		\n"
	"	je	1f			\n"
	"	test	$0x3f,	%edi		\n"
	"	jne	1f			\n"
	"	movl	-60(%edi), %edi		\n"
	"1:					\n"
	"	movl	%ecx,	%ebx		\n"	/* save ECX (clobbered by syscall) */
	SYSCALL
	"	movl	%edx,	%ecx		\n"	/* load result */
	END

	BEGIN(user_processor_control_32,  ".user.syscall_32.procctrl")
	"	movl	%ecx,	%ebx		\n"	/* save ECX (clobbered by syscall) */
	SYSCALL
	END

	BEGIN(user_memory_control_32,	  ".user.syscall_32.memctrl")
	/* Don't call AMD64 MemoryControl with IA-32 parameters;
	   they may not have the same meaning. */
	"	movl	$0,	%eax		\n"	/* set failing result */
	END
);
