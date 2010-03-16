/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     kdb/glue/v4-powerpc/ipc.cc
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
 * $Id$
 *                
 ********************************************************************/
#include <debug.h>
#include INC_API(tcb.h)

#define TLB(x) "tlb" #x ".0", "tlb" #x ".1", "tlb" #x ".2", "pid" #x

const char* ctrlxfer_item_idname[ctrlxfer_item_t::id_max] = 
{
	"gpregs0",	"gpregs1",	"gpregsx", 	"fpuregs",
#ifdef CONFIG_X_PPC_SOFTHVM
	"mmu",		"except",	"ivor",         "timer",	
	"config",	"debug",	"icache",	 "dcache",	
	"shadow_tlb", "tlb0",         "tlb1",         "tlb2",
	"tlb3",       	"tlb4",        "tlb5",          "tlb6",
	"tlb7",         "tlb8",	"tlb9",          "tlb10",
	"tlb11",	"tlb12",	"tlb13",         "tlb14",	
        "tlb15",
#endif
};


static const char* reg_names[][16] = {
    { "r0",  "r1(sp)", "r2", "r3", "r4", "r5", "r6", "r7",
      "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15", },
    { "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
      "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31", },
    { "xer", "cr",  "ctr", "lr",  "ip", "events" },
    { /* FPU */ },
    { "mmucr", "pid" },
    { "msr", "esr", "mcsr", "dear", "srr0", "srr1", "csrr0", "csrr1",
      "mcsrr0", "mcsrr1",  " event"}, 
    { "ivor0", "ivor1", "ivor2", "ivor3", "ivor4", "ivor5", "ivor6", "ivor7", 
      "ivor8", "ivor9", "ivor10", "ivor11", "ivor12", "ivor13", "ivor14", "ivor15", },
    { "tcr", "tsr", "tbl", "tbu", "dec", "decar", },
    { "ccr0", "ccr1", "rstcfg", "ivpr", "pir", "pvr", },
    { "dbsr", "dbdr", "dbcr0", "dbcr1", "dbcr2", "dac0", "dac1", 
      "dvc0", "dvc1", "iac0", "iac1", "iac2", "iac3", "icdbdr", 
      "icdbt0", "icdbt1" }, //"dcdbt0", "dcdbt1", },
    { "ivlim", "inv0", "inv1", "inv2", "inv3", "itv0", "itv1", "itv2", "itv3", },
    { "dvlim", "dnv0", "dnv1", "dnv2", "dnv3", "dtv0", "dtv1", "dtv2", "dtv3", },
    { "shtlb0", "shtlb1", "shtlb2", "shpid", },
    { TLB(0),  TLB(1),  TLB(2),  TLB(3), },
    { TLB(4),  TLB(5),  TLB(6),  TLB(7), },
    { TLB(8),  TLB(9),  TLB(10), TLB(11),},
    { TLB(12), TLB(13), TLB(14), TLB(15),},
    { TLB(16), TLB(17), TLB(18), TLB(19),},
    { TLB(20), TLB(21), TLB(22), TLB(23),},
    { TLB(24), TLB(25), TLB(26), TLB(27),},
    { TLB(28), TLB(29), TLB(30), TLB(31),},
    { TLB(32), TLB(33), TLB(34), TLB(35),},
    { TLB(36), TLB(37), TLB(38), TLB(39),},
    { TLB(40), TLB(41), TLB(42), TLB(43),},
    { TLB(44), TLB(45), TLB(46), TLB(47),},
    { TLB(48), TLB(49), TLB(50), TLB(51),},
    { TLB(52), TLB(53), TLB(54), TLB(55),},
    { TLB(56), TLB(57), TLB(58), TLB(59),},
    { TLB(60), TLB(61), TLB(62), TLB(63),},
};

const char* ctrlxfer_item_t::get_idname(const word_t id)
{ 
    return ctrlxfer_item_idname[id]; 
}

const char* ctrlxfer_item_t::get_hwregname(const word_t id, const word_t reg)
{
    return reg_names[id][reg];
}

