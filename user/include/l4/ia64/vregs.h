/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/ia64/vregs.h
 * Description:   IA64 virtual register ABI
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
 * $Id: vregs.h,v 1.8 2003/09/24 19:06:24 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__IA64__VREGS_H__
#define __L4__IA64__VREGS_H__


//#define __L4_OLD_IPCABI


L4_INLINE L4_Word_t * __L4_IA64_Utcb (void) __attribute__ ((const));
L4_INLINE L4_Word_t * __L4_IA64_Utcb (void)
{
    L4_Word_t * utcb;

    __asm__ (
	"/* __L4_IA64_Utcb() */			\n"
	"	mov %0 = ar.k6			\n"

	: /* outputs */
	"=r" (utcb)

	/* no inputs */
	/* no clobbers */
	);

    return utcb;
}



/*
 * Location of TCRs within UTCB.
 */

#define __L4_TCB_MR_OFFSET			(48)
#define __L4_TCR_THREAD_WORD_1			(44)
#define __L4_TCR_THREAD_WORD_0			(43)
#define __L4_TCB_BR_OFFSET			(10)
#define __L4_TCR_ERROR_CODE			(9)
#define __L4_TCR_VIRTUAL_ACTUAL_SENDER		(8)
#define __L4_TCR_INTENDED_RECEIVER		(7)
#define __L4_TCR_XFER_TIMEOUT			(6)
#define __L4_TCR_PREEMPT_FLAGS			(5)
#define __L4_TCR_COP_FLAGS			(5)
#define __L4_TCR_EXCEPTION_HANDLER		(4)
#define __L4_TCR_PAGER				(3)
#define __L4_TCR_USER_DEFINED_HANDLE		(2)
#define __L4_TCR_PROCESSOR_NO			(1)



/*
 * Thread Control Registers.
 */

L4_INLINE L4_Word_t __L4_TCR_MyGlobalId (void)
{
    L4_Word_t tid;

    __asm__ (
	"/* __L4_TCR_MyGlobalId() */		\n"
	"	mov %0 = ar.k5			\n"

	: /* outputs */
	"=r" (tid)

	/* no inputs */
	/* no clobbers */
	);

    return tid;
}

L4_INLINE L4_Word_t __L4_TCR_MyLocalId (void)
{
    L4_Word_t *dummy = __L4_IA64_Utcb ();
    return (L4_Word_t) dummy;
}

L4_INLINE L4_Word_t __L4_TCR_ProcessorNo (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_PROCESSOR_NO];
}

L4_INLINE L4_Word_t __L4_TCR_UserDefinedHandle (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_USER_DEFINED_HANDLE];
}

L4_INLINE void __L4_TCR_Set_UserDefinedHandle (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_USER_DEFINED_HANDLE] = w;
}

L4_INLINE L4_Word_t __L4_TCR_Pager (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_PAGER];
}

L4_INLINE void __L4_TCR_Set_Pager (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_PAGER] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ExceptionHandler (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_EXCEPTION_HANDLER];
}

L4_INLINE void __L4_TCR_Set_ExceptionHandler (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_EXCEPTION_HANDLER] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ErrorCode (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_ERROR_CODE];
}

L4_INLINE L4_Word_t __L4_TCR_XferTimeout (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_XFER_TIMEOUT];
}

L4_INLINE void __L4_TCR_Set_XferTimeout (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_XFER_TIMEOUT] = w;
}

L4_INLINE L4_Word_t __L4_TCR_IntendedReceiver(void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_INTENDED_RECEIVER];
}

L4_INLINE L4_Word_t __L4_TCR_ActualSender (void)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_VIRTUAL_ACTUAL_SENDER];
}

L4_INLINE void __L4_TCR_Set_VirtualSender (L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_VIRTUAL_ACTUAL_SENDER] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ThreadWord (L4_Word_t n)
{
    return (__L4_IA64_Utcb ())[__L4_TCR_THREAD_WORD_0 + n];
}

L4_INLINE void __L4_TCR_Set_ThreadWord (L4_Word_t n, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCR_THREAD_WORD_0 + n] = w;
}

L4_INLINE void L4_Set_CopFlag (L4_Word_t n)
{
    L4_Word8_t old, readval;

    do {
	old = *(L4_Word8_t *) &(__L4_IA64_Utcb ())[__L4_TCR_COP_FLAGS];

	__asm__ __volatile__ (
	    "	mov	r14 = ar.ccv			\n"
	    "	mov	ar.ccv = %2 ;;			\n"
	    "	cmpxchg1.acq %0 = [%1], %3, ar.ccv ;;	\n"
	    "	mov	ar.ccv = r14			\n"
	    :
	    "=r" (readval)
	    :
	    "r" (&(__L4_IA64_Utcb ())[__L4_TCR_COP_FLAGS]),
	    "r" (old),
	    "r" (old | (1 << n))
	    :
	    "r14");
    } while (old != readval);
}

