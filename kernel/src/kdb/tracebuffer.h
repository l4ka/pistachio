/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/tracebuffer.h
 * Description:   Kernel tracebuffer facility
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
 * $Id: tracebuffer.h,v 1.3 2007/02/05 14:44:05 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KDB__TRACEBUFFER_H__
#define __KDB__TRACEBUFFER_H__



#if defined(CONFIG_TRACEBUFFER)

#define TB_DEFAULT				(1 << 0)
#define TB_USERID_START				(100)
/*
 * Wrap tracepoint events with event type arguments
 */

extern void tbuf_dump (word_t count, word_t usec, word_t tp_id = 0, word_t cpumask=~0UL);

#define DEBUG_KERNEL_DETAILS
#if defined(DEBUG_KERNEL_DETAILS)
#define TBUF_DEFAULT_MASK			(0xFFFFFFFF)
#else
#define TBUF_DEFAULT_MASK			(0xFFFFFFFF & ~(TP_DETAIL << 16))
#endif

#include INC_ARCH(tracebuffer.h)
#include <stdarg.h>	/* for va_list, ... comes with gcc */

INLINE void tbuf_inc_counter (word_t counter)
{
    TBUF_INCREASE_COUNTER (counter);
}

#define tbuf_record_event(type, tpid, str, args...)			\
    __tbuf_record_event(type, tpid, str, ##args, TRACEBUFFER_MAGIC);

INLINE void __tbuf_record_event(word_t type, word_t tpid, const char *str, ...)
{

    va_list args;
    word_t arg;
    
    word_t addr = TBUF_GET_NEXT_RECORD (type, tpid);
    
    if (addr == 0)
	return;
   
    TBUF_STORE_STR  (addr, str);

    va_start(args, str);
    
    word_t i;
    for (i=0; i < tracerecord_t::num_args; i++)
    {
	arg = va_arg(args, word_t);
	if (arg == TRACEBUFFER_MAGIC)
	    break;
	
	TBUF_STORE_DATA(addr, i, arg);
	
    }
    
    va_end(args);
}

#else /* !CONFIG_TRACEBUFFER */
#define tbuf_inc_counter(counter)
#define tbuf_record_event(args...)
#endif


#if defined(CONFIG_TBUF_LIGHT)
# define TBUF_REC_TRACEPOINT(tptype, tpid, str, args...)
#else

# define TBUF_REC_TRACEPOINT(tptype, tpid, str, args...)	\
    tbuf_record_event (tptype, tpid, str, ##args)
#endif

#endif /* !__KDB__TRACEBUFFER_H__ */
