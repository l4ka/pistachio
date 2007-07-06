/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     glue/v4-tmplarch/config.h
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
 * $Id: config.h,v 1.5 2003/09/24 19:04:53 skoglund Exp $
 *                
 ********************************************************************/


#warning PORTME
/**
 * Size of a kernel TCB in bytes
 */
#define KTCB_SIZE	1024


/**
   attributes for system call functions
   @param x is the name of the system call lacking the leading sys_ .
   This makes it possible to place every system call in its own section
   if required. Default is empty.
 */
#define SYSCALL_ATTR(x)


#warning PORTME
/**
 * endianess and word size
 */
#define KIP_API_FLAGS	{endian:0, word_size:0} // 32-bit, little endian

#warning PORTME
/**
 * minimum size of UTCB area and number of UTCBs in this
 */
#define KIP_UTCB_AREA	{size:12, no:8}   // 8 treads, 4KB

#warning PORTME
/**
 * size of kernel interface page
 */
#define KIP_KIP_AREA	{size:12}   // 4KB

#warning PORTME
/**
 * supported page sizes and access bits
 */
#define KIP_ARCH_PAGEINFO {rwx:6, size_mask:(0) >> 10}

#warning PORTME
/**
 * Base address of the root task's UTCB area
 */
#define ROOT_UTCB_START		0

#warning PORTME
/**
 * Address of the KIP in the root task
 */
#define ROOT_KIP_START		0
