/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/tcb.h
 * Description:   TCB related functions for Version 4, IA-32
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
 * $Id: tcb.h,v 1.69 2007/01/22 21:03:13 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X32__TCB_H__
#define __GLUE_V4_X86__X32__TCB_H__

#include INC_ARCH_SA(tss.h)
#include INC_ARCH(trapgate.h)
#include INC_API(syscalls.h)
#include INC_GLUE(resource_functions.h)



/**********************************************************************
 * 
 *            utcb state manipulation
 *
 **********************************************************************/


/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);
    ASSERT(count > 0);
    word_t dummy;

#if defined(CONFIG_X86_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_KDS));
#endif

    /* use optimized IA32 copy loop -- uses complete cacheline
       transfers */
    __asm__ __volatile__ (
	"cld\n"
	"rep  movsl (%0), (%1)\n"
	: /* output */
	"=S"(dummy), "=D"(dummy), "=c"(dummy)
	: /* input */
	"c"(count), "S"(&get_utcb()->mr[start]), 
	"D"(&dest->get_utcb()->mr[start]));

#if defined(CONFIG_X86_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_UDS));
#endif
}




/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

#if !defined(BUILD_TCB_LAYOUT)
#include <tcb_layout.h>
#include <kdb/tracepoints.h>
#include <kdb/tracebuffer.h>

/**
 * initial_switch_to: switch to first thread
 */
INLINE void NORETURN initial_switch_to (tcb_t * tcb)
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(tcb->stack));

    while (true)
	/* do nothing */;
}

/**
 * tcb_t::switch_to: switches to specified tcb
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    word_t dummy;
    
    if ( EXPECT_FALSE(resource_bits) )
	resources.save(this);

    /* modify stack in tss */
    if (this != get_kdebug_tcb() && dest != get_kdebug_tcb())
    {

	ASSERT(dest->stack);
	ASSERT(dest != this);
	ASSERT(get_cpu() == dest->get_cpu());
	
	tss.set_esp0((u32_t)dest->get_stack_top());
	tbuf_record_event(TP_DEFAULT, 0, "switch %wt (pdir=%p) ->  %wt (pdir=%p)\n",
			  this, this->pdir_cache, dest, dest->pdir_cache);
	
    }


#ifdef CONFIG_SMP
    active_cpu_space.set(get_cpu(), dest->space);
#endif

#if defined(CONFIG_X86_SMALL_SPACES)
    __asm__ __volatile__ (
	"/* switch_to_thread */				\n"
	"	pushl	%%ebp				\n"
	"	pushl	$9f				\n"

	"	/* switch stacks */			\n"
	"	movl	%%esp, %c4(%1)			\n"
	"	movl	%c4(%2), %%esp			\n"

	"	/* is kernel thread? */			\n"
	"	cmpl	$0, %c5(%2)			\n"
	"	je	4f				\n"

	"	movl	(%7), %%ecx			\n"
	"	movl	__is_small, %%edx		\n"
	"	movl	%c6(%1), %%ebp			\n"
//	"	movl	%%cr3, %%ebp			\n"
	"	testl	%%ecx, %%ecx			\n"
	"	je	2f				\n"

	"	/* switch to small space */		\n"
	"	cmpl	%c6(%2), %%ebp			\n"
	"	je	1f				\n"
	"	movl	4(%7), %%ecx			\n"
	"	movl	8(%7), %%eax			\n"
	"	movl	$gdt, %%ebp			\n"
	"	movl	%%ecx, 32(%%ebp)		\n"
	"	movl	%%eax, 36(%%ebp)		\n"
	"	orl	$0x800, %%eax			\n"
	"	movl	%%ecx, 24(%%ebp)		\n"
	"	movl	%%eax, 28(%%ebp)		\n"
	"	movl	$"MKSTR(X86_UDS)", %%ecx	\n"
	"	movl	%%ecx, %%es			\n"
#if !defined(CONFIG_TRACEBUFFER)
	"	movl	%%ecx, %%fs			\n"
#endif        
	"	movl	$"MKSTR(X86_UTCBS)", %%ecx	\n"
	"	movl	%%ecx, %%gs			\n"

	"	testl	%%edx, %%edx			\n"
	"	jne	1f				\n"
	"	movl	$1, __is_small			\n"
	"1:	popl	%%eax				\n"
	"	movl	%3, %%gs:0			\n"
	"	jmp	*%%eax				\n"

	"2:	/* switch to large space */		\n"
	"	testl	%%edx, %%edx			\n"
	"	je	3f				\n"
	"	movl	$0, __is_small			\n"
	"	movl	$gdt, %%eax			\n"
	"	movl	$0x0000ffff, 24(%%eax)		\n"
	"	movl	$0x00cbfb00, 28(%%eax)		\n"
	"	movl	$0x0000ffff, 32(%%eax)		\n"
	"	movl	$0x00cbf300, 36(%%eax)		\n"

	"	movl	$"MKSTR(X86_UDS)", %%edx	\n"
	"	movl	%%edx, %%es			\n"
#if !defined(CONFIG_TRACEBUFFER)
	"	movl	%%edx, %%fs			\n"
#endif
	"	movl	$"MKSTR(X86_UTCBS)", %%edx	\n"
	"	movl	%%edx, %%gs			\n"

	"3:	movl	%c6(%2), %%ecx			\n"
	"	cmpl	%%ebp, %%ecx			\n"
	"	je	4f				\n"
	"	movl	%%ecx, %%cr3			\n"

	"4:	popl	%%edx		/* activation addr */		\n"
	"	movl	%3, %%gs:0	/* update current UTCB */	\n"

	"	jmp	*%%edx						\n"
	"9:	movl	%2, %1		/* restore 'this' */		\n"
	"	popl	%%ebp						\n"

	:
	"=a" (dummy)				// 0
	:
	"b" (this),				// 1
	"S" (dest),				// 2
	"D" (dest->get_local_id().get_raw()),	// 3
	"i" (OFS_TCB_STACK),			// 4
	"i" (OFS_TCB_SPACE),			// 5
	"i" (OFS_TCB_PDIR_CACHE),		// 6
	"a" ((word_t) dest->space->smallid())	// 7
	:
	"ecx", "edx", "memory");

