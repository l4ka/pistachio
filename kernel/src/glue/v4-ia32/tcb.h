/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/tcb.h
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
#ifndef __GLUE__V4_IA32__TCB_H__
#define __GLUE__V4_IA32__TCB_H__

#include <kdb/tracebuffer.h>
#include INC_ARCH(tss.h)
#include INC_ARCH(trapgate.h)
#include INC_API(syscalls.h)
#include INC_GLUE(resource_functions.h)

/* forward declaration */
tcb_t * get_idle_tcb();

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    utcb_t * dummy = (utcb_t*)NULL;
    myself_local.set_raw (utcb_location + ((word_t)&dummy->mr[0]));
}

INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t * dummy = (utcb_t*)NULL;
    return myself_local.get_raw() - ((word_t)&dummy->mr[0]);
}

INLINE void tcb_t::set_cpu(cpuid_t cpu) 
{ 
    this->cpu = cpu;

    // only update UTCB if there is one
    if (get_utcb())
	get_utcb()->processor_no = cpu;

    // update the pdir cache on migration
    if (space) 
	this->pdir_cache = (word_t)space->get_pdir(get_cpu());
}


/**
 * tcb_t::get_mr: returns value of message register
 * @index: number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    ASSERT(index < IPC_NUM_MR);
    return get_utcb()->mr[index];
}

/**
 * tcb_t::set_mr: sets the value of a message register
 * @index: number of message register
 * @value: value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    ASSERT(index < IPC_NUM_MR);
    get_utcb()->mr[index] = value;
}

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

#if defined(CONFIG_IA32_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_KDS));
#endif

    /* use optimized IA32 copy loop -- uses complete cacheline
       transfers */
    __asm__ __volatile__ (
	"cld\n"
	"rep   movsl (%%esi), (%%edi)\n"
	: /* output */
	"=S"(dummy), "=D"(dummy), "=c"(dummy)
	: /* input */
	"c"(count), "S"(&get_utcb()->mr[start]), 
	"D"(&dest->get_utcb()->mr[start]));

#if defined(CONFIG_IA32_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (X86_UDS));
#endif
}

/**
 * tcb_t::get_br: returns value of buffer register
 * @index: number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    ASSERT(index < IPC_NUM_BR);
    return get_utcb()->br[32U-index];
}

/**
 * tcb_t::set_br: sets the value of a buffer register
 * @index: number of buffer register
 * @value: value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    ASSERT(index < IPC_NUM_BR);
    get_utcb()->br[32U-index] = value;
}




INLINE void tcb_t::allocate()
{
    __asm__ __volatile__(
	"orl $0, %0\n"
	: 
	: "m"(*this));

}

INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    this->pdir_cache = (word_t)space->get_pdir(get_cpu());
}

INLINE word_t * tcb_t::get_stack_top()
{
    return (word_t*)addr_offset(this, KTCB_SIZE);
}

INLINE void tcb_t::init_stack()
{
    stack = get_stack_top();
    //TRACE("stack = %p\n", stack);
}



/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

#ifndef BUILD_TCB_LAYOUT
#include <tcb_layout.h>

/**
 * tcb_t::switch_to: switches to specified tcb
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    word_t dummy;

    ASSERT(dest->stack);
    ASSERT(dest != this);
    ASSERT(get_cpu() == dest->get_cpu());

    if ( EXPECT_FALSE(resource_bits) )
	resources.save(this);

    /* modify stack in tss */
    tss.set_esp0((u32_t)dest->get_stack_top());

    tbuf_record_event (1, "switch %t => %t", (word_t)this, (word_t)dest);

#ifdef CONFIG_SMP
    active_cpu_space.set(get_cpu(), dest->space);
#endif

#if defined(CONFIG_IA32_SMALL_SPACES)
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
	"	movl	$"MKSTR(IA32_UDS)", %%ecx	\n"
	"	movl	%%ecx, %%es			\n"
#if !defined(CONFIG_TRACEBUFFER)
	"	movl	%%ecx, %%fs			\n"
#endif        
	"	movl	$"MKSTR(IA32_UTCB)", %%ecx	\n"
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

	"	movl	$"MKSTR(IA32_UDS)", %%edx	\n"
	"	movl	%%edx, %%es			\n"
#if !defined(CONFIG_TRACEBUFFER)
	"	movl	%%edx, %%fs			\n"
#endif
	"	movl	$"MKSTR(IA32_UTCB)", %%edx	\n"
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
	"a" ((word_t) dest->space
	     + (CONFIG_SMP_MAX_CPUS * IA32_PAGE_SIZE)
	     + (SMALL_SPACE_ID >> IA32_PAGEDIR_BITS) * 4) // 7
	:
	"ecx", "edx", "memory");

