/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:     l4/amd64/tracebuffer.h
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
#ifndef __L4__AMD64__TRACEBUFFER_H__
#define __L4__AMD64__TRACEBUFFER_H__

# define __PLUS32	+ 32

/*
 * Access to stack pointer, timestamp, and performance monitoring counters
 */
#define __L4_TBUF_RDTSC	"	rdtsc					\n"	\
			"	shl	$32, %%rdx			\n"	\
			"	movl	%%eax, %%edx			\n"	\
			"	mov	%2, %%fs:2*%c9(%0)		\n"	
    

#if defined(L4_PERFMON)
#define __L4_TBUF_RDPMC_0   "	rdpmc					\n"	\
			    "	shl	$32, %%rdx			\n"	\
			    "	movl	%%eax, %%edx			\n"	\
			    "	mov	%2, %%fs:4*%c9(%0)		\n"	

#define __L4_TBUF_RDPMC_1   "	rdpmc					\n"	\
			    "	shl	$32, %%rdx			\n"	\
			    "	movl	%%eax, %%edx			\n"	\
			    "	mov	%2, %%fs:5*%c9(%0)		\n"	


# if !defined(L4_CONFIG_CPU_AMD64_P4)
#  define __L4_TBUF_PMC_SEL_0 "	xorl	%%ecx, %%ecx		\n"
#  define __L4_TBUF_PMC_SEL_1 "	inc	%%ecx			\n"
# elif defined(L4_CONFIG_CPU_AMD64_P4)
#  define __L4_TBUF_PMC_SEL_0 "	movl	$12, %%ecx		\n"
#  define __L4_TBUF_PMC_SEL_1 "	add	$2, %%ecx		\n"
# endif

#else /* L4_PERFMON */

# define __L4_TBUF_PMC_SEL_0
# define __L4_TBUF_PMC_SEL_1
# define __L4_TBUF_RDPMC_0
# define __L4_TBUF_RDPMC_1

#endif /* L4_PERFMON */

#include __L4_INC_ARCH(../ia32/tracebuffer.h)



#endif /* !__L4__AMD64__TRACEBUFFER_H__ */
