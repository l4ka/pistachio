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

#define __L4_TRACEBUFFER_SIZE	(4 * 1024 * 1024)

/* Turn preprocessor symbol definition into string */
#define	MKSTR(sym)	MKSTR2(sym)
#define	MKSTR2(sym)	#sym

#if defined(L4_64BIT)
# define __PLUS32	+ 32
#else
# define __PLUS32
#endif


/*
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
typedef struct
{
    struct {
	L4_Word_t	utype	: 16;
	L4_Word_t	__pad0	: 16 __PLUS32;
	L4_Word_t	cpu	: 16;
	L4_Word_t	__pad1	: 16 __PLUS32;
    } X;
    
    L4_Word_t	tsc;
    L4_Word_t	thread;
    L4_Word_t	pmc0;
    L4_Word_t	pmc1;
    L4_Word_t	str;
    L4_Word_t	data[9];
} L4_TraceRecord_t;


/*
 * Access to performance monitoring counters
 */

/*
 * Access to stack pointer, timestamp, and performance monitoring counters
 */
#define __L4_TBUF_RDTSC  "	rdtsc					\n" \
			 "	mov	%3, %%fs:2*%c9(%0)		\n"	
    

#if defined(L4_PERFMON)
# define __L4_TBUF_RDPMC_0   "	rdpmc				\n"	\
			     "	mov	%3, %%fs:4*%c9(%0)	\n"
# define __L4_TBUF_RDPMC_1   "	rdpmc				\n"	\
			"	mov	%3, %%fs:5*%c9(%0)	\n"

# if !defined(L4_CONFIG_CPU_IA32_P4)
#  define __L4_TBUF_PMC_SEL_0 "	xorl	%%ecx, %%ecx		\n"
#  define __L4_TBUF_PMC_SEL_1 "	inc	%%ecx			\n"
# elif defined(L4_CONFIG_CPU_IA32_P4)
#  define __L4_TBUF_PMC_SEL_0 "	movl	$12, %%ecx		\n"
#  define __L4_TBUF_PMC_SEL_1 "	add	$2, %%ecx		\n"
# endif
#else
# define __L4_TBUF_PMC_SEL_0 "	xor	%3, %3			\n"
# define __L4_TBUF_PMC_SEL_1
# define __L4_TBUF_RDPMC_0   "	mov	%3, %%fs:4*%c9(%0)	\n"
# define __L4_TBUF_RDPMC_1   "	mov	%3, %%fs:5*$c9(%9)	\n"
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
	"	inc	%%fs:8*%c1(%0)		\n"		\
	:							\
	:							\
	"r" ((ctr & 0x7) * 4),					\
	"i" (sizeof(L4_Word_t)));				\
} while (0)


#define __L4_TBUF_GET_NEXT_RECORD(tid, event)				\
    ({									\
	L4_Word_t _dummy, _addr;					\
	asm volatile (							\
	    /* Check wheter to filter the event */			\
	    "	mov	%%fs:2*%c9, %3			\n"		\
	    "	and	%1, %3				\n"		\
	    "	xor	%%fs:3*%c9, %3			\n"		\
	    "	jnz	2f				\n"		\
									\
	    /* Get record offset into EDI */				\
	    "1:	mov	%%fs:1*%c9, %3			\n"		\
	    "	mov	%8, %0				\n"		\
	    "	mov	%0, %2				\n"		\
	    "	add	%3, %0				\n"		\
	    "	and	$"MKSTR(__L4_TRACEBUFFER_SIZE-1)", %0	\n"	\
	    "	cmovz	%2, %0			      	\n"		\
	    __L4_TBUF_LOCK						\
	    "	cmpxchg	%0, %%fs:1*%c9			\n"		\
	    "	jnz	1b				\n"		\
									\
	    /* Store type, cpu, thread, counters */			\
	    "	movl	%%ecx, %%fs:(%0)		\n"		\
	    "	mov	%%gs:0, %1			\n"		\
	    "	mov 	"MKSTR(4 *__L4_TCR_PROCESSOR_NO)"(%1), %2\n"	\
	    "	movl 	%%edx, %%fs:4(%0)		\n"		\
	    "	mov 	"MKSTR(4 *__L4_TCR_MY_GLOBAL_ID)"(%1), %2\n"	\
	    "	movl 	%%edx, %%fs:3*%c9(%0)		\n"		\
	    __L4_TBUF_RDTSC						\
	    __L4_TBUF_PMC_SEL_0						\
	    __L4_TBUF_RDPMC_0						\
	    __L4_TBUF_PMC_SEL_1						\
	    __L4_TBUF_RDPMC_1						\
	    "2:						\n"		\
	    :								\
		"=D" (_addr),				/* 0  */	\
		"=c" (_dummy),				/* 1  */	\
		"=d" (_dummy),				/* 2  */	\
		"=a" (_dummy)				/* 3  */	\
	    :								\
		"0" (0),				/* 4  */	\
		"1" (event & 0xffff),			/* 5  */	\
		"2" (0),				/* 6  */	\
		"3" (0),				/* 7  */	\
		"i" (sizeof (L4_TraceRecord_t)),	/* 8  */	\
		"i" (sizeof(L4_Word_t))			/* 9  */	\
	    );								\
	_addr;								\
    })


/**
 * Record (format) string into event buffer.
 *
 * @param addr		offset of event record
 * @param offset	string to be recorded
 */
#define __L4_TBUF_STORE_STR(addr, str)				\
do {								\
    asm volatile (						\
	"mov   %0, %%fs:6*%c2(%1)\n"				\
	:							\
	:							\
	"r" (str),						\
	"D" (addr),						\
	"i" (sizeof(L4_Word_t)));				\
} while (0)


/**
 * Record arguments into event buffer at indicated location.
 *
 * @param addr		offset of event record
 * @param offset	offset within event record
 * @param item		value to be recorded
 */
#define __L4_TBUF_STORE_DATA(addr, offset, item)		\
do {								\
    L4_Word_t _dummy;						\
    asm volatile (						\
	"mov  %2, %%fs:(%1)\n"					\
	:							\
	"=D" (_dummy)						\
	:							\
	"0" (addr + (7 + offset) * sizeof(L4_Word_t)),	\
	"r" (item));						\
} while (0)



#endif /* !__L4__IA32__TRACEBUFFER_H__ */
