/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/user.cc
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
 * $Id: user.cc,v 1.32 2006/11/16 20:06:12 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <asmsyms.h>
#include <tcb_layout.h>
#include INC_API(user.h)
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(ipc.h)
#include INC_API(syscalls.h)

#include INC_GLUE(timer.h)

#include INC_ARCH(trapgate.h)

#define OFS_USER_UTCB_MYGLOBAL	(OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR)
#define OFS_USER_UTCB_PROC	(OFS_UTCB_PROCESSOR_NO - OFS_UTCB_MR)

#define SYS_IPC_NUM			0x30
#define SYS_SYSCALL_NUM			0x31


#define SYSCALL_STUB(name)		\
    extern "C" void entry_##name ();	\
    extern "C" void user_##name ()
    

#define SYSCALL_LABEL(name)		\
    "	.global entry_"#name"	\n"	\
    "entry_"#name":		\n"	\
    "	int	%0		\n"


extern "C" void user_ipc() 
{
    __asm__ (
	"	movl	%%esi, (%%edi)	\n"	// store MR0 into UTCB
	"	popl	%%ebx		\n"	// get return address
#if defined(CONFIG_X86_SMALL_SPACES) && defined(CONFIG_X86_SYSENTER)
	"	pushl	%3		\n"
	"	pushl	%%ebx		\n"
#endif
	"	test	$0x3f,%%al	\n"	// check for local dest id
	"	je	3f		\n"
	"1:	test	$0x3f,%%dl	\n"	// check for local fromspec
	"	je	4f		\n"
	"2:				\n"
#if defined (CONFIG_X86_SYSENTER)
	"	movl	%%esp, %%ebp	\n"	// store return stack pointer
	"	sysenter		\n"
#else
	"	int	 %0		\n"
#endif
	"3:	test	%%eax,%%eax	\n"	// check for nilthread
	"	je	1b		\n"
	"	movl	%c1(%%eax),%%eax\n"	// load global id
	"	test	$0x3f, %%dl	\n"	// check for local fromspec
	"	jne	2b		\n"
	"4:	test	%%edx, %%edx	\n"	// check for nilthread
	"	je	2b		\n"
	"	movl	%%edx, %%ebp	\n"	// check for anylocal
	"	addl	%2, %%ebp	\n"
	"	jz	2b		\n"
	"	movl	%c1(%%edx),%%edx\n"
#if defined (CONFIG_X86_SYSENTER)
	"	movl	%%esp, %%ebp	\n"	// store return stack pointer
	"	sysenter		\n"
#else
	"	int	%0		\n"
#endif
	: 
	:
	"i"(SYS_IPC_NUM),
	"i"(OFS_USER_UTCB_MYGLOBAL), 
	"i"(1 << L4_LOCAL_ID_ZERO_BITS),
	"i"(X86_UCS));
}

extern "C" void user_lipc() 
{
    __asm__ (
	"	movl	%%esi, (%%edi)	\n"	// store MR0 into UTCB
	"	popl	%%ebx		\n"	// get return address
#if defined(CONFIG_X86_SMALL_SPACES) && defined(CONFIG_X86_SYSENTER)
	"	pushl	%3		\n"
	"	pushl	%%ebx		\n"
#endif
	"	test	$0x3f,%%al	\n"	// check for local dest id
	"	je	3f		\n"
	"1:	test	$0x3f,%%dl	\n"	// check for local fromspec
	"	je	4f		\n"
	"2:				\n"
#if defined (CONFIG_X86_SYSENTER)
	"	movl	%%esp, %%ebp	\n"	// store return stack pointer
	"	sysenter		\n"
#else
	"	int	 %0		\n"
#endif
	"3:	test	%%eax,%%eax	\n"	// check for nilthread
	"	je	1b		\n"
	"	movl	%c1(%%eax),%%eax\n"	// load global id
	"	test	$0x3f, %%dl	\n"	// check for local fromspec
	"	jne	2b		\n"
	"4:	test	%%edx, %%edx	\n"	// check for nilthread
	"	je	2b		\n"
	"	movl	%%edx, %%ebp	\n"	// check for anylocal
	"	addl	%2, %%ebp	\n"
	"	jz	2b		\n"	
	"	movl	%c1(%%edx),%%edx\n"
#if defined (CONFIG_X86_SYSENTER)
	"	movl	%%esp, %%ebp	\n"	// store return stack pointer
	"	sysenter		\n"
#else
	"	int	%0		\n"
#endif
	: 
	:
	"i"(SYS_IPC_NUM),
	"i"(OFS_USER_UTCB_MYGLOBAL), 
	"i"(1 << L4_LOCAL_ID_ZERO_BITS),
	"i"(X86_UCS));
}

