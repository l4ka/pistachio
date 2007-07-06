/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     l4/arm/syscalls.h
 * Description:   ARM system call ABI
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
 * $Id: syscalls.h,v 1.19 2004/12/09 00:11:24 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __L4__ARM__SYSCALLS_H__
#define __L4__ARM__SYSCALLS_H__

#include <l4/types.h>
#include <l4/message.h>

/* gcc broken with fp in clobber list */
#define __L4_SYSCALL                   \
	"str	fp,	[sp, #-4]! \n" \
	"mov	lr,	pc	   \n" \
	"mov	pc,	ip	   \n" \
	"ldr	fp,	[sp],	#4 \n"

/* Always clobber these regs (not used as input regs) */
#define __L4_ALWAYS_CLOBBER  "r8", "r9", "r10"

/* Memory attributes for ARM memory control */
#define L4_UncachedMemory	1
#define L4_FlushICache		29
#define L4_FlushDCache		30
#define L4_FlushCache		31

/* Only the KernelInterface call is implemented here, the others are done via
 * the info page.
 */
L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    register L4_Word_t base_address asm ("r0");
    register L4_Word_t _ApiVersion  asm ("r1");
    register L4_Word_t _ApiFlags    asm ("r2");
    register L4_Word_t _KernelId    asm ("r3");

    __asm__ __volatile__ (
            "mov     lr,     pc\n"
            "mov     pc,     %[trap_kip]\n"
            : "=r" (base_address), "=r" (_ApiVersion), "=r" (_ApiFlags),
              "=r" (_KernelId)
            : [trap_kip] "r" (L4_TRAP_KIP)
            : "lr", "memory" );

    if (ApiVersion) *ApiVersion = _ApiVersion;
    if (ApiFlags) *ApiFlags = _ApiFlags;
    if (KernelId) *KernelId = _KernelId;

    return (void *)base_address;
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
    register L4_ThreadId_t r_dest   asm ("r0") = dest;
    register L4_Word_t r_control    asm ("r1") = control;
    register L4_Word_t r_sp	    asm ("r2") = sp;
    register L4_Word_t r_ip	    asm ("r3") = ip;
    register L4_Word_t r_flags	    asm ("r4") = flags;
    register L4_Word_t r_userhandle asm ("r5") = UserDefHandle;
    register L4_ThreadId_t r_pager  asm ("r6") = pager;
    register __L4_ExchangeRegisters_t r_syscall asm ("r12") 
            = __L4_ExchangeRegisters;

    __asm__ __volatile (
        __L4_SYSCALL
    : "=r" (r_dest), "=r" (r_control), "=r" (r_sp), "=r" (r_ip),
      "=r" (r_flags), "=r" (r_userhandle), "=r" (r_pager), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_dest), "r" (r_control), "r" (r_sp), 
      "r" (r_ip), "r" (r_flags), "r" (r_userhandle), "r" (r_pager)
    : "r7", "lr", __L4_ALWAYS_CLOBBER
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
    register L4_Word_t result	   asm ("r0");
    register L4_ThreadId_t r_dest  asm ("r0") = dest;
    register L4_ThreadId_t r_space asm ("r1") = SpaceSpecifier;
    register L4_ThreadId_t r_sched asm ("r2") = Scheduler;
    register L4_ThreadId_t r_pager asm ("r3") = Pager;
    register void *r_utcb          asm ("r4") = UtcbLocation;
    register __L4_ThreadControl_t r_syscall asm ("r12") = __L4_ThreadControl;

    __asm__ __volatile__ (
        __L4_SYSCALL
    : "=r" (result), "=r" (r_space), "=r" (r_sched), "=r" (r_pager), 
      "=r" (r_utcb), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_dest), "r" (r_space), "r" (r_sched), 
      "r" (r_pager), "r" (r_utcb)
    : "r5", "r6", "r7", "lr", __L4_ALWAYS_CLOBBER
    ); 

    return result;
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
    register L4_Word_t r_dest    asm ("r0") = dest.raw;
    register L4_Word_t r_time    asm ("r1") = TimeControl; 
    register L4_Word_t r_proc    asm ("r2") = ProcessorControl;
    register L4_Word_t r_prio    asm ("r3") = prio;
    register L4_Word_t r_preempt asm ("r4") = PreemptionControl;
    register __L4_Schedule_t r_syscall asm ("r12") = __L4_Schedule;

    __asm__ __volatile__ (
        __L4_SYSCALL
    : "=r" (r_dest), "=r" (r_time), "=r" (r_proc), "=r" (r_prio), 
      "=r" (r_preempt), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_dest), "r" (r_time), "r" (r_proc), 
      "r" (r_prio), "r" (r_preempt)
    : "r5", "r6", "r7", "lr", __L4_ALWAYS_CLOBBER
    );

    if(old_TimeControl)
	*old_TimeControl = r_time;

    return r_dest;
}

