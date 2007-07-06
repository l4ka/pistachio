/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     arch/mips64/sync.h
 * Description:   MIPS64 sychronization primitives
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
#ifndef __ARCH__MIPS64__SYNC_H__
#define __ARCH__MIPS64__SYNC_H__

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
#define DEFINE_SPINLOCK(name) spinlock_t name = ((spinlock_t) {{_lock: 0}})

DECLARE_SPINLOCK(printf_spin_lock);

INLINE void spinlock_t::lock (void)
{
    __asm__ __volatile__ (
	"   .set noreorder	\n"
	"   move    $4, %0	\n"
	"   1:			\n"
	"   ll	    $5, ($4)	\n"
	"   bnez    $5, 1b	\n"
	"   nop			\n"
	"   li	    $5, 1	\n"
	"   sc	    $5, ($4)	\n"
	"   beqz    $5, 1b	\n"
	"   nop			\n"
	"   .set reorder	\n"
	:: "r" (&this->_lock)
	: "$4", "$5"
    );
}

INLINE void spinlock_t::unlock (void)
{
    __asm__ __volatile__ (
	"sync;"
	::: "memory"
    );
    this->_lock = 0;
}


#endif /* !__ARCH__MIPS64__SYNC_H__ */