L4_INLINE void L4_Clr_CopFlag (L4_Word_t n)
{
    L4_Word8_t old, readval;

    do {
	old = *(L4_Word8_t *) &(__L4_IA64_Utcb ())[__L4_TCR_COP_FLAGS];

	__asm__ __volatile__ (
	    "	mov	r14 = ar.ccv			\n"
	    "	mov	ar.ccv = %2 ;;			\n"
	    "	cmpxchg1.acq %0 = [%1], %3, ar.ccv ;;	\n"
	    "	mov	ar.ccv = r14			\n"
	    :
	    "=r" (readval)
	    :
	    "r" (&(__L4_IA64_Utcb ())[__L4_TCR_COP_FLAGS]),
	    "r" (old),
	    "r" (old & ~(1 << n))
	    :
	    "r14");
    } while (old != readval);
}

L4_INLINE L4_Bool_t L4_EnablePreemptionFaultException (void)
{
    return 0;
}

L4_INLINE L4_Bool_t L4_DisablePreemptionFaultException (void)
{
    return 0;
}

L4_INLINE L4_Bool_t L4_EnablePreemption (void)
{
    return 0;
}

L4_INLINE L4_Bool_t L4_DisablePreemption (void)
{
    return 0;
}

L4_INLINE L4_Bool_t L4_PreemptionPending (void)
{
    return 0;
}



/*
 * Message Registers.
 */

#if defined(__L4_OLD_IPCABI)
L4_INLINE int __L4_IsNaTCollection (L4_Word_t * addr)
{
    return ((L4_Word_t) addr & 0x1f8) == 0x1f8;
}

L4_INLINE L4_Word_t * __L4_PrevNaTCollection (L4_Word_t * addr)
{
    return (L4_Word_t *) (((L4_Word_t) addr & ~0x1ff) - 8);
}

L4_INLINE void L4_StoreMR (int i, L4_Word_t * w)
{
    L4_Word_t * ptr = &(__L4_IA64_Utcb ())[__L4_TCB_MR_OFFSET + i];
    L4_Word_t * nat = __L4_PrevNaTCollection (ptr);

    if ((ptr - nat) <= i)
	ptr++;

    if (__L4_IsNaTCollection (ptr))
	ptr++;

    *w = *ptr;
}

L4_INLINE void L4_LoadMR (int i, L4_Word_t w)
{
    L4_Word_t * ptr = &(__L4_IA64_Utcb ())[__L4_TCB_MR_OFFSET + i];
    L4_Word_t * nat = __L4_PrevNaTCollection (ptr);

    if ((ptr - nat) <= i)
	ptr++;

    if (__L4_IsNaTCollection (ptr))
	ptr++;
    
    *ptr = w;
}

L4_INLINE void L4_StoreMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_MR_OFFSET + i;
    L4_Word_t * nat = __L4_PrevNaTCollection (mr);

    if ((mr - nat) <= i)
	mr++;

    while (k-- > 0)
    {
	if (__L4_IsNaTCollection (mr))
	    mr++;
	*w++ = *mr++;
    }
}

L4_INLINE void L4_LoadMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_MR_OFFSET + i;
    L4_Word_t * nat = __L4_PrevNaTCollection (mr);

    if ((mr - nat) <= i)
	mr++;

    while (k-- > 0)
    {
	if (__L4_IsNaTCollection (mr))
	    mr++;
	*mr++ = *w++;
    }
}

#else /* !__L4_OLD_IPCABI */

L4_INLINE void L4_StoreMR (int i, L4_Word_t * w)
{
    *w = (__L4_IA64_Utcb ())[__L4_TCB_MR_OFFSET + i];
}

L4_INLINE void L4_LoadMR (int i, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCB_MR_OFFSET + i] = w;
}

L4_INLINE void L4_StoreMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_MR_OFFSET + i;

    while (k-- > 0)
	*w++ = *mr++;
}

L4_INLINE void L4_LoadMRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_MR_OFFSET + i;

    while (k-- > 0)
	*mr++ = *w++;
}

#endif





/*
 * Buffer Registers.
 */

L4_INLINE void L4_StoreBR (int i, L4_Word_t * w)
{
    *w = (__L4_IA64_Utcb ())[__L4_TCB_BR_OFFSET + i];
}

L4_INLINE void L4_LoadBR (int i, L4_Word_t w)
{
    (__L4_IA64_Utcb ())[__L4_TCB_BR_OFFSET + i] = w;
}

L4_INLINE void L4_StoreBRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_BR_OFFSET + i;

    while (k-- > 0)
	*w++ = *mr++;
}

L4_INLINE void L4_LoadBRs (int i, int k, L4_Word_t * w)
{
    L4_Word_t * mr = (__L4_IA64_Utcb ()) + __L4_TCB_BR_OFFSET + i;

    while (k-- > 0)
	*mr++ = *w++;
}

#endif /* !__L4__IA64__VREGS_H__ */
