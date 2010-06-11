/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/ipc.h
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
#ifndef __GLUE__V4_POWERPC__IPC_H__
#define __GLUE__V4_POWERPC__IPC_H__


class arch_ctrlxfer_item_t
{ 
public:
    enum id_e {
	id_gpregs0,	// r0-r15
	id_gpregs1,	// r16-r31
	id_gpregsx,	// xer, cr, ctr, lr, ip
 	id_fpuregs,
#ifdef CONFIG_X_PPC_SOFTHVM
	id_mmu,		// mmucr, pid
	id_except,	// msr, esr, mcsr, dear, srr0, srr1, csrr0, csrr1, mcsrr0, mcsrr1, event
	id_ivor,	// ivor0..ivor15
	id_timer,	// tcr, tsr, tbu, tbl, dec, decar
	id_config,	// ccr0, ccr1, rstcfg, ivpr, rsttlb0, rsttlb1, rsttlb2
	id_debug,	// dbsr, dbdr, dbcr0..dbcr2, dac0/1, dvc0/1, iac0..3, icdbdr, icdbt0/1, dcdbt0/1
	id_icache,	// ivlim, inv0..inv3, itv0..itv3,
	id_dcache,	// dvlim, dnv0..dnv3, dtv0..dtv3
	id_shadow_tlb,
	id_tlb0,	// TLB0..TLB3
	id_tlb1,
	id_tlb2,
	id_tlb3,
	id_tlb4,	// TLB16..TLB19
	id_tlb5,
	id_tlb6,
	id_tlb7,
	id_tlb8,	// TLB32..TLB35
	id_tlb9,
	id_tlb10,
	id_tlb11,
	id_tlb12,	// TLB48..TLB51
	id_tlb13,
	id_tlb14,
	id_tlb15,
#endif
	id_max,
    };
};

#endif /* !__GLUE__V4_POWERPC__IPC_H__ */