SYSCALL_STUB(exchange_registers)
{
    __asm__ __volatile__ (
	"	test	$0x3f,%%al		\n"
	"	jne	1f			\n"
	"	movl	-60(%%eax),%%eax	\n"
	"1:					\n"
	SYSCALL_LABEL (exchange_registers)
	:
	: "i"(SYS_SYSCALL_NUM));

}

SYSCALL_STUB(system_clock)
{
#if defined(CONFIG_X86_TSC)
    __asm__ __volatile__ (
	"call	9f			\n"
	"1:				\n"
	"add	$kip-1b,%%ecx		\n"
	"add	%c0(%%ecx), %%ecx	\n"	/* procdesc */
#ifdef CONFIG_SMP
	/* get processor # --> allow for differently clocked CPUs */
	"mov	%%gs:0, %%eax		\n"	/* myutcb */
	"mov	%c2(%%eax), %%eax	\n"
	"shl	%3, %%eax		\n"
	"add	%%eax, %%ecx		\n"
#endif
	"mov	%c1(%%ecx), %%eax	\n"	/* processor freq kHz */
	"xorl	%%edx, %%edx		\n"
	"mov	%4, %%ecx		\n"
	"divl	%%ecx			\n"
	"mov	%%eax, %%ecx		\n"
	"rdtsc				\n"

	/*
	 * Clock = W*D + A, where W = 1^32.
	 *
	 * We want to calculate:
	 *
	 *   Clock/S = W*D/S + A/S
	 */

	/*
	 *   A/S
	 */
	"mov	%%edx, %%esi		\n"
	"xor	%%edx, %%edx		\n"
	"div	%%ecx			\n"

	/*
	 *   D/S = Dq + Dr  (Dq = quotient, Dr = remainder)
	 */
	"xchg	%%eax, %%esi		\n"
	"xor	%%edx, %%edx		\n"
	"div	%%ecx			\n"

	/*
	 *   Dr*W/S 
	 */
	"mov	%%eax, %%edi		\n"
	"xor	%%eax, %%eax		\n"
	"div	%%ecx			\n"

	/*
	 *   Clock/S = W*Dq + Dr*W/S + A/S
	 */
	"mov	%%edi, %%edx		\n"
	"add	%%esi, %%eax		\n"
	"adc	$0, %%edx		\n"
	"ret				\n"

	"9:				\n"
	"mov	(%%esp), %%ecx		\n"
	"ret				\n"
	:
	: "i"(OFS_KIP_PROCDESC), "i"(OFS_PROCDESC_INTFREQ), 
	  "i"(OFS_USER_UTCB_PROC), "i"(KIP_PROC_DESC_LOG2SIZE),
	  "i"(1000));
#else /* !CONFIG_X86_TSC */
    __asm__ __volatile__ (
	"call	9f			\n"
	"1:				\n"
	"add	$kip-1b,%%ecx		\n"
	"add    %0,%%ecx		\n"
	
	"mov    (%%ecx),%%eax           \n"
	"mov    $%c1,%%ebx		\n"
	"mul    %%ebx                   \n"
	
	"ret                            \n"

	"9:				\n"
	"mov	(%%esp), %%ecx		\n"
	"ret				\n"
	:
	/* Dirty hack to get the offset of the tick variable in the
	 * dark part of the KIP */
	: "r"((word_t)&ticks & ~X86_PAGE_MASK),
	  "i"(TIMER_TICK_LENGTH)
	);
#endif /* !CONFIG_X86_TSC */
}

SYSCALL_STUB(thread_switch)
{
    __asm__ __volatile__ (
	"	test	%%eax,%%eax		\n"
	"	je	1f			\n"
	"	test	$0x3f,%%al		\n"
	"	jne	1f			\n"
	"	movl	-60(%%eax),%%eax	\n"
	"1:					\n"
	SYSCALL_LABEL (thread_switch)
	:
	: "i"(SYS_SYSCALL_NUM));
}

SYSCALL_STUB(schedule)
{
    __asm__ __volatile__ (
	"	test	$0x3f,%%al		\n"
	"	jne	1f			\n"
	"	movl	-60(%%eax),%%eax	\n"
	"1:					\n"
	SYSCALL_LABEL (schedule)
	:
	: "i"(SYS_SYSCALL_NUM));
}

