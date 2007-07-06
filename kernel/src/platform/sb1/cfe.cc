/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  University of New South Wales
 *                
 * File path:     platform/sb1/smp.cc
 * Description:   mips64 sibyte MP implementation
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
 * $Id: cfe.cc,v 1.4 2006/03/01 14:10:32 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_ARCH(addrspace.h)
#include INC_PLAT(cfe_xiocb.h)

#define CFE_EPTSEAL 0x43464531
#ifdef CONFIG_UNCACHED
#define CFE_APIENTRY		(0x1FC00500 | KSEG1)
#define CFE_APISEAL		(0x1FC004E0 | KSEG1)
#define CFE_APISEAL_RE		(0x1FC004E8 | KSEG1)
#define CFE_APISEAL_OLD		(0x1FC00508 | KSEG1)
#else
#define CFE_APIENTRY		(0x1FC00500 | KSEG0)
#define CFE_APISEAL		(0x1FC004E0 | KSEG0)
#define CFE_APISEAL_RE		(0x1FC004E8 | KSEG0)
#define CFE_APISEAL_OLD		(0x1FC00508 | KSEG0)
#endif

typedef int (cfe_call_t)(long, cfe_xiocb_t *);
static int (*cfe_call)(long handle, cfe_xiocb_t *xiocb) = 0;

static cfe_xuint_t cfe_handle = 0;

int cfe_init(word_t arg)
{
    u32_t * api = (u32_t *)CFE_APISEAL;
    u32_t * api_re = (u32_t *)CFE_APISEAL_RE;
    u32_t * api_old = (u32_t *)CFE_APISEAL_OLD;

    if ((*api != CFE_EPTSEAL) &&
	(*api_re != CFE_EPTSEAL) &&
	(*api_old != CFE_EPTSEAL))
	return -1;

    if (arg != 0) cfe_handle = arg;
    cfe_call = (cfe_call_t *)CFE_APIENTRY;

    return 0;
}

int cfe_do(cfe_xiocb_t *xiocb)
{
    if (!cfe_call) return -1;

    return (*cfe_call)(cfe_handle,xiocb);
}

int cfe_start_cpu(int cpu, void (*fn)(void), long sp, long gp, long a1)
{
    cfe_xiocb_t xiocb;

    xiocb.xiocb_fcode = CFE_CMD_FW_CPUCTL;
    xiocb.xiocb_status = 0;
    xiocb.xiocb_handle = 0;
    xiocb.xiocb_flags  = 0;
    xiocb.xiocb_psize = sizeof(xiocb_cpuctl_t);
    xiocb.plist.xiocb_cpuctl.cpu_number = cpu;
    xiocb.plist.xiocb_cpuctl.cpu_command = CFE_CPU_CMD_START;
    xiocb.plist.xiocb_cpuctl.gp_val = gp;
    xiocb.plist.xiocb_cpuctl.sp_val = sp;
    xiocb.plist.xiocb_cpuctl.a1_val = a1;
    xiocb.plist.xiocb_cpuctl.start_addr = (long)fn;

    cfe_do(&xiocb);

    return xiocb.xiocb_status;
}

