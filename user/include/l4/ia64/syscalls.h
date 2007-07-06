/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     l4/ia64/syscalls.h
 * Description:   IA64 system call ABI
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
 * $Id: syscalls.h,v 1.36 2004/08/23 18:43:12 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__SYSCALLS_H__
#define __L4__IA64__SYSCALLS_H__

#include __L4_INC_ARCH(vregs.h)
#include __L4_INC_ARCH(runconv.h)
#include __L4_INC_ARCH(specials.h)
#include <l4/message.h>

#include <l4/kdebug.h>

#define __L4_ASM_ENTER_KDEBUG(str)				\
	"{.mlx						\n"	\
	"	break.m	0x3				\n"	\
	"	movl r0 = 9f ;;				\n"	\
	"}						\n"	\
	"	.rodata					\n"	\
	"9:	stringz " #str "			\n"	\
	"	.previous				\n"

#define __L4_ASM_CALL(func)					\
	"	add	r31 = -40, sp			\n"	\
	"	add	r30 = -32, sp			\n"	\
	"	add	sp  = -40, sp			\n"	\
	"	mov	r29 = ar.unat			\n"	\
	"	mov	r28 = rp			\n"	\
	"	mov	r27 = ar.pfs			\n"	\
	"	mov	r26 = ar.fpsr			\n"	\
	"	;;					\n"	\
	"	st8	[r31] = r29, 16			\n"	\
	"	st8	[r30] = r28, 16			\n"	\
	"	;;					\n"	\
	"	st8	[r31] = r27			\n"	\
	"	st8	[r30] = r26			\n"	\
	"						\n"	\
	"	br.call.sptk.few rp = " #func "		\n"	\
	"						\n"	\
	"      	mov	r31 = sp			\n"	\
	"	add	r30 = 8, sp			\n"	\
	"	;;					\n"	\
	"	ld8	r29 = [r31], 16			\n"	\
	"	ld8	r28 = [r30], 16			\n"	\
	"	;;					\n"	\
	"	ld8	r27 = [r31]			\n"	\
	"	ld8	r26 = [r30]			\n"	\
	"	;;					\n"	\
	"	mov	ar.unat = r29			\n"	\
	"	mov	rp = r28			\n"	\
	"	mov	ar.pfs = r27			\n"	\
	"	mov	ar.fpsr = r26			\n"	\
	"	add	sp = 40, sp			\n"	\
	"	;;					\n"


L4_INLINE void * L4_KernelInterface (L4_Word_t *ApiVersion,
				     L4_Word_t *ApiFlags,
				     L4_Word_t *KernelId)
{
    register void * base_address	asm ("r8");
    register L4_Word_t api_version	asm ("r9");
    register L4_Word_t api_flags	asm ("r10");
    register L4_Word_t kernel_id	asm ("r11");

    __asm__ __volatile__ (
	"/* L4_KernelInterface() */ 			\n"
	"{ .mlx						\n"
	"	break.m	0x1face				\n"
	"	movl	r0 = 0x0 ;;			\n"
	"}						\n"
	: /* outputs */
	"=r" (base_address),
	"=r" (api_version),
	"=r" (api_flags),
	"=r" (kernel_id));

    *ApiVersion = api_version;
    *ApiFlags = api_flags;
    *KernelId = kernel_id;

    return base_address;
}


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
    register L4_ThreadId_t r_dest	asm ("r14") = dest;
    register L4_Word_t r_control	asm ("r15") = control;
    register L4_Word_t r_sp		asm ("r16") = sp;
    register L4_Word_t r_ip		asm ("r17") = ip;
    register L4_Word_t r_flags		asm ("r18") = flags;
    register L4_Word_t r_userhandle	asm ("r19") = UserDefHandle;
    register L4_ThreadId_t r_pager	asm ("r20") = pager;

    __asm__ __volatile__ (
	"/* L4_ExchangeRegisters() */				\n"
	__L4_ASM_CALL(__L4_ExchangeRegisters)

	: /* outputs */
	"+r" (r_dest),
	"+r" (r_control),
	"+r" (r_sp),
	"+r" (r_ip),
	"+r" (r_flags),
	"+r" (r_userhandle),
	"+r" (r_pager)

	: /* inputs (same as outputs) */

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r8","r9","r10","r11"));

    *old_control = r_control;
    *old_sp = r_sp;
    *old_ip = r_ip;
    *old_flags = r_flags;
    *old_UserDefHandle = r_userhandle;
    *old_pager = r_pager;

    return r_dest;
}


