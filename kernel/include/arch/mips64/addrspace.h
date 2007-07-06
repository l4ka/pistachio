/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   MIPS Address Map
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
 * $Id: addrspace.h,v 1.5 2003/09/24 19:04:29 skoglund Exp $
 *                
 ********************************************************************/

#ifndef _ARCH_MIPS64_ADDRSPACE_H_
#define _ARCH_MIPS64_ADDRSPACE_H_

/* 32-bit Compatible Segments */
#define KUSEG	    0x0000000000000000
#define KSEG0	    0xffffffff80000000
#define KSEG1	    0xffffffffa0000000
#define KSEG2	    0xffffffffc0000000
#define KSEG3	    0xffffffffe0000000

#define KUSEG_SIZE  0x80000000
#define KSEG0_SIZE  0x20000000
#define KSEG1_SIZE  0x20000000
#define KSEG2_SIZE  0x20000000
#define KSEG3_SIZE  0x20000000

#define KUSEG_MASK  (KUSEG_SIZE-1)
#define KSEG0_MASK  (KSEG0_SIZE-1)
#define KSEG1_MASK  (KSEG1_SIZE-1)
#define KSEG2_MASK  (KSEG2_SIZE-1)
#define KSEG3_MASK  (KSEG3_SIZE-1)

/* 64-bit Memory Segments */
#define XKUSEG	    0x0000000000000000
#define XKSSEG	    0x4000000000000000
#define XKPHYS	    0x8000000000000000
#define XKSEG	    0xc000000000000000
#define CKSEG0	    0xffffffff80000000
#define CKSEG1	    0xffffffffa0000000
#define CKSSEG	    0xffffffffc0000000
#define CKSEG3	    0xffffffffe0000000

/* Address generation */
#define MIPS64_ADDR_K0(pa)	(KSEG0 | (pa & KSEG0_MASK))
#define MIPS64_ADDR_K1(pa)	(KSEG1 | (pa & KSEG1_MASK))

#endif /* _ARCH_MIPS64_ADDRSPACE_H_ */
