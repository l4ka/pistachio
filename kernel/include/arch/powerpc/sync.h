/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	arch/powerpc/sync.h
 * Description:	PowerPC SMP synchronization primitives.
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
 * $Id: sync.h,v 1.6 2003/09/24 19:04:30 skoglund Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__SYNC_H__
#define __ARCH__POWERPC__SYNC_H__

class spinlock_t
{
public:
    void init( word_t val=0 ) 
	{ this->_lock = val; }
    void lock();
    void unlock();

public: // to allow initializers
    volatile word_t _lock;
};

#define DECLARE_SPINLOCK(name) extern spinlock_t name;
#define DEFINE_SPINLOCK(name) spinlock_t name = ((spinlock_t) {{_lock: 0}})

// TODO: do I satisfy synchronization criteria for PowerPC?
// TODO: should i eieio when grabbing the lock?

INLINE void spinlock_t::lock()
{
    word_t old_val;

    // The only expense of a lwarx is that the CPU will snoop memory traffic.
    // So we should be able to use it in the spin.
    asm volatile ( "\n\
1:	lwarx	%0, 0, %1	/* Load the lock. */\n\
	cmpwi	0, %0, 0	/* Is the lock 0? */\n\
	bne-	1b		/* Retry the lock. */\n\
	stwcx.	%2, 0, %1	/* Try to store the new lock. */\n\
	bne-	1b		/* Retry if we failed to store. */\n\
	"
	: "=&r" (old_val)
	: "r" (&this->_lock), "r" (1)
	: "cr0", "memory"
	);
}

INLINE void spinlock_t::unlock()
{
    // Ensure memory ordering before we unlock.
    asm volatile ("eieio" : : : "memory");

    this->_lock = 0;
}

INLINE word_t ppc_load_reserve( addr_t addr )
{
    word_t val;
    asm volatile ("lwarx %0, 0, %1" : "=r" (val) : "r" (addr) );
    return val;
}

INLINE word_t ppc_store_reserve( addr_t addr, word_t val )
{
    word_t out;

    asm volatile (
	    "stwcx. %1, 0, %2 ;" 
	    "mfcr %0 ;"
	    : /* outputs */
	      "=r" (out)
	    : /* inputs */
	      "r" (val), "r" (addr) 
	    : /* clobbers */
	      "cr0", "memory"
	    );

    return (out >> 28);	// return condition register 0
}

INLINE word_t ppc_atomic_or( void volatile *addr, word_t mask )
{
    word_t tmp;

    asm volatile (
	    "1: lwarx %0, 0, %1 ;"
	    "or %0, %0, %2 ;"
	    "stwcx. %0, 0, %1 ;"
	    "bne- 1b ;"
	    : /* outputs */
	      "=&r" (tmp)
	    : /* inputs */
	      "r" (addr), "r" (mask)
	    : /* clobbers */
	      "cr0", "memory"
	    );

    return tmp;
}

INLINE word_t ppc_atomic_and( void volatile *addr, word_t mask )
{
    word_t tmp;

    asm volatile (
	    "1: lwarx %0, 0, %1 ;"
	    "and %0, %0, %2 ;"
	    "stwcx. %0, 0, %1 ;"
	    "bne- 1b ;"
	    : /* outputs */
	      "=&r" (tmp)
	    : /* inputs */
	      "r" (addr), "r" (mask)
	    : /* clobbers */
	      "cr0", "memory"
	    );

    return tmp;
}


#endif 	/* __ARCH__POWERPC__SYNC_H__ */
