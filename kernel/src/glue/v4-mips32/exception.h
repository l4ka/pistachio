/*********************************************************************
 *                
 * Copyright (C) 2006,  Karlsruhe University
 *                
 * File path:     glue/v4-mips32/exception.h
 * Description:   MIPS32 exception message definitions
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
 * $Id: exception.h,v 1.1 2006/02/23 21:07:40 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS32__EXCEPTION_H__
#define __GLUE__V4_MIPS32__EXCEPTION_H__

/*
 * Generic exception message format
 * All exceptions not handled by the kernel or other exception message.
 */

#define EXCEPT_IPC_GEN_MR_IP		1
#define EXCEPT_IPC_GEN_MR_SP		2
#define EXCEPT_IPC_GEN_MR_FLAGS		3
#define EXCEPT_IPC_GEN_MR_EXCEPTNO	4
#define EXCEPT_IPC_GEN_MR_ERRORCODE	5
#define EXCEPT_IPC_GEN_MR_LOCALID	6

#define EXCEPT_IPC_GEN_MR_NUM		6

#define EXCEPT_IPC_GEN_LABEL	(-5ul) // XXX (-5ul << 4)

/*
 * System call exception
 * System calls not handled by L4 are redirected via system call exception IPC
 */

#define EXCEPT_IPC_SYS_MR_V0		1
#define EXCEPT_IPC_SYS_MR_V1		2
#define EXCEPT_IPC_SYS_MR_A0		3
#define EXCEPT_IPC_SYS_MR_A1		4
#define EXCEPT_IPC_SYS_MR_A2		5
#define EXCEPT_IPC_SYS_MR_A3		6
#define EXCEPT_IPC_SYS_MR_A4		7
#define EXCEPT_IPC_SYS_MR_A5		8
#define EXCEPT_IPC_SYS_MR_A6		9
#define EXCEPT_IPC_SYS_MR_A7		10
#define EXCEPT_IPC_SYS_MR_IP		11
#define EXCEPT_IPC_SYS_MR_SP		12
#define EXCEPT_IPC_SYS_MR_FLAGS		13

#define EXCEPT_IPC_SYS_MR_NUM		13

#define EXCEPT_IPC_SYS_LABEL		(-5ul) // XXX (-5ul << 4)


#endif /* !__GLUE__V4_MIPS32__EXCEPTION_H__ */
