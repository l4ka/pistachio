/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  University of New South Wales
 *                
 * File path:     l4/sparc64/syscalls.h
 * Description:   SPARC v9 system call ABI
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
 * $Id: syscalls.h,v 1.7 2004/08/23 18:43:17 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__SPARC64__SYSCALLS_H__
#define __L4__SPARC64__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

#include <l4/sparc64/vregs.h>
#include <l4/sparc64/kdebug.h>

L4_INLINE void *
L4_KernelInterface(L4_Word_t *ApiVersion,
		   L4_Word_t *ApiFlags,
		   L4_Word_t *KernelId)
{
    register void* result asm ("o0");
    register L4_Word_t version asm ("o1");
    register L4_Word_t flags asm ("o2");
    register L4_Word_t id asm ("o3");

    /* trap to the kernel */
    asm volatile (
	"ta	0x70\n\t"
	:
	"=r" (result), "=r" (version), "=r" (flags), "=r" (id)
    );

    if(ApiVersion) *ApiVersion = version;
    if(ApiFlags) *ApiFlags = flags;
    if(KernelId) *KernelId = id;
    return result;
}

typedef L4_Word_t (*__L4_ExchangeRegisters_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

L4_INLINE L4_ThreadId_t
L4_ExchangeRegisters(L4_ThreadId_t dest,
		     L4_Word_t control,
		     L4_Word_t sp,
		     L4_Word_t ip,
		     L4_Word_t flags,
		     L4_Word_t UserDefHandle,
		     L4_ThreadId_t pager,
		     L4_Word_t *old_control,
		     L4_Word_t *old_sp,
		     L4_Word_t *old_ip,
		     L4_Word_t *old_flags,
		     L4_Word_t *old_UserDefHandle,
		     L4_ThreadId_t *old_pager)
{
    register L4_Word_t r_control asm ("o1");
    register L4_Word_t r_sp asm ("o2");
    register L4_Word_t r_ip asm ("o3");
    register L4_ThreadId_t r_pager asm ("o4");
    register L4_Word_t r_UserDefHandle asm ("o5");
    register L4_Word_t r_flags asm ("g4");
    L4_ThreadId_t r_dest;

    r_flags = flags;

    asm volatile ("" :: "r" (r_flags));
    r_dest.raw = __L4_ExchangeRegisters(dest.raw, control, sp, ip, pager.raw,
					UserDefHandle);
    asm volatile ("" : "=r" (r_control), "=r" (r_sp), "=r" (r_ip), "=r" (r_pager),
		  "=r" (r_UserDefHandle), "=r" (r_flags));

    *old_control = r_control;
    *old_sp = r_sp;
    *old_ip = r_ip;
    *old_flags = r_flags;
    *old_UserDefHandle = r_UserDefHandle;
    *old_pager = r_pager;

    return r_dest;
}

typedef L4_Word_t (*__L4_ThreadControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t
L4_ThreadControl(L4_ThreadId_t dest,
		 L4_ThreadId_t SpaceSpecifier,
		 L4_ThreadId_t Scheduler,
		 L4_ThreadId_t Pager,
		 void * utcb)
{
    return __L4_ThreadControl(dest.raw, SpaceSpecifier.raw, Scheduler.raw,
			      Pager.raw, (L4_Word_t)utcb);
} // L4_threadControl()

typedef L4_Clock_t (*__L4_SystemClock_t)( void );
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t
L4_SystemClock(void)
{
    return __L4_SystemClock();
} // L4_SystemClock()

typedef L4_Word_t (*__L4_ThreadSwitch_t)( L4_Word_t );
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE L4_Word_t
L4_ThreadSwitch(L4_ThreadId_t dest)
{
    return __L4_ThreadSwitch(dest.raw);
} // L4_ThreadSwitch()

typedef L4_Word_t (*__L4_Schedule_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t
L4_Schedule(L4_ThreadId_t dest,
	    L4_Word_t TimeControl,
	    L4_Word_t ProcessorControl,
	    L4_Word_t prio,
	    L4_Word_t PreemptionControl,
	    L4_Word_t * old_TimeControl)
{
    register L4_Word_t r_timecontrol asm("o1");
    L4_Word_t result;

    result = __L4_Schedule(dest.raw, TimeControl, ProcessorControl, prio,
			   PreemptionControl);
    asm volatile ("" : "=r" (r_timecontrol));

    if(old_TimeControl) {
	*old_TimeControl = r_timecontrol;
    }
    
    return result;
} // L4_Schedule()

typedef L4_Word_t (*__L4_Ipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t
L4_Ipc(L4_ThreadId_t to,
       L4_ThreadId_t FromSpecifier,
       L4_Word_t Timeouts,
       L4_ThreadId_t * from)
{
    L4_MsgTag_t tag;
    register L4_ThreadId_t to_from asm ("o0") = to;
    register L4_ThreadId_t from_specifier asm ("o1") = FromSpecifier;
    register L4_Word_t timeout asm ("o2") = Timeouts;
    register L4_Word_t mr0 asm ("l0");
    register L4_Word_t mr1 asm ("l1");
    register L4_Word_t mr2 asm ("l2");
    register L4_Word_t mr3 asm ("l3");
    register L4_Word_t mr4 asm ("l4");
    register L4_Word_t mr5 asm ("l5");
    register L4_Word_t mr6 asm ("l6");
    register L4_Word_t mr7 asm ("l7");

    // Only load MRs if send phase is included
    if (! L4_IsNilThread (to))
    {
	mr0 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  0];
	mr1 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  1];
	mr2 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  2];
	mr3 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  3];
	mr4 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  4];
	mr5 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  5];
	mr6 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  6];
	mr7 = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET +  7];

	asm ("" : :
	    "r" (mr0), "r" (mr1), "r" (mr2), "r" (mr3),
	    "r" (mr4), "r" (mr5), "r" (mr6), "r" (mr7)
	    : "g1", "g4", "g5", "g6", "o3", "o4", "o5",
	      "i0", "i1", "i2", "i3", "i4", "i5"
	    );   /* i7, o7, fp, sp saved */
    }

