/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     kdb/arch/powerpc64/single_step.cc
 * Description:   User Single Step Support
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
 * $Id: single_step.cc,v 1.2 2004/06/04 06:49:14 cvansch Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(msr.h)

extern tcb_t * kdb_get_tcb();

/**
 * cmd_powerpc64_single: set powerpc64 single step
 */
DECLARE_CMD (cmd_powerpc64_single, arch, 'S', "single", "enable/disable user single stepping");

CMD(cmd_powerpc64_single, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb)
    {
	powerpc64_irq_context_t *frame = (powerpc64_irq_context_t *)((word_t)tcb + POWERPC64_PAGE_SIZE) - 1;

	char choice = get_choice ( "Enable user single step", "y/n", 'y' );
	word_t msr = frame->srr1;

	if( choice == 'y' )
	    msr |= MSR_SE;	/* Single Step */
	else
	    msr &= ~MSR_SE;	/* Single Step */

	frame->srr1 = msr;
	printf( "\n" );
    }

    return CMD_NOQUIT;
}


/**
 * cmd_powerpc64_branch: set powerpc64 branch step
 */
DECLARE_CMD (cmd_powerpc64_branch, arch, 'B', "branch", "enable/disable user branch stepping");

CMD(cmd_powerpc64_branch, cg)
{
    tcb_t * tcb = kdb_get_tcb();

    if (tcb)
    {
	powerpc64_irq_context_t *frame = (powerpc64_irq_context_t *)((word_t)tcb + POWERPC64_PAGE_SIZE) - 1;

	char choice = get_choice ( "Enable user branch step", "y/n", 'y' );
	word_t msr = frame->srr1;

	if( choice == 'y' )
	    msr |= MSR_BE;	/* Branch Step */
	else
	    msr &= ~MSR_BE;	/* Branch Step */

	frame->srr1 = msr;
	printf( "\n" );
    }

    return CMD_NOQUIT;
}

