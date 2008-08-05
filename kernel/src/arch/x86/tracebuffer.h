/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2008,  Karlsruhe University
 *                
 * File path:     arch/x86/tracebuffer.h
 * Description:   IA32 specific tracebuffer
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

#ifndef __ARCH__X86__TRACEBUFFER_H__
#define __ARCH__X86__TRACEBUFFER_H__

#include INC_ARCH_SA(tracebuffer.h)

#define TRACEBUFFER_SIZE	(4 * 1024 * 1024)

/**
 * A tracebuffer record indicates the type of event, the time of the
 * event, the current thread, a number of event specific parameters,
 * and potentially the current performance counters.
 */
class tracerecord_t
{
public:
    static const word_t num_args = 9;

    struct {
	word_t		utype	: 16;
	word_t		ktype	: 16;
	word_t			: BITS_WORD-32;
	word_t		cpu	: 16;
	word_t		id	: 16;
	word_t			: BITS_WORD-32;
    };
    word_t		tsc;
    word_t		thread;
    word_t		pmc0;
    word_t		pmc1;
    const char *	str;
    word_t		arg[num_args];

    bool is_kernel_event (void) { return ktype && ! utype; }
    word_t get_type (void) { return (utype == 0) ? ktype : utype; }

    friend class kdb_t;
    
};


/**
 * The tracebuffer is a region in memory accessible by kernel and user
 * through the FS segment.  The first words of the region indicate the
 * current tracebuffer record and which events should be recorded.
 * The remaining part of the region hold the counters and the
 * tracebuffer records.
 */
class tracebuffer_t
{
    word_t	magic;
    word_t	current;
    word_t	mask;
    word_t	sizemask;
    word_t	__pad[4];
    word_t	counters[8];
    tracerecord_t tracerecords[];

public:
    
    enum offset_e
    {
	ofs_counters	= sizeof(word_t) * 8,
    };

    void initialize (void)
	{
	    magic = TRACEBUFFER_MAGIC;
	    current = 0;
	    mask = TBUF_DEFAULT_MASK;
	    sizemask = TRACEBUFFER_SIZE-1;
	}

    bool is_valid (void) { return magic == TRACEBUFFER_MAGIC; }

    friend class tbuf_handler_t;
};

INLINE tracebuffer_t * get_tracebuffer (void)
{
    extern tracebuffer_t * tracebuffer;
    return tracebuffer;
}


/*
 * Access to performance monitoring counters
 */

# if defined(CONFIG_TBUF_PERFMON) 

#   if defined(CONFIG_CPU_X86_I686) || defined(CONFIG_CPU_X86_K8)
#       define TBUF_PMC_SEL_0		"       xor  %1, %1		\n"
#       define TBUF_PMC_SEL_1		"	mov  $1, %1		\n"
#   elif defined(CONFIG_CPU_X86_P4) 
        /* PMC_MSR_IQ_COUNTER 0 and 2 */
#       define TBUF_PMC_SEL_0	        "	mov	$12, %1		\n"
#       define TBUF_PMC_SEL_1	        "	add	$ 2, %1		\n"
#   else
#       define TBUF_PMC_SEL_0
#       define TBUF_PMC_SEL_1
#   endif

# else

# define TBUF_PMC_SEL_0
# define TBUF_PMC_SEL_1
# define TBUF_RDPMC_0
# define TBUF_RDPMC_1

# endif /* defined(CONFIG_TBUF_PERFMON) */


/*
 * Make sure cmpxchg is atomic
 */

#if defined(CONFIG_SMP)
# define TBUF_LOCK "lock;"
#else
# define TBUF_LOCK
#endif


/**
 * Start recording new event into tracebuffer.
 *
 * @param event		type of event
 *
 * @returns index to current event record
 */
#define TBUF_GET_NEXT_RECORD(type, id)					\
    ({									\
	word_t dummy, addr;						\
	asm volatile (							\
	    /* Check whether to filter the event */			\
	    "	mov	%%fs:2*%c9, %3			\n"		\
	    "	and	%1, %3				\n"		\
	    "	jz	2f				\n"		\
	    "	or	%2, %1				\n"		\
									\
	    /* Get record offset into EDI */				\
	    "1:	mov	%%fs:1*%c9, %3			\n"		\
	    "	mov	%8, %0				\n"		\
	    "	mov	%0, %2				\n"		\
	    "	add	%3, %0				\n"		\
	    "	and	$"MKSTR(TRACEBUFFER_SIZE-1)", %0\n"		\
	    "	cmovz	%2, %0			      	\n"		\
	    TBUF_LOCK							\
	    "	cmpxchg	%0, %%fs:1*%c9			\n"		\
	    "	jnz	1b				\n"		\
 									\
	    /* Store type, cpu, id, thread, counters */			\
	    "	mov	%1, %2			        \n"		\
	    "	xorw	%%cx, %%cx			\n"		\
	    "	movl	%%ecx, %%fs:(%0)		\n"		\
	    "	shl	$16, %%edx			\n"		\
	    "	lea	__idle_tcb, %3			\n"		\
	    "	movw 	"MKSTR(OFS_TCB_CPU)"(%3), %%dx	\n"		\
	    "	movl	%%edx, %%fs:1*%c9(%0)		\n"		\
	    TBUF_PMC_SEL_0						\
	    TBUF_RDPMC_0						\
	    TBUF_PMC_SEL_1						\
	    TBUF_RDPMC_1						\
	    TBUF_RDTSC							\
	    TBUF_SP							\
	    "2:						\n"		\
	    :								\
		"=D" (addr),				/* 0  */	\
		"=c" (dummy),				/* 1  */	\
		"=d" (dummy),				/* 2  */	\
		"=a" (dummy)				/* 3  */	\
	    :								\
		"0" (0),				/* 4  */	\
		"1" ((type & 0xffff)<<16),		/* 5  */	\
		"2" (id & 0xffff),			/* 6  */	\
		"3" (0),				/* 7  */	\
		"i" (sizeof (tracerecord_t)),		/* 8  */	\
		"i" (sizeof(word_t))			/* 9  */	\
	    );								\
	addr;								\
    })




/**
 * Increase a tracebuffer counter.
 *
 * @param ctr		counter number
 */
#define TBUF_INCREASE_COUNTER(ctr)				\
do {								\
    asm volatile (						\
	TBUF_LOCK						\
	"	inc	%%fs:8*%c1(%0)		\n"		\
	:							\
	:							\
	"r" ((ctr & 0x7) * 4),					\
	"i" (sizeof(word_t)));					\
} while (0)


/**
 * Record item into event buffer at indicated location.
 *
 * @param addr		offset of event record
 * @param offset	string to be recorded
 */
#define TBUF_STORE_STR(addr, str)				\
do {								\
    asm volatile (						\
	"mov   %0, %%fs:6*%c2(%1)\n"				\
	:							\
	:							\
	"r" (str),						\
	"D" (addr),						\
	"i" (sizeof(word_t)));					\
} while (0)


/**
 * Record arguments into event buffer at indicated location.
 *
 * @param addr		offset of event record
 * @param offset	offset within event record
 * @param item		value to be recorded
 */
#define TBUF_STORE_DATA(addr, offset, item)			\
do {								\
    word_t dummy;						\
    asm volatile (						\
	"mov  %2, %%fs:(%1)\n"					\
	:							\
	"=D" (dummy)						\
	:							\
	"0" (addr + (7 + offset) * sizeof(word_t)),		\
	"r" (item));						\
} while (0)




#endif /* !__ARCH__X86__TRACEBUFFER_H__ */
