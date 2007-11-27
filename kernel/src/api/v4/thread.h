/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     api/v4/thread.h
 * Description:   thread ids
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
 * $Id: thread.h,v 1.19 2006/10/18 11:24:22 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__THREAD_H__
#define __API__V4__THREAD_H__

#include INC_API(config.h)
#include INC_API(kernelinterface.h)

#if !defined(TID_GLOBAL_VERSION_BITS)
#define TID_GLOBAL_VERSION_BITS		L4_GLOBAL_VERSION_BITS
#define TID_GLOBAL_THREADNO_BITS	L4_GLOBAL_THREADNO_BITS
#define TID_LOCAL_ID_ZERO_BITS		L4_LOCAL_ID_ZERO_BITS
#define TID_LOCAL_ID_BITS		L4_LOCAL_ID_BITS
#endif /* !defined(TID_GLOBAL_VERSION_BITS) */

class threadid_t
{
public:
    static threadid_t anythread()
    {
	threadid_t tid;
	tid.raw = (word_t) (-1UL);
	return tid;
    };

    static threadid_t anylocalthread()
    {
	threadid_t tid;
	tid.local.zero = tid.local.id = 0;
	tid.local.id -= 1;
	return tid;
    }
    
    static threadid_t nilthread()
    {
	threadid_t tid;
	tid.raw = 0;
	return tid;
    }

    static threadid_t irqthread(word_t irq)
    {
	threadid_t tid;
	tid.global.version = 1;
	tid.global.threadno = irq;
	return tid;
    }

    static threadid_t threadid(word_t threadno, word_t version)
    {
	threadid_t tid;
	tid.global.version = version;
	tid.global.threadno = threadno;
	return tid;
    }

    inline bool is_global()	{ return local.zero != 0;	}
    bool is_local()		{ return local.zero == 0;	}
    bool is_interrupt()		{ return (global.version == 1) &&
					 (global.threadno < get_kip()->thread_info.get_system_base()); }

    /* check for specific (well known) thread ids */
    bool is_nilthread()		{ return this->raw == 0; }
    bool is_anythread()		{ return this->raw == (word_t) (-1UL); }
    bool is_anylocalthread()	{ return this->raw == threadid_t::anylocalthread().raw; }

    word_t get_threadno()	{ return global.threadno; }
    word_t get_version()	{ return global.version; }
    word_t get_irqno()		{ return get_threadno(); }
    void set_global_id(word_t threadno, word_t version);

    word_t get_raw()		{ return this->raw; }
    void set_raw(word_t raw)	{ this->raw = raw; }

    /* operators */
    bool operator == (const threadid_t & tid)
    {
	return this->raw == tid.raw;
    }

    bool operator != (const threadid_t & tid)
    {
	return this->raw != tid.raw;
    }

private:
    union {
	word_t raw;

	struct {
	    BITFIELD2( word_t,
		       zero	: TID_LOCAL_ID_ZERO_BITS,
		       id	: TID_LOCAL_ID_BITS );
	} local;

	struct {
	    BITFIELD2( word_t,
		       version	: TID_GLOBAL_VERSION_BITS,
		       threadno	: TID_GLOBAL_THREADNO_BITS );
	} global;
    };
} __attribute__((packed));

INLINE void threadid_t::set_global_id(word_t threadno, word_t version)
{
    global.threadno = threadno;
    global.version = version;
}

INLINE threadid_t threadid(word_t rawid)
{
    threadid_t t;
    t.set_raw(rawid);
    return t;
}

/* special thread ids */
#define NILTHREAD	(threadid_t::nilthread())
#define ANYTHREAD	(threadid_t::anythread())
#define ANYLOCALTHREAD	(threadid_t::anylocalthread())


#endif /* !__API__V4__THREAD_H__ */
