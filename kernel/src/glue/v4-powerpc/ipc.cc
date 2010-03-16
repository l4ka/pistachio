/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/ipc.cc
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
#include INC_ARCH(msr.h)
#include INC_ARCH(frame.h)
#include INC_API(tcb.h)
#include INC_GLUE(tracepoints.h)

#define EXC_REG(x) (offsetof(except_regs_t, x) / BYTES_WORD)

static const word_t frame_gprs0[] = {
    EXC_REG(r0), EXC_REG(r1_stack), EXC_REG(r2_local_id), EXC_REG(r3), 
    EXC_REG(r4), EXC_REG(r5), EXC_REG(r6), EXC_REG(r7), 
    EXC_REG(r8), EXC_REG(r9), EXC_REG(r10), EXC_REG(r11), 
    EXC_REG(r12), EXC_REG(r13), EXC_REG(r14), EXC_REG(r15),
};

static const word_t frame_gprs1[] = {
    EXC_REG(r16), EXC_REG(r17), EXC_REG(r18), EXC_REG(r19), 
    EXC_REG(r20), EXC_REG(r21), EXC_REG(r22), EXC_REG(r23), 
    EXC_REG(r24), EXC_REG(r25), EXC_REG(r26), EXC_REG(r27), 
    EXC_REG(r28), EXC_REG(r29), EXC_REG(r30), EXC_REG(r31),
};

static const word_t frame_gprx[] = {
    EXC_REG(xer), EXC_REG(cr), EXC_REG(ctr), EXC_REG(lr), EXC_REG(srr0_ip),
};

#ifdef CONFIG_X_PPC_SOFTHVM

#include INC_ARCH(softhvm.h)

extern ppc_softhvm_t __dummysofthvm;
#define HVM_REG(x) ((((word_t)(&__dummysofthvm.x)) - (word_t)(&(__dummysofthvm))) / BYTES_WORD)

static const word_t hwreg_mmu[] = {
    HVM_REG(mmucr), HVM_REG(pid)
};

static const word_t hwreg_except[] = { 
    HVM_REG(msr), HVM_REG(esr), HVM_REG(mcsr), HVM_REG(dear), 
    HVM_REG(srr0), HVM_REG(srr1), HVM_REG(csrr0), HVM_REG(csrr1),
    HVM_REG(mcsrr0), HVM_REG(mcsrr1), HVM_REG(event_inject)
};

static const word_t hwreg_ivor[] = {
    HVM_REG(ivor[0]), HVM_REG(ivor[1]), HVM_REG(ivor[2]), HVM_REG(ivor[3]),
    HVM_REG(ivor[4]), HVM_REG(ivor[5]), HVM_REG(ivor[6]), HVM_REG(ivor[7]),
    HVM_REG(ivor[8]), HVM_REG(ivor[9]), HVM_REG(ivor[10]), HVM_REG(ivor[11]),
    HVM_REG(ivor[12]), HVM_REG(ivor[13]), HVM_REG(ivor[14]), HVM_REG(ivor[15]) 
};

static const word_t hwreg_timer[] = {
    HVM_REG(tcr), HVM_REG(tsr), HVM_REG(tbl), HVM_REG(tbu), HVM_REG(dec), HVM_REG(decar)
};

static const word_t hwreg_config[] = {
    HVM_REG(ccr0), HVM_REG(ccr1), HVM_REG(rstcfg), HVM_REG(ivpr),
    HVM_REG(pir),  HVM_REG(pvr),
};

static const word_t hwreg_debug[] = {
    HVM_REG(dbsr), HVM_REG(dbdr), 
    HVM_REG(dbcr[0]), HVM_REG(dbcr[1]), HVM_REG(dbcr[2]), 
    HVM_REG(dac[0]), HVM_REG(dac[1]), HVM_REG(dvc[0]), HVM_REG(dvc[1]),
    HVM_REG(iac[0]), HVM_REG(iac[1]), HVM_REG(iac[2]), HVM_REG(iac[3]),
    HVM_REG(icdbdr), HVM_REG(icdbt[0]), HVM_REG(icdbt[1]), 
    HVM_REG(dcdbt[0]), HVM_REG(dcdbt[1])
};

static const word_t hwreg_icache[] = {
    HVM_REG(ivlim), 
    HVM_REG(inv[0]), HVM_REG(inv[1]), HVM_REG(inv[2]), HVM_REG(inv[3]), 
    HVM_REG(itv[0]), HVM_REG(itv[1]), HVM_REG(itv[2]), HVM_REG(itv[3])
};

static const word_t hwreg_dcache[] = {
    HVM_REG(dvlim), 
    HVM_REG(dnv[0]), HVM_REG(dnv[1]), HVM_REG(dnv[2]), HVM_REG(dnv[3]), 
    HVM_REG(dtv[0]), HVM_REG(dtv[1]), HVM_REG(dtv[2]), HVM_REG(dtv[3])
};

static const word_t hwreg_shadow_tlb[] = {
    HVM_REG(shadow_tlb.tlb0), HVM_REG(shadow_tlb.tlb1), 
    HVM_REG(shadow_tlb.tlb2), HVM_REG(shadow_tlb.pid)
};
#endif

const word_t* const ctrlxfer_item_t::hwregs[] = {
    frame_gprs0, frame_gprs1, frame_gprx, NULL, 
#ifdef CONFIG_X_PPC_SOFTHVM
    hwreg_mmu, hwreg_except, hwreg_ivor, 
    hwreg_timer, hwreg_config, hwreg_debug,
    hwreg_icache, hwreg_dcache, hwreg_shadow_tlb,
    NULL, NULL, NULL, NULL, // TLBs (probably not required)
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, 
#endif
};

const word_t ctrlxfer_item_t::num_hwregs[] = {
    sizeof(frame_gprs0) / sizeof(word_t),
    sizeof(frame_gprs1) / sizeof(word_t),
    sizeof(frame_gprx) / sizeof(word_t),
    0,	// fprs are transported directly in FPU
#ifdef CONFIG_X_PPC_SOFTHVM
    sizeof(hwreg_mmu) / sizeof(word_t),
    sizeof(hwreg_except) / sizeof(word_t),
    sizeof(hwreg_ivor) / sizeof(word_t),
    sizeof(hwreg_timer) / sizeof(word_t),
    sizeof(hwreg_config) / sizeof(word_t),
    sizeof(hwreg_debug) / sizeof(word_t),
    sizeof(hwreg_icache) / sizeof(word_t),
    sizeof(hwreg_dcache) / sizeof(word_t),
    sizeof(hwreg_shadow_tlb) / sizeof(word_t),
    16, 16, 16, 16,	// TLB 
    16, 16, 16, 16,
    16, 16, 16, 16,
    16, 16, 16, 16,
#endif
};
