/*********************************************************************
 *                
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-mips64/resource_fucntions.h
 * Description:   Resource helpers for mips64
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
 * $Id: resource_functions.h,v 1.2 2004/06/04 02:32:31 cvansch Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_MIPS64__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_MIPS64__RESOURCE_FUNCTIONS_H__

#include INC_API(tcb.h)
#include INC_API(resources.h)

/**
 * Mark the thread as being in an exception IPC
 *
 * @param tcb			current TCB
 */
INLINE void thread_resources_t::set_exception_ipc (tcb_t * tcb)
{
    tcb->resource_bits += EXCEPTION;
}


/**
 * Clear the exception IPC bit
 *
 * @param tcb			current TCB
 */
INLINE void thread_resources_t::clear_exception_ipc (tcb_t * tcb)
{
    tcb->resource_bits -= EXCEPTION;
}

#endif /*__GLUE__V4_MIPS64__RESOURCE_FUNCTIONS_H__*/
