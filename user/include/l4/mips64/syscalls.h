/*********************************************************************
 *                
 * Copyright (C) 2002-2004,   University of New South Wales
 *                
 * File path:     l4/mips64/syscalls.h
 * Description:   MIPS64 system call ABI
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
 * $Id: syscalls.h,v 1.14 2005/08/02 01:22:37 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __L4__MIPS64__SYSCALLS_H__
#define __L4__MIPS64__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>


/* The application gets the kernel info page by doing some illegal
 * instruction, with at ($1) == 0x1face (interface) ca11 (call) 14 (L4) e1f
 * (ELF) 64 (MIPS64)
 */
#define __L4_MAGIC_KIP_REQUEST          (0x1FACECA1114e1f64ULL)

/* Memory attributes for MIPS64 memory control */
#define L4_UncachedMemory	1
#define L4_WriteBackMemory	2
#define L4_WriteThroughMemory	3
#define L4_WriteThroughNoAllocMemory	4
#define L4_CoherentMemory	5
#define L4_FlushICache		29
#define L4_FlushDCache		30
#define L4_FlushCache		31


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    register void * base_address	asm ("$8");  /* t0 */
    register L4_Word_t api_version	asm ("$9");  /* t1 */
    register L4_Word_t api_flags	asm ("$10"); /* t2 */
    register L4_Word_t kernel_id	asm ("$11"); /* t3 */

    __asm__ __volatile__ (".set noat;\n\t");
    register L4_Word_t i asm("$1")= __L4_MAGIC_KIP_REQUEST;

    __asm__ __volatile__ (
	".word 0x07FFFFFF;\r\n"
	".set at;     \n\r"
	: "=r" (base_address), "=r" (api_version), "=r" (api_flags),
	"=r" (kernel_id)
	: "r" (i)
    );


    if( ApiVersion ) *ApiVersion = api_version;
    if( ApiFlags ) *ApiFlags = api_flags;
    if( KernelId ) *KernelId = kernel_id;

    return base_address;
}

/* Implemented in exregs.S */
typedef L4_ThreadId_t (*__L4_ExchangeRegisters_t)(L4_ThreadId_t dest,
						  L4_Word_t control,
						  L4_Word_t sp,
						  L4_Word_t ip,
						  L4_Word_t flags,
						  L4_Word_t UserDefHandle,
						  L4_ThreadId_t pager);
extern __L4_ExchangeRegisters_t __L4_ExchangeRegisters;

L4_INLINE L4_ThreadId_t L4_ExchangeRegisters (L4_ThreadId_t dest,
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
    register L4_ThreadId_t r_dest	asm ("$2");
    register L4_Word_t r_control	asm ("$4");
    register L4_Word_t r_sp		asm ("$5");
    register L4_Word_t r_ip		asm ("$6");
    register L4_Word_t r_flags		asm ("$7");
    register L4_Word_t r_userhandle	asm ("$8");
    register L4_ThreadId_t r_pager	asm ("$9");

    __L4_ExchangeRegisters(dest, control, sp, ip, flags, UserDefHandle, pager);
    __asm__ __volatile__ ( ""
	    : "=r" (r_dest), "=r" (r_control), "=r" (r_sp), "=r" (r_ip),
	      "=r" (r_flags), "=r" (r_userhandle), "=r" (r_pager)
    );

    *old_control = r_control;
    *old_sp = r_sp;
    *old_ip = r_ip;
    *old_flags = r_flags;
    *old_UserDefHandle = r_userhandle;
    *old_pager = r_pager;

    return r_dest;
}

typedef L4_Word_t (*__L4_ThreadControl_t)(L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, L4_ThreadId_t, void *);
extern __L4_ThreadControl_t __L4_ThreadControl;

L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void * UtcbLocation)
{
    return __L4_ThreadControl(dest, SpaceSpecifier, Scheduler, Pager, UtcbLocation);
}

typedef L4_Word64_t (*__L4_SystemClock_t)(void);
extern __L4_SystemClock_t __L4_SystemClock;

