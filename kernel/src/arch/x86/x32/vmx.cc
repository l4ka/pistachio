/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     arch/ia32/vmx/vmcs.cc
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

#include <kmemory.h>
#include <debug.h>
#include INC_ARCH(segdesc.h)
#include INC_ARCH_SA(vmx.h)

DECLARE_KMEM_GROUP (kmem_vmcs);


INLINE bool is_canonical_address (word_t addr)
{
    return true;
}


enum x86_x32_vmcs_access_e {
    uc = 1,
    wb = 6
};

typedef union {
    u64_t raw;
    struct {
	u64_t vmcs_rev_id	: 32;
	u64_t vmcs_sz		: 13;
	u64_t res1		:  5;
	u64_t vmcs_memtype	:  4;
	u64_t res2		: 10;
    };
} basic_msr_t;

INLINE u16_t get_vmcs_sz ()
{
    basic_msr_t msr;
    msr.raw = x86_rdmsr (X86_MSR_VMX_BASIC);
    return msr.vmcs_sz;
}

INLINE x86_x32_vmcs_access_e get_vmcs_access_mode ()
{
    basic_msr_t msr;
    msr.raw = x86_rdmsr (X86_MSR_VMX_BASIC);
    return (x86_x32_vmcs_access_e)msr.vmcs_memtype;
}

INLINE u32_t get_vmcs_rev_id ()
{
    basic_msr_t msr;
    msr.raw = x86_rdmsr (X86_MSR_VMX_BASIC);
    return msr.vmcs_rev_id;
}


/*******************************************************************************
 *
 * VMCS
 *
 *******************************************************************************/

vmcs_t *current_vmcs UNIT("cpulocal");


vmcs_t *vmcs_t::alloc_vmcs ()
{
    // Kernel Memory Allocator gives WB memory.
    if (get_vmcs_access_mode() != wb)
	return NULL;

    // Allocate VMCS Region, must be aligned to page boundary
    word_t sz = X86_PAGE_SIZE;
    addr_t vmcs = kmem.alloc(kmem_vmcs, sz);
    if (!vmcs)
	return NULL;

    // Set Revision.
    u32_t *ptr = (u32_t *) vmcs;
    *ptr = get_vmcs_rev_id();

    //TRACEF ("New VMCS created: va: %p  pa: %p  sz: %x rev: %lx\n",
    //       vmcs, virt_to_phys(vmcs), sz, *ptr);

    return (vmcs_t *) virt_to_phys(vmcs);
}


void vmcs_t::free_vmcs (vmcs_t *vmcs)
{
    kmem.free(kmem_vmcs, phys_to_virt(vmcs), get_vmcs_sz());
}


void vmcs_t::init ()
{
    gs.init ();
    exec_ctr.init ();
    exit_ctr.init ();
    entry_ctr.init ();
}


/*******************************************************************************
 *
 * VMCS Entry Control
 *
 *******************************************************************************/

void vmcs_entryctrarea_t::init ()
{
    vmcs_entryctr_t i_entryctr;
    entryctr		= i_entryctr;

    vmcs_int_t i_iif;
    iif			= i_iif;

    msr_ld_cnt		= 0;
    msr_ld_addr		= 0;

    instr_len		= 0;

    eec			= 0;
}


/*******************************************************************************
 *
 * VMCS Execution Control
 *
 *******************************************************************************/

void vmcs_exectrarea_t::init ()
{
    vmcs_exectr_pinbased_t i_pinbased;
    pinbased	= i_pinbased;

    vmcs_exectr_cpubased_t i_cpubased;
    cpubased	= i_cpubased;

    vmcs_exectr_excbmp_t i_except_bmp;
    except_bmp	= i_except_bmp;

    pferrmask	= 0;
    pferrmatch	= 0;

    vmcs_exectr_tprth_t i_tprth;
    tprthr	= i_tprth;

    iobmpa	= 0;
    iobmpb	= 0;

    tscoff	= 0;
    vapicaddr	= 0;

    // Masks: Owned by the host.
    cr0mask	= ~0UL;
    cr4mask	= ~0UL;

    // Shadow: The values seen by the guest, if all bits in the masks are set.
    cr0shadow	= X86_CR0_ET | X86_CR0_NW | X86_CR0_CD;
    cr4shadow	= 0;

    // cr3
    cr3targetcnt = 0;
    cr3val0	= 0;
    cr3val1	= 0;
    cr3val2	= 0;
    cr3val3	= 0;
}


