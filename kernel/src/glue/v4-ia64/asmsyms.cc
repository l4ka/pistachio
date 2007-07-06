/*********************************************************************
 *                
 * Copyright (C) 2002-2005,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/asmsyms.cc
 * Description:   Various asm definitions for ia64
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
 * $Id: asmsyms.cc,v 1.11 2005/10/19 16:22:24 skoglund Exp $
 *                
 ********************************************************************/
#include <mkasmsym.h>

#include INC_GLUE(context.h)
#include INC_GLUE(utcb.h)
#include INC_API(procdesc.h)
#include INC_API(kernelinterface.h)
#include INC_API(threadstate.h)


MKASMSYM (SIZEOF_SWITCH_CONTEXT, sizeof (ia64_switch_context_t));
MKASMSYM (SIZEOF_EXCEPTION_CONTEXT, sizeof (ia64_exception_context_t));

MKASMSYM (EXC_CONTEXT_IPSR_OFFSET, offsetof (ia64_exception_context_t, ipsr));
MKASMSYM (PROCDESC_ARCH1_OFFSET, offsetof (procdesc_t, arch1));
MKASMSYM (KIP_PROCDESC_PTR_OFFSET,
	  offsetof (kernel_interface_page_t, proc_desc_ptr));
MKASMSYM (SWITCH_CTX_PFS_OFFSET, offsetof (ia64_switch_context_t, pfs));

MKASMSYM (UTCB_MR_OFFSET, offsetof (utcb_t, mr));
MKASMSYM (UTCB_PROCESSOR_OFFSET, offsetof (utcb_t, processor_no));

MKASMSYM (TSTATE_POLLING, (word_t) thread_state_t::polling);
MKASMSYM (TSTATE_WAITING_FOREVER, (word_t) thread_state_t::waiting_forever);
MKASMSYM (TSTATE_RUNNING, (word_t) thread_state_t::running);
