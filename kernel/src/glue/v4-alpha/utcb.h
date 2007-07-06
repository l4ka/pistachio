/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/utcb.h
 * Description:   
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
 * $Id: utcb.h,v 1.6 2006/10/20 16:29:32 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_ALPHA__UTCB_H__
#define __GLUE__V4_ALPHA__UTCB_H__

#include INC_API(types.h)
#include INC_API(thread.h)

/* On ev6, the L1 cache size is 64 bytes, on ev4 and ev5m size is 32 bytes */

class utcb_t
{
public:
    bool allocate(threadid_t tid);
    void free() {this->my_global_id = NILTHREAD;}

public:
    threadid_t		my_global_id;		/* 0 */
    word_t		processor_no;		/* 8 */
    word_t		user_defined_handle;	/* 16 */
    threadid_t		pager;			/* 24 */
    threadid_t		exception_handler;	/* 32 */
    u8_t		preempt_flags;		/* 40 */
    u8_t		cop_flags;              /* 41 */
    u16_t		__reserved0;           
    u32_t               __reserved1;
    timeout_t		xfer_timeout;		/* 48 */
    word_t		error_code;		/* 56 */
    threadid_t		intended_receiver;	/* 64 */
    threadid_t		virtual_sender;		/* 72 */
    word_t              __padding0[6];          /* 80 .. 128 */
    word_t		mr[IPC_NUM_MR];		/* 128 .. 640 */
    word_t		br[IPC_NUM_BR];		/* 640 .. 904 */
    word_t              __padding1[15];         /* 904 .. 1024 */

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

} __attribute__((packed));

#include INC_API(generic-utcb.h)

/**
 * try to allocate a UTCB and mark it as used
 * @param tid the global ID of the associated thread
 * @return true on success, false if UTCB is already allocated
 */
INLINE bool utcb_t::allocate( threadid_t tid )
{
#if defined(CONFIG_SMP)
#warning utcb_t::allocate not SMP safe
#endif
    if(this->my_global_id.is_nilthread()) {
    	this->my_global_id.set_raw(tid.get_raw());
	return true;
    } else
	return false;
}

#endif /* !__GLUE__V4_ALPHA__UTCB_H__ */
