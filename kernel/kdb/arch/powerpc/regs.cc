/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     kdb/arch/powerpc/regs.cc
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
#include <kdb/kdb.h>

#include INC_ARCH(pvr.h)
#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(bat.h)
#include INC_ARCH(pghash.h)
#include INC_ARCH(frame.h)

#include INC_API(tcb.h)

DECLARE_CMD (cmd_sysregs, arch, 's', "sysregs", "System registers");
DECLARE_CMD (cmd_except_regs, arch, 'r', "excregs", "Exception registers");
DECLARE_CMD (cmd_user_except_regs, arch, 'u', "userregs", "User exception registers");
DECLARE_CMD (cmd_print_msr, arch, 'm', "printmsr", "Print msr");
DECLARE_CMD (cmd_print_except_msr, arch, 'e', "printmsr", "Print exception msr");
#if defined(CONFIG_PPC_MMU_SEGMENTS)
DECLARE_CMD (cmd_print_bats, arch, 'b', "printbats", "Print bat registers");
#elif defined(CONFIG_PPC_MMU_TLB)
DECLARE_CMD (cmd_print_tlb, arch, 'T', "printtlb", "Print TLB");
#endif

static void dbg_print_sysregs( void )
{
    printf( " srr0: 0x%08x  srr1: 0x%08x\n", ppc_get_srr0(), ppc_get_srr1() );
    printf( "  msr: 0x%08x  sdr1: 0x%08x\n", ppc_get_msr(), ppc_get_sdr1() );

#ifndef CONFIG_PPC_BOOKE
    for( int j = 0; j < 4; j++ ) {
    	for( int i = 0; i < 4; i++ )
    	    printf( "%s sr%02d: 0x%08x", i ? " ":"", j*4+i, 
		    ppc_get_sr(j*4+i) );
	printf( "\n" );
    }
#endif
    printf( "  dar: 0x%08x dsisr: 0x%08x\n", ppc_get_dar(), ppc_get_dsisr() );
    printf( "sprg0: 0x%08x sprg1: 0x%08x sprg2: 0x%08x sprg3: 0x%08x\n",
	    ppc_get_sprg(0), ppc_get_sprg(1), ppc_get_sprg(2), ppc_get_sprg(3));
#ifdef CONFIG_PPC_BOOKE
    printf( "sprg4: 0x%08x sprg5: 0x%08x sprg6: 0x%08x sprg7: 0x%08x\n",
	    ppc_get_sprg(4), ppc_get_sprg(5), ppc_get_sprg(6), ppc_get_sprg(7));
    ppc_mmucr_t mmucr; mmucr.read();
    printf( "  pid: 0x%08x mmucr: 0x%08x [sid: 0x%02x, spc=%d]\n", 
	    ppc_get_pid(), mmucr.read().raw, mmucr.search_id, 
	    mmucr.search_translation_space);
#endif
    printf( "  tbl: 0x%08x   tbu: 0x%08x\n", ppc_get_tbl(), ppc_get_tbu() );
    printf( " dabr: 0x%08x   ear: 0x%08x   dec: 0x%08x\n",
	    ppc_get_dabr(), ppc_get_ear(), ppc_get_dec() );
    //printf( " fpscr: 0x%16X\n", ppc_get_fpscr());
    
    
}

