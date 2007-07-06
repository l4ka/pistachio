/*********************************************************************
 *                
 * Copyright (C) 2004-2006,  Karlsruhe University
 *                
 * File path:     bootinfo.h
 * Description:   generic bootinfo creation functions
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
 * $Id: bootinfo.h,v 1.2 2006/10/22 19:38:02 reichelt Exp $
 *                
 ********************************************************************/
#ifndef __KICKSTART__BOOTINFO_H__
#define __KICKSTART__BOOTINFO_H__

#include <config.h>

#include "mbi.h"

#if defined(__L4__BOOTINFO_H__)
#error do not include <l4/bootinfo.h> before including this file
#endif

namespace BI32
{

typedef L4_Word32_t L4_Word_t;

#include <l4/bootinfo.h>
#undef __L4__BOOTINFO_H__

L4_BootRec_t * init_bootinfo (L4_BootInfo_t * bi);

L4_BootRec_t * record_bootinfo_modules (L4_BootInfo_t * bi,
					L4_BootRec_t * rec,
					mbi_t * mbi,
					mbi_module_t orig_mbi_modules[],
					unsigned int decode_count);

L4_BootRec_t * record_bootinfo_mbi (L4_BootInfo_t * bi,
				    L4_BootRec_t * rec,
				    mbi_t * mbi);

}

namespace BI64
{

typedef L4_Word64_t L4_Word_t;

#include <l4/bootinfo.h>
#undef __L4__BOOTINFO_H__

L4_BootRec_t * init_bootinfo (L4_BootInfo_t * bi);

L4_BootRec_t * record_bootinfo_modules (L4_BootInfo_t * bi,
					L4_BootRec_t * rec,
					mbi_t * mbi,
					mbi_module_t orig_mbi_modules[],
					unsigned int decode_count);

L4_BootRec_t * record_bootinfo_mbi (L4_BootInfo_t * bi,
				    L4_BootRec_t * rec,
				    mbi_t * mbi);

}

#endif /* !__KICKSTART__BOOTINFO_H__ */
