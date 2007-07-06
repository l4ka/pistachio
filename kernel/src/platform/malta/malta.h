/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     platform/malta/malta.h
 * Description:   Malta platform definitions
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
 * $Id: malta.h,v 1.1 2006/02/23 21:07:42 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__MALTA__MALTA_H__
#define __PLATFORM__MALTA__MALTA_H__

#define MALTA_BASE          0xbf000000
#define MALTA_DISPLAY_BASE  0xbf000400

#define MALTA_LEDBAR      0x08
#define MALTA_ASCIIWORD   0x10
#define MALTA_ASCII_POS0  0x18
#define MALTA_ASCII_POS1  0x20
#define MALTA_ASCII_POS2  0x28
#define MALTA_ASCII_POS3  0x30
#define MALTA_ASCII_POS4  0x38
#define MALTA_ASCII_POS5  0x40
#define MALTA_ASCII_POS6  0x48
#define MALTA_ASCII_POS7  0x50

#endif /* !__PLATFORM__MALTA__MALTA_H__ */
