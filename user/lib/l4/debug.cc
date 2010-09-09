/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      debug.cc
 * Author:        Jan Stoess <stoess@kit.edu>
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
 ********************************************************************/
#include <l4/kip.h>
#include <l4/tracebuffer.h>

L4_TraceBuffer_t *__L4_Tracebuffer = 0;

L4_TraceBuffer_t *L4_GetTraceBuffer()
{
    static L4_Bool_t initialized = false;
    
    if (!initialized)
    {
        L4_Word_t dummy;
        L4_KernelInterfacePage_t *kip;
        initialized = true;
    
        kip = (L4_KernelInterfacePage_t *)  L4_KernelInterface( &dummy, &dummy, &dummy );
        
        if (L4_HasFeature("tracebuffer"))
        {
            for( L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++ )
            {

                L4_MemoryDesc_t *mdesc = L4_MemoryDesc( kip, i );
                if( L4_MemoryDescType(mdesc) == L4_ReservedMemoryType && L4_IsVirtual(mdesc))
                {
                    __L4_Tracebuffer = (L4_TraceBuffer_t*) L4_MemoryDescLow(mdesc);
                    
                    if (__L4_Tracebuffer->magic != L4_TRACEBUFFER_MAGIC)
                        __L4_Tracebuffer = 0;
                    break;
                }
           
            }
        }
    }    
    return __L4_Tracebuffer;
}
