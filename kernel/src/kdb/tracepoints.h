/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/tracepoints.h
 * Description:   Tracepoint interface
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
 * $Id: tracepoints.h,v 1.13 2007/01/22 21:02:07 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KDB__TRACEPOINTS_H__
#define __KDB__TRACEPOINTS_H__

#include <debug.h>
#include <kdb/linker_set.h>
#include <kdb/tracebuffer.h>

#define TP_DEFAULT				(1 << 0)
#define TP_DETAIL				(1 << 1)

#define TRACE_SCHEDULE_DETAILS(x...)		TRACEPOINT(SCHEDULE_DETAILS, x)
#define TRACE_IPC_DETAILS(x...) 		TRACEPOINT(IPC_DETAILS, x)
#define TRACE_XIPC_DETAILS(x...)		TRACEPOINT(IPC_XCPU_DETAILS, x) 
#define TRACE_IPC_ERROR(x...) 			TRACEPOINT(IPC_ERROR, x)
#define TRACE_IRQ_DETAILS(x...)			TRACEPOINT(INTERRUPT_DETAILS, x)

// avoid including api/smp.h for non-SMP case
#if !defined(CONFIG_SMP)
# define TP_CPU 0
#else
extern u16_t dbg_get_current_cpu();
# define TP_CPU dbg_get_current_cpu()
#endif

extern word_t dbg_get_current_tcb();
#define TP_TCB dbg_get_current_tcb()

class tracepoint_t
{
public:
    const char	*name;
    word_t	id;
    word_t	type;
    word_t	enabled;
    word_t	enter_kdb;
    word_t	counter[CONFIG_SMP_MAX_CPUS];

    void reset_counter ()
	{ for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; counter[cpu++] = 0); }
    
};

#define EXTERN_TRACEPOINT(tp)				\
    extern tracepoint_t __tracepoint_##tp

extern void init_tracepoints();

/*
 * Wrapper class for accessing tracepoint set.
 */

class tracepoint_list_t
{
public:
    linker_set_t	*tp_set;

    inline void reset (void)
	{ tp_set->reset (); }

    inline tracepoint_t * next (void)
	{ return (tracepoint_t *) tp_set->next (); }

    inline word_t size (void)
	{ return tp_set->size (); }

    inline tracepoint_t * get (word_t n)
	{ return (tracepoint_t *) tp_set->get (n); }
};

extern tracepoint_list_t tp_list;

#if defined(CONFIG_TRACEPOINTS)

#define DECLARE_TRACEPOINT(tp)							\
    tracepoint_t __tracepoint_##tp = { #tp, 0, TP_DEFAULT, 0, 0, { 0, } };	\
    PUT_SET (tracepoint_set, __tracepoint_##tp)

#define DECLARE_TRACEPOINT_DETAIL(tp)						\
    tracepoint_t __tracepoint_##tp = { #tp, 0, TP_DETAIL, 0, 0, { 0, } };	\
    PUT_SET (tracepoint_set, __tracepoint_##tp)


#define TRACEPOINT(tp, str, args...)				\
do {								\
    tracepoint_t *_tp = &__tracepoint_##tp;			\
    TBUF_REC_TRACEPOINT (_tp->type, _tp->id, str, ##args);	\
    _tp->counter[TP_CPU]++;					\
    if (_tp->enabled & (1UL << TP_CPU))				\
    {								\
	{ printf("tcb %t cpu %d: ", TP_TCB, TP_CPU);		\
	    printf(str, ##args); printf("\n");}			\
    }								\
    if (_tp->enter_kdb & (1UL << TP_CPU))			\
	enter_kdebug (#tp);					\
} while (0)


#define TRACEPOINT_NOTB(tp, code...)				\
do {								\
    tracepoint_t *_tp = &__tracepoint_##tp;			\
    tp->counter[TP_CPU]++;					\
    if (tp->enabled & (1UL << TP_CPU))				\
    {								\
	{code;}							\
    }								\
    if (tp->enter_kdb & (1UL << TP_CPU))			\
	enter_kdebug (#tp);					\
    								\
} while (0)

#define ENABLE_TRACEPOINT(tp, cpumask, kdbmask)	\
do {						\
    __tracepoint_##tp.enabled = cpumask;	\
    __tracepoint_##tp.enter_kdb = kdbmask;	\
} while (0)

#define TRACEPOINT_ENTERS_KDB(tp)		\
   (__tracepoint_##tp.enter_kdb)


#else /* !CONFIG_TRACEPOINTS */

#define DECLARE_TRACEPOINT(tp)		        \
    tracepoint_t __tracepoint_##tp = { #tp, 0, TP_DEFAULT, 0, 0, { 0, } };	

#define DECLARE_TRACEPOINT_DETAIL(tp)		\
    tracepoint_t __tracepoint_##tp = { #tp, 0, TP_DETAIL, 0, 0, { 0, } };	


#define TRACEPOINT(tp, str, args...)				\
    do {							\
	TBUF_REC_TRACEPOINT (__tracepoint_##tp.type, __tracepoint_##tp.id, str, ##args); \
} while (0)

#define TRACEPOINT_NOTB(tp, code...)

#define ENABLE_TRACEPOINT(tp, cpumask, kdbmask)
#define TRACEPOINT_ENTERS_KDB(tp) (0)

#endif


#endif /* !__KDB__TRACEPOINTS_H__ */