typedef L4_ThreadId_t (*__L4_Ipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Ipc_t __L4_Ipc;

L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
    register L4_ThreadId_t r_dest asm ("r0") = to;
    register L4_ThreadId_t r_from asm ("r1") = FromSpecifier;
    register L4_Word_t r_timeout  asm ("r2") = Timeouts;
    register L4_Word_t r_mr0      asm ("r3");
    register L4_Word_t r_mr1      asm ("r4");
    register L4_Word_t r_mr2      asm ("r5");
    register L4_Word_t r_mr3      asm ("r6");
    register L4_Word_t r_mr4      asm ("r7");
    register __L4_Ipc_t r_syscall asm ("r12") = __L4_Ipc;


    // Only load MRs if send phase is included
    if (!L4_IsNilThread(to)) {
        r_mr0 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0];
        r_mr1 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1];
        r_mr2 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2];
        r_mr3 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3];
        r_mr4 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4];

        __asm__ __volatile__ (
            ""
        :
        : "r" (r_mr0), "r" (r_mr1), "r" (r_mr2), "r" (r_mr3), "r" (r_mr4)
        );
    }

    __asm__ __volatile (
        __L4_SYSCALL
    : "=r" (r_dest),  "=r" (r_mr0), "=r" (r_mr1), "=r" (r_mr2), "=r" (r_mr3),
      "=r" (r_mr4), "=r" (r_from), "=r" (r_timeout), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_dest), "r" (r_from), "r" (r_timeout)
    : "lr", __L4_ALWAYS_CLOBBER
    );

    if (!L4_IsNilThread(FromSpecifier)) {
	*from = r_dest;

        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0] = r_mr0;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1] = r_mr1;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2] = r_mr2;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3] = r_mr3;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4] = r_mr4;
    }
    {
	    L4_MsgTag_t tag;

	    tag.raw = r_mr0;
	    return tag;
    }
}

typedef L4_ThreadId_t (*__L4_Lipc_t)(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier, L4_Word_t Timeouts);
extern __L4_Lipc_t __L4_Lipc;

L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
    register L4_ThreadId_t r_dest asm ("r0") = to;
    register L4_ThreadId_t r_from asm ("r1") = FromSpecifier;
    register L4_Word_t r_timeout  asm ("r2") = Timeouts;
    register L4_Word_t r_mr0      asm ("r3");
    register L4_Word_t r_mr1      asm ("r4");
    register L4_Word_t r_mr2      asm ("r5");
    register L4_Word_t r_mr3      asm ("r6");
    register L4_Word_t r_mr4      asm ("r7");
    register __L4_Ipc_t r_syscall asm ("r12") = __L4_Lipc;

   // Only load MRs if send phase is included
    if (!L4_IsNilThread(to)) {
        r_mr0 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0];
        r_mr1 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1];
        r_mr2 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2];
        r_mr3 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3];
        r_mr4 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4];

        __asm__ __volatile__ (
            ""
        :
        : "r" (r_mr0), "r" (r_mr1), "r" (r_mr2), "r" (r_mr3), "r" (r_mr4)
        );
    }

    __asm__ __volatile (
        __L4_SYSCALL
    : "=r" (r_dest),  "=r" (r_mr0), "=r" (r_mr1), "=r" (r_mr2), "=r" (r_mr3),
      "=r" (r_mr4), "=r" (r_from), "=r" (r_timeout), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_dest), "r" (r_from), "r" (r_timeout)
    : "lr", __L4_ALWAYS_CLOBBER
    );

    if (!L4_IsNilThread(FromSpecifier)) {
        *from = r_dest;

	(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0] = r_mr0;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1] = r_mr1;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2] = r_mr2;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3] = r_mr3;
        (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4] = r_mr4;
    }
    {
	    L4_MsgTag_t tag;

	    tag.raw = r_mr0;
	    return tag;
    }
}


