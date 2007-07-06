/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/thread.h
 * Description:   thread ids for Compatibility Mode
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
 * $Id: thread.h,v 1.2 2006/10/20 14:46:44 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_AMD64__IA32__THREAD_H__
#define __GLUE__V4_AMD64__IA32__THREAD_H__

#include <debug.h>

#include INC_API(thread.h)

#include INC_GLUE(ia32/types.h)

#undef TID_GLOBAL_VERSION_BITS
#undef TID_GLOBAL_THREADNO_BITS
#undef TID_LOCAL_ID_ZERO_BITS
#undef TID_LOCAL_ID_BITS

#define TID_GLOBAL_VERSION_BITS		L4_GLOBAL_VERSION_BITS_32
#define TID_GLOBAL_THREADNO_BITS	L4_GLOBAL_THREADNO_BITS_32
#define TID_LOCAL_ID_ZERO_BITS		L4_LOCAL_ID_ZERO_BITS_32
#define TID_LOCAL_ID_BITS		L4_LOCAL_ID_BITS_32

namespace ia32 {

#undef __API__V4__THREAD_H__
#include INC_API(thread.h)

}

INLINE ia32::threadid_t threadid_32(threadid_t id)
{
    if (id.is_anythread()) {
	return ia32::threadid_t::anythread();
    } else if (id.is_local()) {
	if (id.is_anylocalthread()) {
	    return ia32::threadid_t::anylocalthread();
	} else {
	    return ia32::threadid(id.get_raw());
	}
    } else {
	return ia32::threadid_t::threadid(id.get_threadno(), id.get_version());
    }
}

INLINE threadid_t threadid(ia32::threadid_t id)
{
    if (id.is_anythread()) {
	return threadid_t::anythread();
    } else if (id.is_local()) {
	if (id.is_anylocalthread()) {
	    return threadid_t::anylocalthread();
	} else {
	    return threadid(id.get_raw());
	}
    } else if (id.is_interrupt()) {
	return threadid_t::irqthread(id.get_irqno());
    } else {
	return threadid_t::threadid(id.get_threadno(), id.get_version());
    }
}

#undef TID_GLOBAL_VERSION_BITS
#undef TID_GLOBAL_THREADNO_BITS
#undef TID_LOCAL_ID_ZERO_BITS
#undef TID_LOCAL_ID_BITS

#endif /* !__GLUE__V4_AMD64__IA32__THREAD_H__ */
