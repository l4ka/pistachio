/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     arch/sparc64/asi.h
 * Description:   SPARC v9 alternative space identifiers (ASIs).
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
 * $Id: asi.h,v 1.3 2003/09/24 19:04:31 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __ARCH__SPARC64__ASI_H__
#define __ARCH__SPARC64__ASI_H__

#include INC_CPU(asi.h)  /* Additional ASIs defined by CPU implementation */

#define ASI_N     0x04 /* Nucleus                              */
#define ASI_NL    0x0c /* Nucleus, little endian               */
#define ASI_AIUP  0x10 /* Primary, as if user                  */
#define ASI_AIUS  0x11 /* Secondary, as if user                */
#define ASI_AIUPL 0x18 /* Primary, as if user, little endian   */
#define ASI_AIUSL 0x19 /* Secondary, as if user, little endian */
#define ASI_P     0x80 /* Primary                              */
#define ASI_S     0x81 /* Secondary                            */
#define ASI_PNF   0x82 /* Primary, no fault                    */
#define ASI_SNF   0x83 /* Secondary, no fault                  */
#define ASI_PL    0x88 /* Primary, little endian               */
#define ASI_SL    0x89 /* Secondary, little endian             */
#define ASI_PNFL  0x8A /* Primary, no fault, little endian     */
#define ASI_SNFL  0x8B /* Secondary, no fault, little endian   */


#endif /* !__ARCH__SPARC64__ASI_H__ */
