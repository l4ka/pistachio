/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/exception.h
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
 * $Id: exception.h,v 1.6 2004/12/02 00:10:11 cvansch Exp $
 *
 ********************************************************************/


#ifndef __GLUE__V4_ARM__EXCEPTION_H__
#define __GLUE__V4_ARM__EXCEPTION_H__

/*
 * Generic exception message format
 * All exceptions not handled by the kernel or other exception
 * messages.
 */

#define EXCEPT_IPC_GEN_MR_IP		1
#define EXCEPT_IPC_GEN_MR_SP		2
#define EXCEPT_IPC_GEN_MR_FLAGS		3
#define EXCEPT_IPC_GEN_MR_EXCEPTNO	4
#define EXCEPT_IPC_GEN_MR_ERRORCODE	5
#define EXCEPT_IPC_GEN_MR_LOCALID	6
#define EXCEPT_IPC_GEN_MR_NUM		6

#define EXCEPT_IPC_GEN_LABEL		(-5ul << 4)
#define EXCEPT_IPC_GEN_TAG		((EXCEPT_IPC_GEN_LABEL << 16) | EXCEPT_IPC_GEN_MR_NUM)

/*
 * System call exception
 */
#define EXCEPT_IPC_SYS_MR_PC            1
#define EXCEPT_IPC_SYS_MR_R0            2
#define EXCEPT_IPC_SYS_MR_R1            3
#define EXCEPT_IPC_SYS_MR_R2            4
#define EXCEPT_IPC_SYS_MR_R3            5
#define EXCEPT_IPC_SYS_MR_R4            6
#define EXCEPT_IPC_SYS_MR_R5            7
#define EXCEPT_IPC_SYS_MR_R6            8
#define EXCEPT_IPC_SYS_MR_R7            9
#define EXCEPT_IPC_SYS_MR_SP            10
#define EXCEPT_IPC_SYS_MR_LR            11
#define EXCEPT_IPC_SYS_MR_SYSCALL       12
#define EXCEPT_IPC_SYS_MR_FLAGS         13
#define EXCEPT_IPC_SYS_MR_NUM           13

#define EXCEPT_IPC_SYS_LABEL            (-5ul << 4)
#define EXCEPT_IPC_SYS_TAG              ((EXCEPT_IPC_SYS_LABEL << 16) | EXCEPT_IPC_SYS_MR_NUM)


#endif /* __GLUE__V4_ARM__EXCEPTION_H__ */
