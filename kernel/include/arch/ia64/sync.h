/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/sync.h
 * Description:   IA64 sychronization primitives
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
 * $Id: sync.h,v 1.4 2003/09/25 16:59:57 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__SYNC_H__
#define __ARCH__IA64__SYNC_H__

#include <debug.h>

class spinlock_t 
{
public:
    void init (word_t val = 0)
	{ this->_lock= val; }
    void lock (void);
    void unlock (void);
    
public: // to allow initializers
    volatile word_t _lock;
};

#define DECLARE_SPINLOCK(name) extern spinlock_t name;
#define DEFINE_SPINLOCK(name) spinlock_t name = {_lock: 0}

DECLARE_SPINLOCK(printf_spin_lock);

INLINE void spinlock_t::lock (void)
{
#if 0
    if (this != (&printf_spin_lock) && _lock)
	printf ("spinlock %p locked\n", this);
#endif
    volatile register word_t * lock_addr asm ("r31") = &this->_lock;

    __asm__ __volatile__ (
	"1:	mov	r30 = 1					\n"
	"	mov	ar.ccv = r0				\n"
	"	;;						\n"
	"	cmpxchg8.acq r30 = [%0], r30, ar.ccv		\n"
	"	;;						\n"
	"	cmp.ne	p15,p0 = r30, r0			\n"
	"	;;						\n"
	"(p15)	movl	r30 = 1b				\n"
	"(p15)	br.spnt.few ia64_spinlock_contention		\n"
	:
	: "r" (lock_addr)
	: "ar.ccv", "p15", "b7", "r30", "memory");
}

INLINE void spinlock_t::unlock (void)
{
    __asm__ __volatile__ (
	"	st8.rel	[%0] = r0				\n"
	"	;;						\n"
	:
	: "r" (&this->_lock));
}


#endif /* !__ARCH__IA64__SYNC_H__ */
