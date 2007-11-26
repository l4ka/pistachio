/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007,  Karlsruhe University
 *                
 * File path:     api/v4/ipc.h
 * Description:   IPC declarations
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
 * $Id: ipc.h,v 1.24 2006/10/19 22:57:34 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__IPC_H__
#define __API__V4__IPC_H__

#include INC_API(fpage.h)
#include <debug.h>

class tcb_t;


/**
 * Error codes
 */
#define ERR_IPC_TIMEOUT				(1)
#define ERR_IPC_NON_EXISTING			(2)
#define ERR_IPC_CANCELED			(3)
#define ERR_IPC_MSG_OVERFLOW(off)		(4 + ((off) << 3))
#define ERR_IPC_XFER_TIMEOUT_CURRENT(off)	(5 + ((off) << 3))
#define ERR_IPC_XFER_TIMEOUT_PARTNER(off)	(6 + ((off) << 3))
#define ERR_IPC_ABORTED(off)			(7 + ((off) << 3))

/**
 * Error encoding
 */
#define IPC_SND_ERROR(err)		((err << 1) | 0)
#define IPC_RCV_ERROR(err)		((err << 1) | 1)

/**
 * MR0 values
 */
#define IPC_MR0_PROPAGATED		(1 << 12)
#define IPC_MR0_REDIRECTED		(1 << 13)
#define IPC_MR0_XCPU			(1 << 14)
#define IPC_MR0_ERROR			(1 << 15)
#define IPC_MR0_PAGEFAULT		((-2UL) << 4)

#define IPC_NUM_SAVED_MRS		3

class msg_tag_t 
{
public:
    word_t get_label() { return x.label; }
    word_t get_typed() { return x.typed; }
    word_t get_untyped() { return x.untyped; }

    void clear_flags() { raw &= ~(0xf << 12);}
    void clear_receive_flags() { raw &= ~(0xe << 12); }

    void set(word_t typed, word_t untyped, word_t label)
	{
	    this->raw = 0;
	    this->x.typed = typed;
	    this->x.untyped = untyped;
	    this->x.label = label;
	}
	    
    bool is_error() { return x.error; }
    void set_error() { x.error = 1; }
    
    bool is_redirected() { return x.redirected; }
    void set_redirected() { x.redirected = 1; }
    
    bool is_propagated() { return x.propagated; }
    void set_propagated(bool val = true) { x.propagated = val; }

    bool is_xcpu() { return x.xcpu; }
    void set_xcpu() { x.xcpu = 1; }

    static msg_tag_t error_tag() 
	{ 
	    msg_tag_t tag; 
	    tag.raw = 0; 
	    tag.set_error(); 
	    return tag;
	}

    static msg_tag_t tag(word_t typed, word_t untyped, word_t label)
	{
	    msg_tag_t tag;
	    tag.set(typed, untyped, label);
	    return tag;
	}

    static msg_tag_t irq_tag()
	{
	    return tag(0, 0, -1UL << 4);
	}

    static msg_tag_t preemption_tag()
	{
	    return tag (0, 0, -3UL << 4);
	}

    static msg_tag_t pagefault_tag(bool read, bool write, bool exec)
	{
	    return tag (0, 2, (-2UL << 4) | 
			(read  ? 1 << 2 : 0) | 
			(write ? 1 << 1 : 0) | 
			(exec  ? 1 << 0 : 0));
	}
public:
    union {
	word_t raw;
	struct {
	    BITFIELD7(word_t,
		      untyped		: 6,
		      typed		: 6,
		      propagated	: 1,
		      redirected	: 1,
		      xcpu	       	: 1,
		      error		: 1,
		      label		: BITS_WORD - 16);
	} x;
    };
};

INLINE msg_tag_t msgtag (word_t rawtag)
{
    msg_tag_t t;
    t.raw = rawtag;
    return t;
}

class msg_item_t
{
public:
    inline bool is_map_item() 
	{ return x.type == 4; }

    inline bool is_grant_item() 
	{ return x.type == 5; }

    inline bool is_string_item() 
	{ return (x.type & 4) == 0; }

    inline bool more_strings()
	{ return (x.continued); }

    inline bool get_string_cache_hints() 
	{ ASSERT(is_string_item()); return (x.type & 3); }

    inline word_t get_string_length()
	{ ASSERT(is_string_item()); return (x.length); }

    inline word_t get_string_ptr_count()
	{ ASSERT(is_string_item()); return (x.num_ptrs + 1); }

    inline bool is_string_compound()
	{ ASSERT(is_string_item()); return (x.continuation); }

    inline word_t get_snd_base()
	{ return raw & (~0x3ff); }

    inline void operator = (word_t raw) 
	{ this->raw = raw; }
public:
    union {
	word_t raw;
	struct {
	    BITFIELD5(word_t,
		      continued		: 1,
		      type		: 3,
		      num_ptrs		: 5,
		      continuation	: 1,
		      length		: (sizeof(word_t)*8) - 10);
	} x __attribute__((packed));
    };
};

class acceptor_t
{
public:
    inline void clear()
	{ this->raw = 0; }

    inline void operator = (word_t raw) 
	{ this->raw = raw; }

    inline bool accept_strings()
	{ return x.strings; }

    inline word_t get_rcv_window()
	{ return x.rcv_window << 4; }

    inline void set_rcv_window(fpage_t fpage)
	{ x.rcv_window = (fpage.raw >> 4); };

    fpage_t get_arch_specific_rcvwindow(tcb_t *dest);
    
public:
    union {
	word_t raw;
	struct {
	    BITFIELD3(word_t,
		      strings		: 1,
		      reserved		: 3,
		      rcv_window	: (sizeof(word_t)*8) - 4);
	} x;
    };
};


/**
 * Implements all non-untyped transfers
 *
 * @param src		Source tcb the message is sent from
 * @param dst		Destination tcb the message is sent to
 * @param msgtag	Message tag of the message being transferred
 *
 * @returns the message tag
 */
msg_tag_t extended_transfer(tcb_t * src, tcb_t * dst, msg_tag_t msgtag);

#endif /* !__API__V4__IPC_H__ */
