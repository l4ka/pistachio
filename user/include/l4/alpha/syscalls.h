/*********************************************************************
 *                
 * Copyright (C) 2002-2004,   University of New South Wales
 *                
 * File path:     l4/alpha/syscalls.h
 * Description:   Alpha system call ABI
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
 * $Id: syscalls.h,v 1.7 2004/08/23 18:43:10 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__ALPHA__SYSCALLS_H__
#define __L4__ALPHA__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

#include <l4/alpha/vregs.h>
#include <l4/alpha/runconv.h>
#include <l4/alpha/pal.h>
#include <l4/alpha/kdebug.h>

#define MAGIC_KIP_REQUEST          (0x4c34754b4b495034)


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    register void * base_address	asm ("$0");
    register L4_Word_t api_version	asm ("$16") = MAGIC_KIP_REQUEST;
    register L4_Word_t api_flags	asm ("$17");
    register L4_Word_t kernel_id	asm ("$18");

    __asm__ __volatile__ (
	"call_pal %4"
	: /* outputs */
	"=r" (base_address),
	"+r" (api_version),
	"=r" (api_flags),
	"=r" (kernel_id)
	: "i" (PAL_cserve));

    if( ApiVersion ) *ApiVersion = api_version;
    if( ApiFlags ) *ApiFlags = api_flags;
    if( KernelId ) *KernelId = kernel_id;

    return base_address;
}

#define DO_SYSCALL(name) 						\
	"       lda     $27, __L4_"#name "                      \n"	\
	"       ldq     $27, 0($27)                             \n"	\
	"	jsr	$26, ($27)                              \n"



typedef L4_ThreadId_t (*__L4_Ipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Ipc_t __L4_Ipc;


/* Implemented in exregs.S */
typedef L4_ThreadId_t (*__L4_ExchangeRegisters_t)(L4_ThreadId_t dest,
						  L4_Word_t control,
						  L4_Word_t sp,
						  L4_Word_t ip,
						  L4_Word_t flags,
						  L4_Word_t UserDefHandle,
						  L4_ThreadId_t pager);
extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

#ifdef __cplusplus
extern "C"
#endif
L4_ThreadId_t __alpha_L4_ExchangeRegisters (L4_ThreadId_t dest,
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
					    L4_ThreadId_t *old_pager);


L4_INLINE
L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
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
    return __alpha_L4_ExchangeRegisters(dest,
					control,
					sp,
					ip,
					flags,
					UserDefHandle,
					pager,
					old_control,
					old_sp,
					old_ip,
					old_flags,
					old_UserDefHandle,
					old_pager);
}


typedef L4_Word_t (*__L4_ThreadControl_t)(L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, void *);
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void *utcb)
{
    return __L4_ThreadControl(dest, SpaceSpecifier, Scheduler, Pager, utcb);
}

typedef L4_Clock_t (*__L4_SystemClock_t)(void);
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t L4_SystemClock (void)
{
    return __L4_SystemClock();
}

typedef L4_Word_t (*__L4_ThreadSwitch_t)(L4_ThreadId_t);
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE L4_Word_t L4_ThreadSwitch (L4_ThreadId_t dest)
{
    return __L4_ThreadSwitch(dest);
}

typedef L4_Word_t (*__L4_Schedule_t)(L4_ThreadId_t dest, L4_Word_t TimeControl, 
				      L4_Word_t ProcessorControl, L4_Word_t prio, L4_Word_t PreemptionControl); 
extern __L4_Schedule_t __L4_Schedule;