L4_INLINE L4_Clock_t L4_SystemClock (void)
{
    return (L4_Clock_t){ raw: __L4_SystemClock() };
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
    register L4_Word_t r_result          asm ("$2");
    register L4_ThreadId_t r_dest        asm ("$4"); /* out old_TimeControl */

    __L4_Schedule(dest, TimeControl, ProcessorControl, prio, PreemptionControl);
    __asm__ __volatile__ ("" : "=r" (r_result), "=r" (r_dest));

    if(old_TimeControl)
	*old_TimeControl = r_dest.raw;

    return r_result;
}

typedef L4_ThreadId_t (*__L4_Ipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
    L4_MsgTag_t tag;
    register L4_ThreadId_t to_r asm ("$4") = to;
    register L4_ThreadId_t from_r asm ("$5") = FromSpecifier;
    register L4_Word_t timeout_r asm ("$6") = Timeouts;
    register L4_ThreadId_t result asm ("$2");
    register L4_Word_t mr0 asm ("$3");
    register L4_Word_t mr1 asm ("$16");
    register L4_Word_t mr2 asm ("$17");
    register L4_Word_t mr3 asm ("$18");
    register L4_Word_t mr4 asm ("$19");
    register L4_Word_t mr5 asm ("$20");
    register L4_Word_t mr6 asm ("$21");
    register L4_Word_t mr7 asm ("$22");
    register L4_Word_t mr8 asm ("$23");

    // Only load MRs if send phase is included
    if (! L4_IsNilThread (to))
    {
	mr0 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  0];
	mr1 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  1];
	mr2 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  2];
	mr3 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  3];
	mr4 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  4];
	mr5 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  5];
	mr6 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  6];
	mr7 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  7];
	mr8 = (__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  8];

	__asm__ __volatile__ ("" : :
	    "r" (mr0), "r" (mr1), "r" (mr2), "r" (mr3),
	    "r" (mr4), "r" (mr5), "r" (mr6), "r" (mr7), "r" (mr8)
	    : "$1", "$7", "$8", "$9", "$10", "$11",
	      "$12", "$13", "$14", "$15",
	      "$24", "$25", /*"$28",*/ "$31"
	    );   /* s8, sp saved */
    }

//    result = __L4_Ipc(to, FromSpecifier, Timeouts);
    __asm__ __volatile__ (
	"   ld		$2, __L4_Ipc	    \n\r"
	"   jalr	$2		    \n\r"
	:
	"=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
	"=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7), "=r" (mr8),
	"=r" (result), "+r" (to_r), "+r" (from_r), "+r" (timeout_r)
	:
	: "$1", "$7", "$8", "$9", "$10", "$11", "$12",
	  "$13", "$14", "$15", "$24", "$25", /*"$28",*/ "$31",
	  "memory"
	);
   
    if( !L4_IsNilThread(FromSpecifier) ) {
	*from = result;

	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  1] = mr1;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  2] = mr2;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  3] = mr3;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  4] = mr4;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  5] = mr5;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  6] = mr6;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  7] = mr7;
	(__L4_Mips64_Utcb())[__L4_TCR_MR_OFFSET +  8] = mr8;
    }

    /* Return MR0 */
    tag.raw = mr0;
    return tag;
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

    result = __L4_Lipc(to, FromSpecifier, Timeouts);
   
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
    register L4_Word_t r_result         asm ("$2");
    register L4_ThreadId_t r_space	asm ("$4") = SpaceSpecifier; /* and old_control */

    __L4_SpaceControl(SpaceSpecifier, control, KernelInterfacePageArea, UtcbArea, redirector);
    __asm__ __volatile__ ("" : "=r" (r_result), "=r" (r_space));

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
				L4_Word_t attr0, L4_Word_t attr1,
				L4_Word_t attr2, L4_Word_t attr3);
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
				 const L4_Word_t * attributes)
{
    return __L4_MemoryControl(control, attributes[0], attributes[1], attributes[2], attributes[3]);
}

#endif /* !__L4__MIPS64__SYSCALLS_H__ */
