/*********************************************************************
 *                
 * Copyright (C) 2002-2005, 2007-2010,  Karlsruhe University
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
#include INC_GLUE(ipc.h)
#include <debug.h>
#include <kdb/tracepoints.h>

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

#define IPC_NESTING_LEVEL	1	


class msg_tag_t 
{
public:
    inline msg_tag_t () { }
    inline msg_tag_t (word_t raw)
	{ this->raw = raw; }

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
	    msg_tag_t tag = 0; 
	    tag.set_error(); 
	    return tag;
	}

    static msg_tag_t tag(word_t typed, word_t untyped, word_t label)
	{
	    msg_tag_t tag;
	    tag.set(typed, untyped, label);
	    return tag;
	}

    static msg_tag_t irq_tag(word_t untyped = 0)
	{
	    return tag(0, untyped, -1UL << 4);
	}

   
    static msg_tag_t preemption_tag()
	{
	    return tag (0, 2, (-3UL << 4));
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
	{ return type == 4; }

    inline bool is_grant_item() 
	{ return type == 5; }

    inline bool is_string_item() 
	{ return (type & 4) == 0; }

    inline bool is_ctrlxfer_item() 
	{ return type == 6; }

    inline bool more_strings()
	{ return (continued); }

    inline bool get_string_cache_hints() 
	{ ASSERT(is_string_item()); return (type & 3); }

    inline word_t get_string_length()
	{ ASSERT(is_string_item()); return (length); }

    inline word_t get_string_ptr_count()
	{ ASSERT(is_string_item()); return (num_ptrs + 1); }

    inline bool is_string_compound()
	{ ASSERT(is_string_item()); return (continuation); }

    inline word_t get_snd_base()
	{ return raw & (~0x3ff); }

    inline bool more_ctrlxfer_items()
	{ return (continued); }

    
    inline word_t get_ctrlxfer_id()
	{ ASSERT(is_ctrlxfer_item()); return id; }
    
    inline word_t get_ctrlxfer_mask()
	{ ASSERT(is_ctrlxfer_item()); return mask; }
    
    inline void operator = (word_t raw) 
	{ this->raw = raw; }
public:
    union {
	word_t raw;
	union {
	    struct{
		BITFIELD3(word_t,
			  continued		: 1,
			  type			: 3,
						: (sizeof(word_t)*8) - 4);
		
	    };
	    struct{
		BITFIELD4(word_t,
						: 4,
			  num_ptrs		: 5,
			  continuation		: 1,
			  length		: (sizeof(word_t)*8) - 10);
	    };
	    struct{
		BITFIELD3(word_t,
			  			: 4,	
			  id			: 8,
			  mask			: (sizeof(word_t)*8) - 12);
	    };    
	} __attribute__((packed));
    };
};

class acceptor_t
{
public:
    inline acceptor_t () { }
    inline acceptor_t (word_t raw)
	{ this->raw = raw; }
    
    inline void clear()
	{ this->raw = 0; }

    inline void operator = (word_t raw) 
	{ this->raw = raw; }

    inline bool accept_strings()
	{ return x.strings; }

    inline bool accept_ctrlxfer()
	{ return x.ctrlxfer; }

    inline word_t get_rcv_window()
	{ return x.rcv_window << 4; }

    inline void set_rcv_window(fpage_t fpage)
	{ x.rcv_window = (fpage.raw >> 4); };

    fpage_t get_arch_specific_rcvwindow(tcb_t *dest);
    
public:
    union {
	word_t raw;
	struct {
	    BITFIELD4 (word_t,
		       strings		: 1,
		       ctrlxfer		: 1,
		       reserved		: 2,
		       rcv_window	: (sizeof(word_t)*8) - 4);
	} x;
    };
    
};

#if !defined(CONFIG_X_CTRLXFER_MSG)
#define IPC_NUM_SAVED_MRS	3
#else

#define IPC_NUM_SAVED_MRS	4
#define IPC_CTRLXFER_STDFAULTS	4

class ctrlxfer_item_t : public arch_ctrlxfer_item_t
{ 

public:
    /* members */
    msg_item_t item;
    union
    {
	word_t regs[];
    };

    static msg_item_t kernel_fault_item(word_t fault)
	{
	    msg_item_t item;
	    item.raw = 0;
	    item.continued = 0;
	    item.type = 6;
	    item.mask = 0x3ff;	
	    item.id = fault; // we operate with 0-based fault IDs
	    return item;
	}

    static msg_item_t fault_item(id_e id)
	{
	    msg_item_t item;
	    item.raw = 0;
	    item.continued = 0;
	    item.type = 6;
	    item.mask = (1 << num_hwregs[id]) - 1;	
	    item.id = id;
	    return item;	
	}

    static const void mask_hwregs(const word_t  id, word_t &val)
	{ val &= (1UL << num_hwregs[id])-1; }

#if defined(CONFIG_DEBUG)
    static const char* get_idname(const word_t id);
    static const char* get_hwregname(const word_t id, const word_t reg);
#endif

    static const word_t num_hwregs[id_max];
    static const word_t * const hwregs[id_max];
};

#endif


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
