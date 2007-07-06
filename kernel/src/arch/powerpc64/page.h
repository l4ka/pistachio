/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/page.h
 * Description:	PowerPC specific page constants.
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
 * $Id: page.h,v 1.4 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__PAGE_H__
#define __ARCH__POWERPC64__PAGE_H__

#define POWERPC64_SEGMENT_BITS	28
#define POWERPC64_SEGMENT_SIZE	(1ul << POWERPC64_SEGMENT_BITS)
#define POWERPC64_SEGMENT_MASK	(~(POWERPC64_SEGMENT_SIZE - 1))

#define POWERPC64_PAGE_BITS	12
#define POWERPC64_PAGE_SIZE	(1ul << POWERPC64_PAGE_BITS)
#define POWERPC64_PAGE_MASK	(~(POWERPC64_PAGE_SIZE - 1))

#define POWERPC64_LARGE_BITS	24
#define POWERPC64_LARGE_SIZE	(1ul << POWERPC64_LARGE_BITS)
#define POWERPC64_LARGE_MASK	(~(POWERPC64_LARGE_SIZE - 1))

#endif	/* __ARCH__POWERPC64__PAGE_H__ */
