/*********************************************************************
 *                
 * Copyright (C) 2002, 2003  University of New South Wales
 *                
 * File path:     glue/v4-alpha/ipcregs.h
 * Created:       09/03/2003 15:30:05 by Simon Winwood (sjw)
 * Description:   IPC register definition 
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
 * $Id: ipcregs.h,v 1.3 2003/09/24 19:04:34 skoglund Exp $
 *                
 ********************************************************************/

#ifndef __GLUE__V4_ALPHA__IPCREGS_H__
#define __GLUE__V4_ALPHA__IPCREGS_H__

#ifdef __LANGUAGE_ASSEMBLY__
#define DECLARE_REGISTER(a) a
#else
#define DECLARE_REGISTER(a) #a
#endif

#define R_MR0		DECLARE_REGISTER($15)
#define R_MR1		DECLARE_REGISTER($7)
#define R_MR2		DECLARE_REGISTER($8)
#define R_MR3		DECLARE_REGISTER($9)
#define R_MR4		DECLARE_REGISTER($10)
#define R_MR5		DECLARE_REGISTER($11)
#define R_MR6		DECLARE_REGISTER($12)
#define R_MR7		DECLARE_REGISTER($13)
#define R_MR8		DECLARE_REGISTER($14)

#define R_TO_TID	DECLARE_REGISTER($16)
#define R_FROM_TID	DECLARE_REGISTER($17)
#define R_TIMEOUT	DECLARE_REGISTER($18)	
#define R_RESULT	DECLARE_REGISTER($0)	

#endif /* __GLUE__V4_ALPHA__IPCREGS_H__ */
