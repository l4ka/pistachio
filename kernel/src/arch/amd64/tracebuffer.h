/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     arch/amd64/tracebuffer.h
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
#ifndef __ARCH__AMD64__TRACEBUFFER_H__
#define __ARCH__AMD64__TRACEBUFFER_H__

#define TRACEBUFFER_MAGIC 	0x1464b123acebf
#define TRACEBUFFER_SIZE	(1UL << 21)


/**
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
class tracerecord_t
{
    struct {
	u32_t	utype	: 16;
	u32_t	ktype	: 16;
    };
    u32_t	tsc;
    u32_t	pmc0;
    u32_t	pmc1;
    word_t	thread;
    word_t	str;
    word_t	data[4];

    bool is_kernel_event (void) { return ktype && ! utype; }
    word_t get_type (void) { return (utype == 0) ? ktype : utype; }

    friend class kdb_t;
};


/**
 * The tracebuffer is a region in memory accessible by kernel and user
 * through the FS segment.  The first words of the region indicate the
 * current tracebuffer record and which events should be recorded.
 * The remaining part of the region hold the tracebuffer records.
 */
class tracebuffer_t
{
    word_t	magic;
    word_t	current;
    word_t	mask;
    word_t	eventsel;
    u32_t	counters[8];

    tracerecord_t tracerecords[];

public:
    void initialize (void)
	{
	    magic = TRACEBUFFER_MAGIC;
	    current = 0;
	    mask = eventsel = 0;
	}

    bool is_valid (void) { return magic == TRACEBUFFER_MAGIC; }

    friend class kdb_t;
};

INLINE tracebuffer_t * get_tracebuffer (void)
{
    extern tracebuffer_t * tracebuffer;
    return tracebuffer;
}


/*
 * Access to performance monitoring counters
 */

#if defined(CONFIG_TBUF_PERFMON)

# define RDPMC_0        "	rdpmc				\n"	\
			"	movl	%%eax, %%fs:8(%%rdi)	\n"
# define RDPMC_1        "	rdpmc				\n"	\
			"	movl	%%eax, %%fs:12(%%rdi)	\n"
# if defined(CONFIG_CPU_AMD64_K8)
# define PMC_SEL_0		"xorl  %%ecx, %%ecx		\n"
# define PMC_SEL_1		"inc   %%ecx			\n"
# elif defined(CONFIG_CPU_AMD64_P4)
/* PMC_MSR_IQ_COUNTER 0 and 2 */
#  define PMC_SEL_0	"	movl	$12, %%ecx		\n"
#  define PMC_SEL_1	"	add	$14, %%ecx		\n"
# endif
#else
# define PMC_SEL_0
# define PMC_SEL_1
# define RDPMC_0
# define RDPMC_1
#endif


/*
 * Make sure cmpxchg is atomic
 */

#if defined(CONFIG_SMP)
# define TBUF_LOCK "lock;"
#else
# define TBUF_LOCK
#endif


/**
 * Increase a tracebuffer counter.
 *
 * @param ctr		counter number
 */
#define TBUF_INCREASE_COUNTER(ctr)				\
do {								\
    asm volatile (						\
	TBUF_LOCK						\
	"	incl	%%fs:32(%0)		\n"		\
	:							\
	:							\
	"r" ((ctr & 0x7) * 4));					\
} while (0)


/**
 * Start recording new event into tracebuffer.
 *
 * @param event		type of event
 *
 * @returns index to current event record
 */
#define TBUF_GET_NEXT_RECORD(event)				\
({								\
    word_t addr, dummy;						\
    asm volatile (						\
	/* Check wheter to filter the event */			\
	"	movl	%%fs:16, %%edx		\n"		\
	"	andl	%%ecx, %%edx		\n"		\
	"	xorl	%%fs:24, %%edx		\n"		\
	"	jnz	2f			\n"		\
								\
	/* Get record offset into RDI */			\
	"1:	movq	%%fs:8, %%rax		\n"		\
	"	movq	%2, %%rdi		\n"		\
	"	movq	%%rdi, %%rdx		\n"		\
	"	addq	%%rax, %%rdi		\n"		\
	"	andq   	%3, %%rdi		\n"		\
	"	cmovzq	%%rdx, %%rdi		\n"		\
	TBUF_LOCK						\
	"	cmpxchg	%%rdi, %%fs:8		\n"		\
	"	jnz	1b			\n"		\
								\
	/* Store type, thread, and counters into record */	\
	"	rdtsc				\n"		\
	"	movl	%%ecx, %%fs:0(%%rdi)	\n"		\
	"	movl	%%eax, %%fs:4(%%rdi)	\n"		\
	"	movq	%%rsp, %%fs:16(%%rdi)	\n"		\
	PMC_SEL_0						\
	RDPMC_0							\
	PMC_SEL_1						\
	RDPMC_1							\
	"2:					\n"		\
	:							\
	"=D" (addr), 						\
	"=c" (dummy)						\
	:							\
	"i" (sizeof (tracerecord_t)), 				\
	"i" (TRACEBUFFER_SIZE - 1),				\
	"c" ((event & 0xffff) | 0x8000), 			\
	"D" (0) 						\
	:							\
	"rax", "rdx");						\
    addr;							\
})


/**
 * Record value into event buffer at indicated location.
 *
 * @param addr		offset of event record
 * @param offset	offset within event record
 * @param item		value to be recorded
 */
#define TBUF_STORE_ITEM(addr, offset, item)			\
do {								\
    asm volatile (						\
	"	movq	%0, %%fs:("#offset"*8+24)(%%rdi)"	\
	:							\
	:							\
	"r" (item),						\
	"D" (addr));						\
} while (0)


#endif /* !__ARCH__AMD64__TRACEBUFFER_H__ */
