/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     glue/v4-ia32/hvm/vmx/vcpu.h
 * Description:   Vanderpool Virtual Machine Extensions
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
#ifndef __GLUE__V4_X86__X32__HVM_VMX_H__
#define __GLUE__V4_X86__X32__HVM_VMX_H__

#include INC_ARCH_SA(tss.h)
#include INC_ARCH_SA(vmx.h)
#include INC_GLUE(hvm-vtlb.h)


class tcb_t;
class x86_exceptionframe_t;

#define X86_HVM_NUM_CREGS           8
#define X86_HVM_NUM_DREGS           8
#define X86_HVM_NUM_CSREGS          4
#define X86_HVM_NUM_SSREGS          4
#define X86_HVM_NUM_DSREGS          4
#define X86_HVM_NUM_ESREGS          4
#define X86_HVM_NUM_FSREGS          4
#define X86_HVM_NUM_GSREGS          4
#define X86_HVM_NUM_TRREGS          4
#define X86_HVM_NUM_LDTRREGS        4
#define X86_HVM_NUM_IDTRREGS        2
#define X86_HVM_NUM_GDTRREGS        2
#define X86_HVM_NUM_NONREGEXC       10
#define X86_HVM_NUM_EXECCTRL        3
#define X86_HVM_NUM_OTHERREGS       9

#define X86_HVM_CREG2HVMIDX	    { /* software-emulated */ }
#define X86_HVM_DREG2HVMIDX	    { 0 /* DR0 */, 1 /* DR1 */, 2 /* DR3 */, 3 /* DR3 */, \
				      6 /* DR6 */, 7 /* DR7 */ }
#define X86_HVM_CSREG2HVMIDX	    { VMCS_IDX_G_CS_SEL, VMCS_IDX_G_CS_BASE, VMCS_IDX_G_CS_LIMIT, VMCS_IDX_G_CS_ATTR }
#define X86_HVM_SSREG2HVMIDX	    { VMCS_IDX_G_SS_SEL, VMCS_IDX_G_SS_BASE, VMCS_IDX_G_SS_LIMIT, VMCS_IDX_G_SS_ATTR }
#define X86_HVM_DSREG2HVMIDX	    { VMCS_IDX_G_DS_SEL, VMCS_IDX_G_DS_BASE, VMCS_IDX_G_DS_LIMIT, VMCS_IDX_G_DS_ATTR }
#define X86_HVM_ESREG2HVMIDX	    { VMCS_IDX_G_ES_SEL, VMCS_IDX_G_ES_BASE, VMCS_IDX_G_ES_LIMIT, VMCS_IDX_G_ES_ATTR }
#define X86_HVM_FSREG2HVMIDX	    { VMCS_IDX_G_FS_SEL, VMCS_IDX_G_FS_BASE, VMCS_IDX_G_FS_LIMIT, VMCS_IDX_G_FS_ATTR }
#define X86_HVM_GSREG2HVMIDX	    { VMCS_IDX_G_GS_SEL, VMCS_IDX_G_GS_BASE, VMCS_IDX_G_GS_LIMIT, VMCS_IDX_G_GS_ATTR }
#define X86_HVM_TRREG2HVMIDX	    { VMCS_IDX_G_TR_SEL, VMCS_IDX_G_TR_BASE, VMCS_IDX_G_TR_LIMIT, VMCS_IDX_G_TR_ATTR }
#define X86_HVM_LDTRREG2HVMIDX	    { VMCS_IDX_G_LDTR_SEL, VMCS_IDX_G_LDTR_BASE, VMCS_IDX_G_LDTR_LIMIT, VMCS_IDX_G_LDTR_ATTR }
#define X86_HVM_IDTRREG2HVMIDX	    { VMCS_IDX_G_IDTR_BASE, VMCS_IDX_G_IDTR_LIMIT }
#define X86_HVM_GDTRREG2HVMIDX	    { VMCS_IDX_G_GDTR_BASE, VMCS_IDX_G_GDTR_LIMIT }
#define X86_HVM_NONREGEXC	             { /* software-emulated */ } 
#define X86_HVM_EXECCTRL             { /* software-emulated */ } 
#define X86_HVM_OTHERREG             { /* software-emulated */ } 


#define X86_HVM_FLAGS		    (2)
#define X86_HVM_EFLAGS_MASK	    ~(1<<1)
/* Flags allowed to set by the user */
#define X86_HVM_CR0_MASK	    (X86_CR0_PE | X86_CR0_MP | X86_CR0_EM | X86_CR0_NE | \
				     X86_CR0_WP | X86_CR0_AM | X86_CR0_NW | X86_CR0_CD | X86_CR0_PG)
    
#define X86_HVM_CR4_MASK	    (X86_CR4_PVI | X86_CR4_PSE | X86_CR4_PGE | X86_CR4_PCE | \
				     X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT)


class x32_hvm_vmx_t
{
protected:
    bool load_vmcs()
	{ return (vmcs && vmcs->load()); }

    /* Initialize L4-specific VMCS data. */
    void init_vmcs ();

#if defined(CONFIG_IO_FLEXPAGES)
    /* Set I/O permission bitmap. */
    void set_io_pbm (addr_t paddr);
#endif

    /* Handle specific vmexit reasons. */
    bool handle_debug_exit(vmcs_ei_qual_t qual);
    bool handle_nomath_exit ();
    bool handle_pagefault_exit (vmcs_ei_qual_t qual);
    void handle_invd_exit ();
    void handle_invlpg_exit (vmcs_ei_qual_t qual);


    /* Save/restore DRs. */
    void save_guest_drs ();
    void restore_guest_drs  ();
#if defined(CONFIG_DEBUG)
    /* Logic to pass through KDB DRs */
    word_t kdb_dr_mask;
    bool kdb_branchstep;
#endif
    /* VCPU parts */
    vmcs_t *vmcs;
    x86_hvm_vtlb_t vtlb;
   

    /* Virtual registers cache. */
    word_t guest_dr[8];
    static word_t host_dr[8];
    word_t guest_cr2;

    word_t guest_cr0_mask;
    word_t guest_cr4_mask;

    struct 
    {
	vmcs_int_t entry_info;
	word_t	   entry_eec;
	word_t	   entry_ilen;
	vmcs_int_t exit_info;
	word_t	   exit_eec;
	vmcs_int_t idt_info;
	word_t	   idt_eec;
    } exc;
    
    /* Flags */
    struct {
	bool injecting;
	bool cr2_write;
	bool fpu_ts_bit;
	bool movdr;
	bool bp;
	bool db;
    } flags;
    
    friend class x86_hvm_space_t;

};

typedef x32_hvm_vmx_t x86_svmx_hvm_t;



#endif /* !__GLUE__V4_X86__X32__HVM_VMX_H__ */