L4_INLINE L4_Word_t L4_ThreadControl (L4_ThreadId_t dest,
				      L4_ThreadId_t SpaceSpecifier,
				      L4_ThreadId_t Scheduler,
				      L4_ThreadId_t Pager,
				      void * UtcbLocation)
{
    register L4_Word_t result 			asm ("r8");
    register L4_ThreadId_t r_dest		asm ("r14") = dest;
    register L4_ThreadId_t r_SpaceSpecifier	asm ("r15") = SpaceSpecifier;
    register L4_ThreadId_t r_Scheduler		asm ("r16") = Scheduler;
    register L4_ThreadId_t r_Pager		asm ("r17") = Pager;
    register void * r_UtcbLocation		asm ("r18") = UtcbLocation;

    __asm__ __volatile__ (
	"/* L4_ThreadControl() */				\n"
	__L4_ASM_CALL(__L4_ThreadControl)

	: /* outputs */
	"=r" (result),

	/* inputs (clobbered on output) */ 
	"+r" (r_dest),
	"+r" (r_SpaceSpecifier),
	"+r" (r_Scheduler),
	"+r" (r_Pager),
	"+r" (r_UtcbLocation)

	: /* no unclobbered inputs */

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r9","r10","r11","r19","r20"));

    return result;
}


L4_INLINE L4_Clock_t L4_SystemClock (void)
{
    register L4_Clock_t clock			asm ("r8");

    __asm__ __volatile__ (
	"/* L4_SystemClock() */\n"
	__L4_ASM_CALL(__L4_SystemClock)

	: /* outputs */
	"=r" (clock.raw)

	: /* no inputs */

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r9","r10","r11",
				  "r14","r15","r16","r17","r18","r19","r20"));

    return clock;
}


L4_INLINE void L4_ThreadSwitch (L4_ThreadId_t dest)
{
    register L4_ThreadId_t r_dest		asm ("r14") = dest;

    __asm__ __volatile__ (
	"/* L4_ThreadSwitch() */\n"
	__L4_ASM_CALL(__L4_ThreadSwitch)

	: /* no outputs */

	/* inputs (clobbered on output) */ 
	"+r" (r_dest)

	: /* no unclobbered inputs */

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r8","r9","r10","r11",
				  "r15","r16","r17","r18","r19","r20"));
}


