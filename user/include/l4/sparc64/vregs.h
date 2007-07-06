/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  University of New South Wales
 *                
 * File path:     l4/sparc64/vregs.h
 * Description:   
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
 * $Id: vregs.h,v 1.3 2004/02/10 03:49:37 philipd Exp $
 *                
 ********************************************************************/

#ifndef __L4__SPARC64__VREGS_H__
#define __L4__SPARC64__VREGS_H__

L4_INLINE L4_Word_t * __L4_Sparc64_Utcb(void) __attribute__ ((const));

L4_INLINE L4_Word_t * __L4_Sparc64_Utcb(void)
{
    register L4_Word_t *utcb asm ("g7");
    return utcb;
}

/************************
* TCR locations in UTCB *
************************/

#define __L4_TCR_GLOBAL_ID             0
#define __L4_TCR_PAGER                 1
#define __L4_TCR_EXCEPTION_HANDLER     2
#define __L4_TCR_USER_DEFINED_HANDLE   3
#define __L4_TCR_XFER_TIMEOUT          4
#define __L4_TCR_ERROR_CODE            5
#define __L4_TCR_INTENDED_RECEIVER     6
#define __L4_TCR_VIRTUAL_ACTUAL_SENDER 7
#define __L4_TCR_PROCESSOR_NO          8
#define __L4_TCR_PREEMPT_FLAGS         9
#define __L4_TCR_COP_FLAGS             9
#define __L4_TCR_THREAD_WORD0          10
#define __L4_TCR_THREAD_WORD1          11
#define __L4_TCR_BR_OFFSET             31
#define __L4_TCR_MR_OFFSET             64


/***************************
* Thread Control Registers *
***************************/

L4_INLINE L4_Word_t
__L4_TCR_MyGlobalId(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_GLOBAL_ID];
} // __L4_TCR_MyGlobalId()

L4_INLINE L4_Word_t
__L4_TCR_MyLocalId(void)
{
    return (L4_Word_t)__L4_Sparc64_Utcb();
} // __L4_TCR_MyLocalId()

L4_INLINE L4_Word_t
__L4_TCR_ProcessorNo(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_PROCESSOR_NO];
} // __L4_TCR_ProcessorNo()

L4_INLINE L4_Word_t
__L4_TCR_UserDefinedHandle(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_USER_DEFINED_HANDLE];
} // __L4_TCR_UserDefinedHandle()

L4_INLINE void
__L4_TCR_Set_UserDefinedHandle(L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_USER_DEFINED_HANDLE] =  w;
} // __L4_TCR_Set_UserDefinedHandle()

L4_INLINE L4_Word_t
__L4_TCR_Pager(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_PAGER];

} // __L4_TCR_Pager()

L4_INLINE void
__L4_TCR_Set_Pager(L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_PAGER] = w;
} // __L4_TCR_Set_Pager()

L4_INLINE L4_Word_t
__L4_TCR_ExceptionHandler(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_EXCEPTION_HANDLER];
} // __L4_TCR_ExceptionHandler()

L4_INLINE void
__L4_TCR_Set_ExceptionHandler(L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_EXCEPTION_HANDLER] = w;
} // __L4_TCR_Set_ExceptionHandler()

L4_INLINE L4_Word_t
__L4_TCR_ErrorCode(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_ERROR_CODE];
} // __L4_TCR_Error_Code()

L4_INLINE L4_Word_t
__L4_TCR_XferTimeout(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_XFER_TIMEOUT];
} // __L4_TCR_XferTimeout()

L4_INLINE void
__L4_TCR_Set_XferTimeout(L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_XFER_TIMEOUT] = w;
} // __L4_TCR_Set_XferTimeout()

L4_INLINE L4_Word_t
__L4_TCR_IntendedReceiver(void)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_INTENDED_RECEIVER];
} // __L4_TCR_IntendedReceiver()

L4_INLINE L4_Word_t
__L4_TCR_ActualSender()
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_VIRTUAL_ACTUAL_SENDER];
} // __L4_TCR_ActualSender()

L4_INLINE void
__L4_TCR_Set_VirtualSender(L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_VIRTUAL_ACTUAL_SENDER] = w;
} // __L4_TCR_Set_VirtualSender()

