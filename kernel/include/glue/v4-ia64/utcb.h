/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/utcb.h
 * Description:   UTCB definition
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
 * $Id: utcb.h,v 1.14 2006/10/20 16:30:22 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__UTCB_H__
#define __GLUE__V4_IA64__UTCB_H__

#include <debug.h> // For UNIMPLEMENTED()

#include INC_API(thread.h)
#include INC_API(types.h)

class utcb_t
{
public:
    threadid_t		my_global_id;		// 0	(0)
    word_t		processor_no;		// 8	(1)
    word_t		user_defined_handle;	// 16	(2)
    threadid_t		pager;			// 24	(3)
    threadid_t		exception_handler;	// 32	(4)
    u8_t		preempt_flags;		// 40	(5)
    u8_t		cop_flags;		// 41
    u16_t		__rv1;			// 42
    u32_t		__rv2;			// 44
    timeout_t		xfer_timeout;		// 48	(6)
    threadid_t		intended_receiver;	// 56	(7)
    threadid_t		virtual_sender;		// 64	(8)
    word_t		error_code;		// 72	(9)

    word_t		br[IPC_NUM_BR];		// 80	(10)
    word_t		__padding1[5];		// 344	(43)

    word_t		mr[IPC_NUM_MR];		// 384	(48)
    word_t		__padding2[16];		// 896	(112)

public:
    bool allocate (threadid_t tid);
    void free (void);

public:
    void set_my_global_id(threadid_t tid);
    word_t get_user_defined_handle();
    void set_user_defined_handle(word_t handle);
    threadid_t get_pager();
    void set_pager(threadid_t tid);
    threadid_t get_exception_handler();
    void set_exception_handler(threadid_t tid);
    u8_t get_preempt_flags();
    void set_preempt_flags(u8_t flags);
    u8_t get_cop_flags();
    word_t get_error_code();
    void set_error_code(word_t err);
    timeout_t get_xfer_timeout();
    threadid_t get_intended_receiver();
    threadid_t get_virtual_sender();
    void set_virtual_sender(threadid_t tid);

};

#include INC_API(generic-utcb.h)

INLINE bool utcb_t::allocate (threadid_t tid)
{
    word_t ret;

    __asm__ __volatile__ (
	"	mov	ar.ccv = r0 ;;			\n"
	"	cmpxchg8.acq %0 = [%1], %2, ar.ccv	\n"
	:
	"=r" (ret)
	:
	"r" (&__padding1[0]),
	"r" (tid.get_raw ())
	:
	"ar.ccv");

    return !ret;
}

INLINE void utcb_t::free (void)
{
    this->__padding1[0] = NILTHREAD.get_raw ();
}


#endif /* !__GLUE__V4_IA64__UTCB_H__ */