//    result = __L4_Ipc(to, FromSpecifier, Timeouts);
    asm ("setx	__L4_Ipc, %%g4, %%g1\n\t"
	 "ldx	[ %%g1 ], %%g1\n\t"
	 "call	%%g1\n\t"
	 "nop"
	: "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
	  "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
	  "=r" (to_from)
	: "r" (to_from), "r" (from_specifier), "r" (timeout)
	: "g1", "g4", "g5", "g6", "o3", "o4", "o5", "o7",
	  "i0", "i1", "i2", "i3", "i4", "i5"
	);
   
    if( !L4_IsNilThread(FromSpecifier) ) {
	*from = to_from;

	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 1] = mr1;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 2] = mr2;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 3] = mr3;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 4] = mr4;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 5] = mr5;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 6] = mr6;
	(__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + 7] = mr7;
    }

    /* Return MR0 */
    tag.raw = mr0;
    return tag;
} // L4_Ipc()

typedef L4_Word_t (*__L4_Lipc_t)( L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t
L4_Lipc(L4_ThreadId_t to,
	L4_ThreadId_t FromSpecifier,
	L4_Word_t Timeouts,
	L4_ThreadId_t *from)
{
    return L4_Ipc(to, FromSpecifier, Timeouts, from);
} // L4_Lipc()

typedef void (*__L4_Unmap_t)( L4_Word_t );
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void
L4_Unmap(L4_Word_t control)
{
    __L4_Unmap(control);
} // L4_Unmap()

typedef L4_Word_t (*__L4_SpaceControl_t)( L4_Word_t, L4_Word_t, L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_SpaceControl_t __L4_SpaceControl;

L4_INLINE L4_Word_t
L4_SpaceControl(L4_ThreadId_t SpaceSpecifier,
		L4_Word_t control,
		L4_Fpage_t KernelInterfacePageArea,
		L4_Fpage_t UtcbArea,
		L4_ThreadId_t redirector,
		L4_Word_t *old_control)
{
    register L4_Word_t r_control asm("o1");
    L4_Word_t result;

    result = __L4_SpaceControl(SpaceSpecifier.raw, control, KernelInterfacePageArea.raw,
			       UtcbArea.raw, redirector.raw);
    asm volatile ("" : "=r" (r_control));

    if(old_control) {
	*old_control = r_control;
    }

    return control;
} // L4_SpaceControl()

typedef L4_Word_t (*__L4_ProcessorControl_t)( L4_Word_t,  L4_Word_t,
	L4_Word_t, L4_Word_t );
extern __L4_ProcessorControl_t __L4_ProcessorControl;

L4_INLINE L4_Word_t
L4_ProcessorControl(L4_Word_t ProcessorNo,
		    L4_Word_t control,
		    L4_Word_t InternalFrequency,
		    L4_Word_t ExternalFrequency,
		    L4_Word_t voltage)
{
    /* XXX what is control for? How many arguments does this syscall have? */
    return __L4_ProcessorControl(ProcessorNo, InternalFrequency,
				 ExternalFrequency, voltage);
} // L4_ProcessorControl()

typedef L4_Word_t (*__L4_MemoryControl_t)( L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t, L4_Word_t );
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t
L4_MemoryControl(L4_Word_t control,
		 const L4_Word_t * attributes)
{
    return __L4_MemoryControl(control, attributes[0], attributes[1],
			      attributes[2], attributes[4]);
} // L4_MemoryControl()


#endif /* !__L4__SPARC64__SYSCALLS_H__ */
