/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	kdb/arch/powerpc/instr.cc
 * Description:	Deal with instructions, such as single stepping.
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
 * $Id: instr.cc,v 1.18 2004/06/02 15:55:56 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(msr.h)
#include INC_ARCH(phys.h)
#include INC_ARCH(frame.h)

#if defined(CONFIG_KDB_DISAS) && defined(CONFIG_DEBUG)

/* This is an ugly hack to incorporate the disassembler.  It lives outside
 * the kernel tree in the contrib branch, so we have to 'include' it
 * in our source.
 */
#ifndef PARAMS
# define PARAMS(a) a
#endif

/* Undefine conflicting macros. */
#undef MB
#undef UNUSED

#include "../../contrib/disas/ppc.h"
#include "../../contrib/disas/ppc-opc.cc"
#include "../../contrib/disas/ppc-dis.cc"

/* Undefine conflicting macros. */
#undef MB
#undef UNUSED

#endif

DECLARE_CMD (cmd_single_step, root, 's', "singlestep", "Single step");
DECLARE_CMD (cmd_branch_trace, root, 'b', "branchtrace", "Branch trace");
DECLARE_CMD (cmd_srr0_disasm, arch, 'd', "srr0-disasm", "Disassemble the instruction at srr0");
DECLARE_CMD (cmd_disasm, arch, 'D', "disasm", "Disassemble");

static void print_address( unsigned long addr )
{
    printf( "0x%08x", addr );
}

void dbg_addr_disasm( word_t addr )
{
    word_t insn = *(word_t *)addr;
    printf( "%08x\t", addr );

#if defined(CONFIG_KDB_DISAS) && defined(CONFIG_DEBUG)
    print_insn_big_powerpc( addr, insn );
    printf( "\n" );
#else	/* !CONFIG_KDB_DISAS */
    printf( "0x%08x\n", insn );
#endif	/* !CONFIG_KDB_DISAS */
}

CMD(cmd_single_step, cg)
{
    except_info_t *frame = (except_info_t *)kdb.kdb_param;

    if( frame == NULL )
	printf( "KDB error: unknown exception state, unable to single step.\n");
    else {
	dbg_addr_disasm( frame->regs->srr0_ip );
	frame->regs->srr1_flags = MSR_SET( frame->regs->srr1_flags, MSR_SE );
	return CMD_QUIT;
    }

    return CMD_NOQUIT;
}

CMD(cmd_branch_trace, cg)
{
    except_info_t *frame = (except_info_t *)kdb.kdb_param;

    if( frame == NULL )
	printf( "KDB error: unknown exception state, "
		"unable to branch trace.\n" );
    else {
	dbg_addr_disasm( frame->regs->srr0_ip );
	frame->regs->srr1_flags = MSR_SET( frame->regs->srr1_flags, MSR_BE );
	return CMD_QUIT;
    }

    return CMD_NOQUIT;
}

CMD(cmd_srr0_disasm, cg)
{
    except_info_t *frame = (except_info_t *)kdb.kdb_param;

    if( frame == NULL ) {
	printf( "KDB error: no exception state to disassemble.\n" );
	return CMD_NOQUIT;
    }

    dbg_addr_disasm( frame->regs->srr0_ip );
    return CMD_NOQUIT;
}

CMD(cmd_disasm, cg)
{
    static word_t addr = KERNEL_OFFSET + PHYS_EXCEPT_START;
    int i;

    // Ask for the start address.
    addr = get_hex( "Disassemble address", addr );

    // Until requested to quit, dump a page of disassembled code.
    do {
	for( i = 0; i < 24; i++ ) {
	    dbg_addr_disasm( addr );
	    addr += sizeof(word_t);
	}
    } while( get_choice("continue?", "Continue/Quit", 'c') != 'q' );

    return CMD_NOQUIT;
}

