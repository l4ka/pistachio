/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   MIPS Register Descriptions
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
 * $Id: regdef.h,v 1.3 2003/09/24 19:06:25 skoglund Exp $
 *                
 ********************************************************************/

#ifndef _ARCH_MIPS64_REGDEF_H_
#define _ARCH_MIPS64_REGDEF_H_

#define zero		$0  /* hardwired to zero */
#define AT		$1  /* assembler temporary */
#define v0		$2  /* result */
#define v1		$3
/* argument registers */
#define a0		$4
#define a1		$5
#define a2		$6
#define a3		$7
#define a4		$8  /* ABI64 names */
#define a5		$9
#define a6		$10
#define a7		$11
/* temporary registers */
#define t0		$8
#define t1		$9
#define t2		$10
#define t3		$11
#define t4		$12
#define t5		$13
#define t6		$14
#define t7		$15
/* saved registers */
#define s0		$16
#define s1		$17
#define s2		$18
#define s3		$19
#define s4		$20
#define s5		$21
#define s6		$22
#define s7		$23
/* extra temporaries */
#define t8		$24
#define t9		$25
/* kernel registers - don't touch */
#define k0		$26
#define k1		$27

#define gp		$28	/* global pointer */
#define sp		$29	/* stack pointer */
#define fp		$30	/* frame pointer */
#define s8		$30	/* or saved register */
#define ra		$31	/* return address */

#endif /* _ARCH_MIPS64_REGDEF_H_ */
