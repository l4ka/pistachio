/*********************************************************************
 *                
 * Copyright (C) 2001, 2002, 2003,  Karlsruhe University
 *                
 * File path:     l4/sigma0.h
 * Description:   Sigma0 RPC protocol
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
 * $Id: sigma0.h,v 1.10 2003/09/24 19:06:21 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __L4__SIGMA0_H__
#define __L4__SIGMA0_H__

#include <l4/types.h>
#include <l4/kip.h>
#include <l4/message.h>
#include <l4/ipc.h>

L4_INLINE L4_Fpage_t L4_Sigma0_GetPage_RcvWindow (L4_ThreadId_t s0,
						  L4_Fpage_t f,
						  L4_Fpage_t RcvWindow)
{
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    if (L4_IsNilThread (s0))
    {
	L4_KernelInterfacePage_t * kip = (L4_KernelInterfacePage_t *)
	    L4_GetKernelInterface ();
	s0 = L4_GlobalId (kip->ThreadInfo.X.UserBase, 1);
    }

    L4_MsgClear (&msg);
    L4_MsgAppendWord (&msg, f.raw);
    L4_MsgAppendWord (&msg, 0);
    L4_Set_MsgLabel (&msg, (L4_Word_t) -6UL << 4);
    L4_MsgLoad (&msg);
    L4_Accept (L4_MapGrantItems (RcvWindow));
	
    tag = L4_Call (s0);
    if (L4_IpcFailed (tag))
	return L4_Nilpage;

    L4_StoreMR (2, &f.raw);
    return f;
}

#if defined(__cplusplus)
L4_INLINE L4_Fpage_t L4_Sigma0_GetPage (L4_ThreadId_t s0,
					L4_Fpage_t f,
					L4_Fpage_t RcvWindow)
{
    return L4_Sigma0_GetPage_RcvWindow (s0, f, RcvWindow);
}
#endif

L4_INLINE L4_Fpage_t L4_Sigma0_GetPage (L4_ThreadId_t s0, L4_Fpage_t f)
{
    return L4_Sigma0_GetPage_RcvWindow (s0, f, L4_CompleteAddressSpace);
}

L4_INLINE L4_Fpage_t L4_Sigma0_GetAny (L4_ThreadId_t s0,
				      L4_Word_t	s,
				      L4_Fpage_t RcvWindow)
{
    return L4_Sigma0_GetPage_RcvWindow (s0, L4_FpageLog2 (~0UL, s), RcvWindow);
}


#endif /* !__L4__SIGMA0_H__ */