L4_INLINE L4_Word_t  L4_Schedule (L4_ThreadId_t dest,
				  L4_Word_t TimeCtrl,
				  L4_Word_t ProcessorCtrl,
				  L4_Word_t prio,
				  L4_Word_t PreemptionCtrl,
				  L4_Word_t * old_TimeCtrl)
{
    register L4_Word_t result 			asm ("r8");
    register L4_Word_t ret_TimeCtrl		asm ("r9");
    register L4_ThreadId_t r_dest		asm ("r14") = dest;
    register L4_Word_t r_TimeCtrl		asm ("r15") = TimeCtrl;
    register L4_Word_t r_ProcessorCtrl		asm ("r16") = ProcessorCtrl;
    register L4_Word_t r_prio			asm ("r17") = prio;
    register L4_Word_t r_PreemptionCtrl		asm ("r18") = PreemptionCtrl;

    __asm__ __volatile__ (
	"/* L4_Schedule() */					\n"
	__L4_ASM_CALL(__L4_Schedule)

	: /* outputs */
	"=r" (result),
	"=r" (ret_TimeCtrl),

	/* inputs (clobbered on output) */ 
	"+r" (r_dest),
	"+r" (r_TimeCtrl),
	"+r" (r_ProcessorCtrl),
	"+r" (r_prio),
	"+r" (r_PreemptionCtrl)

	: /* no unclobbered inputs */

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r10","r11","r19","r20"));

    *old_TimeCtrl = ret_TimeCtrl;

    return result;
}


L4_INLINE L4_MsgTag_t L4_Ipc (L4_ThreadId_t to,
			      L4_ThreadId_t FromSpecifier,
			      L4_Word_t Timeouts,
			      L4_ThreadId_t * from)
{
    register L4_ThreadId_t r_to			asm ("r14") = to;
    register L4_ThreadId_t r_FromSpecifier	asm ("r15") = FromSpecifier;
    register L4_Word_t r_Timeouts		asm ("r16") = Timeouts;
    register L4_ThreadId_t ret_from		asm ("r9");
    L4_Word_t * mrs = __L4_IA64_Utcb () + __L4_TCB_MR_OFFSET;

    register L4_Word_t mr0 asm ("out0");
    register L4_Word_t mr1 asm ("out1");
    register L4_Word_t mr2 asm ("out2");
    register L4_Word_t mr3 asm ("out3");
    register L4_Word_t mr4 asm ("out4");
    register L4_Word_t mr5 asm ("out5");
    register L4_Word_t mr6 asm ("out6");
    register L4_Word_t mr7 asm ("out7");
    L4_MsgTag_t tag;

    L4_Word_t ar_lc, ar_ec;
    __asm__ __volatile__ ("	;;			\n"
			  "	mov	%0 = ar.lc	\n"
			  "	mov	%1 = ar.ec	\n"
			  :
			  "=r" (ar_lc), "=r" (ar_ec));

    // Only load MRs if send phase is included
    if (! L4_IsNilThread (to))
    {
	r_to = to;
	r_FromSpecifier = FromSpecifier;
	r_Timeouts = Timeouts;

	mr0 = mrs[0];
	mr1 = mrs[1];
	mr2 = mrs[2];
	mr3 = mrs[3];
	mr4 = mrs[4];
	mr5 = mrs[5];
	mr6 = mrs[6];
	mr7 = mrs[7];

	__asm__ __volatile__ (
	    "/* L4_Ipc() */\n"
	    __L4_ASM_CALL (__L4_Ipc)

	    : /* outputs */
	    "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
	    "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
	    "=r" (ret_from),

	    /* inputs (clobbered on output) */ 
	    "+r" (r_to), "+r" (r_FromSpecifier), "+r" (r_Timeouts)

	    : /* inputs */
	    "0" (mr0), "1" (mr1), "2" (mr2), "3" (mr3),
	    "4" (mr4), "5" (mr5), "6" (mr6), "7" (mr7)

	    : /* clobbers */
	    "r2",  "r3", "r8", "r10", "r11",
	    "r17", "r18", "r19", "r20", "r21", "r22",
	    "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	    __L4_CALLER_SAVED_FP_REGS, __L4_CALLER_SAVED_PREDICATE_REGS,
	    __L4_CALLER_SAVED_BRANCH_REGS, __L4_CALLEE_SAVED_REGS);
    }
    else
    {
	r_to = to;
	r_FromSpecifier = FromSpecifier;
	r_Timeouts = Timeouts;

	__asm__ __volatile__ (
	    "/* L4_Ipc() */\n"
	    __L4_ASM_CALL (__L4_Ipc)
 
	    : /* outputs */
	    "=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
	    "=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),
	    "=r" (ret_from),

	    /* inputs (clobbered on output) */ 
	    "+r" (r_to), "+r" (r_FromSpecifier), "+r" (r_Timeouts)

	    : /* no unclobbered inputs */

	    : /* clobbers */
	    "r2",  "r3", "r8", "r10", "r11",
	    "r17", "r18", "r19", "r20", "r21", "r22",
	    "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	    __L4_CALLER_SAVED_FP_REGS, __L4_CALLER_SAVED_PREDICATE_REGS,
	    __L4_CALLER_SAVED_BRANCH_REGS, __L4_CALLEE_SAVED_REGS);
    }

    // Set sender id and store received MRs
    if (! L4_IsNilThread (FromSpecifier))
    {
	*from = ret_from;

	mrs[1] = mr1;
	mrs[2] = mr2;
	mrs[3] = mr3;
	mrs[4] = mr4;
	mrs[5] = mr5;
	mrs[6] = mr6;
	mrs[7] = mr7;
    }

    __asm__ __volatile__ ("	mov	ar.lc = %0	\n"
			  "	mov	ar.ec = %1	\n"
			  :
			  :
			  "r" (ar_lc), "r" (ar_ec));

    tag.raw = mr0;
    return tag;
}


L4_INLINE L4_MsgTag_t L4_Lipc (L4_ThreadId_t to,
			       L4_ThreadId_t FromSpecifier,
			       L4_Word_t Timeouts,
			       L4_ThreadId_t * from)
{
    return L4_Ipc (to, FromSpecifier, Timeouts, from);
}


L4_INLINE void L4_Unmap (L4_Word_t control)
{
    register L4_Word_t r_control	asm ("r14") = control;
    register L4_Word_t mr0 		asm ("out0");
    register L4_Word_t mr1 		asm ("out1");
    register L4_Word_t mr2 		asm ("out2");
    register L4_Word_t mr3 		asm ("out3");
    register L4_Word_t mr4 		asm ("out4");
    register L4_Word_t mr5 		asm ("out5");
    register L4_Word_t mr6 		asm ("out6");
    register L4_Word_t mr7 		asm ("out7");

    L4_Word_t * mrs = __L4_IA64_Utcb () + __L4_TCB_MR_OFFSET;

    mr0 = mrs[0];
    mr1 = mrs[1];
    mr2 = mrs[2];
    mr3 = mrs[3];
    mr4 = mrs[4];
    mr5 = mrs[5];
    mr6 = mrs[6];
    mr7 = mrs[7];

    __asm__ __volatile__ (
	"/* L4_Unmap() */\n"
	__L4_ASM_CALL(__L4_Unmap)
	: /* outputs */
	"=r" (mr0), "=r" (mr1), "=r" (mr2), "=r" (mr3),
	"=r" (mr4), "=r" (mr5), "=r" (mr6), "=r" (mr7),

	/* inputs (clobbered on output) */ 
	"+r" (r_control)

	: /* inputs */
	"0" (mr0), "1" (mr1), "2" (mr2), "3" (mr3),
	"4" (mr4), "5" (mr5), "6" (mr6), "7" (mr7)

	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS_NOOUT ("r8", "r9", "r10", "r11",
					"r15", "r16", "r17", "r18",
					"r19", "r20"));

    mrs[0] = mr0;
    mrs[1] = mr1;
    mrs[2] = mr2;
    mrs[3] = mr3;
    mrs[4] = mr4;
    mrs[5] = mr5;
    mrs[6] = mr6;
    mrs[7] = mr7;
}


L4_INLINE L4_Word_t L4_SpaceControl (L4_ThreadId_t SpaceSpecifier,
				     L4_Word_t control,
				     L4_Fpage_t KernelInterfacePageArea,
				     L4_Fpage_t UtcbArea,
				     L4_ThreadId_t redirector,
				     L4_Word_t *old_control)
{
    register L4_Word_t result		asm ("r8");
    register L4_Word_t ret_control	asm ("r9");
    register L4_ThreadId_t r_space	asm ("r14") = SpaceSpecifier;
    register L4_Word_t r_control	asm ("r15") = control;
    register L4_Fpage_t r_kiparea	asm ("r16") = KernelInterfacePageArea;
    register L4_Fpage_t r_utcbarea	asm ("r17") = UtcbArea;
    register L4_ThreadId_t r_redirector	asm ("r18") = redirector;

    __asm__ __volatile__ (
	"/* L4_SpaceControl() */				\n"
	__L4_ASM_CALL(__L4_SpaceControl)

	: /* outputs */
	"=r" (result),
	"=r" (ret_control),

	/* inputs (clobbered on output) */ 
	"+r" (r_space),
	"+r" (r_control),
	"+r" (r_kiparea),
	"+r" (r_utcbarea),
	"+r" (r_redirector)

	: /* no unclobbered inputs */
	
	: /* clobbers */
	__L4_CLOBBER_CALLER_REGS ("r10","r11","r19","r20"));

    *old_control = ret_control;

    return result;
}


L4_INLINE L4_Word_t L4_ProcessorControl (L4_Word_t ProcessorNo,
					 L4_Word_t InternalFrequency,
					 L4_Word_t ExternalFrequency,
					 L4_Word_t voltage)
{
    L4_KDB_Enter ("ProcessorControl");
    return 0;
}


L4_INLINE L4_Word_t L4_MemoryControl (L4_Word_t control,
				      const L4_Word_t * attributes)
{
    register L4_Word_t result		asm ("r8");
    register L4_Word_t r_ctrl		asm ("r14") = control;
    register L4_Word_t r_att0		asm ("r15") = attributes[0];
    register L4_Word_t r_att1		asm ("r16") = attributes[1];
    register L4_Word_t r_att2		asm ("r17") = attributes[2];
    register L4_Word_t r_att3		asm ("r18") = attributes[3];
    register L4_Word_t mr0 		asm ("out0");
    register L4_Word_t mr1 		asm ("out1");
    register L4_Word_t mr2 		asm ("out2");
    register L4_Word_t mr3 		asm ("out3");
    register L4_Word_t mr4 		asm ("out4");
    register L4_Word_t mr5 		asm ("out5");
    register L4_Word_t mr6 		asm ("out6");
    register L4_Word_t mr7 		asm ("out7");

    L4_Word_t * mrs = __L4_IA64_Utcb () + __L4_TCB_MR_OFFSET;

    mr0 = mrs[0];
    mr1 = mrs[1];
    mr2 = mrs[2];
    mr3 = mrs[3];
    mr4 = mrs[4];
    mr5 = mrs[5];
    mr6 = mrs[6];
    mr7 = mrs[7];

    __asm__ __volatile__ (
	"/* L4_MemoryControl */				\n"
	__L4_ASM_CALL(__L4_MemoryControl)

	: /* outputs */
	"=r" (result),

	"+r" (mr0), "+r" (mr1), "+r" (mr2), "+r" (mr3),
	"+r" (mr4), "+r" (mr5), "+r" (mr6), "+r" (mr7),

	/* inputs (clobbered on output) */ 
	"+r" (r_ctrl),
	"+r" (r_att0),
	"+r" (r_att1),
	"+r" (r_att2),
	"+r" (r_att3)
	:
	:
	__L4_CLOBBER_CALLER_REGS_NOOUT ("r9", "r10", "r11", "r19","r20"));

    return result;
}


L4_INLINE L4_Word_t L4_PAL_Call (L4_Word_t idx,
				 L4_Word_t a1, L4_Word_t a2, L4_Word_t a3,
				 L4_Word_t *r1, L4_Word_t *r2, L4_Word_t *r3)
{
    register L4_Word_t status		asm ("r8");
    register L4_Word_t ret1		asm ("r9");
    register L4_Word_t ret2		asm ("r10");
    register L4_Word_t ret3		asm ("r11");
    register L4_Word_t out0		asm ("r28") = idx;
    register L4_Word_t out1		asm ("r29") = a1;
    register L4_Word_t out2		asm ("r30") = a2;
    register L4_Word_t out3		asm ("r31") = a3;

    __asm__ __volatile__ (
	"/* L4_PAL_Call */				\n"
	__L4_ASM_CALL(__L4_PAL_Call)
	:
	"=r" (status),
	"=r" (ret1),
	"=r" (ret2),
	"=r" (ret3),

	/* inputs (clobbered on output) */ 
	"+r" (out0),
	"+r" (out1),
	"+r" (out2),
	"+r" (out3)
	:
	:
	"r2",  "r3", "r14", "r15", "r16", "r17", "r18", "r19",
	"r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
	"out4",	"out5", "out6", "out7",
	__L4_CALLER_SAVED_FP_REGS, __L4_CALLER_SAVED_PREDICATE_REGS,
	__L4_CALLER_SAVED_BRANCH_REGS, "ar.pfs");

    *r1 = ret1;
    *r2 = ret2;
    *r3 = ret3;

    return status;
}


L4_INLINE L4_Word_t L4_SAL_Call (L4_Word_t idx,
				 L4_Word_t a1, L4_Word_t a2, L4_Word_t a3,
				 L4_Word_t a4, L4_Word_t a5, L4_Word_t a6,
				 L4_Word_t *r1, L4_Word_t *r2, L4_Word_t *r3)
{
    register L4_Word_t status		asm ("r8");
    register L4_Word_t ret1		asm ("r9");
    register L4_Word_t ret2		asm ("r10");
    register L4_Word_t ret3		asm ("r11");
    register L4_Word_t out0		asm ("out0") = idx;
    register L4_Word_t out1		asm ("out1") = a1;
    register L4_Word_t out2		asm ("out2") = a2;
    register L4_Word_t out3		asm ("out3") = a3;
    register L4_Word_t out4		asm ("out4") = a4;
    register L4_Word_t out5		asm ("out5") = a5;
    register L4_Word_t out6		asm ("out6") = a6;

    __asm__ __volatile__ (
	"/* L4_SAL_Call */				\n"
	__L4_ASM_CALL(__L4_SAL_Call)
	:
	"=r" (status),
	"=r" (ret1),
	"=r" (ret2),
	"=r" (ret3),

	/* inputs (clobbered on output) */ 
	"+r" (out0),
	"+r" (out1),
	"+r" (out2),
	"+r" (out3),
	"+r" (out4),
	"+r" (out5),
	"+r" (out6)
	:
	:
	__L4_CLOBBER_CALLER_REGS_NOOUT ("r14", "r15", "r16", "r17", 
					"r18", "r19", "r20", "out7"));

    *r1 = ret1;
    *r2 = ret2;
    *r3 = ret3;

    return status;
}


#endif /* !__L4__IA64__SYSCALLS_H__ */