/*******************************************************************************
 *
 * VMCS Exit Control
 *
 *******************************************************************************/

void vmcs_exitctrarea_t::init ()
{
    vmcs_exitctr_t i_exitctr;
    exitctr	= i_exitctr;

    msr_st_cnt	= 0;
    msr_ld_cnt	= 0;
    msr_st_addr	= 0;
    msr_ld_addr	= 0;
}


/*******************************************************************************
 *
 * VMCS Guest State
 *
 *******************************************************************************/

void vmcs_gsarea_t::init ()
{
    vmcs_segattr_t i_attr;

    rflags		= X86_FLAGS_VM | X86_FLAGS_IOPL(3) | 0x2;
    rip			= 0;
    rsp			= 0;

    cs_sel = ds_sel = es_sel = fs_sel = gs_sel = ss_sel
	   = tr_sel = ldtr_sel = 0;
    cs_base = ds_base = es_base = fs_base = gs_base = ss_base
	    = tr_base = ldtr_base = gdtr_base = idtr_base = 0;
    cs_lim = ds_lim = es_lim = fs_lim = gs_lim = ss_lim
	   = tr_lim = ldtr_lim = gdtr_lim = idtr_lim = 0xffff;
    
    i_attr.raw		= 0xf3;
    cs_attr		= i_attr;
    ds_attr		= i_attr;
    es_attr		= i_attr;
    fs_attr		= i_attr;
    gs_attr		= i_attr;
    ss_attr		= i_attr;

    i_attr.raw		= 0x0808b;
    tr_attr		= i_attr;

    i_attr.raw		= 0x10082;
    ldtr_attr		= i_attr;
	
    vmcs_gs_ias_t i_ias;
    ias			= i_ias;

    vmcs_gs_as_t	i_as;
    i_as.state		= vmcs_gs_as_t::active;
    as			= i_as;

    vmcs_gs_pend_dbg_except_t i_dbge;
    pend_dbg_except	= i_dbge;

    linkptr		= ~0ULL;

    dbg_ctl		= 0;

    sysenter_esp	= 0;
    sysenter_eip	= 0;
}


/*******************************************************************************
 *
 * Debugging
 *
 *******************************************************************************/

#if defined(CONFIG_DEBUG)

static const u32_t BITS_15_03 = ((1UL<<14) - 1) << 3;
static const u32_t BITS_15_02 = ((1UL<<15) - 1) << 2;
static const u32_t BITS_31_04 = ((1UL<<29) - 1) << 4;
static const u32_t BITS_31_15 = ((1UL<<18) - 1) << 15;
static const u32_t BITS_31_16 = ((1UL<<17) - 1) << 16;
static const u32_t BITS_31_17 = ((1UL<<16) - 1) << 17;
static const u32_t BITS_31_20 = ((1UL<<13) - 1) << 20;
static const u32_t BITS_30_12 = ((1UL<<19) - 1) << 12;
static const u32_t BITS_11_00 = ((1UL<<12) - 1);
static const u32_t BITS_11_04 = ((1UL<<8) - 1) << 4;
static const u32_t BITS_11_08 = ((1UL<<4) - 1) << 8;
static const u64_t BITS_63_15 =	((1ULL<<50) - 1) << 15;
static const u64_t BITS_63_32 = ((1ULL<<33) - 1) << 32;
static const u64_t BITS_63_22 = ((1ULL<<43) - 1) << 22;


class vmcs_segsel_t {
public:
    vmcs_segsel_t ()
	{ raw = 0; }

public:
    // Table Indicator
    enum ti_e {
       gdt = 0, ldt = 1
    };

public:
    union {
	u16_t raw;
	struct {
	    u16_t rpl   : 2;
	    u16_t ti    : 1; // ti_e
	    u16_t idx   :13;
	};
    };
};


void vmcs_t::do_vmentry_checks ()
{
    // VT-Specification Chapter 4.1

    // 1. Virtual-8068 or Compatibility Mode (-> invalid-opcode)
    // 2. CPL is != 0.
    // 3. No Current VMCS.
    ASSERT(is_loaded());
    // 4a. MOV-SS blocking.
    //vmcs_gs_ias_t interruptililty = gs.ias;
    //ASSERT(interruptililty.bl_movss == 0);
    // 4b. VMLAUNCH on non-clear VMCS.
    // 4c. VMRESUME on non-launched VMCS.
    
    vmcs_entryctr_t i_entryctr = entry_ctr.entryctr;
    vmcs_int_t i_iif = entry_ctr.iif;
    vmcs_exitctr_t i_exitctr = exit_ctr.exitctr;

    gs.do_vmentry_checks(i_entryctr, i_iif);
    hs.do_vmentry_checks(i_entryctr, i_exitctr);
    exec_ctr.do_vmentry_checks();
    exit_ctr.do_vmentry_checks();
    entry_ctr.do_vmentry_checks();
}


