/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  University of New South Wales
 *                
 * File path:     pistachio/cvs/kernel/kdb/arch/alpha/alpha.cc
 * Description:   Alpha specific debug functionality 
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
 * $Id: alpha.cc,v 1.4 2004/06/02 08:41:41 sgoetz Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <linear_ptab.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include INC_ARCH(palcalls.h)
#include INC_API(config.h)
#include INC_API(space.h)
#include INC_ARCH(pgent.h)
#include INC_GLUE(intctrl.h)
#include INC_API(tcb.h)


DECLARE_CMD (cmd_halt, root, 'h', "halt", "halt system");

CMD(cmd_halt, cg)
{
    PAL::halt();

    return CMD_NOQUIT;
}

DECLARE_CMD (cmd_irq, arch, 'i', "irq", "Display IRQ status");

CMD(cmd_irq, cg)
{
    get_interrupt_ctrl()->print_status();

    return CMD_NOQUIT;    
}

DECLARE_CMD (cmd_frame, arch, 'f', "frame", "Display user exception frame");

static void SECTION(SEC_KDEBUG) dump_frame(tcb_t *tcb)
{
    alpha_context_t *ctx = get_alpha_context(tcb);

    printf("=== ID: %t =======================\n", tcb);
    printf(" PS: %p     PC: %p     GP: %p\n", ctx->ps, ctx->pc, ctx->gp);  
    printf(" a0: %p     a1: %p     a2: %p\n", ctx->a0, ctx->a1, ctx->a2);  
}

extern tcb_t *kdb_get_tcb();

CMD(cmd_frame, cg)
{
    tcb_t * tcb = kdb_get_tcb();
    if (tcb)
	dump_frame(tcb);

    return CMD_NOQUIT;    
}

DECLARE_CMD (cmd_pcb, arch, 'p', "pcb", "Display a thread's pcb.");

static void SECTION(SEC_KDEBUG) dump_pcb(tcb_t *tcb)
{
    alpha_pcb_t *pcb = &tcb->get_arch()->pcb;

    printf("=== ID: %p =======================\n", tcb);
    printf(" KSP: %p     USP   : %p     PTBR  : %p\n", pcb->ksp, pcb->usp, pcb->ptbr); 
    printf(" ASN: %d     Cycles: %d     Unique: %p\n", pcb->asn, pcb->pcc, pcb->unique);  
    printf(" Flags: FP %s, PM %s\n", (pcb->flags & 0x1) ? "enabled" : "disabled", (pcb->flags & 0x1ull << 62) ? "enabled" : "disabled");
}
    
CMD(cmd_pcb, cg)
{
    tcb_t * tcb = kdb_get_tcb();
    if (tcb)
	dump_pcb(tcb);

    return CMD_NOQUIT;    
}
