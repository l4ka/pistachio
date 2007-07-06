/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006,  Karlsruhe University
 *                
 * File path:     api/v4/smp.h
 * Description:   multiprocessor handling
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
 * $Id: smp.h,v 1.14 2006/09/27 14:14:42 stoess Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__SMP_H__
#define __API__V4__SMP_H__

#include INC_API(types.h)
#include INC_API(tcb.h)

#if defined (CONFIG_SMP)

#define ON_CONFIG_SMP(x) do { x; } while(0)

/**
 * central SMP handler function; should be called in processor_sleep
 * deals with both, sync and async 
 */
void process_xcpu_mailbox();

/**
 * Architecture specific XCPU trigger function (IPI) processing XCPU
 * mailboxes 
 */
void smp_xcpu_trigger(cpuid_t cpu);


/**********************************************************************
 *
 *                  Asynchronous XCPU handling
 *
 **********************************************************************/

// maximum number of outstanding XCPU requests
#define MAX_MAILBOX_ENTRIES	32

class cpu_mb_entry_t;
typedef void (*xcpu_handler_t)(cpu_mb_entry_t *);

// mailbox entry
class cpu_mb_entry_t
{
public:
    void set(xcpu_handler_t handler, tcb_t * tcb, 
	     word_t param0, word_t param1, word_t param2)
	{
	    this->handler = handler;
	    this->tcb = tcb;
	    this->param[0] = param0;
	    this->param[1] = param1;
	    this->param[2] = param2;
	}


    void set(xcpu_handler_t handler, tcb_t * tcb,
	     word_t param0, word_t param1, word_t param2, word_t param3,
	     word_t param4, word_t param5, word_t param6, word_t param7)
	{
	    set (handler, tcb, param0, param1, param2);
	    this->param[3] = param3;
	    this->param[4] = param4;
	    this->param[5] = param5;
	    this->param[6] = param6;
	    this->param[7] = param7;
	}

public:
    xcpu_handler_t handler;
    tcb_t * tcb;
    word_t param[8];
};

/**
 * Asynchronous XCPU mailbox
 * currently not very efficient using a spin-lock for the mailbox
 */
class cpu_mb_t
{
public:
    void walk_mailbox();
    void dump_mailbox();
    cpu_mb_entry_t * alloc()
	{
	    lock.lock();
	    if ( ((first_free + 1) % MAX_MAILBOX_ENTRIES) == first_alloc )
	    {
		lock.unlock();
		return NULL;
	    }
	    unsigned idx = first_free;
	    first_free = (first_free + 1) % MAX_MAILBOX_ENTRIES;
	    return &entries[idx];
	}

    void commit (cpu_mb_entry_t * entry)
	{
	    lock.unlock();
	}

    bool enter (xcpu_handler_t handler, tcb_t * tcb,
		word_t param0, word_t param1, word_t param2)
	{
	    cpu_mb_entry_t * entry = alloc();
	    if (!entry) return false;

	    entry->set(handler, tcb, param0, param1, param2);
	    commit(entry);
	    return true;
	}

    bool enter (xcpu_handler_t handler, tcb_t * tcb,
		word_t param0, word_t param1, word_t param2, word_t param3,
		word_t param4, word_t param5, word_t param6, word_t param7)
	{
	    cpu_mb_entry_t * entry = alloc();
	    if (!entry) return false;

	    entry->set (handler, tcb,
			param0, param1, param2, param3,
			param4, param5, param6, param7);
	    commit(entry);
	    return true;
	}

private:
    unsigned first_alloc;
    unsigned first_free;
    spinlock_t lock;
    cpu_mb_entry_t entries[MAX_MAILBOX_ENTRIES] 
    __attribute__((aligned (CACHE_LINE_SIZE)));
} __attribute__ ((aligned (CACHE_LINE_SIZE)));


extern cpu_mb_t cpu_mailboxes[];
INLINE cpu_mb_t * get_cpu_mailbox (cpuid_t dst)
{
    ASSERT(dst < CONFIG_SMP_MAX_CPUS);
    return &cpu_mailboxes[dst];
}

INLINE void xcpu_request(cpuid_t dstcpu, xcpu_handler_t handler, 
			 tcb_t * tcb = NULL, 
			 word_t param0 = 0, word_t param1 = 0, 
			 word_t param2 = 0 )
{
    bool entered = get_cpu_mailbox(dstcpu)->enter(handler, tcb, 
						  param0, param1, param2);

    if (!entered)
    {
	printf("Failing XCPU requests are unimplemented\n");
	enter_kdebug("BUG");
	spin_forever();
    }

#ifndef CONFIG_SMP_IDLE_POLL
    /* trigger an IPI */
    smp_xcpu_trigger(dstcpu);
#endif
}

INLINE void xcpu_request(cpuid_t dstcpu, xcpu_handler_t handler, tcb_t * tcb,
			 word_t param0, word_t param1, 
			 word_t param2, word_t param3,
			 word_t param4 = 0, word_t param5 = 0,
			 word_t param6 = 0, word_t param7 = 0)
{
    bool entered =  get_cpu_mailbox(dstcpu)->enter(handler, tcb, 
						   param0, param1, param2, param3,
						   param4, param5, param6, param7);
    
    if (!entered)
    {
	printf("Failing XCPU requests are unimplemented\n");
	enter_kdebug("BUG");
	spin_forever();
    }
    
    
#ifndef CONFIG_SMP_IDLE_POLL
    /* trigger an IPI */
    smp_xcpu_trigger(dstcpu);
#endif
}


/**********************************************************************
 *
 *                   Synchronous XCPU handling
 *
 **********************************************************************/
#ifdef CONFIG_SMP_SYNC_REQUEST

/*
 * synchronous XCPU request handling, depends on the hardware
 * architecture. Needed e.g. on IA32 for TLB shoot-downs. See
 * api/v4/smp.cc for a detailed description. 
 */

class sync_entry_t : public cpu_mb_entry_t
{
public:
    void set_pending(cpuid_t cpu);
    void clear_pending(cpuid_t cpu);
    void ack(cpuid_t cpu);

    void handle_sync_requests();

public:
    word_t pending_mask;
    word_t ack_mask;
};

void sync_xcpu_request(cpuid_t dstcpu, xcpu_handler_t handler,
		       tcb_t * tcb = NULL, word_t param0 = 0, 
		       word_t param1 = 0, word_t param2 = 0);

#endif /* CONFIG_SMP_SYNC_REQUEST */

#include INC_GLUE(smp.h)

#else /* ! CONFIG_SMP */

#define ON_CONFIG_SMP(x) do { } while(0)

#endif /* CONFIG_SMP */

#endif /* !__API__V4__SMP_H__ */
