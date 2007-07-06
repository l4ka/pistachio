/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/tracepoints.h
 * Description:	Defines a macro to watch the stack lower water point,
 * 		using the tracepoint infrastructure.
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
 * $Id: tracepoints.h,v 1.6 2003/09/24 19:04:51 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__TRACEPOINTS_H__
#define __GLUE__V4_POWERPC__TRACEPOINTS_H__

#include <kdb/tracepoints.h>

#if defined(CONFIG_TRACEPOINTS)

#define TRACEEXTERN(tp) extern tracepoint_t __tracepoint_##tp

#define TRACESTACK(tp)						\
do {								\
    word_t stack;						\
    asm ("mr %0, 1" : "=r" (stack) );				\
    if( !__tracepoint_##tp.counter ||				\
	    ((stack & ~KTCB_MASK) < __tracepoint_##tp.counter))	\
	__tracepoint_##tp.counter = stack & ~KTCB_MASK;		\
    ASSERT( (stack & ~KTCB_MASK) > sizeof(tcb_t) );		\
} while (0)

#define TRACEMIN(tp,val)					\
do {								\
    if( !__tracepoint_##tp.counter ||				\
	    (val < __tracepoint_##tp.counter) )			\
	__tracepoint_##tp.counter = val;			\
} while (0)

#define TRACEMAX(tp,val)					\
do {								\
    if( !__tracepoint_##tp.counter ||				\
	    (val > __tracepoint_##tp.counter) )			\
	__tracepoint_##tp.counter = val;			\
} while (0)

#else

#define TRACESTACK(tp)
#define TRACEMIN(tp)
#define TRACEMAX(tp)
#define TRACEEXTERN(tp)

#endif	/* CONFIG_DEBUG */

#endif	/* __GLUE__V4_POWERPC__TRACEPOINTS_H__ */