void vmcs_entryctrarea_t::do_vmentry_checks ()
{
    // Entry Control.		(22.2.1.3)
    vmcs_entryctr_t i_entryctr;
    i_entryctr	      = entryctr;
    u64_t i_entrydefs = x86_rdmsr (X86_MSR_VMX_ENTRY_CTLS);
    u32_t ALLOWED0 = (u32_t)i_entrydefs;
    u32_t ALLOWED1 = i_entrydefs >> 32;

    vmcs_entryctr_t i_entryctr_check;
    i_entryctr_check.raw = i_entryctr.raw | ALLOWED0;
    i_entryctr_check.raw = i_entryctr_check.raw & ALLOWED1;
    if (i_entryctr_check.raw != i_entryctr.raw)
    {
	printf("ALLOWED0   %lx\n", ALLOWED0);
	printf("ALLOWED1   %lx\n", ALLOWED1);
	printf("i_entryctr %lx\n", i_entryctr.raw);
	printf("..._check  %lx\n", i_entryctr_check.raw);
	ASSERT(i_entryctr_check.raw == i_entryctr.raw);
    }

    // Event Injection.		(22.2.1.3)
    vmcs_int_t i_iif;
    i_iif	= iif;
    if (i_iif.valid) {
	ASSERT((i_iif.type != 1) &&
	       (i_iif.type != 7));

	u32_t i_eec;
	i_eec	= eec;
	//	printf("iif: %x, eec: %x\n", i_iif.raw, i_eec);
	if (i_iif.type == vmcs_int_t::hw_nmi)
	    ASSERT(i_iif.vector == 2);
	if (i_iif.type == vmcs_int_t::hw_except)
	    ASSERT(i_iif.vector <= 31);

	if (i_iif.err_code_valid == 1) {
	    ASSERT(i_iif.type == vmcs_int_t::hw_except);
	}

	if (i_iif.raw & (BITS_30_12))
	    printf("i_iif.raw %p BITS_30_12 %p\n", i_iif.raw, BITS_30_12);
	ASSERT((i_iif.raw & (BITS_30_12)) == 0);
	if (i_iif.err_code_valid == 1) {
	    ASSERT((i_eec & (BITS_31_15)) == 0);
	}

	u32_t i_instr_len;
	i_instr_len	= instr_len;
	if (i_iif.type >= vmcs_int_t::sw_int) 
	{
	    ASSERT((i_instr_len >= 1) && (i_instr_len <= 15));
	}
    }

    // MSRs.
    u32_t i_msr_ld_cnt;
    i_msr_ld_cnt	= msr_ld_cnt;
    if(i_msr_ld_cnt != 0) {
	UNIMPLEMENTED();
    }

    // SMM checks missing.

    ASSERT( !((i_entryctr.entry_smm == 1) && (i_entryctr.deact_dmt == 1)) );
}


void vmcs_exectrarea_t::do_vmentry_checks ()
{
    // Pin Based.		(22..2.1.1)
    vmcs_exectr_pinbased_t i_pb;
    i_pb	= pinbased;
    u64_t i_pbctls = x86_rdmsr (X86_MSR_VMX_PINBASED_CTLS);
    u32_t ALLOWED0 = (u32_t)i_pbctls;
    u32_t ALLOWED1 = i_pbctls >> 32;

    vmcs_exectr_pinbased_t i_pb_check;
    i_pb_check.raw = i_pb.raw | ALLOWED0;
    i_pb_check.raw = i_pb_check.raw & ALLOWED1;
    if (i_pb_check.raw != i_pb.raw)
    {
	printf("ALLOWED0   %lx\n", ALLOWED0);
	printf("ALLOWED1   %lx\n", ALLOWED1);
	printf("i_pb       %lx\n", i_pb.raw);
	printf("i_pb_check %lx\n", i_pb_check.raw);
	ASSERT(i_pb_check.raw == i_pb.raw);
    }

    // CPU Based.
    vmcs_exectr_cpubased_t i_cb;
    i_cb		= cpubased;
    u64_t i_cbctls	= x86_rdmsr (X86_MSR_VMX_CPUBASED_CTLS);
    ALLOWED0 = (u32_t)i_cbctls;
    ALLOWED1 = i_cbctls >> 32;
    vmcs_exectr_cpubased_t i_cb_check;
    i_cb_check.raw = i_cb.raw | ALLOWED0;
    i_cb_check.raw = i_cb_check.raw & ALLOWED1;
    if (i_cb_check.raw != i_cb.raw)
    {
	printf("ALLOWED0   %lx\n", ALLOWED0);
	printf("ALLOWED1   %lx\n", ALLOWED1);
	printf("i_cb       %lx\n", i_cb.raw);
	printf("i_cb_check %lx\n", i_cb_check.raw);
	ASSERT(i_cb_check.raw == i_cb.raw);
    }

    // Cr3.
    u32_t i_cr3_count = 0;
    i_cr3_count	= cr3targetcnt;
    ASSERT(i_cr3_count <= 4);

    // IO-Bitmap.
    if (i_cb.iobitm)
    {
	u64_t i_iobmpa = iobmpa;
	ASSERT((i_iobmpa & ~X86_PAGE_MASK) == 0);
    }

    // TPR Shadow.
    if (i_cb.tpr_shadow)
	UNIMPLEMENTED();

    // MSR-Bitmaps.
    if (i_cb.msrbitm)
	UNIMPLEMENTED();
}


