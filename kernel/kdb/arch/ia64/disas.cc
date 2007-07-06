/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/disas.cc
 * Description:   Disassembler wrapper for IA-32
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
 * $Id: disas.cc,v 1.7 2003/11/06 18:49:19 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_ARCH(cr.h)
#include INC_API(space.h)
#include INC_API(tcb.h)

extern "C" int disas (addr_t pc);

DECLARE_CMD (cmd_disas, root, 'U', "disas", "disassemble");

CMD (cmd_disas, cg)
{
    char c;
    word_t pc;
restart:
    if ((pc = get_hex("IP", (word_t) cr_get_iip ())) == ABORT_MAGIC)
	return CMD_NOQUIT;

    do {
	printf ("%p: ", pc);
	pc += disas ((addr_t) pc);
	printf ("\n");
	c = get_choice (NULL, "c/u/q", 'c');
    } while ((c != 'q') && (c != 'u'));
    if (c == 'u')
	goto restart;

    return CMD_NOQUIT;
}


extern "C" int readmem (addr_t vaddr, addr_t contents, int size)
{
    word_t value;

    if (vaddr < ia64_phys_to_rr (7, (addr_t) 0))
    {
	space_t * space = kdb.kdb_current->get_space ();

	if (! space->readmem (vaddr, &value))
	    return 0;

	vaddr = (addr_t) &value;
    }

    switch (size)
    {
    case 1: *(u8_t *)  contents = *(u8_t *)  vaddr; break;
    case 2: *(u16_t *) contents = *(u16_t *) vaddr; break;
    case 4: *(u32_t *) contents = *(u32_t *) vaddr; break;
    case 8: *(u64_t *) contents = *(u64_t *) vaddr; break;
    default:
	return 0;
    }

    return 1;
}
