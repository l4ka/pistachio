/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     arch/alpha/irq.cc
 * Description:   Generic trap handling 
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
 * $Id: irq.cc,v 1.13 2004/04/06 01:20:28 benno Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>

#include INC_API(types.h)
#include INC_API(schedule.h)
#include INC_API(interrupt.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_ARCH(palcalls.h)
#include INC_GLUE(intctrl.h)

#include INC_GLUE(resources_inline.h)

DECLARE_TRACEPOINT (SYS_KERNEL_INTERFACE);

word_t alpha_cycles = 0;
word_t alpha_nticks = 0;

static word_t last_pcc = 0;

static word_t get_pcc(void)
{
    word_t reg;

    asm ("rpcc %0; extll %0, 0, %0" : "=r" (reg));

    return reg;
}

static word_t update_pcc(void)
{
    word_t delta, pcc = get_pcc();
    if(pcc < last_pcc) {
	delta = (pcc + (1ull << 32)) - last_pcc;
    } else {
	delta = pcc - last_pcc;
    }
    last_pcc = pcc;
    alpha_cycles += delta;

    return alpha_cycles;
}

extern "C" word_t sys_system_clock(void) 
{
    return update_pcc();
}

extern "C" void handle_IRQ(word_t type, word_t a1, word_t a2)
{
    switch(type) {
    case PAL_INT_IPI:
	enter_kdebug("Got an IPI");
	break;
    case PAL_INT_CLOCK:
    {
	alpha_nticks++;

	get_current_scheduler()->handle_timer_interrupt();
	break;
    }
    case PAL_INT_ERR:
	printf("Got a correctable error or machine check (vector 0x%lx)\n", a1);
	handle_interrupt(MCHECK_IRQ);
	break;

    case PAL_INT_DEV:
    {
	int irq = get_interrupt_ctrl()->decode_irq(a1);
	get_interrupt_ctrl()->mask(irq);
	get_interrupt_ctrl()->ack(irq);

//	printf("Got a device interrupt %d\n", irq);	

	handle_interrupt(irq);
	break;
    }
    case PAL_INT_PERF:
	printf("Got a performance interrupt (vector 0x%lx)\n", a1);	
	handle_interrupt(PERF_IRQ);
	
	break;

    default:
	printf("Unknown interrupt type: %d 0x%lx 0x%lx", type, a1, a2);
	enter_kdebug("Unknown interrupt");
	break;
    }
}

extern "C" void handle_arith(word_t a0, word_t a1, word_t a2)
{
    printf("Arithmetic exception: 0x%lx, 0x%lx\n", a0, a1);
    enter_kdebug("Arithmetic exception");
}

extern "C" void handle_if(word_t type, word_t a1, word_t a2, alpha_context_t *ctx, alpha_savedregs_t *regs)
{
    word_t result = 0;

    switch(type) {
    case PAL_IF_BPT:
	enter_kdebug("Breakpoint hit");
	break;
    case PAL_IF_BUGCHK:
	enter_kdebug("Bugchk hit");
	break;
    case PAL_IF_GENTRAP:
	result = handle_user_trap(ctx->a0, ctx->a1);
	break;
    case PAL_IF_FEN: 
    {
	tcb_t *current_tcb = get_current_tcb();
	current_tcb->resources.alpha_fpu_unavail_exception( current_tcb );
	return;
    }
    case PAL_IF_OPDEC:
	/* The application gets the kernel info page by doing some privileged PAL call, with 
	   a0 ($16) == {'L', '4', 'u', 'K', 'K', 'I', 'P', '4'} == 0x4c34754b4b495034
	*/
	if(ctx->a0 == MAGIC_KIP_REQUEST) {
	    regs->r0 = (word_t) get_current_space()->get_kip_page_area().get_base();
	    ctx->a0  = get_kip()->api_version;
	    ctx->a1 = get_kip()->api_flags;
	    ctx->a2 = (4 << 24) | (1 << 16);

	    TRACEPOINT (SYS_KERNEL_INTERFACE,
			printf ("KernelInterface() @ %p in %p\n",
				ctx->pc, get_current_tcb()));

	    return;
	}
	printf("OPDEC hit @ %p\n", ctx->pc);
	break;
    default:
	enter_kdebug("Unknown entIF");
	break;
    }

    regs->r0 = result;
}

extern "C" void handle_una(word_t a0, word_t a1, word_t a2, alpha_context_t *ctx, alpha_savedregs_t *regs)
{
    printf("Unaligned access: VA: 0x%lx, OP: 0x%lx, REG: 0x%lx, PC: 0x%lx\n", a0, a1, a2, ctx->pc);
    enter_kdebug("Unaligned exception");
}
