/*********************************************************************
 *                
 * Copyright (C) 2002, 2003  University of New South Wales
 *                
 * File path:     glue/v4-ia32/offsets.h
 * Description:   Addresses used for C++, asm AND linker scripts
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
 * $Id: offsets.h,v 1.3 2003/09/24 19:04:34 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_ALPHA__OFFSETS_H__
#define __GLUE__V4_ALPHA__OFFSETS_H__

/* DON'T USE 0x........UL HERE. THE LINKER WILL NOT UNDERSTAND THAT */

/* The offset of the .text section's virtual and load address */
#if CONFIG_ALPHA_ADDRESS_BITS == 43
KERNEL_OFFSET = 0xfffffc0000000000;
#else
KERNEL_OFFSET = 0xffff800000000000;
#endif /* CONFIG_ALPHA_ADDRESS_BITS */

#endif /* !__GLUE__V4_IA32__OFFSETS_H__ */
