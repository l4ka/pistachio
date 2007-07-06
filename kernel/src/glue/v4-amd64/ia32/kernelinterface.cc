/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/ia32/kernelinterface.cc
 * Description:   kernel interface page for 32-bit programs
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
 * $Id: kernelinterface.cc,v 1.3 2006/10/27 17:18:53 reichelt Exp $
 *                
 ********************************************************************/

#include <debug.h>

#define KIP_SECONDARY

#include INC_GLUE(ia32/user.h)
#include INC_GLUE(ia32/kernelinterface.h)

#undef KIP_SYSCALL
#undef KIP_API_FLAGS

#define KIP_SYSCALL		KIP_SYSCALL_32
#define KIP_API_FLAGS		KIP_API_FLAGS_32

#define KIP_MEMDESCS		memory_descriptors_32
#define KIP_MEMDESCS_SIZE	_memory_descriptors_size_32
#define KIP_MEMDESCS_RAW	_memory_descriptors_raw_32

namespace ia32 {

#include "../../../api/v4/kernelinterface.cc"

}
