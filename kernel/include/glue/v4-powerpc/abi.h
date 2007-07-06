/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/abi.h
 * Description:	User-kernel ABI related declarations (such as register 
 * 		allocation).
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
 * $Id: abi.h,v 1.5 2004/06/04 17:26:14 joshua Exp $
 *
 ***************************************************************************/

#ifndef __GLUE__V4_POWERPC__ABI_H__
#define	__GLUE__V4_POWERPC__ABI_H__

#define ABI_LOCAL_ID		2

#define KIP_ABI_ADDR		3
#define KIP_ABI_API_VERSION	4
#define KIP_ABI_API_FLAGS	5
#define KIP_ABI_KERNEL_ID	6

#define EXREG_ABI_DEST		3
#define EXREG_ABI_CONTROL	4
#define EXREG_ABI_SP		5
#define EXREG_ABI_IP		6
#define EXREG_ABI_FLAGS		7
#define EXREG_ABI_USER_HANDLE	8
#define EXREG_ABI_PAGER		9
#define EXREG_ABI_IS_LOCAL	10
#define EXREG_ABI_RESULT	3

#define EXREG_ABI_CONTROL_P	7

#define TCTRL_ABI_DEST		3
#define TCTRL_ABI_SPACE		4
#define TCTRL_ABI_SCHEDULER	5
#define TCTRL_ABI_PAGER		6
#define TCTRL_ABI_UTCB		7
#define TCTRL_ABI_RESULT	3

#define CLOCK_ABI_LOWER		4
#define CLOCK_ABI_UPPER		3

#define TSWITCH_ABI_DEST	3

#define SCHEDULE_ABI_DEST	3
#define SCHEDULE_ABI_TIME	4
#define SCHEDULE_ABI_CPU	5
#define SCHEDULE_ABI_PRIO	6
#define SCHEDULE_ABI_PREEMPT	7
#define SCHEDULE_ABI_RESULT	3

#define UNMAP_ABI_CONTROL	15

#define SPACE_ABI_SPACE		3
#define SPACE_ABI_CONTROL	4
#define SPACE_ABI_KIP		5
#define SPACE_ABI_UTCB		6
#define SPACE_ABI_REDIRECTOR	7
#define SPACE_ABI_RESULT	3

#define CPU_ABI_CPU		3
#define CPU_ABI_INTERNAL_FREQ	4
#define CPU_ABI_EXTERNAL_FREQ	5
#define CPU_ABI_VOLTAGE		6

#define MEM_ABI_CONTROL		15
#define MEM_ABI_ATTR0		16
#define MEM_ABI_ATTR1		17
#define MEM_ABI_ATTR2		18
#define MEM_ABI_ATTR3		19

#define IPC_ABI_TO_TID		15
#define IPC_ABI_FROM_TID	16
#define IPC_ABI_TIMEOUTS	17

#define IPC_ABI_MR0             14
#define IPC_ABI_MR1             3
#define IPC_ABI_MR2             4
#define IPC_ABI_MR3             5
#define IPC_ABI_MR4             6
#define IPC_ABI_MR5             7
#define IPC_ABI_MR6             8
#define IPC_ABI_MR7             9
#define IPC_ABI_MR8             10
#define IPC_ABI_MR9             0

/* Number of message registers transfered in the register file.
 */
#define IPC_ABI_MR_TOT          10


#endif	/* __GLUE__V4_POWERPC__ABI_H__ */
