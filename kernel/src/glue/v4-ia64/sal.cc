/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/sal.cc
 * Description:   User-level interface to SAL procedures
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
 * $Id: sal.cc,v 1.5 2003/09/24 19:05:35 skoglund Exp $
 *                
 ********************************************************************/
#include INC_GLUE(syscalls.h)
#include INC_ARCH(sal.h)
#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT (SYSCALL_SAL_CALL);


SYS_SAL_CALL (word_t idx, word_t a1, word_t a2, word_t a3,
	      word_t a4, word_t a5, word_t a6)
{
    switch (idx)
    {
    case SAL_PCI_CONFIG_READ:
    case SAL_PCI_CONFIG_WRITE:
	/*
	 * Access to PCI configuration space.
	 * TODO: Restrict access to configuration space.
	 */

	TRACEPOINT (SYSCALL_SAL_CALL,
		    printf ("SYS_SAL_CALL: PCI_Config%s (%p, %d, %p)\n",
			    idx == SAL_PCI_CONFIG_READ ? "Read" : "Write",
			    a1, a2, a3));

	return ia64_sal_entry ((sal_function_id_e) idx,
			       a1, a2, a3, a4, a5, a6, 0);
    }

    /*
     * Unsupported/unimplemented SAL call.
     */

    sal_return_t ret;
    ret.status = SAL_UNIMPLEMENTED;
    ret.raw[1] = ret.raw[2] = ret.raw[3] = 0;

    TRACEPOINT (SYSCALL_SAL_CALL,
		printf ("SYS_SAL_CALL: idx=%p (%p, %p, %p, %p, %p, %p)\n",
			idx, a1, a2, a3, a4, a5, a6));

    return ret;
}

