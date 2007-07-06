/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     l4/arm/vregs.h
 * Description:   ARM virtual register ABI
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
 * $Id: vregs.h,v 1.11 2006/02/23 21:02:29 ud3 Exp $
 *
 ********************************************************************/

#ifndef __L4__ARM__VREGS_H__
#define __L4__ARM__VREGS_H__

#define L4_TRAP_KPUTC		0xffffffa0
#define L4_TRAP_KGETC		0xffffffa4
#define L4_TRAP_KGETC_NB	0xffffffa8
#define L4_TRAP_KDEBUG		0xffffffac
#define L4_TRAP_GETUTCB		0xffffffb0
#define L4_TRAP_KIP		0xffffffb4
#define L4_TRAP_KSET_THRD_NAME	0xffffffb8

#define USER_UTCB_REF           0xff000000

L4_INLINE L4_Word_t *__L4_ARM_Utcb( void ) __attribute__ ((const));
L4_INLINE L4_Word_t *__L4_ARM_Utcb( void )
{
    return *(L4_Word_t **)(USER_UTCB_REF);
}

/*
 * Location of TCRs within UTCB.
 */

#define __L4_TCR_BR_OFFSET		        (80)
#define __L4_TCR_MR_OFFSET		        (16)
#define __L4_TCR_VIRTUAL_ACTUAL_SENDER		(9)
#define __L4_TCR_INTENDED_RECEIVER		(8)
#define __L4_TCR_ERROR_CODE			(7)
#define __L4_TCR_XFER_TIMEOUT			(6)
#define __L4_TCR_COP_FLAGS			(5)
#define __L4_TCR_PREEMPT_FLAGS			(5)
#define __L4_TCR_EXCEPTION_HANDLER		(4)
#define __L4_TCR_PAGER				(3)
#define __L4_TCR_USER_DEFINED_HANDLE		(2)
#define __L4_TCR_PROCESSOR_NO			(1)
#define __L4_TCR_MY_GLOBAL_ID			(0)

/*
 * Thread Control Registers.
 */

L4_INLINE L4_Word_t __L4_TCR_MyGlobalId( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_MY_GLOBAL_ID ];
}

L4_INLINE L4_Word_t __L4_TCR_MyLocalId( void )
{
    return (L4_Word_t) __L4_ARM_Utcb();
}

L4_INLINE L4_Word_t __L4_TCR_ProcessorNo( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_PROCESSOR_NO ];
}

L4_INLINE L4_Word_t __L4_TCR_UserDefinedHandle( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_USER_DEFINED_HANDLE ];
}

L4_INLINE void __L4_TCR_Set_UserDefinedHandle( L4_Word_t w )
{
    (__L4_ARM_Utcb())[ __L4_TCR_USER_DEFINED_HANDLE ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_Pager( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_PAGER ];
}

L4_INLINE void __L4_TCR_Set_Pager( L4_Word_t w )
{
    (__L4_ARM_Utcb())[ __L4_TCR_PAGER ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ExceptionHandler( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_EXCEPTION_HANDLER ];
}

L4_INLINE void __L4_TCR_Set_ExceptionHandler( L4_Word_t w )
{
    (__L4_ARM_Utcb())[ __L4_TCR_EXCEPTION_HANDLER ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_ErrorCode( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_ERROR_CODE ];
}

L4_INLINE L4_Word_t __L4_TCR_XferTimeout( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_XFER_TIMEOUT ];
}

L4_INLINE void __L4_TCR_Set_XferTimeout( L4_Word_t w )
{
    (__L4_ARM_Utcb())[ __L4_TCR_XFER_TIMEOUT ] = w;
}

L4_INLINE L4_Word_t __L4_TCR_IntendedReceiver( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_INTENDED_RECEIVER ];
}

L4_INLINE L4_Word_t __L4_TCR_ActualSender( void )
{
    return (__L4_ARM_Utcb())[ __L4_TCR_VIRTUAL_ACTUAL_SENDER ];
}

L4_INLINE void __L4_TCR_Set_VirtualSender( L4_Word_t w )
{
    (__L4_ARM_Utcb())[ __L4_TCR_VIRTUAL_ACTUAL_SENDER ] = w;
}

L4_INLINE void L4_Set_CopFlag( L4_Word_t n )
{
    (__L4_ARM_Utcb())[ __L4_TCR_COP_FLAGS ] |= (1 << n);
}

L4_INLINE void L4_Clr_CopFlag( L4_Word_t n )
{
    (__L4_ARM_Utcb())[ __L4_TCR_COP_FLAGS ] &= ~(1 << n);
}

L4_INLINE L4_Bool_t L4_EnablePreemptionFaultException( void )
{
    L4_Bool_t old = ((__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 5) & 1;
    (__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] |= (1 << 5);
    return old;
}

L4_INLINE L4_Bool_t L4_DisablePreemptionFaultException( void )
{
    L4_Bool_t old = ((__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 5) & 1;
    (__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] &= ~(1 << 5);
    return old;
}

L4_INLINE L4_Bool_t L4_EnablePreemption( void )
{
    L4_Bool_t old = ((__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 6) & 1;
    (__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] |= (1 << 6);
    return old;
}

L4_INLINE L4_Bool_t L4_DisablePreemption( void )
{
    L4_Bool_t old = ((__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] >> 6) & 1;
    (__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ] &= ~(1 << 6);
    return old;
}

L4_INLINE L4_Bool_t L4_PreemptionPending( void )
{
    L4_Bool_t retval = (((__L4_ARM_Utcb())[ __L4_TCR_PREEMPT_FLAGS ]) >> 7) & 1;
    return retval;
}



/*
 * Message Registers.
 */

L4_INLINE void L4_StoreMR( int i, L4_Word_t * w )
{
    *w = (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET +  i ];
}

L4_INLINE void L4_LoadMR( int i, L4_Word_t w )
{
    (__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET + i ] = w;
}

L4_INLINE void L4_StoreMRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * mr = &(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET + i];

    while( k-- > 0 ) {
	*w++ = *mr++;
    }
}

L4_INLINE void L4_LoadMRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * mr = &(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET + i];

    while( k-- > 0 )
	*mr++ = *w++;
}

/*
 * Buffer Registers.
 */

L4_INLINE void L4_StoreBR( int i, L4_Word_t * w )
{
    *w = (__L4_ARM_Utcb())[__L4_TCR_BR_OFFSET + i ];
}

L4_INLINE void L4_LoadBR( int i, L4_Word_t w )
{

    (__L4_ARM_Utcb())[__L4_TCR_BR_OFFSET + i ] = w;
}

L4_INLINE void L4_StoreBRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * br = &(__L4_ARM_Utcb())[__L4_TCR_BR_OFFSET + i];

    while( k-- > 0 )
	*w++ = *br++;
}

L4_INLINE void L4_LoadBRs( int i, int k, L4_Word_t * w )
{
    L4_Word_t * br = &(__L4_ARM_Utcb())[__L4_TCR_BR_OFFSET +  i];

    while( k-- > 0 )
	*br++ = *w++;
}


#endif	/* __L4__ARM__VREGS_H__ */