L4_INLINE L4_Word_t
__L4_TCR_ThreadWord(L4_Word_t n)
{
    return (__L4_Sparc64_Utcb())[__L4_TCR_THREAD_WORD0 + n];
} // __L4_TCR_ThreadWord()

L4_INLINE void
__L4_TCR_Set_ThreadWord(L4_Word_t n, L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_THREAD_WORD0 + n] = w;
} // __L4_TCR_Set_ThreadWord()

L4_INLINE void
L4_Set_CopFlag(L4_Word_t n)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_COP_FLAGS + n] |= (1 << (8 + n));
} // L4_Set_CopFlag()

L4_INLINE void
L4_Clr_CopFlag(L4_Word_t n)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_COP_FLAGS + n] &= ~(1 << (8 + n));
} // L4_Clr_CopFlag()

L4_INLINE L4_Bool_t
L4_EnablePreemptionFaultException(void)
{
    L4_Bool_t old = ((__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] >> 5) & 1;
    (__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] |= (1 << 5);
    return old;
} // L4_EnablePreemptionFaultException()

L4_INLINE L4_Bool_t
L4_DisablePreemptionFaultException(void)
{
    L4_Bool_t old = ((__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] >> 5) & 1;
    (__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] &= ~(1 << 5);
    return old;
} // L4_DisablePreemptionFaultException()

L4_INLINE L4_Bool_t
L4_EnablePreemption(void)
{
    L4_Bool_t old = ((__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] >> 6) & 1;
    (__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] |= (1 << 6);
    return old;
} // L4_EnablePreemption()

L4_INLINE L4_Bool_t
L4_DisablePreemption(void)
{
    L4_Bool_t old = ((__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] >> 6) & 1;
    (__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] &= ~(1 << 6);
    return old;
} // L4_DisablePreemption()

L4_INLINE L4_Bool_t
L4_PreemptionPending(void)
{
    return ((__L4_Sparc64_Utcb())[__L4_TCR_PREEMPT_FLAGS] >> 7) & 1;
} // L4_PreemptionPending()


/********************
* Message Registers *
********************/

L4_INLINE void
L4_StoreMR(int i, L4_Word_t * w)
{
    *w = (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + i];
} // L4_StoreMR()

L4_INLINE void
L4_LoadMR(int i, L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_MR_OFFSET + i] = w;
} // L4_LoadMR()

L4_INLINE void
L4_StoreMRs(int i, int k, L4_Word_t * w)
{
    L4_Word_t* mr = __L4_Sparc64_Utcb() + __L4_TCR_MR_OFFSET + i;

    while (k-- > 0) {
	*w++ = *mr++;
    }
} // L4_StoreMRs()

L4_INLINE void
L4_LoadMRs(int i, int k, L4_Word_t * w)
{
    L4_Word_t* mr = __L4_Sparc64_Utcb() + __L4_TCR_MR_OFFSET + i;

    while (k-- > 0) {
	*mr++ = *w++;
    }
} // L4_LoadMRs()


/*******************
* Buffer Registers *
*******************/

L4_INLINE void
L4_StoreBR(int i, L4_Word_t * w)
{
    *w = (__L4_Sparc64_Utcb())[__L4_TCR_BR_OFFSET + i];
} // L4_StoreBR()

L4_INLINE void
L4_LoadBR(int i, L4_Word_t w)
{
    (__L4_Sparc64_Utcb())[__L4_TCR_BR_OFFSET + i] = w;
} // L4_LoadBR()

L4_INLINE void
L4_StoreBRs(int i, int k, L4_Word_t * w)
{
    L4_Word_t* br = __L4_Sparc64_Utcb() + __L4_TCR_BR_OFFSET + i;

    while (k-- > 0) {
	*w++ = *br++;
    }
} // L4_StoreBRs()

L4_INLINE void
L4_LoadBRs(int i, int k, L4_Word_t * w)
{
    L4_Word_t* br = __L4_Sparc64_Utcb() + __L4_TCR_BR_OFFSET + i;

    while (k-- > 0) {
	*br++ = *w++;
    }
} // L4_LoadBRs()


#endif /* !__L4__SPARC64__VREGS_H__ */
