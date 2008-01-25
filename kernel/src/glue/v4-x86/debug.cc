/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2006-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/debug.cc
 * Description:   Debugging support
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
 * $Id: debug.cc,v 1.12 2006/06/19 08:01:59 stoess Exp $
 *                
 ********************************************************************/

#if defined(CONFIG_DEBUG)

#define X86_EXC_KDB

#include <debug.h>
#include <ctors.h>
#include <kdb/tracepoints.h>
#include INC_API(kernelinterface.h)
#include INC_API(tcb.h)
#include INC_API(smp.h)
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(apic.h)
#include INC_GLUE(cpu.h)
#include INC_GLUE(debug.h)
#include INC_PLAT(nmi.h)


static void do_return_from_kdb(void);

#if defined(DEBUG_LOCK)
DECLARE_SPINLOCK(printf_spin_lock);
DECLARE_TRACEPOINT(DEBUG_LOCK);
static bool sync_dbg_enter = false;

extern "C" void sync_debug (word_t address)
{
   
    if (get_current_tcb() == get_kdebug_tcb())
	ENABLE_TRACEPOINT(DEBUG_LOCK, ~0, 0);
    
    if (!sync_dbg_enter)
    {
	printf_spin_lock.unlock();
	sync_dbg_enter = true;
 	TRACEPOINT(DEBUG_LOCK, "CPU %d, tcb %t, spinlock BUG (lock %x) @ %x\n", 
		   get_current_cpu(), get_current_tcb(), 
		   address, __builtin_return_address((0)));
	enter_kdebug("spinlock BUG");
    }
    sync_dbg_enter = false;
}
#endif


class cpu_kdb_t
{
private:
    whole_tcb_t __kdb_tcb;
    utcb_t	__kdb_utcb;
    
    debug_param_t param;    
    tcb_t *user_tcb, *kdb_tcb;
    
public:
    tcb_t *get_kdb_tcb()  { return kdb_tcb; }
    tcb_t *get_user_tcb() { return user_tcb; }
    
    cpu_kdb_t()
	{
	    user_tcb = NULL;
	    kdb_tcb = (tcb_t *) &__kdb_tcb;
	    get_idle_tcb()->create_kernel_thread(NILTHREAD, &__kdb_utcb);
	    kdb_tcb->priority = MAX_PRIO;
	    kdb_tcb->set_cpu(get_current_cpu());
	    kdb_tcb->set_space(get_kernel_space());
	}

    void do_enter_kdebug(x86_exceptionframe_t *frame, const word_t exception) 
	{
	    if (get_current_tcb() == get_kdebug_tcb())
		return;

	    void (*entry)(word_t) = (void (*)(word_t)) get_kip()->kdebug_entry;
	    void (*exit)(void) = do_return_from_kdb;
	    
	    kdb_tcb->stack = kdb_tcb->get_stack_top();
	    kdb_tcb->notify(exit);
	    kdb_tcb->notify(entry, (word_t) &param);
	    
	    param.exception = exception;
	    param.frame = frame;
	    param.space = (get_current_space() ? get_current_space() : get_kernel_space());
	    param.tcb = get_current_tcb();
	    
	    user_tcb = get_current_tcb();
	    
	    get_current_tcb()->switch_to(kdb_tcb);
	}
};

cpu_kdb_t cpu_kdb UNIT("cpulocal") CTORPRIO(CTORPRIO_CPU, 1);

tcb_t *get_kdebug_tcb() { return cpu_kdb.get_kdb_tcb(); }


void do_enter_kdebug(x86_exceptionframe_t *frame, const word_t exception)
{
    cpu_kdb.do_enter_kdebug(frame, X86_EXC_DEBUG);
}

void do_return_from_kdb(void)
{
    ASSERT(get_current_tcb() == get_kdebug_tcb());
    get_current_tcb()->switch_to(cpu_kdb.get_user_tcb());
}

X86_EXCNO_ERRORCODE(exc_breakpoint, X86_EXC_BREAKPOINT)
{
     cpu_kdb.do_enter_kdebug(frame, X86_EXC_BREAKPOINT);
}

X86_EXCNO_ERRORCODE(exc_debug, X86_EXC_DEBUG)
{
    cpu_kdb.do_enter_kdebug(frame, X86_EXC_DEBUG);
}

X86_EXCNO_ERRORCODE(exc_nmi, X86_EXC_NMI)
{
    cpu_kdb.do_enter_kdebug(frame, X86_EXC_NMI);
}    
#ifdef CONFIG_SMP
X86_EXCNO_ERRORCODE(exc_debug_ipi, 0)
{
    
}
#endif

#undef X86_EXC_KDB

#endif /* CONFIG_DEBUG */


