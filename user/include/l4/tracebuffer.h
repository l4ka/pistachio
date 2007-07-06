/*********************************************************************
 *                
 * Copyright (C) 2007,  Karlsruhe University
 *                
 * File path:     l4/tracebuffer.h
 * Description:   Access to L4 kernel tracebuffer
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
 * $Id: tracebuffer.h,v 1.2 2007/01/23 14:12:50 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__TRACEBUFFER_H__
#define __L4__TRACEBUFFER_H__
#if defined(L4_TRACEBUFFER)

#include __L4_INC_ARCH(tracebuffer.h)

L4_INLINE void L4_Tbuf_IncCounter (L4_Word_t counter)
{
    __L4_TBUF_INCREASE_COUNTER (counter);
}

L4_INLINE void L4_Tbuf_RecordEvent_t0 (L4_ThreadId_t tid,
				       L4_Word_t event, char * str)
{
    L4_Word_t addr = __L4_TBUF_GET_NEXT_RECORD (tid, event);
    if (addr == 0)
	return;
    __L4_TBUF_STORE_ITEM (addr, 0, (L4_Word_t) str);
}

L4_INLINE void L4_Tbuf_RecordEvent_t1 (L4_ThreadId_t tid,
				       L4_Word_t event, char * str,
				       L4_Word_t p0)
{
    L4_Word_t addr = __L4_TBUF_GET_NEXT_RECORD (tid, event);
    if (addr == 0)
	return;
    __L4_TBUF_STORE_ITEM (addr, 0, (L4_Word_t) str);
    __L4_TBUF_STORE_ITEM (addr, 1, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent_t2 (L4_ThreadId_t tid,
				       L4_Word_t event, char * str,
				       L4_Word_t p0, L4_Word_t p1)
{
    L4_Word_t addr = __L4_TBUF_GET_NEXT_RECORD (tid, event);
    if (addr == 0)
	return;
    __L4_TBUF_STORE_ITEM (addr, 0, (L4_Word_t) str);
    __L4_TBUF_STORE_ITEM (addr, 1, p0);
    __L4_TBUF_STORE_ITEM (addr, 2, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent_t3 (L4_ThreadId_t tid,
				       L4_Word_t event, char * str,
				       L4_Word_t p0, L4_Word_t p1,
				       L4_Word_t p2)
{
    L4_Word_t addr = __L4_TBUF_GET_NEXT_RECORD (tid, event);
    if (addr == 0)
	return;
    __L4_TBUF_STORE_ITEM (addr, 0, (L4_Word_t) str);
    __L4_TBUF_STORE_ITEM (addr, 1, p0);
    __L4_TBUF_STORE_ITEM (addr, 2, p1);
    __L4_TBUF_STORE_ITEM (addr, 3, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent_t4 (L4_ThreadId_t tid,
				       L4_Word_t event, char * str,
				       L4_Word_t p0, L4_Word_t p1,
				       L4_Word_t p2, L4_Word_t p3)
{
    L4_Word_t addr = __L4_TBUF_GET_NEXT_RECORD (tid, event);
    if (addr == 0)
	return;
    __L4_TBUF_STORE_ITEM (addr, 0, (L4_Word_t) str);
    __L4_TBUF_STORE_ITEM (addr, 1, p0);
    __L4_TBUF_STORE_ITEM (addr, 2, p1);
    __L4_TBUF_STORE_ITEM (addr, 3, p2);
    __L4_TBUF_STORE_ITEM (addr, 3, p3);
}

L4_INLINE void L4_Tbuf_RecordEvent_0 (L4_Word_t event, char * str)
{
    L4_Tbuf_RecordEvent_t0 (L4_MyGlobalId (), event, str);
}

L4_INLINE void L4_Tbuf_RecordEvent_1 (L4_Word_t event, char * str,
				      L4_Word_t p0)
{
    L4_Tbuf_RecordEvent_t1 (L4_MyGlobalId (), event, str, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent_2 (L4_Word_t event, char * str,
				      L4_Word_t p0, L4_Word_t p1)
{
    L4_Tbuf_RecordEvent_t2 (L4_MyGlobalId (), event, str, p0, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent_3 (L4_Word_t event, char * str,
				      L4_Word_t p0, L4_Word_t p1,
				      L4_Word_t p2)
{
    L4_Tbuf_RecordEvent_t3 (L4_MyGlobalId (), event, str, p0, p1, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent_4 (L4_Word_t event, char * str,
				      L4_Word_t p0, L4_Word_t p1,
				      L4_Word_t p2, L4_Word_t p3)
{
    L4_Tbuf_RecordEvent_t4 (L4_MyGlobalId (), event, str, p0, p1, p2, p3);
}

#if defined(__cplusplus)
L4_INLINE void L4_Tbuf_RecordEvent (L4_ThreadId_t tid,
				    L4_Word_t event, char * str)
{
    L4_Tbuf_RecordEvent_0 (event, str);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_ThreadId_t tid,
				    L4_Word_t event, char * str,
				    L4_Word_t p0)
{
    L4_Tbuf_RecordEvent_1 (event, str, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_ThreadId_t tid,
				    L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1)
{
    L4_Tbuf_RecordEvent_2 (event, str, p0, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_ThreadId_t tid,
				    L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2)
{
    L4_Tbuf_RecordEvent_3 (event, str, p0, p1, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_ThreadId_t tid,
				    L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2, L4_Word_t p3)
{
    L4_Tbuf_RecordEvent_4 (event, str, p0, p1, p2, p3);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t event, char * str)
{
    L4_Tbuf_RecordEvent_t0 (L4_MyGlobalId (), event, str);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t event, char * str,
				    L4_Word_t p0)
{
    L4_Tbuf_RecordEvent_t1 (L4_MyGlobalId (), event, str, p0);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1)
{
    L4_Tbuf_RecordEvent_t2 (L4_MyGlobalId (), event, str, p0, p1);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2)
{
    L4_Tbuf_RecordEvent_t3 (L4_MyGlobalId (), event, str, p0, p1, p2);
}

L4_INLINE void L4_Tbuf_RecordEvent (L4_Word_t event, char * str,
				    L4_Word_t p0, L4_Word_t p1,
				    L4_Word_t p2, L4_Word_t p3)
{
    L4_Tbuf_RecordEvent_t4 (L4_MyGlobalId (), event, str, p0, p1, p2, p3);
}
#endif /* __cplusplus */

#else /* !TRACEBUFFER */
#define L4_Tbuf_RecordEvent_0(args...)
#define L4_Tbuf_RecordEvent_1(args...)
#define L4_Tbuf_RecordEvent_2(args...)
#define L4_Tbuf_RecordEvent_3(args...)
#define L4_Tbuf_RecordEvent_4(args...)
#define L4_Tbuf_RecordEvent(args...)
#endif

#endif /* !__L4__TRACEBUFFER_H__ */
