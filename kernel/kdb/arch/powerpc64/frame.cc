/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     kdb/arch/powerpc64/frame.cc
 * Description:   Exception frame dumping
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
 * $Id: frame.cc,v 1.6 2004/01/06 01:11:45 cvansch Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(frame.h)
#include INC_API(tcb.h)

extern tcb_t * kdb_get_tcb();


void SECTION(SEC_KDEBUG) powerpc64_dump_frame( powerpc64_irq_context_t *context )
{
    printf ( "== Stack frame: %p == \n", context );
    printf ( "srr0 =  %16lx, srr1 = %16lx \n", context->srr0, context->srr1 );
    printf ( "lr =    %16lx, ctr =  %16lx, xer =  %16lx\n", context->lr, context->ctr, context->xer );
    printf ( "dsisr = %16lx, dar =  %16lx, cr  =  %16lx\n", context->dsisr, context->dar, context->cr );
    printf ( "r0 = %16lx, r1 = %16lx, r2 = %16lx, r3 = %16lx\n", context->r0, context->r1, context->r2, context->r3 );
    printf ( "r4 = %16lx, r5 = %16lx, r6 = %16lx, r7 = %16lx\n", context->r4, context->r5, context->r6, context->r7 );
    printf ( "r8 = %16lx, r9 = %16lx, r10= %16lx, r11= %16lx\n", context->r8, context->r9, context->r10, context->r11 );
    printf ( "r12= %16lx, r13= %16lx, r14= %16lx, r15= %16lx\n", context->r12, context->r13, context->r14, context->r15 );
    printf ( "r16= %16lx, r17= %16lx, r18= %16lx, r19= %16lx\n", context->r16, context->r17, context->r18, context->r19 );
    printf ( "r20= %16lx, r21= %16lx, r22= %16lx, r23= %16lx\n", context->r20, context->r21, context->r22, context->r23 );
    printf ( "r24= %16lx, r25= %16lx, r26= %16lx, r27= %16lx\n", context->r24, context->r25, context->r26, context->r27 );
    printf ( "r28= %16lx, r29= %16lx, r30= %16lx, r31= %16lx\n", context->r28, context->r29, context->r30, context->r31 );
}

void SECTION(SEC_KDEBUG) dump_fprs( tcb_t *tcb )
{
    u64_t * fprs = (u64_t *)&tcb->resources;
    for (int i = 0; i < 32; i++)
	printf ("f%d\t= %16lx\n", i, fprs[i]);
    printf("FPSCR\t= %16lx\n", fprs[32]);
}


/**
 * cmd_dump_current_frame: show exception frame of current thread
 */
DECLARE_CMD (cmd_dump_current_frame, root, ' ', "frame",
	     "show current user exception frame");

CMD (cmd_dump_current_frame, cg)
{
    powerpc64_irq_context_t *frame = (powerpc64_irq_context_t *)(kdb.kdb_param);

    powerpc64_dump_frame(frame);

    return CMD_NOQUIT;
}


/**
 * cmd_dump_user_frame: show exception frame of user thread
 */
DECLARE_CMD (cmd_dump_user_frame, root, 'F', "user_frame",
	     "show current user exception frame");

CMD (cmd_dump_user_frame, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb) {
	powerpc64_irq_context_t *frame =
		(powerpc64_irq_context_t *)((word_t)tcb + POWERPC64_PAGE_SIZE);

	powerpc64_dump_frame(--frame);
    }

    return CMD_NOQUIT;
}


/**
 * cmd_dump_fprs: dump floating point registers
 */
DECLARE_CMD (cmd_dump_fprs, root, 'f', "fpr",
	     "show floating point registers");

CMD (cmd_dump_fprs, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb) {
	tcb->resources.powerpc64_fpu_spill( tcb );
	dump_fprs( tcb );
    }

    return CMD_NOQUIT;
}

