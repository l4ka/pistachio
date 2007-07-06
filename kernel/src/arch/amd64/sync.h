// Warning: This is just a copy of arch/ia32/sync.h
// I have no idea if it will work with AMD64


/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  Karlsruhe University
 *                
 * File path:     arch/amd64/sync.h
 * Description:   synchronization primitives for AMD64
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
 * $Id: sync.h,v 1.2 2006/09/27 10:24:38 stoess Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__AMD64__SYNC_H__
#define __ARCH__AMD64__SYNC_H__

#ifndef CONFIG_SMP
# error should not be included!
#endif

class spinlock_t 
{
public:
    void init(word_t val = 0)
	{ this->_lock= val; }
    void lock();
    void unlock();
    bool is_locked() { return _lock; }
    
public: // to allow initializers
    volatile word_t _lock;
};

#define DECLARE_SPINLOCK(name) extern spinlock_t name;
#define DEFINE_SPINLOCK(name) spinlock_t name = {_lock: 0}

INLINE void spinlock_t::lock()
{
    word_t dummy;
    __asm__ __volatile__ (
        "1:                     \n"
	"xchg	%1, %2		\n"
	"orq	$0, %2		\n"
	"jnz	2f		\n"
        ".section .spinlock     \n"
        "2:testb $1, %1		\n"
        "jne    2b              \n"
        "jmp    1b              \n"
        ".previous              \n"
        : "=r"(dummy)
        : "m"(this->_lock), "0"(1ULL));
}

INLINE void spinlock_t::unlock()
{
  this->_lock = 0;
}

#endif /* !__ARCH__AMD64__SYNC_H__ */