#else /* !CONFIG_IA32_SMALL_SPACES */

    __asm__ __volatile__ (
	"/* switch_to_thread */	\n\t"
	"pushl	%%ebp		\n\t"
	
	"pushl	$3f		\n\t"	/* store return address	*/
	
	"movl	%%esp, %c4(%1)	\n\t"	/* switch stacks	*/
	"movl	%c4(%2), %%esp	\n\t"
#ifndef CONFIG_CPU_IA32_P4
	"movl	%%cr3, %8	\n\t"	/* load current ptab */
	"cmpl	$0, %c5(%2)	\n\t"	/* if kernel thread-> use current */
	"je	2f		\n\t"
#endif
	"cmpl	%7, %8		\n\t"	/* same page dir?	*/
	"je	2f		\n\t"
#ifdef CONFIG_CPU_IA32_P4
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
#ifdef CONFIG_CPU_IA32_P4
	"c" (this->pdir_cache)			// 8
#else
	"c" (dest->pdir_cache)			// dummy
#endif
	: "edx", "memory"
	);

#endif /* CONFIG_IA32_SMALL_SPACES */

    if ( EXPECT_FALSE(resource_bits) )
	resources.load(this);
}
#endif


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
    asm("pushl	%%ebp		\n"
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
	: "edi");
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

/* 
 * access functions for ex-regs'able registers
 */
INLINE addr_t tcb_t::get_user_ip()
{
    return (addr_t)get_stack_top()[KSTACK_UIP];
}

INLINE addr_t tcb_t::get_user_sp()
{
    return (addr_t)get_stack_top()[KSTACK_USP];
}

INLINE word_t tcb_t::get_user_flags()
{
    return (word_t)get_stack_top()[KSTACK_UFLAGS];
}

INLINE void tcb_t::set_user_ip(addr_t ip)
{
    get_stack_top()[KSTACK_UIP] = (u32_t)ip;
}

INLINE void tcb_t::set_user_sp(addr_t sp)
{
    get_stack_top()[KSTACK_USP] = (u32_t)sp;
}

INLINE void tcb_t::set_user_flags(const word_t flags)
{
    get_stack_top()[KSTACK_UFLAGS] = (get_user_flags() & (~IA32_USER_FLAGMASK)) | (flags & IA32_USER_FLAGMASK);
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
	: "r"(&get_stack_top()[- sizeof(ia32_exceptionframe_t)/4 - 2]));
}


/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

INLINE void tcb_t::adjust_for_copy_area (tcb_t * dst,
					 addr_t * saddr, addr_t * daddr)
{
    resources.enable_copy_area (this, saddr, dst, daddr);
}

INLINE void tcb_t::release_copy_area (void)
{
    resources.release_copy_area (this, true);
}

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    ASSERT (space->is_copy_area (addr));

    word_t copyarea_num = 
	(((word_t) addr - COPY_AREA_START) >> IA32_PAGEDIR_BITS) /
	(COPY_AREA_SIZE >> IA32_PAGEDIR_BITS);

    return addr_offset (resources.copy_area_real_address (copyarea_num),
			(word_t) addr & (COPY_AREA_SIZE-1));
}


/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

INLINE tcb_t * addr_to_tcb (addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}

INLINE tcb_t * get_current_tcb()
{
    addr_t stack;
    asm ("lea -4(%%esp), %0" :"=r" (stack));
    return addr_to_tcb (stack);
}

#ifdef CONFIG_SMP
INLINE cpuid_t get_current_cpu()
{
    return get_idle_tcb()->get_cpu();
}
#endif

/**
 * initial_switch_to: switch to first thread
 */
INLINE void __attribute__((noreturn)) initial_switch_to(tcb_t * tcb) 
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(tcb->stack));

    while (true)
	/* do nothing */;
}

/**
 * ipc_string_copy: architecture specific string copy.  
 * TODO: consider the string copy memory hints.
 */
#define IPC_STRING_COPY ipc_string_copy
INLINE void ipc_string_copy(void *dst, const void *src, word_t len)
{
    word_t dummy1, dummy2, dummy3;
#if defined(CONFIG_IA32_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (IA32_KDS));
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
#if defined(CONFIG_IA32_SMALL_SPACES)
    asm volatile ("mov %0, %%es" : : "r" (IA32_UDS));
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

#endif /* __GLUE__V4_IA32__TCB_H__ */