void vmcs_exitctrarea_t::do_vmentry_checks ()
{
    // Exit Control.		(22.2.1.2)
    vmcs_exitctr_t i_exitctr;
    i_exitctr		= exitctr;
    u64_t i_exitdefs	= x86_rdmsr (X86_MSR_VMX_EXIT_CTLS);
    u32_t ALLOWED0	= (u32_t)i_exitdefs;
    u32_t ALLOWED1	= i_exitdefs >> 32;

    vmcs_exitctr_t i_exitctr_check;
    i_exitctr_check.raw = i_exitctr.raw | ALLOWED0;
    i_exitctr_check.raw = i_exitctr_check.raw & ALLOWED1;
    if (i_exitctr_check.raw != i_exitctr.raw)
    {
	printf("ALLOWED0  %lx\n", ALLOWED0);
	printf("ALLOWED1  %lx\n", ALLOWED1);
	printf("i_exitctr %lx\n", i_exitctr.raw);
	printf("..._check %lx\n", i_exitctr_check.raw);
	ASSERT(i_exitctr_check.raw == i_exitctr.raw);
    }

    // MSRs.
    if (msr_st_cnt != 0)
	UNIMPLEMENTED();
    if (msr_ld_cnt != 0)
	UNIMPLEMENTED();
}


