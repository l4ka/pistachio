/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006-2007,  Karlsruhe University
 *                
 * File path:     arch/x86/sync.h
 * Description:   synchronization primitives for x86
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
 * $Id$
 *
 ********************************************************************/
#ifndef __ARCH__X86__SYNC_H__
#define __ARCH__X86__SYNC_H__

#ifndef CONFIG_SMP
# error should not be included!
#endif


class spinlock_t
{
public:
    void init(word_t val = 0)
        { this->_lock = val; }
    void lock();
    void unlock()
	    { this->_lock = 0; }
    bool is_locked()
        { return this->_lock; }

public: // to allow initializers
    volatile word_t _lock;
};

#define DECLARE_SPINLOCK(name) extern spinlock_t name;
#define DEFINE_SPINLOCK(name) spinlock_t name = {_lock: 0}

#undef DEBUG_LOCK
#define SYNC_THRESHOLD	0x8000000

extern "C" void sync_debug (word_t lock);

INLINE void spinlock_t::lock()
{
    word_t dummy;
#if defined(DEBUG_LOCK)
    __asm__ __volatile__ (
        "1:                     		\n\t"
	"mov $1, %2				\n\t"
	"xchg	%1, %2				\n\t"
	"test	$0xff, %2			\n\t"
	"jnz	2f				\n\t"
        ".section .spinlock     		\n\t"
	"2:					\n\t"
	"mov $"MKSTR(SYNC_THRESHOLD)", %2	\n\t" 
	"3:					\n\t"
	"rep; nop				\n\t"
	"dec	%2				\n\t"
	"jnz 4f					\n\t"
	"lea %1, %0				\n\t"
	"push %0				\n\t"
	"call sync_debug			\n\t"
	"pop %0					\n\t"
	"4:					\n\t"
	"testb $1, %1				\n\t" 
	"jne 3b					\n\t"
        "jmp    1b              		\n\t"
        ".previous              		\n\t"
        : "=D" (dummy)
        : "m"(this->_lock), 
 	  "0"  ((word_t) 1)
	);
#else
    __asm__ __volatile__ (
        "1:                     		\n\t"
	"xchg	%1, %2				\n\t"
	"test	$0xff, %2			\n\t"
	"jnz	2f				\n\t"
        ".section .spinlock     		\n\t"
	"2:					\n\t"
	"rep; nop				\n\t"
        "testb $1, %1				\n\t"
        "jne    2b              		\n\t"
        "jmp    1b              		\n\t"
        ".previous              		\n\t"
        : "=D" (dummy)
        : "m"(this->_lock), 
 	  "0"  ((word_t) 1)
	);
#endif

}



#endif /* !__ARCH__X86__SYNC_H__ */
