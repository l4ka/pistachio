/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/arch/x86/stepping.cc
 * Description:   Single stepping for IA-32
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
 * $Id: stepping.cc,v 1.5 2003/09/24 19:05:05 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include INC_ARCH(trapgate.h)
#include INC_ARCH(cpu.h)

bool x86_single_step_on_branches = false;
word_t x86_last_ip = ~0U;

DECLARE_CMD (cmd_singlestep, root, 's', "singlestep", "Single step");

CMD(cmd_singlestep, cg)
{
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    x86_exceptionframe_t* f = param->frame;

    x86_last_ip = f->regs[x86_exceptionframe_t::ipreg];
    f->regs[x86_exceptionframe_t::freg] |= (1 << 8) + (1 << 16); /* RF + TF */

    return CMD_QUIT;
}


#if defined(CONFIG_CPU_X86_I686) || defined(CONFIG_CPU_X86_P4)
DECLARE_CMD (cmd_branchstep, root, 'j', "branchstep",
	     "execute until next taken branch");

CMD (cmd_branchstep, cg)
{
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    x86_exceptionframe_t* f = param->frame;

    f->regs[x86_exceptionframe_t::freg] |= (1 << 8) + (1 << 16); /* RF + TF */
    x86_wrmsr (X86_MSR_DEBUGCTL, ((1 << 0) + (1 << 1))); /* LBR + BTF */
    x86_single_step_on_branches = true;

    return CMD_QUIT;
}
#endif