void vmcs_gsarea_t::do_vmentry_checks (vmcs_entryctr_t i_entryctr, vmcs_int_t i_iif)
{
    vmcs_gs_ias_t i_ias;
    i_ias	= ias;

    vmcs_gs_as_t  i_as;
    i_as	= as;

    word_t i_rip	= rip;
    word_t i_cr0	= cr0;
    word_t i_cr3	= cr3;
    word_t i_dr7	= dr7;
    word_t i_sysenter_esp = sysenter_esp;
    word_t i_sysenter_eip = sysenter_eip;;
    u32_t i_gdtr_limit	= gdtr_lim;
    u32_t i_idtr_limit	= idtr_lim;

    word_t i_rfl	= rflags;
    word_t i_cr4	= cr4;

    bool virt8086 = (i_rfl & X86_FLAGS_VM);
    bool ia32e = (i_entryctr.ia32e_mode);
    ASSERT(ia32e == false);

    // CR0		(22.3.1.1)
    ASSERT(x86_x32_vmx_t::check_fixed_bits_cr0 (i_cr0));

    // CR3
    ASSERT(is_canonical_address (i_cr3));

    // CR4
    ASSERT(x86_x32_vmx_t::check_fixed_bits_cr4 (i_cr4));
    if (i_entryctr.ia32e_mode)
	ASSERT((i_cr4 & X86_CR4_PAE) != 0);

    // DRs		(22.3.1.1)
    ASSERT((i_dr7 & (BITS_63_32)) == 0);

    // Sysenter / -exit (22.3.1.1)
    ASSERT(is_canonical_address (i_sysenter_esp));
    ASSERT(is_canonical_address (i_sysenter_eip));

    // Segments		(22.3.1.2)
    vmcs_segattr_t i_cs_attr;
    vmcs_segattr_t i_ss_attr;
    vmcs_segattr_t i_ds_attr;
    vmcs_segattr_t i_es_attr;
    vmcs_segattr_t i_fs_attr;
    vmcs_segattr_t i_gs_attr;
    vmcs_segattr_t i_tr_attr;
    vmcs_segattr_t i_ldtr_attr;
    i_cs_attr	= cs_attr;
    i_ss_attr	= ss_attr;
    i_ds_attr	= ds_attr;
    i_es_attr	= es_attr;
    i_fs_attr	= fs_attr;
    i_gs_attr	= gs_attr;
    i_tr_attr	= tr_attr;
    i_ldtr_attr	= ldtr_attr;

    vmcs_segsel_t i_cs_sel;
    vmcs_segsel_t i_ss_sel;
    vmcs_segsel_t i_ds_sel;
    vmcs_segsel_t i_es_sel;
    vmcs_segsel_t i_fs_sel;
    vmcs_segsel_t i_gs_sel;
    vmcs_segsel_t i_tr_sel;
    vmcs_segsel_t i_ldtr_sel;
    i_cs_sel.raw	= cs_sel;
    i_ss_sel.raw	= ss_sel;
    i_ds_sel.raw	= ds_sel;
    i_es_sel.raw	= es_sel;
    i_fs_sel.raw	= fs_sel;
    i_gs_sel.raw	= gs_sel;
    i_tr_sel.raw	= tr_sel;
    i_ldtr_sel.raw	= ldtr_sel;

    u32_t i_cs_lim;
    u32_t i_ss_lim;
    u32_t i_ds_lim;
    u32_t i_es_lim;
    u32_t i_fs_lim;
    u32_t i_gs_lim;
    u32_t i_tr_lim;
    u32_t i_ldtr_lim;
    i_cs_lim		= cs_lim;
    i_ss_lim		= ss_lim;
    i_ds_lim		= ds_lim;
    i_es_lim		= es_lim;
    i_fs_lim		= fs_lim;
    i_gs_lim		= gs_lim;
    i_tr_lim		= tr_lim;
    i_ldtr_lim		= ldtr_lim;
    
    u64_t i_cs_base	= cs_base;
    u64_t i_ss_base	= ss_base;
    u64_t i_ds_base	= ds_base;
    u64_t i_es_base	= es_base;
    u64_t i_fs_base	= fs_base;
    u64_t i_gs_base	= gs_base;
    // 22.3.1.2 Chapter 22-8
    ASSERT (i_tr_sel.ti == 0);
    if (i_ldtr_attr.uu == 0)
	ASSERT (i_ldtr_sel.ti == 0);
    if (!virt8086)
	//ASSERT(i_ss_sel.rpl == i_cs_sel.rpl);
      	if(i_ss_sel.rpl != i_cs_sel.rpl) 
	{
	    i_cs_sel.rpl = i_ss_sel.rpl = 0;
	    cs_sel = i_cs_sel.raw;
	    ss_sel = i_ss_sel.raw;
	    i_cs_attr.dpl = 0;
	    i_ss_attr.dpl = 0;
	    cs_attr = i_cs_attr;
	    ss_attr = i_ss_attr;
	}
    
    ASSERT ((i_cs_base & (BITS_63_32)) == 0);

    if (i_ss_attr.uu == 0)
	ASSERT ((ss_base & (BITS_63_32)) == 0);
    if (i_ds_attr.uu == 0)
	ASSERT ((ds_base & (BITS_63_32)) == 0);
    if (i_es_attr.uu == 0)
	ASSERT ((es_base & (BITS_63_32)) == 0);

    // Limit fields, access rights.
    if (virt8086) 
    {
	if  (i_cs_base != ((u32_t) i_cs_sel.raw) << 4)
	    printf("%x vs %x\n", i_cs_base, i_cs_sel.raw);
	ASSERT(i_cs_base == ((u32_t) i_cs_sel.raw) << 4);
	ASSERT(i_ss_base == ((u32_t) i_ss_sel.raw) << 4);
	ASSERT(i_ds_base == ((u32_t) i_ds_sel.raw) << 4);
	ASSERT(i_es_base == ((u32_t) i_es_sel.raw) << 4);
	ASSERT(i_fs_base == ((u32_t) i_fs_sel.raw) << 4);
	ASSERT(i_gs_base == ((u32_t) i_gs_sel.raw) << 4);

	ASSERT(i_cs_lim == 0x0000FFFF);
	ASSERT(i_ss_lim == 0x0000FFFF);
	ASSERT(i_ds_lim == 0x0000FFFF);
	ASSERT(i_es_lim == 0x0000FFFF);
	ASSERT(i_fs_lim == 0x0000FFFF);
	ASSERT(i_gs_lim == 0x0000FFFF);

	ASSERT(i_cs_attr.raw == 0x000000F3);
	ASSERT(i_ss_attr.raw == 0x000000F3);
	ASSERT(i_ds_attr.raw == 0x000000F3);
	ASSERT(i_es_attr.raw == 0x000000F3);
	ASSERT(i_fs_attr.raw == 0x000000F3);
	ASSERT(i_gs_attr.raw == 0x000000F3);	 
    }
    else 
    {
	// CS.
	ASSERT ((i_cs_attr.type & (1<<0)) != 0);
	ASSERT ((i_cs_attr.type & (1<<3)) != 0);
	// SS:
	if (i_ss_attr.uu == 0)
	    ASSERT((i_ss_attr.type == 3) ||
		   (i_ss_attr.type == 7));
	// DS.
	if (i_ds_attr.uu == 0) {
	    ASSERT((i_ds_attr.type & (1<<0)) != 0);
	    if ((i_ds_attr.type & (1<<3)) == 1)
		ASSERT((i_ds_attr.type & (1<<1)) != 0);
	}
	// ES.
	if (i_es_attr.uu == 0) {
	    ASSERT((i_es_attr.type & (1<<0)) != 0);
	    if ((i_es_attr.type & (1<<3)) != 0)
		ASSERT((i_es_attr.type & (1<<1)) != 0);
	}

	// FS.
	if (i_fs_attr.uu == 0) {
	    ASSERT((i_fs_attr.type & (1<<0)) != 0);
	    if ((i_fs_attr.type & (1<<3)) != 0)
		ASSERT((i_fs_attr.type & (1<<1)) != 0);
	}
	// GS.
	if (i_gs_attr.uu == 0) {
	    ASSERT((i_gs_attr.type & (1<<0)) != 0);
	    if ((i_gs_attr.type & (1<<3)) != 0)
		ASSERT((i_gs_attr.type & (1<<1)) != 0);
	}

	// S bit.
	ASSERT (i_cs_attr.s == 1);
	if (i_ss_attr.uu == 0)
	    ASSERT (i_ss_attr.s == 1);
	if (i_ds_attr.uu == 0)
	    ASSERT (i_ds_attr.s == 1);
	if (i_es_attr.uu == 0)
	    ASSERT (i_es_attr.s == 1);
	if (i_fs_attr.uu == 0)
	    ASSERT (i_fs_attr.s == 1);
	if (i_gs_attr.uu == 0)
	    ASSERT (i_gs_attr.s == 1);

	// DPL for CS.
	if ((i_cs_attr.type >= 8) && (i_cs_attr.type <= 11))
	    ASSERT(i_cs_attr.dpl == i_cs_sel.rpl);
	if ((i_cs_attr.type >= 13) && (i_cs_attr.type <= 15))
	    ASSERT(i_cs_attr.dpl <= i_cs_sel.rpl);
	// DPL for SS
	ASSERT(i_ss_attr.dpl == i_ss_sel.rpl);
	// DPL for DS,ES,FS,GS
	if ((i_ds_attr.uu == 0) && (i_ds_attr.type  <= 11))
	    ASSERT(i_ds_attr.dpl >= i_ds_sel.rpl);
	if ((i_es_attr.uu == 0) && (i_es_attr.type  <= 11))
	    ASSERT(i_es_attr.dpl >= i_es_sel.rpl);
	if ((i_fs_attr.uu == 0) && (i_fs_attr.type  <= 11))
	    ASSERT(i_fs_attr.dpl >= i_fs_sel.rpl);
	if ((i_gs_attr.uu == 0) && (i_gs_attr.type  <= 11))
	    ASSERT(i_gs_attr.dpl >= i_gs_sel.rpl);

	// P.
	if (i_cs_attr.uu == 0)
	    ASSERT(i_cs_attr.p == 1);
	// 11:8 Reserved.
	ASSERT((i_cs_attr.raw & (BITS_11_08)) == 0);
	if (i_ss_attr.uu == 0)
	    ASSERT((i_ss_attr.raw & (BITS_11_08)) == 0);
	if (i_ds_attr.uu == 0)
	    ASSERT((i_ds_attr.raw & (BITS_11_08)) == 0);
	if (i_es_attr.uu == 0)
	    ASSERT((i_es_attr.raw & (BITS_11_08)) == 0);
	if (i_fs_attr.uu == 0)
	    ASSERT((i_fs_attr.raw & (BITS_11_08)) == 0);
	if (i_gs_attr.uu == 0)
	    //	    ASSERT((i_gs_attr.raw & (BITS_11_08)) == 0);
	    if(	(i_gs_attr.raw & (BITS_11_08)) != 0) {
		printf("GS raw: %x\n", i_gs_attr.raw);
		ASSERT(0);
	    }

	// 14 d/b.
	if (ia32e && (i_cs_attr.l == 1))
	    ASSERT (i_cs_attr.db == 0);

	// G.
	if (i_cs_lim & ((BITS_11_00) == 0)) {
	    ASSERT(i_cs_attr.g == 0);
	}
	if ((i_cs_lim & (BITS_31_20)) != 0) {
	    ASSERT(i_cs_attr.g == 1);
	}
	ASSERT((i_cs_attr.raw & BITS_31_17) == 0);
	// reserved, G, for SS
	if (i_ss_attr.uu == 0) {
	    if ((i_ss_lim & (BITS_11_00)) == 0)
		ASSERT(i_ss_attr.g == 0);
	    if ((i_ss_lim & (BITS_31_20)) != 0)
		ASSERT(i_ss_attr.g == 1);
	    ASSERT((i_ss_attr.raw & BITS_31_17) == 0);
	}
	// reserved, G, for DS
	if (i_ds_attr.uu == 0) {
	    if ((i_ds_lim & (BITS_11_00)) == 0)
		ASSERT(i_ds_attr.g == 0);
	    if ((i_ds_lim & (BITS_31_20)) != 0)
		ASSERT(i_ds_attr.g == 1);
	    ASSERT((i_ds_attr.raw & BITS_31_17) == 0);
	}
	// reserved, G, for ES
	if (i_es_attr.uu == 0) {
	    if ((i_es_lim & (BITS_11_00)) == 0)
		ASSERT(i_es_attr.g == 0);
	    if ((i_es_lim & (BITS_31_20)) != 0)
		ASSERT(i_es_attr.g == 1);
	    ASSERT((i_es_attr.raw & BITS_31_17) == 0);
	}
	// reserved, G, for FS
	if (i_fs_attr.uu == 0) {
	    if ((i_fs_lim & (BITS_11_00)) == 0)
		ASSERT(i_fs_attr.g == 0);
	    if ((i_fs_lim & (BITS_31_20)) != 0)
		ASSERT(i_fs_attr.g == 1);
	    ASSERT((i_fs_attr.raw & BITS_31_17) == 0);
	}
	// reserved, G, for GS
	if (i_gs_attr.uu == 0) {
	    if ((i_gs_lim & (BITS_11_00)) == 0)
		ASSERT(i_gs_attr.g == 0);
	    if ((i_gs_lim & (BITS_31_20)) != 0)
		ASSERT(i_gs_attr.g == 1);
	    ASSERT((i_gs_attr.raw & BITS_31_17) == 0);
	}
    } // Access Rights (!virt8086)

    // TR.
    if (!virt8086) 
    {
	ASSERT ((i_tr_attr.type == 3) ||
		(i_tr_attr.type == 11));
    }
    if (ia32e)
	ASSERT (i_tr_attr.type == 11);
    ASSERT (i_tr_attr.s == 0);
    ASSERT (i_tr_attr.p == 1);
    if ((i_tr_lim & (BITS_11_00)) == 0) {
	ASSERT (i_tr_attr.g == 0);
    }
    if ((i_tr_lim & (BITS_31_20)) != 0) {
	ASSERT (i_tr_attr.g == 1);
    }
    ASSERT (i_tr_attr.uu == 0);
    ASSERT ((i_tr_attr.raw & (BITS_31_17)) == 0);

    if (i_ldtr_attr.uu == 0) {
	ASSERT (i_ldtr_attr.type == 2);
	ASSERT (i_ldtr_attr.s == 0);
	ASSERT (i_ldtr_attr.p == 1);
	ASSERT ((i_ldtr_attr.raw & (BITS_11_08)) == 0);
	if ((i_ldtr_lim & (BITS_11_00)) != (BITS_11_00))
	    ASSERT (i_ldtr_attr.g == 0);
	if ((i_ldtr_lim & (BITS_31_20)) != 0)
	    ASSERT (i_ldtr_attr.g == 1);
	ASSERT ((i_ldtr_attr.raw & (BITS_31_17)) == 0);

    }

    // Descriptor-Table Registers (22.3.1.3)
    ASSERT((i_gdtr_limit & (BITS_31_16)) == 0);
    ASSERT((i_idtr_limit & (BITS_31_16)) == 0);

    // RIP		(22.3.1.4)
    if (i_entryctr.ia32e_mode == 0)
    {
	ASSERT((i_rip & BITS_63_32) == 0);
    }
    ASSERT(is_canonical_address(i_rip));

    // RFLAGS		(22.3.1.4)
    ASSERT((i_rfl & (BITS_63_22 | (1UL<<15) | (1UL<<5) | (1UL<<3))) == 0);
    ASSERT((i_rfl & (1<<1)) != 0);
    if (i_entryctr.ia32e_mode == 1)
	ASSERT ((i_rfl & X86_FLAGS_VM) == 0);
    if ((i_iif.valid == 1) &&
	(i_iif.type == vmcs_int_t::ext_int))
	ASSERT((i_rfl & (X86_FLAGS_IF)) != 0);

    // Activity State	(22.3.1.5)
    ASSERT(i_as.raw <= 3);
    if (i_as.state == vmcs_gs_as_t::hlt)
	ASSERT(i_ss_attr.dpl == 0);
    if ((i_ias.bl_movss == 1) || (i_ias.bl_sti == 1))
	ASSERT(i_as.state == vmcs_gs_as_t::active);
    if (i_iif.valid == 1)
    {
	if (i_as.state == vmcs_gs_as_t::hlt)
	    enter_kdebug("checks missing hlt");
	if (i_as.state == vmcs_gs_as_t::shutdown)
	    enter_kdebug("checks missing shutdown");
	if (i_as.state == vmcs_gs_as_t::wf_ipi)
	    enter_kdebug("checks missing wait-for-ipi");
    }

    // Interuptibility State.	(22.3.1.5)
    ASSERT((i_ias.raw & BITS_31_04) == 0);
    ASSERT( !((i_ias.bl_sti == 1) && (i_ias.bl_movss == 1)));
    if ((i_rfl & X86_FLAGS_IF) == 0)
	ASSERT(i_ias.bl_sti == 0);
    if ((i_iif.valid == 1) & (i_iif.type == vmcs_int_t::ext_int))
    {
	ASSERT(i_ias.bl_sti == 0);
	ASSERT(i_ias.bl_movss == 0);
    }
    if (i_entryctr.entry_smm == 1)
	ASSERT(i_ias.bl_smi == 1);
    else
	ASSERT(i_ias.bl_smi == 0);
    if ((i_iif.valid == 1) &&
	(i_iif.type == 2))
	ASSERT(i_ias.bl_sti == 0);

    // Pending debug exceptions.
    vmcs_gs_pend_dbg_except_t i_pend_dbg_except = pend_dbg_except;
    ASSERT((i_pend_dbg_except.raw & (BITS_11_04 | (1UL<<13) | BITS_63_15)) == 0);
    if ((i_ias.bl_sti == 1) ||
	(i_ias.bl_movss == 1) ||
	(i_as.state == vmcs_gs_as_t::hlt))
    {
	// To tight checks.
	u64_t debugctl = dbg_ctl;

	if (((i_rfl & X86_FLAGS_TF) != 0) && ((debugctl & 1) == 0))
	    ASSERT(i_pend_dbg_except.bs == 1);

	if (((i_rfl & X86_FLAGS_TF) == 0) || ((debugctl & 1) == 1))
	    ASSERT(i_pend_dbg_except.bs == 0);
    }

    // VMCS Link Pointer.		(22.3.1.5)
    ASSERT((u64_t)linkptr == (-1ULL));
}


void vmcs_hsarea_t::do_vmentry_checks (vmcs_entryctr_t i_entryctr, vmcs_exitctr_t i_exitctr)
{
    // CRs		(22.2.2)
    word_t i_cr0, i_cr4;
    i_cr0 = cr0;
    i_cr4 = cr4;

    ASSERT(x86_x32_vmx_t::check_fixed_bits_cr0 (i_cr0));
    ASSERT(x86_x32_vmx_t::check_fixed_bits_cr4 (i_cr4));

    if (i_exitctr.host_as_sz == 1)
	ASSERT((i_cr4 & X86_CR4_PAE) == 1);

    // Checks Related to Address Space Size.
    if (i_exitctr.host_as_sz == 0)
    {
	ASSERT(i_entryctr.ia32e_mode == 0);

	u64_t i_rip;
	i_rip = rip;
	ASSERT((i_rip & (BITS_63_32)) == 0);
    }
}

#endif /* !defined(CONFIG_DEBUG) */
