/*********************************************************************
 *                
 * Copyright (C) 2003, University of New South Wales
 *                
 * File path:    kdb/arch/sparc64/registers.cc
 * Description:   
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
 * $Id: mmu.cc,v 1.4 2004/05/21 02:34:54 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include INC_CPU(mmu.h)
#include INC_CPU(tsb.h)

DECLARE_CMD_GROUP(mmu);
DECLARE_CMD(cmd_mmu, arch, 'm', "mmu", "mmu specifics");
DECLARE_CMD(cmd_dump_state, mmu, 's', "MMU state", "Dump MMU state");
DECLARE_CMD(cmd_dump_tlb, mmu, 't', "TLB entry", "Dump a TLB entry");
DECLARE_CMD(cmd_dump_tlb_all, mmu, 'T', "TLB(s)", "Dump whole TLB");
DECLARE_CMD(cmd_dump_tsb, mmu, 'h', "TSB (software TLB) entry",
	    "Dump a TSB entry");
DECLARE_CMD(cmd_dump_tsb_all, mmu, 'H', "TSB (software TLB)",
	    "Dump whole TSB");

CMD(cmd_mmu, cg)
{
    return mmu.interact (cg, "mmu");
}

CMD(cmd_dump_state, cg)
{
    mmu_t mmu;
    tsb_t tsb;

    mmu.print(sfsr_t::both, context_t::reserved);
    tsb.print();

    return CMD_NOQUIT;

} // CMD(cmd_dump_state, cg)

CMD(cmd_dump_tlb, cg)
{
    u16_t entry;
    char selection;
    tlb_t::tlb_e tlb;

    selection = get_choice("TLB to dump", "D-TLB/I-TLB", 'd');

    switch (selection) {
    case 'd':
	tlb = tlb_t::d_tlb;
	break;
    case 'i':
	tlb = tlb_t::i_tlb;
	break;
    default:
	printf("Invalid selection '%c'\n", selection);

	return CMD_NOQUIT;
    }
    entry = get_dec("Entry to dump [0..63]", 0, "0");
    mmu_t::print(entry, tlb);

    return CMD_NOQUIT;

} // CMD(cmd_dump_tlb, cg)

CMD(cmd_dump_tlb_all, cg)
{
    char selection;
    tlb_t::tlb_e tlb;

    selection = get_choice("TLB to dump", "D-TLB/I-TLB/Both", 'd');

    switch (selection) {
    case 'd':
	tlb = tlb_t::d_tlb;
	break;
    case 'i':
	tlb = tlb_t::i_tlb;
	break;
    case 'b':
	tlb = tlb_t::all_tlb;
	break;
    default:
	printf("Invalid selection '%c'\n", selection);

	return CMD_NOQUIT;
    }
    mmu_t::print(tlb);

    return CMD_NOQUIT;

} // CMD(cmd_dump_tlb_all, cg)

CMD(cmd_dump_tsb, cg)
{
    u16_t entry;
    char selection;
    tsb_t::tsb_e tsb;

    selection = get_choice("TSB to dump",
			   "D-TSB 8K 'd'/D-TSB 64K 'e'/"
			   "I-TSB 8K 'i'/D-TSB64K 'j'", 'd');

    switch (selection) {
    case 'd':
	tsb = tsb_t::d8k_tsb;
	break;
    case 'e':
	tsb = tsb_t::d64k_tsb;
	break;
    case 'i':
	tsb = tsb_t::i8k_tsb;
	break;
    case 'j':
	tsb = tsb_t::i64k_tsb;
	break;
    default:
	printf("Invalid selection '%c'\n", selection);

	return CMD_NOQUIT;
    }
    entry = get_dec("Entry to dump [0..512]", 0, "0");
    tsb_t::print(entry, tsb);

    return CMD_NOQUIT;

} // CMD(cmd_dump_tsb, cg)

CMD(cmd_dump_tsb_all, cg)
{
    char selection;
    tsb_t::tsb_e tsb;

    selection =
	get_choice("TSB to dump",
		   "D-TSB 8K 'd'/D-TSB 64K 'e'/I-TSB 8K 'i'/I-TSB 64K 'j'",
		   'd');

    switch (selection) {
    case 'd':
	tsb = tsb_t::d8k_tsb;
	break;
    case 'e':
	tsb = tsb_t::d64k_tsb;
	break;
    case 'i':
	tsb = tsb_t::i8k_tsb;
	break;
    case 'j':
	tsb = tsb_t::i64k_tsb;
	break;
    default:
	printf("Invalid selection '%c'\n", selection);

	return CMD_NOQUIT;
    }
    tsb_t::print(tsb);

    return CMD_NOQUIT;

} // CMD(cmd_dump_tsb_all, cg)