#else /* !CONFIG_X86_SMALL_SPACES */

    __asm__ __volatile__ (
	"/* switch_to_thread */	\n\t"
	"pushl	%%ebp		\n\t"
	
	"pushl	$3f		\n\t"	/* store return address	*/
	
	"movl	%%esp, %c4(%1)	\n\t"	/* switch stacks	*/
	"movl	%c4(%2), %%esp	\n\t"
#if !defined(CONFIG_CPU_X86_P4)
	"movl	%%cr3, %8	\n\t"	/* load current ptab */
	"cmpl	$0, %c5(%2)	\n\t"	/* if kernel thread-> use current */
	"je	2f		\n\t"
#endif
	"cmpl	%7, %8		\n\t"	/* same page dir?	*/
	"je	2f		\n\t"
#if defined(CONFIG_CPU_X86_P4)
	"cmpl	$0, %c5(%2)	\n\t"	/* kernel thread (space==NULL)?	*/
	"jne	1f		\n\t"
	"movl	%8, %c6(%2)	\n\t"	/* rewrite dest->pdir_cache */
	"jmp	2f		\n\t"

	"1:			\n\t"
#endif
	"movl	%7, %%cr3	\n\t"	/* reload pagedir */
	"2:			\n\t"
	"popl	%%edx		\n\t"	/* load activation addr */
	"movl	%3, %%gs:0	\n\t"	/* update current UTCB */

	"jmp	*%%edx		\n\t"
	"3:			\n\t"
	"movl	%2, %1		\n\t"	/* restore this */
	"popl	%%ebp		\n\t"
	"/* switch_to_thread */	\n\t"
	: /* trash everything */
	"=a" (dummy)				// 0
	:
	"b" (this),				// 1
	"S" (dest),				// 2
	"D" (dest->get_local_id().get_raw()),	// 3
	"i" (OFS_TCB_STACK),			// 4
	"i" (OFS_TCB_SPACE),			// 5
	"i" (OFS_TCB_PDIR_CACHE),		// 6
	"a" (dest->pdir_cache),			// 7
#if defined(CONFIG_CPU_X86_P4)
	"c" (this->pdir_cache)			// 8
#else
	"c" (dest->pdir_cache)			// dummy
#endif
	: "edx", "memory"
	);

#endif /* CONFIG_X86_SMALL_SPACES */

    if ( EXPECT_FALSE(resource_bits) )
	resources.load(this);
}

/**********************************************************************
 *
 *                        in-kernel IPC invocation 
 *
 **********************************************************************/