static void dbg_print_except_regs( except_regs_t *cpu )
{
    printf( "srr0: %08x              srr1: %08x\n", 
	    cpu->srr0_ip, cpu->srr1_flags);
    printf( "  r0: %08x  r1: %08x  r2: %08x  r3: %08x\n",
	    cpu->r0, cpu->r1_stack, cpu->r2_local_id, cpu->r3 );
    printf( "  r4: %08x  r5: %08x  r6: %08x  r7: %08x\n",
	    cpu->r4, cpu->r5, cpu->r6, cpu->r7 );
    printf( "  r8: %08x  r9: %08x r10: %08x r11: %08x\n",
	    cpu->r8, cpu->r9, cpu->r10, cpu->r11 );
    printf( " r12: %08x r13: %08x r14: %08x r15: %08x\n",
	    cpu->r12, cpu->r13, cpu->r14, cpu->r15 );
    printf( " r16: %08x r17: %08x r18: %08x r19: %08x\n",
	    cpu->r16, cpu->r17, cpu->r18, cpu->r19 );
    printf( " r20: %08x r21: %08x r22: %08x r23: %08x\n",
	    cpu->r20, cpu->r21, cpu->r22, cpu->r23 );
    printf( " r24: %08x r25: %08x r26: %08x r27: %08x\n",
	    cpu->r24, cpu->r25, cpu->r26, cpu->r27 );
    printf( " r28: %08x r29: %08x r30: %08x r31: %08x\n",
	    cpu->r28, cpu->r29, cpu->r30, cpu->r31 );
    printf( " ctr: %08x xer: %08x  cr: %08x  lr: %08x\n",
	    cpu->ctr, cpu->xer, cpu->cr, cpu->lr );
}

#if defined(CONFIG_PPC_MMU_SEGMENTS)
CMD(cmd_print_bats, cg)
{
    printf( "dbat0l: 0x%08x dbat0u: 0x%08x\n",
	    ppc_get_dbat0l(), ppc_get_dbat0u() );
    printf( "dbat1l: 0x%08x dbat1u: 0x%08x\n",
	    ppc_get_dbat1l(), ppc_get_dbat1u() );
    printf( "dbat2l: 0x%08x dbat2u: 0x%08x\n",
	    ppc_get_dbat2l(), ppc_get_dbat2u() );
    printf( "dbat3l: 0x%08x dbat3u: 0x%08x\n",
	    ppc_get_dbat3l(), ppc_get_dbat3u() );

    printf( "ibat0l: 0x%08x ibat0u: 0x%08x\n",
	    ppc_get_ibat0l(), ppc_get_ibat0u() );
    printf( "ibat1l: 0x%08x ibat1u: 0x%08x\n",
	    ppc_get_ibat1l(), ppc_get_ibat1u() );
    printf( "ibat2l: 0x%08x ibat2u: 0x%08x\n",
	    ppc_get_ibat2l(), ppc_get_ibat2u() );
    printf( "ibat3l: 0x%08x ibat3u: 0x%08x\n",
	    ppc_get_ibat3l(), ppc_get_ibat3u() );

    return CMD_NOQUIT;
}
#endif

#if defined(CONFIG_PPC_MMU_TLB)

#include INC_ARCH(swtlb.h)

CMD(cmd_print_tlb, cg)
{
    for (int i = 0; i < PPC_MAX_TLB_ENTRIES; i++)
    {
	ppc_tlb0_t tlb0;
	ppc_tlb1_t tlb1;
	ppc_tlb2_t tlb2;
	ppc_mmucr_t mmucr;
	tlb0.read(i);
	tlb1.read(i);
	tlb2.read(i);

	printf("%02d: %c [%02x:%d] %08x sz:%08x [%04x:%08x] U:%c%c%c S:%c%c%c C:[%c%c%c%c%c U:%c%c%c%c L1:%c%c%c L2:%c%c]\n",
	       i, tlb0.is_valid() ? 'V' : 'I', mmucr.read().get_search_id(),
	       tlb0.trans_space, tlb0.get_vaddr(), tlb0.get_size(),
	       (word_t)(tlb1.get_paddr() >> 32), (word_t)(tlb1.get_paddr()),
	       tlb2.user_execute ? 'X' : '-', tlb2.user_write ? 'W' : '-', 
	       tlb2.user_read ? 'R' : '-', tlb2.super_execute ? 'X' : '-', 
	       tlb2.super_write ? 'W' : '-', tlb2.super_read ? 'R' : '-',
	       tlb2.write_through ? 'W' : '-', tlb2.inhibit ? 'I' : '-',
	       tlb2.mem_coherency ? 'M' : '-', tlb2.guarded ? 'G' : '-',
	       tlb2.endian ? 'E' : '-',
	       tlb2.user0 ? '0' : '-', tlb2.user1 ? '1' : '-',
	       tlb2.user2 ? '2' : '-', tlb2.user3 ? '3' : '-',
	       tlb2.inhibit_l1i ? 'i' : '-', tlb2.inhibit_l1d ? 'd' : '-', tlb2.wt_l1 ? 'W' : '-',
	       tlb2.inhibit_l2i ? 'i' : '-', tlb2.inhibit_l2d ? 'd' : '-' );
    }
	
    return CMD_NOQUIT;
}
#endif