L4_INLINE L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
				  L4_Word_t TimeControl,
				  L4_Word_t ProcessorControl,
				  L4_Word_t prio,
				  L4_Word_t PreemptionControl,
				  L4_Word_t * old_TimeControl)
{
    register L4_Word_t r_result          asm ("$0");
    register L4_ThreadId_t r_dest        asm ("$16") = dest; /* in and out old_TimeControl */
    register L4_Word_t r_time            asm ("$17") = TimeControl; 
    register L4_Word_t r_processor       asm ("$18") = ProcessorControl;
    register L4_Word_t r_prio	         asm ("$19") = prio;
    register L4_Word_t r_preemption 	 asm ("$20") = PreemptionControl;

    asm __volatile__ (
	DO_SYSCALL(Schedule)
	: "=r" (r_result), "+r" (r_dest) 
	: "r" (r_time), "r" (r_processor), "r" (r_prio), "r" (r_preemption)
	: __L4_CALLER_SAVED_REGS);
    
    if(old_TimeControl)
	*old_TimeControl = r_dest.raw;

    return r_result;
}

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
    L4_MsgTag_t mr0;
    L4_ThreadId_t result;

    result = __L4_Ipc(to, FromSpecifier, Timeouts);
   
    if( !L4_IsNilThread(FromSpecifier) ) {
	*from = result;
    }

    /* Return MR0 */
    L4_StoreMR(0, (L4_Word_t *) &mr0);
    return mr0;
}

typedef L4_ThreadId_t (*__L4_Lipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
    L4_MsgTag_t mr0;
    L4_ThreadId_t result;

    /* sjw (30/04/2003): Not LIPC as this isn't implemented on Alpha */
    result = __L4_Ipc(to, FromSpecifier, Timeouts);
   
    if( !L4_IsNilThread(FromSpecifier) ) {
	*from = result;
    }

    /* Return MR0 */
    L4_StoreMR(0, (L4_Word_t *) &mr0);
    return mr0;
}


typedef void (*__L4_Unmap_t)(L4_Word_t);
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap (L4_Word_t control)
{
    __L4_Unmap(control);
}

typedef void (*__L4_SpaceControl_t)(L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector);
extern __L4_SpaceControl_t __L4_SpaceControl;

L4_INLINE L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector,
				     L4_Word_t *old_control)
{
    register L4_Word_t r_result         asm ("$0");
    register L4_ThreadId_t r_space	asm ("$16") = SpaceSpecifier; /* and old_control */
    register L4_Word_t r_control	asm ("$17") = control;        
    register L4_Fpage_t r_kiparea	asm ("$18") = KernelInterfacePageArea;
    register L4_Fpage_t r_utcbarea	asm ("$19") = UtcbArea;
    register L4_ThreadId_t r_redirector	asm ("$20") = redirector;

    __asm__ __volatile__ (
	DO_SYSCALL(SpaceControl)
	: /* outputs */
	"=r" (r_result),
	"+r" (r_space)
	: /* inputs */
	"r" (r_control),
	"r" (r_kiparea),
	"r" (r_utcbarea),
	"r" (r_redirector)
	: /* clobbers */
	__L4_CALLER_SAVED_REGS);

    if(old_control)
	*old_control = r_space.raw;

    return r_result;
}


typedef L4_Word_t (*__L4_ProcessorControl_t)(L4_Word_t ProcessorNo,
					  L4_Word_t InternalFrequency,
					  L4_Word_t ExternalFrequency,
					  L4_Word_t voltage);
extern __L4_ProcessorControl_t __L4_ProcessorControl;
    
L4_INLINE L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
					 L4_Word_t InternalFrequency,
					 L4_Word_t ExternalFrequency,
					 L4_Word_t voltage)
{
    return __L4_ProcessorControl(ProcessorNo, InternalFrequency, ExternalFrequency, voltage);
}

typedef L4_Word_t (*__L4_MemoryControl_t)(L4_Word_t control,
				     L4_Word_t attr0, L4_Word_t attr1, L4_Word_t attr2, L4_Word_t attr3);
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
				 const L4_Word_t * attributes)
{
    return __L4_MemoryControl(control, attributes[0], attributes[1], attributes[2], attributes[3]);
}

#endif /* !__L4__ALPHA__SYSCALLS_H__ */
