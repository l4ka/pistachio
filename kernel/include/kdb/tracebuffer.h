/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
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

#include INC_ARCH(tracebuffer.h)

INLINE void tbuf_inc_counter (word_t counter)
{
    TBUF_INCREASE_COUNTER (counter);
}

INLINE void tbuf_record_event (word_t event, char * str)
{
    word_t addr = TBUF_GET_NEXT_RECORD (event);
    if (addr == 0)
	return;
    TBUF_STORE_ITEM (addr, 0, (word_t) str);
}

INLINE void tbuf_record_event (word_t event, char * str, word_t p0)
{
    word_t addr = TBUF_GET_NEXT_RECORD (event);
    if (addr == 0)
	return;
    TBUF_STORE_ITEM (addr, 0, (word_t) str);
    TBUF_STORE_ITEM (addr, 1, p0);
}

INLINE void tbuf_record_event (word_t event, char * str, word_t p0, word_t p1)
{
    word_t addr = TBUF_GET_NEXT_RECORD (event);
    if (addr == 0)
	return;
    TBUF_STORE_ITEM (addr, 0, (word_t) str);
    TBUF_STORE_ITEM (addr, 1, p0);
    TBUF_STORE_ITEM (addr, 2, p1);
}

INLINE void tbuf_record_event (word_t event, char * str, word_t p0,
			       word_t p1, word_t p2)
{
    word_t addr = TBUF_GET_NEXT_RECORD (event);
    if (addr == 0)
	return;
    TBUF_STORE_ITEM (addr, 0, (word_t) str);
    TBUF_STORE_ITEM (addr, 1, p0);
    TBUF_STORE_ITEM (addr, 2, p1);
    TBUF_STORE_ITEM (addr, 3, p2);
}

INLINE void tbuf_record_event (word_t event, char * str, word_t p0,
			       word_t p1, word_t p2, word_t p3)
{
    word_t addr = TBUF_GET_NEXT_RECORD (event);
    if (addr == 0)
	return;
    TBUF_STORE_ITEM (addr, 0, (word_t) str);
    TBUF_STORE_ITEM (addr, 1, p0);
    TBUF_STORE_ITEM (addr, 2, p1);
    TBUF_STORE_ITEM (addr, 3, p2);
    TBUF_STORE_ITEM (addr, 3, p3);
}

#else /* !CONFIG_TRACEBUFFER */
#define tbuf_inc_counter(counter)
#define tbuf_record_event(args...)
#endif


/*
 * Wrap tracepoint events with event type arguments
 */

#define TBUF_EV_TRACEPOINT		(1 << 15)
#define TBUF_EV_TRACEPOINT_TB		(1 << 14)

#if defined(CONFIG_TBUF_LIGHT)
# define TBUF_REC_TRACEPOINT(args...)
#else
# define TBUF_REC_TRACEPOINT(args...) \
    tbuf_record_event (TBUF_EV_TRACEPOINT, args)
#endif
#define TBUF_REC_TRACEPOINT_TB(args...) \
    tbuf_record_event (TBUF_EV_TRACEPOINT_TB, args)


#endif /* !__KDB__TRACEBUFFER_H__ */