void dbg_dump_msr( word_t msr )
{
    printf( "msr:" );
    if( MSR_BIT(msr,MSR_POW) == MSR_POW_ENABLED )
	printf( " (power management)" );
    if( MSR_BIT(msr,MSR_EE) == MSR_EE_ENABLED )
	printf( " (external interrupt)" );
    if( MSR_BIT(msr,MSR_PR) == MSR_PR_USER )
	printf( " (user mode)" );
    else
	printf( " (kernel mode)" );
    if( MSR_BIT(msr,MSR_FP) == MSR_FP_ENABLED )
	printf( " (fp available)" );
    if( MSR_BIT(msr,MSR_ME) == MSR_ME_ENABLED )
	printf( " (machine check enabled)" );
    if( MSR_BIT(msr,MSR_SE) == MSR_SE_ENABLED )
	printf( " (single-step)" );
    if( MSR_BIT(msr,MSR_BE) == MSR_BE_ENABLED )
	printf( " (branch trace)" );
    if( MSR_BIT(msr,MSR_IP) == MSR_IP_RAM )
	printf( " (ram prefix)" );
    else
	printf( " (rom prefix)" );
    if( MSR_BIT(msr,MSR_IR) == MSR_IR_ENABLED )
	printf( " (instr translation)" );
    if( MSR_BIT(msr,MSR_DR) == MSR_DR_ENABLED )
	printf( " (data translation)" );
    if( MSR_BIT(msr,MSR_LE) == MSR_LE_BIG_ENDIAN )
	printf( " (big-endian)" );
    else
	printf( " (little-endian)" );
    if( MSR_BIT(msr,MSR_RI) == MSR_RI_IS_RECOVERABLE )
	printf( " (recoverable exception)" );

    printf( "\n" );
}


CMD(cmd_sysregs, cg)
{
    dbg_print_sysregs();
    return CMD_NOQUIT;
}

CMD(cmd_except_regs, cg)
{
    debug_param_t *param = (debug_param_t *)kdb.kdb_param;
    if( param != NULL )
	dbg_print_except_regs( param->frame );
    return CMD_NOQUIT;
}

CMD(cmd_user_except_regs, cg)
{
    except_regs_t *regs = get_user_except_regs( get_current_tcb() );
    if( regs != NULL )
	dbg_print_except_regs( regs );
    return CMD_NOQUIT;
}

CMD(cmd_print_msr, cg)
{
    dbg_dump_msr( ppc_get_msr() );
    return CMD_NOQUIT;
}

CMD(cmd_print_except_msr, cg)
{
    debug_param_t *param = (debug_param_t *)kdb.kdb_param;
    if( param != NULL )
	dbg_dump_msr( param->frame->srr1_flags );
    return CMD_NOQUIT;
}

#ifdef CONFIG_PLAT_PPC44X
#include INC_GLUE(intctrl.h)
DECLARE_CMD (cmd_print_irqctrl, arch, 'i', "printirq", "Print IRQ controller status");
CMD(cmd_print_irqctrl, cg)
{
    get_interrupt_ctrl()->dump();
    return CMD_NOQUIT;
}
#endif
