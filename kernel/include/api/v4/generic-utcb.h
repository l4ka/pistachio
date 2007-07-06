/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     api/v4/generic-utcb.h
 * Description:   Generic V4 UTCB access functions
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
 * $Id: generic-utcb.h,v 1.2 2006/10/20 16:29:11 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__GENERIC_UTCB_H__
#define __API__V4__GENERIC_UTCB_H__


INLINE void utcb_t::set_my_global_id(threadid_t tid)
{
    this->my_global_id = tid;
}

INLINE word_t utcb_t::get_user_defined_handle()
{
    return this->user_defined_handle;
}

INLINE void utcb_t::set_user_defined_handle(word_t handle)
{
    this->user_defined_handle = handle;
}

INLINE threadid_t utcb_t::get_pager()
{
    return this->pager;
}

INLINE void utcb_t::set_pager(threadid_t tid)
{
    this->pager = tid;
}

INLINE threadid_t utcb_t::get_exception_handler()
{
    return this->exception_handler;
}

INLINE void utcb_t::set_exception_handler(threadid_t tid)
{
    this->exception_handler = tid;
}

INLINE u8_t utcb_t::get_preempt_flags()
{
    return this->preempt_flags;
}

INLINE void utcb_t::set_preempt_flags(u8_t flags)
{
    this->preempt_flags = flags;
}

INLINE u8_t utcb_t::get_cop_flags()
{
    return this->cop_flags;
}

INLINE word_t utcb_t::get_error_code()
{
    return this->error_code;
}

INLINE void utcb_t::set_error_code(word_t err)
{
    this->error_code = err;
}

INLINE timeout_t utcb_t::get_xfer_timeout()
{
    return this->xfer_timeout;
}

INLINE void utcb_t::set_virtual_sender(threadid_t tid)
{
    this->virtual_sender = tid;
}

INLINE threadid_t utcb_t::get_virtual_sender()
{
    return this->virtual_sender;
}

INLINE threadid_t utcb_t::get_intended_receiver()
{
    return this->intended_receiver;
}


#endif /* !__API__V4__GENERIC_UTCB_H__ */