typedef void (*__L4_Unmap_t)(L4_Word_t);
extern __L4_Unmap_t __L4_Unmap;

L4_INLINE void L4_Unmap (L4_Word_t control)
{
    register L4_Word_t r_control    asm ("r0") = control;
    register L4_Word_t r_mr0        asm ("r3");
    register L4_Word_t r_mr1        asm ("r4");
    register L4_Word_t r_mr2        asm ("r5");
    register L4_Word_t r_mr3        asm ("r6");
    register L4_Word_t r_mr4        asm ("r7");
    register __L4_Unmap_t r_syscall asm ("r12") = __L4_Unmap;

    r_mr0 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0];
    r_mr1 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1];
    r_mr2 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2];
    r_mr3 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3];
    r_mr4 = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4];

    __asm__ __volatile (
        __L4_SYSCALL
    : "=r" (r_mr0), "=r" (r_mr1), "=r" (r_mr2), "=r" (r_mr3), "=r" (r_mr4),
      "=r" (r_control), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_control), "r" (r_mr0), "r" (r_mr1), "r" (r_mr2), 
      "r" (r_mr3), "r" (r_mr4)
    : "r1", "r2", "lr", __L4_ALWAYS_CLOBBER
    );

    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  0] = r_mr0;
    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  1] = r_mr1;
    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  2] = r_mr2;
    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  3] = r_mr3;
    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  4] = r_mr4;
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
    register L4_ThreadId_t r_space  asm ("r0") = SpaceSpecifier;
    register L4_Word_t r_control    asm ("r1") = control;
    register L4_Fpage_t r_kip_area  asm ("r2") = KernelInterfacePageArea;
    register L4_Fpage_t r_utcb_area asm ("r3") = UtcbArea;
    register L4_ThreadId_t r_redir  asm ("r4") = redirector;
    register __L4_SpaceControl_t r_syscall asm ("r12") = __L4_SpaceControl;

    __asm__ __volatile__ (
        __L4_SYSCALL
    : "=r" (r_space), "=r" (r_control), "=r" (r_kip_area), "=r" (r_utcb_area),
      "=r" (r_redir), "=r" (r_syscall)
    : "r" (r_syscall), "r" (r_space), "r" (r_control), 
      "r" (r_kip_area), "r" (r_utcb_area), "r" (r_redir)
    : "r5", "r6", "r7", "lr", __L4_ALWAYS_CLOBBER
    );

    if(old_control)
	*old_control = r_control;

    return r_space.raw;
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
    return __L4_ProcessorControl(ProcessorNo, InternalFrequency, 
            ExternalFrequency, voltage);
}

typedef L4_Word_t (*__L4_MemoryControl_t)(L4_Word_t control,
				     L4_Word_t attr0, L4_Word_t attr1, L4_Word_t attr2, L4_Word_t attr3);
extern __L4_MemoryControl_t __L4_MemoryControl;

L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
				 const L4_Word_t * attributes)
{
    register L4_Word_t r_control asm ("r0") = control;
    register L4_Word_t r_attrib0 asm ("r1") = attributes[0];
    register L4_Word_t r_attrib1 asm ("r2") = attributes[1];
    register L4_Word_t r_attrib2 asm ("r3") = attributes[2];
    register L4_Word_t r_attrib3 asm ("r4") = attributes[3];
    register __L4_MemoryControl_t r_syscall asm ("r12") = __L4_MemoryControl;

    __asm__ __volatile__ (
        __L4_SYSCALL
    : "=r" (r_control), "=r" (r_attrib0), "=r" (r_attrib1), "=r" (r_attrib2),
      "=r" (r_attrib3)
    : "r" (r_syscall), "r" (r_control), "r" (r_attrib0), 
      "r" (r_attrib1), "r" (r_attrib2), "r" (r_attrib3)
    : "r5", "r6", "r7", "lr", __L4_ALWAYS_CLOBBER
    );

    return r_control;
}

#endif /* !__L4__ARM__SYSCALLS_H__ */