SYSCALL_STUB(unmap)
{
    __asm__ __volatile__ (
	"	movl	%%esi, (%%edi)	\n"	// store MR0 into UTCB
	SYSCALL_LABEL (unmap)
	"	movl	(%%edi), %%esi	\n"	// load MR0 from UTCB
	:
	: "i"(SYS_SYSCALL_NUM));
}

SYSCALL_STUB(thread_control)
{
    __asm__ __volatile__ (
	"	test	%%ecx,%%ecx		\n"
	"	je	1f			\n"
	"	test	$0x3f,%%cl		\n"
	"	jne	1f			\n"
	"	movl	-60(%%ecx),%%ecx	\n"
	"1:	test	%%edx,%%edx		\n"
	"	je	1f			\n"
	"	test	$0x3f,%%dl		\n"
	"	jne	1f			\n"
	"	movl	-60(%%edx),%%edx	\n"
	"1:	test	%%esi,%%esi		\n"
	"	je	1f			\n"
	"	test	$0x3f,%%esi		\n"
	"	jne	1f			\n"
	"	movl	-60(%%esi),%%esi	\n"
	"1:					\n"
	SYSCALL_LABEL (thread_control)
	:
	: "i"(SYS_SYSCALL_NUM));

}

SYSCALL_STUB(space_control)
{
    __asm__ __volatile__(
	"	test	%%edi,%%edi		\n"
	"	je	1f			\n"
	"	test	$0x3f,%%edi		\n"
	"	jne	1f			\n"
	"	movl	-60(%%edi),%%edi	\n"
	"1:					\n"
	SYSCALL_LABEL (space_control)
	:
	: "i"(SYS_SYSCALL_NUM));
}

SYSCALL_STUB(processor_control)
{
    __asm__ __volatile__(
	SYSCALL_LABEL (processor_control)
	:
	: "i"(SYS_SYSCALL_NUM));
}

SYSCALL_STUB(memory_control)
{
    __asm__ __volatile__(
	"	movl	%%esi, (%%edi)	\n"	// store MR0 into UTCB
	SYSCALL_LABEL (memory_control)
	:
	: "i"(SYS_SYSCALL_NUM));
}


#define IS_SYSCALL(x) (entry == entry_##x)

X86_EXCNO_ERRORCODE(exc_user_syscall, 0)
{
    /* eip points to the system call entry in the user's own kip area,
       calculate address in kernel address space */
    addr_t entry = (addr_t) (frame->eip - (u32_t)get_current_space()->get_kip_page_area().get_base() + (u32_t)get_kip() - 2);

    /* syscalls are dispatched by IP */
    if (IS_SYSCALL(exchange_registers))
    {
	// Note: dest was a local id if the zero flag is set
	sys_exchange_registers(threadid(frame->eax),
			       frame->ecx, 
			       frame->edx, frame->esi,
			       frame->edi, frame->ebx,
			       threadid(frame->ebp), 
			       frame->eflags & X86_FLAGS_ZF,
			       frame);
    } 

    else if (IS_SYSCALL(thread_switch))
    {
	sys_thread_switch(threadid(frame->eax));
    }

    else if (IS_SYSCALL(unmap))
    {
	sys_unmap(frame->eax);
    }

    else if ( IS_SYSCALL(schedule) )
    {
	sys_schedule(threadid(frame->eax), 
		     frame->edx, frame->esi, 
		     frame->ecx, frame->edi, frame);
    }

    else if (IS_SYSCALL(thread_control))
    {
	sys_thread_control(threadid(frame->eax), // dest
			   threadid(frame->esi), // space
			   threadid(frame->edx), // sched
			   threadid(frame->ecx), // pager
			   frame->edi, frame);
    }

    else if (IS_SYSCALL(space_control))
    {
	sys_space_control(threadid(frame->eax),		  // dest
			  frame->ecx,			  // control
			  (fpage_t){{ raw: frame->edx }}, // kip
			  (fpage_t){{ raw: frame->esi }}, // utcb
			  threadid(frame->edi),		  // redirector
			  frame);
    }

    else if (IS_SYSCALL(memory_control))
    {
	sys_memory_control(frame->eax, frame->ecx, frame->edx,
			   frame->ebx, frame->ebp, frame);
    }

    else
	printf("unknown syscall\n");

    return;
}
