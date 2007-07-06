/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/exception.h
 * Description:	Exception IPC definitions.
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
 * $Id: exception.h,v 1.4 2003/12/19 18:34:43 joshua Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__EXCEPTION_H__
#define __GLUE__V4_POWERPC__EXCEPTION_H__

/* Information concerning the structure of the generic exception message.
 */
#define GENERIC_EXC_MR_UIP	1
#define GENERIC_EXC_MR_USP	2
#define GENERIC_EXC_MR_UFLAGS	3
#define GENERIC_EXC_MR_NO	4
#define GENERIC_EXC_MR_CODE	5
#define GENERIC_EXC_MR_LOCAL_ID	6
#define GENERIC_EXC_MR_MAX	6

#define GENERIC_EXC_LABEL  (-5 << 4)
#define GENERIC_EXC_TAG	   ((GENERIC_EXC_LABEL << 16) | GENERIC_EXC_MR_MAX)

/* Information concerning the structure of the emulated system call
 * exception message.
 */
#define SC_EXC_MR_UIP		10
#define SC_EXC_MR_USP		11
#define SC_EXC_MR_UFLAGS	12
#define SC_EXC_MR_MAX		12

#define SC_EXC_LABEL		(-5 << 4)
#define SC_EXC_TAG		((SC_EXC_LABEL << 16) | SC_EXC_MR_MAX)

#endif /* __GLUE__V4_POWERPC__EXCEPTION_H__ */
