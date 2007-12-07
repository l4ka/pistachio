/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	sync.h
 * Description:	Synchronization primitives.
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
 * $Id: sync.h,v 1.7 2003/09/24 19:04:24 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __SYNC_H__
#define __SYNC_H__


INLINE void memory_barrier()
{
    __asm__ __volatile__ ("" ::: "memory");
}

#ifndef CONFIG_SMP

#define DEFINE_SPINLOCK(name) spinlock_t name
#define DECLARE_SPINLOCK(name) extern spinlock_t name

class spinlock_t {
public:
    void lock() {}
    void unlock() {}
    void init() {}
    bool is_locked() { return false; }
};


#else /* CONFIG_SMP */
/* 
 * for smp we need a platform specific file 
 * herewith we avoid rewriting empty spinlocks 
 * for each new platform again and again and again.
 */

#include INC_ARCH(sync.h)


class lockstate_t {
public:
    union {
	word_t raw;
	struct {
	    BITFIELD3(word_t,
		      enabled : 1,
		      promote : 1,
		      demote  : 1);
	} X;
    } flags;
    word_t rcu_epoch;

public:
    void init(bool enabled) 
	{
	    flags.raw = 0;
	    flags.X.enabled = enabled;
	}

    bool is_enabled() {
	//return is_active();
	return flags.X.enabled;
    }

    bool is_active() {
	//return true;
	return flags.raw;
    }
};


#endif

#endif /* !__SYNC_H__ */
