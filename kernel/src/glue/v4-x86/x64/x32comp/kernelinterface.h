/*********************************************************************
 *                
 * Copyright (C) 2006-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/x32comp/kernelinterface.h
 * Description:   Version 4 kernel-interface page for Compatibility Mode
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
 * $Id: kernelinterface.h,v 1.2 2006/10/20 14:47:27 reichelt Exp $
 *                
 ********************************************************************/


#ifndef __GLUE__V4_X86__X64__X32COMP__KERNELINTERFACE_H__
#define __GLUE__V4_X86__X64__X32COMP__KERNELINTERFACE_H__

#include INC_GLUE_SA(x32comp/types.h)

#undef KIP
#undef KIP_BITS_WORD
#undef KIP_SECTION
#undef KIP_PROC_DESC_LOG2SIZE

#define KIP			kip_32
#define KIP_BITS_WORD		32
#define KIP_SECTION		"kip_32"
#define KIP_PROC_DESC_LOG2SIZE	4

namespace x32 {

#undef __API__V4__KERNELINTERFACE_H__
#undef __API__V4__MEMDESC_H__
#undef __API__V4__PROCDESC_H__
#undef __GENERIC__MEMREGION_H__
#include INC_API(kernelinterface.h)

}

#ifndef KIP_SECONDARY
#undef KIP
#undef KIP_BITS_WORD
#undef KIP_SECTION
#undef KIP_PROC_DESC_LOG2SIZE
#endif


#endif /* !__GLUE__V4_X86__X64__X32COMP__KERNELINTERFACE_H__ */
