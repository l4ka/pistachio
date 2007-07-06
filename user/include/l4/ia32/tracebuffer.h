/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     l4/ia32/tracebuffer.h
 * Description:   Functions for accessing the tracebuffer
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
 ********************************************************************/
#ifndef __L4__IA32__TRACEBUFFER_H__
#define __L4__IA32__TRACEBUFFER_H__

#define __L4_TRACEBUFFER_SIZE	(1UL << 22)


/*
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
typedef struct
{
    struct {
	L4_Word_t	utype	: 16;
	L4_Word_t	__pad	: 16;
    } X;
    L4_Word_t	tsc;
    L4_Word_t	thread;
    L4_Word_t	str;
    L4_Word_t	data[4];
    L4_Word_t	pmc0;
    L4_Word_t	pmc1;
    L4_Word_t	__pad2[6];
} L4_TraceRecord_t;


/*
 * Access to performance monitoring counters
 */

#if defined(L4_PERFMON)
# define __L4_RDPMC_0   "	rdpmc				\n"	\
			"	movl	%%eax, %%fs:32(%%edi)	\n"
# define __L4_RDPMC_1   "	rdpmc				\n"	\
			"	movl	%%eax, %%fs:36(%%edi)	\n"
# if !defined(L4_CONFIG_CPU_IA32_P4)
#  define __L4_PMC_SEL_0 "	xorl	%%ecx, %%ecx		\n"
#  define __L4_PMC_SEL_1 "	inc	%%ecx			\n"
# elif defined(L4_CONFIG_CPU_IA32_P4)
#  define __L4_PMC_SEL_0 "	movl	$12, %%ecx		\n"
#  define __L4_PMC_SEL_1 "	add	$2, %%ecx		\n"
# endif
#else
# define __L4_PMC_SEL_0 "	xorl	%%eax, %%eax		\n"
# define __L4_PMC_SEL_1
# define __L4_RDPMC_0   "	movl	%%eax, %%fs:32(%%edi)	\n"
# define __L4_RDPMC_1   "	movl	%%eax, %%fs:36(%%edi)	\n"
#endif


/*
 * Make sure cmpxchg is atomic
 */

#if defined(L4_CONFIG_SMP)
# define __L4_TBUF_LOCK "lock;"
#else
# define __L4_TBUF_LOCK
#endif


/*
 * Tracebuffer access macros
 */

#define __L4_TBUF_INCREASE_COUNTER(ctr)				\
do {								\
    asm volatile (						\
	__L4_TBUF_LOCK						\
	"	incl	%%fs:32(%0)		\n"		\
	:							\
	:							\
	"r" ((ctr & 0x7) * 4));					\
} while (0)


#define __L4_TBUF_GET_NEXT_RECORD(tid, event)			\
({								\
    L4_Word_t _addr, _dummy;					\
    asm volatile (						\
	/* Check wheter to filter the event */			\
	"	movl	%%fs:8, %%edx		\n"		\
	"	andl	%%ecx, %%edx		\n"		\
	"	xorl	%%fs:12, %%edx		\n"		\
	"	jnz	2f			\n"		\
								\
	/* Get record offset into EDI */			\
	"1:	movl	%%fs:4, %%eax		\n"		\
	"	movl	%3, %%edi		\n"		\
	"	movl	%%edi, %%edx		\n"		\
	"	addl	%%eax, %%edi		\n"		\
	"	andl   	%4, %%edi		\n"		\
	"	cmovzl	%%edx, %%edi		\n"		\
	__L4_TBUF_LOCK						\
	"	cmpxchg	%%edi, %%fs:4		\n"		\
	"	jnz	1b			\n"		\
								\
	/* Store type, thread, and counters into record */	\
	"	rdtsc				\n"		\
	"	movl	%%ecx, %%fs:0(%%edi)	\n"		\
	"	movl	%%eax, %%fs:4(%%edi)	\n"		\
	"	movl	%%esi, %%fs:8(%%edi)	\n"		\
	__L4_PMC_SEL_0						\
	__L4_RDPMC_0						\
	__L4_PMC_SEL_1						\
	__L4_RDPMC_1						\
	"2:					\n"		\
	:							\
	"=D" (_addr), 						\
	"=c" (_dummy), 						\
	"=S" (_dummy) 						\
	:							\
	"i" (sizeof (L4_TraceRecord_t)), 			\
	"i" (__L4_TRACEBUFFER_SIZE - 1),			\
	"c" ((event & 0xffff)), 				\
	"S" (tid.raw),						\
	"D" (0) 						\
	:							\
	"eax", "edx");						\
    _addr;							\
})


#define __L4_TBUF_STORE_ITEM(addr, offset, item)		\
do {								\
    asm volatile (						\
	"	movl	%0, %%fs:("#offset"*4+12)(%%edi)"	\
	:							\
	:							\
	"r" (item),						\
	"D" (addr));						\
} while (0)


#endif /* !__L4__IA32__TRACEBUFFER_H__ */
