/****************************************************************************
 *
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *
 * File path:	kdb/arch/mips64/disas.cc
 * Description:	Mips disassembler support.
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
 * $Id$
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

/* This is an ugly hack to incorporate the disassembler.  It lives outside
 * the kernel tree in the contrib branch, so we have to 'include' it
 * in our source.
 */
#ifndef PARAMS
# define PARAMS(a) a
#endif

/* Undefine conflicting macros. */
#undef MB

#include "../../contrib/disas/mips.h"
#include "../../contrib/disas/mips-opc.cc"
#include "../../contrib/disas/mips-dis.cc"

/* Undefine conflicting macros. */
#undef MB

#include INC_GLUE(context.h)

DECLARE_CMD (cmd_disasm, arch, 'D', "disasm", "Disassemble");

int print_insn( unsigned long memaddr, unsigned long insn )
{
  return print_insn_mips ( memaddr, insn, 0);
}

static void print_address( unsigned long addr )
{
    printf( "0x%016x", addr );
}

static void dbg_addr_disasm( word_t addr )
{
    u32_t insn = *(u32_t *)addr;
    printf( "%016x:  %08x\t", addr, insn );

    print_insn( addr, insn );
    printf( "\n" );
}

CMD(cmd_disasm, cg)
{
    int i;
    mips64_irq_context_t *frame = (mips64_irq_context_t *)(kdb.kdb_param);

    // Ask for the start address.
    word_t addr = get_hex( "Disassemble address", frame->epc );

    // Until requested to quit, dump a page of disassembled code.
    do {
	for( i = 0; i < 24; i++ ) {
	    dbg_addr_disasm( addr );
	    addr += sizeof(u32_t);
	}
    } while( get_choice("continue?", "Continue/Quit", 'c') != 'q' );

    return CMD_NOQUIT;
}
