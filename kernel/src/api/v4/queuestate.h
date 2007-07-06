/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     api/v4/queuestate.h
 * Description:   queue state
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
 * $Id: queuestate.h,v 1.8 2003/09/24 19:04:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__QUEUESTATE_H__
#define __API__V4__QUEUESTATE_H__

/* VU:
 * The separation of queue_state_t allows architecture specific
 * optimizations. For example, if a certain hardware architecture 
 * has some cheap bit-flipping operations, these can be used.
 */

#define IS_CONSISTENT (true)

class queue_state_t
{
public:
    enum state_e 
    {
	ready		= 1,
	wakeup		= 2,
	late_wakeup	= 4,
	wait		= 8,
	send		= 16,
	xcpu		= 32,
    };
    void init();
    void clear(state_e state);
    void set(state_e state);
    bool is_set(state_e state);

private:
    word_t state;
};

INLINE void queue_state_t::init()
{
    state = 0;
}

INLINE void queue_state_t::clear(state_e state)
{
    this->state &= ~((word_t)state);
    ASSERT(IS_CONSISTENT);
}

INLINE void queue_state_t::set(state_e state)
{
    this->state |= (word_t)state;
    ASSERT(IS_CONSISTENT);
}

INLINE bool queue_state_t::is_set(state_e state)
{
    /* generates better code when checking for the value */
    return (this->state & (word_t)state) == (word_t)state;
}

#endif /* !__API__V4__QUEUESTATE_H__ */

