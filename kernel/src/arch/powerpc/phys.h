/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     src/arch/powerpc/phys.h
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
 * $Id$
 *                
 ********************************************************************/

#ifndef __ARCH__POWERPC__PHYS_H__
#define __ARCH__POWERPC__PHYS_H__

#define PHYS_OS_START		0x0000
#define PHYS_OS_END		0x00ff
#define PHYS_EXCEPT_START	0x0100
#define PHYS_EXCEPT_END		0x0fff
#define PHYS_UNUSED_START	0x1000
#define PHYS_UNUSED_END		0x2fff
#define PHYS_START_AVAIL	0x3000

/*  Exception offsets, starting at physical address 0x0.
 *
 *  Table 6-2, page 223 - Programming Environments Manual for 32-Bit 
 *  Implementations of the PowerPC.
 *
 *  The exceptions have a 256 byte window, which equates to 64 32-bit instrs.
 */
#define EXCEPT_OFFSET_BASE		0x0100	/* The linker already offsets
						   us by 0x100. */
#define EXCEPT_OFFSET_SYSTEM_RESET	0x0100
#define EXCEPT_OFFSET_MACHINE_CHECK	0x0200
#define EXCEPT_OFFSET_DSI		0x0300
#define EXCEPT_OFFSET_ISI		0x0400
#define EXCEPT_OFFSET_EXTERNAL_INT	0x0500
#define EXCEPT_OFFSET_ALIGNMENT		0x0600
#define EXCEPT_OFFSET_PROGRAM		0x0700
#define EXCEPT_OFFSET_FP_UNAVAILABLE	0x0800
#define EXCEPT_OFFSET_DECREMENTER	0x0900
#define EXCEPT_OFFSET_RESERVED1		0x0a00
#define EXCEPT_OFFSET_RESERVED2	        0x0b00
#define EXCEPT_OFFSET_SYSCALL		0x0c00
#define EXCEPT_OFFSET_TRACE		0x0d00
#define EXCEPT_OFFSET_FP_ASSIST		0x0e00
#define EXCEPT_OFFSET_PERFMON		0x0f00

#ifndef CONFIG_PPC_BOOKE
#define EXCEPT_OFFSET_INSTR_BR		0x1300
#define EXCEPT_OFFSET_SYS_MANAGE	0x1400
#define EXCEPT_OFFSET_RESERVED3		0x1500
#define EXCEPT_OFFSET_RESERVED4		0x1600
#define EXCEPT_OFFSET_THERMAL		0x1700
#else
#define EXCEPT_OFFSET_CRITICAL_INPUT	0x1000
#define EXCEPT_OFFSET_AUX_UNAVAILABLE	0x1100
#define EXCEPT_OFFSET_INTERVAL_TIMER	0x1200
#define EXCEPT_OFFSET_WATCHDOG		0x1300
#define EXCEPT_OFFSET_DTLB		0x1400
#define EXCEPT_OFFSET_ITLB		0x1500
#define EXCEPT_OFFSET_DEBUG		0x1600

/* offset for secondary HVM exception handler block; lower 4 bits MBZ */
#define EXCEPT_HVM_OFFSET		0x10000
#endif

/*  The EXCEPT_ID() macro identifies an exception, based on its physical 
 *  location as defined by the PowerPC ISA, for the exception IPC.
 */
//#define EXCEPT_ID(a) (EXCEPT_OFFSET_##a - EXCEPT_OFFSET_BASE)
#define EXCEPT_ID(a) (EXCEPT_OFFSET_##a)

#endif	/* __ARCH__POWERPC__PHYS_H__ */

