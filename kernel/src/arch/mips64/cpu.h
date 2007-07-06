/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/mips64/cpu.h
 * Description:   MIPS CPU ID Numbers. 
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
 * $Id: cpu.h,v 1.4 2003/11/17 05:45:13 cvansch Exp $
 *                
 ********************************************************************/
#ifndef _ARCH_MIPS64_CPU_H_
#define _ARCH_MIPS64_CPU_H_

#define MIPS_IMP_VR41XX	    0x0c00

#define MIPS_REV_VR4121	    0x0060
#define MIPS_REV_VR4181	    0x0050
#define MIPS_REV_MASK_VR    0x00f0

#define MIPS_IMP_RC64574    0x1500
#define MIPS_IMP_R4700	    0x2100
#define MIPS_IMP_SB1	    0x0100	    /* NB: Sibyte has PRid bits 23 to 16 set to Sibyte Manufacturer */

#define MIPS_IMP_MASK	    0xff00
#define MIPS_REV_MASK	    0x00ff

/* Top half of PRID */
#define MIPS_NORMAL	    0x00000000
#define MIPS_MANUF_SIBYTE   0x00040000

#define MIPS_MANUF_MASK	    0x00ff0000

#endif /* _ARCH_MIPS64_CPU_H */