word_t arch_ktcb_t::get_ctrlxfer_reg(word_t id, word_t reg)
{
    except_regs_t *frame = get_user_except_regs(addr_to_tcb(this));
    const word_t *regs = ctrlxfer_item_t::hwregs[id];

    switch(id)
    {
    case ctrlxfer_item_t::id_gpregsx:
    case ctrlxfer_item_t::id_gpregs0:
    case ctrlxfer_item_t::id_gpregs1:
	return ((word_t*)frame)[regs[reg]];
#if defined(CONFIG_X_PPC_SOFTHVM)
    case ctrlxfer_item_t::id_mmu ... ctrlxfer_item_t::id_dcache:
    case ctrlxfer_item_t::id_shadow_tlb:
	return ((word_t*)vm)[regs[reg]];

    case ctrlxfer_item_t::id_tlb0 ... ctrlxfer_item_t::id_tlb15:
    {
	int idx = (id - ctrlxfer_item_t::id_tlb0) * 4 + reg / 4;
	switch(reg % 4) {
	case 0: return vm->tlb[idx].tlb0.raw;
	case 1: return vm->tlb[idx].tlb1.raw;
	case 2: return vm->tlb[idx].tlb2.raw;
	case 3: return vm->tlb[idx].pid;
	}
    }
#endif
    default: enter_kdebug("unhandled ctrlxfer dump");
    }
    return 0;
}

    
void tcb_t::dump_ctrlxfer_state(bool extended)
{
    if (!get_utcb() || is_interrupt_thread())
	return;
    
    if (extended)
    {
	word_t max = 4;
#if defined(CONFIG_X_PPC_SOFTHVM)
	if (get_space()->hvm_mode)
	    max += arch_ktcb_t::fault_max;
#endif
	
	printf("\nfault masks:");
	for (word_t fault=0; fault < max; fault++)
	{
	    if (fault % 4 == 0) printf("\n\t");
	    printf("%s ", fault_ctrlxfer[fault+0].string());
	}
	printf("\n");
    }
    printf("ctrlxfer state:");

    word_t max = 2;
#if defined(CONFIG_X_PPC_SOFTHVM)
    if (get_space()->hvm_mode)
	max = ctrlxfer_item_t::id_tlb0;
#endif

    for (word_t id = 0; id < max; id++)
    {
	printf("\n %10s:", ctrlxfer_item_t::get_idname(id));
	for (word_t reg = 0; reg < ctrlxfer_item_t::num_hwregs[id]; reg++)
	{
	    if (reg && reg % 4 == 0) printf("\n\t    ");
	    printf("%10s: %wx  ", ctrlxfer_item_t::get_hwregname(id, reg), 
		   arch.get_ctrlxfer_reg(id, reg));
	}
    }
    printf("\n");

#if defined(CONFIG_X_PPC_SOFTHVM)
    if (extended && get_space()->hvm_mode)
    {
	for (word_t i = 0; i < 64; i++)
	{
	    ppc_tlb0_t tlb0;
	    ppc_tlb1_t tlb1;
	    ppc_tlb2_t tlb2;

	    tlb0.raw = arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_tlb0 + i / 4, i % 4 * 4 + 0);
	    tlb1.raw = arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_tlb0 + i / 4, i % 4 * 4 + 1);
	    tlb2.raw = arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_tlb0 + i / 4, i % 4 * 4 + 2);
	    word_t pid = arch.get_ctrlxfer_reg(ctrlxfer_item_t::id_tlb0 + i / 4, i % 4 * 4 + 3);

	    if (i % 4 == 0)
		printf("%7d:", ctrlxfer_item_t::id_tlb0 + i / 4);
	    else
		printf("\t");

	    printf("%02d: %c [%02x:%d] %08x sz:%08x [%04x:%08x] U:%c%c%c S:%c%c%c  C:[%c%c%c%c%c]\n",
		   i, tlb0.is_valid() ? 'V' : 'I', pid,
		   tlb0.trans_space, tlb0.get_vaddr(), tlb0.get_size(),
		   (word_t)(tlb1.get_paddr() >> 32), (word_t)(tlb1.get_paddr()),
		   tlb2.user_execute ? 'X' : '-', tlb2.user_write ? 'W' : '-', 
		   tlb2.user_read ? 'R' : '-', tlb2.super_execute ? 'X' : '-', 
		   tlb2.super_write ? 'W' : '-', tlb2.super_read ? 'R' : '-',
		   tlb2.write_through ? 'W' : '-', tlb2.inhibit ? 'I' : '-',
		   tlb2.mem_coherency ? 'M' : '-', tlb2.guarded ? 'G' : '-',
		   tlb2.endian ? 'E' : '-');
	}
    }
#endif
}
