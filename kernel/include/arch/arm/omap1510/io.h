/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/omap1510/io.h
 * Description:   Functions which manipulate the OMAP1510 OS timer
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
 * $Id: io.h,v 1.2 2004/06/04 02:14:23 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM_OMAP1510__IO_H_
#define __ARCH__ARM_OMAP1510__IO_H_

#define IODEVICE_VADDR		(IO_AREA0_VADDR)
#define PHYS_CTL_REG_BASE	0xfff00000

INLINE word_t io_to_virt(word_t ph)
{
    return IODEVICE_VADDR + ph - PHYS_CTL_REG_BASE;
} 

INLINE word_t virt_to_io(word_t vt)
{
    return PHYS_CTL_REG_BASE + vt - IODEVICE_VADDR;
}

#endif /* __ARCH__ARM_OMAP1510__IO_H_*/
