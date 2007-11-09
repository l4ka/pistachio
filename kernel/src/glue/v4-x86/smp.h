/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/smp.h
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
 * $Id: smp.h,v 1.3 2003/09/24 19:04:36 skoglund Exp $
 *                
 ********************************************************************/


#ifndef __GLUE__V4_X86__SMP_H__
#define __GLUE__V4_X86__SMP_H__

#ifndef __API__V4__SMP_H__
#error do not include glue/v4-ia32/smp.h directly -- use api/v4/smp.h
#endif

INLINE void sync_entry_t::set_pending(cpuid_t cpu)
{
    asm ("lock; or %0, %1\n"
	 :
	 : "r"(1 << cpu), "m"(this->pending_mask));
}
 
INLINE void sync_entry_t::clear_pending(cpuid_t cpu)
{ 
    asm ("lock; and %0, %1\n"
	 :
	 : "r"(~(1 << cpu)), "m"(this->pending_mask));
}

INLINE void sync_entry_t::ack(cpuid_t cpu)
{
    ack_mask = 1 << cpu;
}

/**
 * initializes XCPU handling
 */
void init_xcpu_handling();

#endif /* !__GLUE__V4_X86__SMP_H__ */