/**
 * tcb_t::do_ipc: invokes an in-kernel IPC 
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc(threadid_t to_tid, threadid_t from_tid, timeout_t timeout)
{
    msg_tag_t tag;
    word_t mr1, mr2, dummy;
    asm volatile
       ("pushl	%%ebp		\n"
	"pushl	%%ecx		\n"
	"call	sys_ipc		\n"
	"addl	$4, %%esp	\n"
	"movl	%%ebp, %%ecx	\n"
	"popl	%%ebp		\n"
	: "=S"(tag.raw),
	  "=b"(mr1),
	  "=c"(mr2),
	  "=a"(dummy),
	  "=d"(dummy)
	: "a"(to_tid.get_raw()),
	  "d"(from_tid.get_raw()),
	  "c"(timeout.raw)
	: "edi",
	  "memory");    
    set_mr(1, mr1);
    set_mr(2, mr2);
    return tag;
}


			  
/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

/* notify trampoline always removes two parameters */
extern "C" void notify_trampoline(void);

INLINE void tcb_t::notify(void (*func)())
{
    *(--stack) = (word_t)func;
}

INLINE void tcb_t::notify(void (*func)(word_t), word_t arg1)
{
    stack--;
    *(--stack) = arg1;
    *(--stack) = (word_t)notify_trampoline;
    *(--stack) = (word_t)func;
}

INLINE void tcb_t::notify(void (*func)(word_t, word_t), word_t arg1, word_t arg2)
{
    *(--stack) = arg2;
    *(--stack) = arg1;
    *(--stack) = (word_t)notify_trampoline;
    *(--stack) = (word_t)func;
}

INLINE void tcb_t::return_from_ipc (void)
{
    asm("movl %0, %%esp\n"
	"mov  %3, %%ebp\n"
	"ret\n"
	:
	:
	"r" (&get_stack_top ()[KSTACK_RET_IPC]),
	"S" (get_tag ().raw),
	"b" (get_mr (1)),
	"r" (get_mr (2)),
	"D" (get_local_id ().get_raw ()));
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(&get_stack_top()[- sizeof(x86_exceptionframe_t)/4 - 2]));
}

/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/


/**
 * Retrieve the real address associated with a copy area address.
 *
 * @param addr		address within copy area
 *
 * @return address translated into a regular user-level address
 */

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    ASSERT (space->is_copy_area (addr));

    word_t copyarea_num = 
	(((word_t) addr - COPY_AREA_START) >> X86_X32_PDIR_BITS) /
	(COPY_AREA_SIZE >> X86_X32_PDIR_BITS);

    return addr_offset (resources.copy_area_real_address (copyarea_num),
			(word_t) addr & (COPY_AREA_SIZE-1));
}

#endif /* !defined(BUILD_TCB_LAYOUT) */




/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

INLINE tcb_t * get_current_tcb()
{
    addr_t stack;
    asm ("lea -4(%%esp), %0" :"=r" (stack));
    return (tcb_t *) ((word_t) stack & KTCB_MASK);
}

/**
 * ipc_string_copy: architecture specific string copy.  
 * TODO: consider the string copy memory hints.
 */
#define IPC_STRING_COPY ipc_string_copy
INLINE void ipc_string_copy(void *dst, const void *src, word_t len)
{
    word_t dummy1, dummy2, dummy3;
#if defined(CONFIG_X86_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_KDS));
#endif
    asm volatile (
	    "jecxz 1f\n"
	    "repnz movsl (%%esi), (%%edi)\n"
	    "1: test $3, %%edx\n"
	    "jz 1f\n"
	    "mov %%edx, %%ecx\n"
	    "repnz movsb (%%esi), (%%edi)\n"
	    "1:\n"
	    : "=S"(dummy1), "=D"(dummy2), "=c"(dummy3)
	    : "S"(src), "D"(dst), "c"(len >> 2), "d"(len & 3));
#if defined(CONFIG_X86_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_UDS));
#endif
}


/**********************************************************************
 *
 *                  architecture-specific functions
 *
 **********************************************************************/


/**
 * initialize architecture-dependent root server properties based on
 * values passed via KIP
 * @param space the address space this server will run in   
 * @param ip the initial instruction pointer           
 * @param sp the initial stack pointer
 */
INLINE void tcb_t::arch_init_root_server (space_t * space, word_t ip, word_t sp)
{ 
}

#endif /* __GLUE_V4_X86__X32__TCB_H__ */
