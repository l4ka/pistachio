/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	arch/powerpc64/except.h
 * Description:	Constants and macros related to the register state of
 * 		exception handlers.
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
 * $Id: except.h,v 1.4 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__EXCEPT_H__
#define __ARCH__POWERPC64__EXCEPT_H__

//#define EXCEPT_IS_TRAP(srr1) 		((srr1) & 0x00020000)

#define EXCEPT_IS_DSI_MISS(dsisr)	((dsisr) & (1<<30))
#define EXCEPT_IS_DSI_FAULT(dsisr)	((dsisr) & (1<<27))
#define EXCEPT_IS_DSI_WRITE(dsisr)	((dsisr) & (1<<25))
#define EXCEPT_IS_DSI_DABR_MATCH(dsisr)	((dsisr) & (1<<22))
#define EXCEPT_IS_DSI_STAB_MISS(dsisr)	((dsisr) & (1<<21))

#define EXCEPT_IS_ISI_MISS(srr1)	((srr1) & (1<<30))
#define EXCEPT_IS_ISI_STAB_MISS(srr1)	((srr1) & (1<<21))
#define EXCEPT_IS_FAULT(srr1)		((srr1) & (1<<27))

#endif	/* __ARCH__POWERPC64__EXCEPT_H__ */

