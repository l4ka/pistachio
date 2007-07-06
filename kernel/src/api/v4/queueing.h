/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     api/v4/queueing.h
 * Description:   tcb queue management
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
 * $Id: queueing.h,v 1.8 2005/06/03 16:13:45 joshua Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__QUEUEING_H__
#define __API__V4__QUEUEING_H__

#define ENQUEUE_LIST_TAIL(head, tcb, list)	\
do {						\
    if (head == NULL)				\
    {						\
	head = tcb;				\
	tcb->list.next = tcb->list.prev = tcb;	\
    }						\
    else					\
    {						\
	tcb->list.next = head;			\
	tcb->list.prev = head->list.prev;	\
	head->list.prev->list.next = tcb;	\
	head->list.prev = tcb;			\
    }						\
} while(0)

#define ENQUEUE_LIST_HEAD(head, tcb, list)	\
do {						\
    if (head == NULL)				\
	tcb->list.next = tcb->list.prev = tcb;	\
    else					\
    {						\
	tcb->list.next = head;			\
	tcb->list.prev = head->list.prev;	\
	head->list.prev->list.next = tcb;	\
	head->list.prev = tcb;			\
    }						\
    head = tcb;					\
} while(0)

#define DEQUEUE_LIST(head, tcb, list)			\
do {							\
    if (tcb->list.next == tcb)				\
    {							\
	head = NULL;					\
	tcb->list.next = tcb->list.prev = NULL;		\
    }							\
    else						\
    {							\
	if (head == tcb)				\
	    head = tcb->list.next;			\
	(tcb->list.next)->list.prev = tcb->list.prev;	\
	(tcb->list.prev)->list.next = tcb->list.next;	\
    }							\
} while(0)
    
#endif /* !__API__V4__QUEUEING_H__ */
