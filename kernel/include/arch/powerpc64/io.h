/****************************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:	arch/powerpc64/io.h
 * Description:	PowerPC IO functions
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
 * $Id: io.h,v 1.2 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__IO_H__
#define __ARCH__POWERPC64__IO_H__

#ifndef CONFIG_BIGENDIAN
#error Not little endian safe
#endif

/**
 * Little-endian I/O write
 * @param reg	The register offset.
 * @param val	The big-endian value, which will be byte reversed when written.
 */
INLINE void out32le( addr_t reg, u32_t val )
{
    asm volatile( "stwbrx %0, 0, %1 ; eieio ;" 
	    : 
	    : "r" (val), "b" (reg) );
}

/**
 * Big-endian I/O write
 * @param reg	The register offset.
 * @param val	The value to write to the register.
 */
INLINE void out32be( addr_t reg, u32_t val )
{
    asm volatile( "stw %0, 0(%1) ; eieio ;" 
	    : 
	    : "r" (val), "b" (reg) );
}

/**
 * Little-endian I/O read
 * @param reg	The register offset.
 *
 * The return value is converted to big-endian.
 */
INLINE u32_t in32le( addr_t reg )
{
    u32_t val;
    asm volatile( "lwbrx %0, 0, %1 ; eieio ;" 
	    : "=r" (val) 
	    : "b" (reg) );
    return val;
}

/**
 * Big-endian I/O read
 * @param reg	The register offset.
 */
INLINE u32_t in32be( addr_t reg )
{
    u32_t val;
    asm volatile( "lwz %0, 0, %1 ; eieio ;" 
	    : "=r" (val) 
	    : "b" (reg) );
    return val;
}

#endif /*__ARCH__POWERPC64__IO_H__*/
