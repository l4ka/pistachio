/*********************************************************************
 *                
 * Copyright (C) 2002, 2003, 2007,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia32/stepping.cc
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
#include <kdb/kdb.h>
#include INC_ARCH(trapgate.h)	/* for ia32_exceptionframe_t */
#include INC_ARCH(cpu.h)

bool ia32_single_step_on_branches = false;
u32_t ia32_last_eip = ~0U;

DECLARE_CMD (cmd_singlestep, root, 's', "singlestep", "Single step");

CMD(cmd_singlestep, cg)
{
    ia32_exceptionframe_t* f = (ia32_exceptionframe_t*) kdb.kdb_param;

    ia32_last_eip = f->eip;
    f->eflags |= (1 << 8) + (1 << 16); /* RF + TF */

    return CMD_QUIT;
}


#if defined(CONFIG_CPU_IA32_I686) || defined(CONFIG_CPU_IA32_P4)
DECLARE_CMD (cmd_branchstep, root, 'S', "branchstep",
	     "execute until next taken branch");

CMD (cmd_branchstep, cg)
{
    ia32_exceptionframe_t * f = (ia32_exceptionframe_t *) kdb.kdb_param;

    f->eflags |= (1 << 8) + (1 << 16);	/* RF + TF */
    x86_wrmsr (IA32_DEBUGCTL, ((1 << 0) + (1 << 1))); /* LBR + BTF */
    ia32_single_step_on_branches = true;

    return CMD_QUIT;
}
#endif
