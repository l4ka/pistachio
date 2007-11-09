/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2007,  Karlsruhe University
 *                
 * File path:     kdb/arch/x86/x64/disas.cc
 * Description:   Disassembler wrapper for x64
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
 * $Id: disas.cc,v 1.2 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include INC_ARCH(trapgate.h)

extern "C" int disas(addr_t pc);

DECLARE_CMD(cmd_disas, root, 'U', "disas", "disassemble");

CMD(cmd_disas, cg)
{
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    x86_exceptionframe_t* f = param->frame;

    char c;
    u64_t pc;
restart:
    if ((pc = get_hex("IP", f->rip)) == ABORT_MAGIC)
	return CMD_NOQUIT;

    printf("Key strokes: [space]=next instruction, u=new IP, q=quit\n");
    do {
	printf("%x: ", pc);
	pc += disas((addr_t) pc);
	printf("\n");
	c = get_choice(NULL, " /u/q", ' ');
    } while ((c != 'q') && (c != 'u'));
    if (c == 'u')
	goto restart;

    return CMD_NOQUIT;
}

