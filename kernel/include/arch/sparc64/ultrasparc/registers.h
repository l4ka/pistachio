/*********************************************************************
 *                
 * Copyright (C) 2003-2004, University of New South Wales
 *                
 * File path:   arch/sparc64/ultrasparc/registers.h
 * Description: Describes the register specifics of the UltraSPARC
 *              family of SPARC v9 cpus.
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
 * $Id: registers.h,v 1.4 2004/01/21 23:52:45 philipd Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ULTRASPARC__REGISTERS_H__
#define __ARCH__SPARC64__ULTRASPARC__REGISTERS_H__

/********************
* Integer Registers *
********************/

/* The number of register windows is CPU specific */

#if (CONFIG_SPARC64_ULTRASPRC1 || CONFIG_SPARC64_ULTRASPARC2)

#define NWINDOWS 8  

#elif (CONFIG_SPARC64_ULTRASPARC3)

#define NWINDOWS 16

#else

#error Unknown UltraSPARC CPU defined!

#endif /* CONFIG_SPARC64_ULTRASPARC* */

/***********************
* Privileged Registers *
***********************/

/**
 *  Processor state register (PSTATE) 
 */

#define PSTATE_IG (1 << 11) /* Interrupt globals enabled. */
#define PSTATE_MG (1 << 10) /* MMU globals enabled.       */

/* PSTATE_MM, Memory model */
#define PSTATE_PSO  (1 << 6)  /* Partial store order, implemented.  */
#define PSTATE_RMO  (2 << 6)  /* Relaxed memory order, implemented. */

/* PID0 and PID1 definitions. */

#define PSTATE_PID0 mg /* Use MMU global registers.       */
#define PSTATE_PID1 ig /* Use Interrupt global registers. */

#define PSTATE_PID0_CHAR() (pstate.mg ? 'M' : 'm')
#define PSTATE_PID1_CHAR() (pstate.ig ? 'I' : 'i')

/* User-modifiable PSTATE bits */
#define PSTATE_USER_MASK (PSTATE_CLE)

/* Maximum value of the trap level register (TL). This is defined for all
 * SPARCv9 CPUs but the value is UltraSPARC specific. */
#define MAXTL 5

#endif /* !__ARCH__SPARC64__ULTRASPARC__REGISTERS_H__ */
