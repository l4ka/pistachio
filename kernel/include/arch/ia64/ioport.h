/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia64/ioport.h
 * Description:   I/O port access functions.
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
 * $Id: ioport.h,v 1.2 2003/09/24 19:04:28 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA64__IOPORT_H__
#define __ARCH__IA64__IOPORT_H__


extern addr_t ia64_io_port_base;


INLINE addr_t
__ia64_port_address (const u16_t port)
{
    return (addr_t) ((word_t) ia64_io_port_base
		     | ((port >> 2) << 12)
		     | (port & ((1 << 12) - 1)));
}


INLINE void
out_u8 (const u16_t port, const u8_t value)
{
    __asm__ __volatile__ (
	"/* out_u8() */			\n"
	"	st1.rel	[%0] = %1	\n"
	"	mf.a			\n"
	"	mf			\n"
	:
	:
	"r" (__ia64_port_address (port)), "r" (value));
}

INLINE u8_t
in_u8 (const u16_t port)
{
    u8_t result;

    __asm__ __volatile__ (
	"/* in_u8() */			\n"
	"	ld1.acq	%0 = [%1]	\n"
	"	mf.a			\n"
	"	mf			\n"
	:
	"=r" (result)
	:
	"r" (__ia64_port_address (port)));

    return result;
}

INLINE void
out_u16 (const u16_t port, const u16_t value)
{
    __asm__ __volatile__ (
	"/* out_u16() */		\n"
	"	st2.rel	[%0] = %1	\n"
	"	mf.a			\n"
	"	mf			\n"
	:
	:
	"r" (__ia64_port_address (port)), "r" (value));
}

INLINE u16_t
in_u16 (const u16_t port)
{
    u8_t result;

    __asm__ __volatile__ (
	"/* in_u16() */			\n"
	"	ld2.acq	%0 = [%1]	\n"
	"	mf.a			\n"
	"	mf			\n"
	:
	"=r" (result)
	:
	"r" (__ia64_port_address (port)));

    return result;
}


#endif /* !__ARCH__IA64__IOPORT_H__ */
