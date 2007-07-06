/*********************************************************************
 *                
 * Copyright (C) 2002-2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/utcb.h
 * Description:   UTCB for AMD64 compatibility mode
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
 * $Id: utcb.h,v 1.6 2006/10/21 02:02:01 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_AMD64__IA32__UTCB_H__
#define __GLUE__V4_AMD64__IA32__UTCB_H__

#include INC_GLUE(ia32/types.h)
#include INC_GLUE(ia32/thread.h)

namespace ia32 {

#undef __API__V4__GENERIC_UTCB_H__
#include "../../v4-ia32/utcb.h"

}

/* Parts of padding around 32-bit UTCB used by user_exchange_registers_32 */
class utcb_exreg32_t
{
public:
    word_t compatibility_mode;	/* -512 */
    ia32::word_t is_local;	/* -504 */
    ia32::threadid_t pager;	/* -500 */
    ia32::word_t control;	/* -496 */
} __attribute__((packed));

/* UTCB class that handles both 64-bit and 32-bit UTCBs */
class utcb_t
{
public:
    union {
	amd64::utcb_t amd64;
	struct {
	    word_t padding[32];
	    ia32::utcb_t ia32;
	};
	bool compatibility_mode;
	utcb_exreg32_t exreg32;
    };

public:
    bool is_compatibility_mode();
    void set_compatibility_mode(bool cm);

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

INLINE bool utcb_t::is_compatibility_mode()
{
    return EXPECT_FALSE(this->compatibility_mode);
}

INLINE void utcb_t::set_compatibility_mode(bool cm)
{
    this->compatibility_mode = cm;
}

INLINE void utcb_t::set_my_global_id(threadid_t tid)
{
    if (is_compatibility_mode())
	ia32.set_my_global_id(threadid_32(tid));
    else
	amd64.set_my_global_id(tid);
}

INLINE word_t utcb_t::get_user_defined_handle()
{
    if (is_compatibility_mode())
	return ia32.get_user_defined_handle();
    else
	return amd64.get_user_defined_handle();
}

INLINE void utcb_t::set_user_defined_handle(word_t handle)
{
    if (is_compatibility_mode())
	ia32.set_user_defined_handle(handle);
    else
	amd64.set_user_defined_handle(handle);
}

INLINE threadid_t utcb_t::get_pager()
{
    if (is_compatibility_mode())
	return threadid(ia32.get_pager());
    else
	return amd64.get_pager();
}

INLINE void utcb_t::set_pager(threadid_t tid)
{
    if (is_compatibility_mode())
	ia32.set_pager(threadid_32(tid));
    else
	amd64.set_pager(tid);
}

INLINE threadid_t utcb_t::get_exception_handler()
{
    if (is_compatibility_mode())
	return threadid(ia32.get_exception_handler());
    else
	return amd64.get_exception_handler();
}

INLINE void utcb_t::set_exception_handler(threadid_t tid)
{
    if (is_compatibility_mode())
	ia32.set_exception_handler(threadid_32(tid));
    else
	amd64.set_exception_handler(tid);
}

INLINE u8_t utcb_t::get_preempt_flags()
{
    if (is_compatibility_mode())
	return ia32.get_preempt_flags();
    else
	return amd64.get_preempt_flags();
}

INLINE void utcb_t::set_preempt_flags(u8_t flags)
{
    if (is_compatibility_mode())
	ia32.set_preempt_flags(flags);
    else
	amd64.set_preempt_flags(flags);
}

INLINE u8_t utcb_t::get_cop_flags()
{
    if (is_compatibility_mode())
	return ia32.get_cop_flags();
    else
	return amd64.get_cop_flags();
}

INLINE void utcb_t::set_error_code(word_t err)
{
    if (is_compatibility_mode())
	ia32.set_error_code(err);
    else
	amd64.set_error_code(err);
}

INLINE word_t utcb_t::get_error_code()
{
    if (is_compatibility_mode())
	return ia32.get_error_code();
    else
	return amd64.get_error_code();
}

INLINE timeout_t utcb_t::get_xfer_timeout()
{
    if (is_compatibility_mode())
	return timeout(ia32.get_xfer_timeout());
    else
	return amd64.get_xfer_timeout();
}

INLINE threadid_t utcb_t::get_intended_receiver()
{
    if (is_compatibility_mode())
	return threadid(ia32.get_intended_receiver());
    else
	return amd64.get_intended_receiver();
}

INLINE threadid_t utcb_t::get_virtual_sender()
{
    if (is_compatibility_mode())
	return threadid(ia32.get_virtual_sender());
    else
	return amd64.get_virtual_sender();
}

INLINE void utcb_t::set_virtual_sender(threadid_t tid)
{
    if (is_compatibility_mode())
	ia32.set_virtual_sender(threadid_32(tid));
    else
	amd64.set_virtual_sender(tid);
}

#endif /* !__GLUE__V4_AMD64__IA32__UTCB_H__ */
