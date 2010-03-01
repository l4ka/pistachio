/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     arch/x86/x32/vmx_vmcs.h
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

#ifndef __ARCH__X86__VMX_H__
#define __ARCH__X86__VMX_H__

#include INC_ARCH_SA(cpu.h)
#include INC_GLUE(hwspace.h)

/**********************************************************************
 *
 *                  VMCS field template
 *
 **********************************************************************/
template<word_t index, typename T = word_t> class vmcs_field
{
public:
    // Assign T to vmcs_field ; T needs a member raw
    T operator= (T val)
    {
	// "this" stores the physical address of the VMCS.
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	x86_vmwrite (index, val.raw);
	return val;
    }

    // Assign vmcs_field to T ; T needs a member raw
    operator T()
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	T val;
	val.raw = x86_vmread (index);
	return val;
    }
};


// Specialized template for word_t ; word_t does not have a .raw member
template<word_t index> class vmcs_field<index, word_t> {
public:
    // Assign word_t to vmcs_field
    word_t operator= (word_t val)
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	x86_vmwrite (index, val);
	return val;
    }

    // Assign vmcs_field to word_t
    operator word_t ()
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	return x86_vmread (index);
    }
};


// Specialized template for u16_t ; u16_t does not have a .raw member
template<word_t index> class vmcs_field<index, u16_t> {
public:
    // Assign u16_t to vmcs_field
    u16_t operator= (u16_t val)
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	x86_vmwrite (index, val);
	return val;
    }

    // Assign vmcs_field to u16_t
    operator u16_t ()
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	return x86_vmread (index);
    }
};


// Specialized template for u64_t ; u64_t does not have a .raw member
template<word_t index> class vmcs_field<index, u64_t> {
public:
    // Assign u64_t to vmcs_field
    u64_t operator= (u64_t val)
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	x86_vmwrite (index, (u32_t) val);
	x86_vmwrite (index | 0x1, (u32_t) (val >> 32));
	return val;
    }

    // Assign vmcs_field to u64_t
    operator u64_t ()
    {
	ASSERT (x86_vmptrtest ((u64_t) (word_t) this));
	return x86_vmread (index)
	       | (((u64_t) x86_vmread (index | 0x1)) << 32);
    }
};



/**********************************************************************
 *
 *                  VMCS segattr
 *
 **********************************************************************/


class vmcs_segattr_t {
public:
    vmcs_segattr_t ()
	{ raw = 0; }

public:
    union {
	u32_t raw;
	struct {  
	    u32_t type :  4;
	    u32_t s    :  1;
	    u32_t dpl  :  2;
	    u32_t p    :  1;
	    u32_t mbz0 :  4;
	    u32_t avl  :  1;
	    u32_t l    :  1;
	    u32_t db   :  1;
	    u32_t g    :  1;
	    u32_t uu   :  1;
	    u32_t mbz1 : 15;
	} __attribute__((packed));
    };
};


class vmcs_int_t {
public:
    vmcs_int_t ()
	{ raw = 0; }

public:
    enum int_type_e {
	ext_int   = 0,
	hw_nmi    = 2,
	hw_except = 3,
	sw_int    = 4,
	sw_prvlg  = 5,
	sw_except = 6,
    };

public:
    union {
	u32_t raw;
	struct {
	    u32_t vector		:  8;
	    int_type_e type		:  3;   /* info_int_type_e */
	    bool err_code_valid		:  1;
	    bool nmi_unblock		:  1;
	    u32_t mbz0			: 18;
	    bool valid			:  1;
	} __attribute__((packed));
    };
};


/**********************************************************************
 *
 *                  VMX entry control
 *
 **********************************************************************/
/* 32-bit fields */
#define    VMCS_IDX_ENTRY_CTRL            0x4012
#define    VMCS_IDX_ENTRY_MSR_LD_COUNT    0x4014
#define    VMCS_IDX_ENTRY_IIF             0x4016
#define    VMCS_IDX_ENTRY_EXCEPT_ERR_CODE 0x4018
#define    VMCS_IDX_ENTRY_INSTR_LEN       0x401a

/* 64-bit fields */
#define    VMCS_IDX_ENTRY_MSR_LD_ADDR_F   0x200a
#define    VMCS_IDX_ENTRY_MSR_LD_ADDR_H   0x200b



class vmcs_entryctr_t {
public:
    vmcs_entryctr_t ()
	{ raw = (u32_t) x86_rdmsr (X86_MSR_VMX_ENTRY_CTLS); }

public:
    union {
	u32_t raw;
	struct {
	    s32_t res0          :  9;
	    u32_t ia32e_mode    :  1;
	    u32_t entry_smm     :  1;
	    u32_t deact_dmt     :  1;
	    u32_t res1          : 20;
	} __attribute__((packed));
    };
};


class vmcs_entryctr_eec_t {
public:
    vmcs_entryctr_eec_t ()
	{ raw = 0; }

public:
    union {
	u32_t raw;
	u32_t ecode;
    };
};

typedef vmcs_field<VMCS_IDX_ENTRY_CTRL, vmcs_entryctr_t>			vmcsf_entryctr_t;
typedef vmcs_field<VMCS_IDX_ENTRY_MSR_LD_COUNT, u32_t>				vmcsf_entryctr_msr_ld_cnt_t;
typedef vmcs_field<VMCS_IDX_ENTRY_IIF, vmcs_int_t>				vmcsf_entryctr_iif_t;
typedef vmcs_field<VMCS_IDX_ENTRY_INSTR_LEN, u32_t>				vmcsf_entryctr_instrlen_t;
typedef vmcs_field<VMCS_IDX_ENTRY_MSR_LD_ADDR_F, u64_t>				vmcsf_entryctr_msr_ld_addr_t;
typedef vmcs_field<VMCS_IDX_ENTRY_EXCEPT_ERR_CODE, u32_t>			vmcsf_entryctr_eec_t;


/**********************************************************************
 *
 *                  VMX execution control
 *
 **********************************************************************/

/* 32-bit fields */
#define    VMCS_IDX_PIN_BASED_VM_EXEC_CTRL   0x4000
#define    VMCS_IDX_CPU_BASED_VM_EXEC_CTRL   0x4002
#define    VMCS_IDX_EXCEPTION_BITMAP         0x4004
#define    VMCS_IDX_PF_ERR_CODE_MASK         0x4006
#define    VMCS_IDX_PF_ERR_CODE_MATCH        0x4008
#define    VMCS_IDX_CR3_TARGET_COUNT         0x400a
#define    VMCS_IDX_TPR_THRESHOLD            0x401c

/* 64-bit fields */
#define    VMCS_IDX_IO_BITMAP_A_F            0x2000
#define    VMCS_IDX_IO_BITMAP_A_H            0x2001
#define    VMCS_IDX_IO_BITMAP_B_F            0x2002
#define    VMCS_IDX_IO_BITMAP_B_H            0x2003
#define    VMCS_IDX_TSC_OFF_F                0x2010
#define    VMCS_IDX_TSC_OFF_H                0x2011
#define    VMCS_IDX_VAPIC_PAGE_ADDR_F        0x2012
#define    VMCS_IDX_VAPIC_PAGE_ADDR_H        0x2013

/* Word size fields */
#define    VMCS_IDX_CR0_MASK                 0x6000
#define    VMCS_IDX_CR4_MASK                 0x6002
#define    VMCS_IDX_CR0_SHADOW               0x6004
#define    VMCS_IDX_CR4_SHADOW               0x6006
#define    VMCS_IDX_CR3_VAL_0                0x6008
#define    VMCS_IDX_CR3_VAL_1                0x600a
#define    VMCS_IDX_CR3_VAL_2                0x600c
#define    VMCS_IDX_CR3_VAL_3                0x600e


class vmcs_exectr_pinbased_t {
public:
    vmcs_exectr_pinbased_t ()
	{
	    raw		= x86_rdmsr (X86_MSR_VMX_PINBASED_CTLS);
	    extint_exit	= 1;
	    nmi_exit	= 1;
	}

public:
    union {
	u32_t raw;
	struct {
	    s32_t extint_exit   :  1;
	    s32_t res1          :  2;
	    s32_t nmi_exit      :  1;
	    s32_t res2          : 28;
	} __attribute__((packed));
    };
};


class vmcs_exectr_cpubased_t {
public:
    vmcs_exectr_cpubased_t ()
	{
	    raw		= x86_rdmsr (X86_MSR_VMX_CPUBASED_CTLS);
	    hlt		= 1;
	    invlpg	= 1;
	    mwait	= 1;
	    rdpmc	= 1;
	    rdtsc	= 1;
	    movdr	= 1;
	    monitor	= 1;
	    pause	= 1;
	    io		= 1;
	}

public:
    union {
	u32_t raw;
	struct {
	    s32_t res0          : 2;
	    u32_t iw            : 1;
	    u32_t tscoff        : 1;
	    u32_t res1          : 3;
	    u32_t hlt           : 1;
	    s32_t res2          : 1;
	    u32_t invlpg        : 1;
	    u32_t mwait         : 1;
	    u32_t rdpmc         : 1;
	    u32_t rdtsc         : 1;
	    s32_t res3          : 6;
	    u32_t lcr8          : 1;
	    u32_t scr8          : 1;
	    u32_t tpr_shadow    : 1;
	    s32_t res4          : 1;
	    u32_t movdr         : 1;
	    u32_t io            : 1;
	    u32_t iobitm        : 1;
	    s32_t res5          : 2;
	    u32_t msrbitm	: 1;
	    u32_t monitor       : 1;
	    u32_t pause         : 1;
	    s32_t res6          : 1;
	} __attribute__((packed));
    };
};


class vmcs_exectr_excbmp_t {
public:
    vmcs_exectr_excbmp_t ()
	{ raw = -1; }

public:
    union {
	s32_t raw;
	struct {
	    u32_t de		: 1;
	    u32_t db		: 1;
	    u32_t nmi		: 1;
	    u32_t bp		: 1;
	    u32_t of		: 1;
	    u32_t br		: 1;
	    u32_t ud		: 1;
	    u32_t nm		: 1;
	    u32_t df		: 1;
	    u32_t copseg_overrun: 1;
	    u32_t ts		: 1;
	    u32_t np		: 1;
	    u32_t ss		: 1;
	    u32_t gp		: 1;
	    u32_t pf		: 1;
	    u32_t _reserved	: 1;
	    u32_t mf		: 1;
	    u32_t ac		: 1;
	    u32_t mc		: 1;
	    u32_t xf		: 1;
	    u32_t high		:12;
	} __attribute__((packed));
    };
};


class vmcs_exectr_tprth_t {
public:
    vmcs_exectr_tprth_t ()
	{ raw = 0; }

public:
    union {
	u32_t raw;
	struct {
	    u32_t th            :  4;
	    u32_t reserved      : 28;
	} __attribute__((packed));
    };
};

typedef vmcs_field<VMCS_IDX_PIN_BASED_VM_EXEC_CTRL, vmcs_exectr_pinbased_t>	vmcsf_exectr_pinbased_t;
typedef vmcs_field<VMCS_IDX_CPU_BASED_VM_EXEC_CTRL, vmcs_exectr_cpubased_t>	vmcsf_exectr_cpubased_t;
typedef vmcs_field<VMCS_IDX_EXCEPTION_BITMAP, vmcs_exectr_excbmp_t>		vmcsf_exectr_excbmp_t;
typedef vmcs_field<VMCS_IDX_CR3_TARGET_COUNT, u32_t>				vmcsf_exectr_cr3targetcnt_t;
typedef vmcs_field<VMCS_IDX_PF_ERR_CODE_MASK, u32_t>				vmcsf_exectr_pferrmask_t;
typedef vmcs_field<VMCS_IDX_PF_ERR_CODE_MATCH, u32_t>				vmcsf_exectr_pferrmatch_t;
typedef vmcs_field<VMCS_IDX_TPR_THRESHOLD, vmcs_exectr_tprth_t>			vmcsf_exectr_tptrh_t;

typedef vmcs_field<VMCS_IDX_IO_BITMAP_A_F, u64_t>	vmcsf_excetr_iobmpa_t;
typedef vmcs_field<VMCS_IDX_IO_BITMAP_B_F, u64_t>	vmcsf_excetr_iobmpb_t;
typedef vmcs_field<VMCS_IDX_TSC_OFF_F, u64_t>		vmcsf_excetr_tscoff_t;
typedef vmcs_field<VMCS_IDX_CR0_MASK, word_t>		vmcsf_excetr_cr0mask_t;
typedef vmcs_field<VMCS_IDX_CR4_MASK, word_t>		vmcsf_excetr_cr4mask_t;
typedef vmcs_field<VMCS_IDX_CR0_SHADOW, word_t>		vmcsf_excetr_cr0shadow_t;
typedef vmcs_field<VMCS_IDX_CR4_SHADOW, word_t>		vmcsf_excetr_cr4shadow_t;
typedef vmcs_field<VMCS_IDX_CR3_VAL_0, word_t>		vmcsf_excetr_cr3val0_t;
typedef vmcs_field<VMCS_IDX_CR3_VAL_1, word_t>		vmcsf_excetr_cr3val1_t;
typedef vmcs_field<VMCS_IDX_CR3_VAL_2, word_t>		vmcsf_excetr_cr3val2_t;
typedef vmcs_field<VMCS_IDX_CR3_VAL_3, word_t>		vmcsf_excetr_cr3val3_t;
typedef vmcs_field<VMCS_IDX_VAPIC_PAGE_ADDR_F, u64_t>	vmcsf_exectr_vapipageaddr_t;


/**********************************************************************
 *
 *                  VMX exit control
 *
 **********************************************************************/

/* 32-bit fields */
#define    VMCS_IDX_EXIT_CTRL             0x400c
#define    VMCS_IDX_EXIT_MSR_ST_COUNT     0x400e
#define    VMCS_IDX_EXIT_MSR_LD_COUNT     0x4010

/* 64-bit fields */
#define    VMCS_IDX_EXIT_MSR_ST_ADDR_F    0x2006
#define    VMCS_IDX_EXIT_MSR_ST_ADDR_H    0x2007
#define    VMCS_IDX_EXIT_MSR_LD_ADDR_F    0x2008
#define    VMCS_IDX_EXIT_MSR_LD_ADDR_H    0x2009


class vmcs_exitctr_t {
public:
    vmcs_exitctr_t ()
	{ raw = (u32_t) x86_rdmsr (X86_MSR_VMX_EXIT_CTLS); }

public:
    union {
	u32_t raw;
	struct {
	    s32_t res0          :  9;
	    u32_t host_as_sz    :  1;
	    s32_t res1          :  5;
	    u32_t ack_int       :  1;
	    u32_t res2          : 16;
	} __attribute__((packed));
    };
};

typedef vmcs_field<VMCS_IDX_EXIT_CTRL, vmcs_exitctr_t>	vmcsf_exitctr_t;
typedef vmcs_field<VMCS_IDX_EXIT_MSR_ST_COUNT, u32_t>	vmcsf_exitctr_msr_st_cnt_t;
typedef vmcs_field<VMCS_IDX_EXIT_MSR_LD_COUNT, u32_t>	vmcsf_exitctr_msr_ld_cnt_t;
typedef vmcs_field<VMCS_IDX_EXIT_MSR_ST_ADDR_F, u64_t>	vmcsf_exitctr_msr_st_addr_t;
typedef vmcs_field<VMCS_IDX_EXIT_MSR_LD_ADDR_F, u64_t>	vmcsf_exitctr_msr_ld_addr_t;

/**********************************************************************
 *
 *                  VMX exit info
 *
 **********************************************************************/

/* 32-bit fields */
#define    VMCS_IDX_EINFO_VM_INSTR_ERROR           0x4400
#define    VMCS_IDX_EINFO_EXIT_REASON		   0x4402
#define    VMCS_IDX_EINFO_INTERRUPTION_INFO        0x4404
#define    VMCS_IDX_EINFO_INTERRUPTION_ERROR_CODE  0x4406
#define    VMCS_IDX_EINFO_IDT_VECTORING_INFO       0x4408
#define    VMCS_IDX_EINFO_IDT_VECTORING_ERROR_CODE 0x440a
#define    VMCS_IDX_EINFO_INSTR_LEN                0x440c
#define    VMCS_IDX_EINFO_VM_INSTR_INFO            0x440e

/* Word size fields */
#define    VMCS_IDX_EINFO_EXIT_QUAL          0x6400
#define    VMCS_IDX_EINFO_GUEST_LIN_ADDR     0x640A


class vmcs_ei_reason_t {
public:
    vmcs_ei_reason_t ()
	{ raw = 0; }

public:
    enum basic_reason_e {
	be_exp_nmi      =  0,
	be_ext_int      =  1,
	be_tf           =  2,
	be_init         =  3,
	be_sipi         =  4,
	be_smi          =  6,
	be_iw           =  7,
	be_tasksw       =  9,
	be_cpuid        = 10,
	be_hlt          = 12,
	be_invd         = 13,
	be_invlpg       = 14,
	be_rdpmc        = 15,
	be_rdtsc        = 16,
	be_rsm          = 17,
	be_vmcall       = 18,
	be_vmclear      = 19,
	be_vmlaunch     = 20,
	be_vmptrld      = 21,
	be_vmptrst      = 22,
	be_vmread       = 23,
	be_vmresume     = 24,
	be_vmwrite      = 25,
	be_vmxoff       = 26,
	be_vmxon        = 27,
	be_cr           = 28,
	be_dr           = 29,
	be_io           = 30,
	be_rdmsr        = 31,
	be_wrmsr        = 32,
	be_entry_invg   = 33,
	be_entry_msrld  = 34,
	be_mwait        = 36,
	be_monitor      = 39,
	be_pause        = 40,
	be_entry_mce    = 41,
	be_entry_tprtsh = 43,
	be_max
    };
    enum vm_instr_err_e {
	ie_vmcall       =  1,
	ie_vmclear1     =  2,
	ie_vmclear2     =  3,
	ie_vmlaunch     =  4,
	ie_vmresume1    =  5,
	ie_vmresume2    =  6,
	ie_vmentry1     =  7,
	ie_vmentry2     =  8,
	ie_vmptrld1     =  9,
	ie_vmptrld2     = 10,
	ie_vmptrld3     = 11,
	ie_vmrdwr       = 12,
	ie_vmwr         = 13,
	ie_vmentry3     = 26,
    };

public:
    union {
	u32_t raw;
	struct {
	    basic_reason_e basic_reason : 16;
	    u32_t mbz0                  : 13;
	    bool  exit_vmx_root_op      :  1;
	    u32_t mbz1                  :  1;
	    bool is_entry_fail          :  1;
	} __attribute__((packed));
    };
};


class vmcs_ei_vm_instr_t {
public:
    vmcs_ei_vm_instr_t ()
	{ raw = 0; }

public:
    enum gpr_e {
	rax= 0, rcx = 1, rdx = 2, rbx = 3, rsp = 4, rbp = 5, rsi = 6, rdi = 7,
	r8 = 8, r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14,
	r15 = 15,
    };

    enum scaling_e {
	scale0 = 0, scale2 = 1, scale4 = 2, scale8 = 3,
    };

    enum addr_sz_e {
	bit16 = 0, bit32 = 1, bit64 = 2,
    };

    enum mem_reg_e {
	mem = 0, reg = 1
    };

    enum seg_reg_e {
	es = 0, cs = 1, ss = 2, ds = 3, fs = 4, gs = 5,
    };

public:
    union {
	u32_t raw;
	struct {
	    scaling_e	scaling	      :  2;   /* scaling_e */
	    u32_t	mbz0          :  1;
	    gpr_e	reg1          :  4;   /* gpr_e */
	    addr_sz_e	ad66dr_sz     :  3;   /* addr_sz_e */
	    mem_reg_e	mem_reg       :  1;   /* mem_reg_e */
	    u32_t	mbz1	      :  4;
	    seg_reg_e	seg_reg       :  3;   /* seg_reg_e */
	    gpr_e	idx_reg       :  4;   /* gpr_e */
	    bool	idx_reg_inv   :  1;
	    gpr_e	base_reg      :  4;   /* gpr_e */
	    bool	base_reg_inv  :  1;
	    gpr_e	reg2          :  4;   /* gpr_e */
	};
    };
};


class vmcs_ei_qual_t {
public:
    vmcs_ei_qual_t ()
	{ raw = 0; }

public:
    enum source_of_tss_e {
	call            = 0,
	iret            = 1,
	jmp             = 2,
	task_gate       = 3,
    };

    enum mem_reg_e {
	mem = 0, reg = 1
    };

    enum gpr_e {
	rax= 0, rcx = 1, rdx = 2, rbx = 3, rsp = 4, rbp = 5, rsi = 6, rdi = 7,
	r8 = 8, r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14,
	r15 = 15,
    };

    enum soa_e {
	u8 = 0, u16 = 1, u32 = 3,
    };

    enum access_type_e {
	to_cr = 0, from_cr = 1, clts = 2, lmsw = 3
    };

    enum dir_e {
	out = 0, in = 1,
    };

    enum op_encoding_e {
	dx = 0, immediate = 1,
    };

public:
    union {
	word_t raw;

	// Page Fault Address.
	word_t faddr;

	struct {
	    u32_t bx            :  4;
	    u32_t mbz0          :  9;
	    u32_t bd            :  1;
	    u32_t bs            :  1;
	    u32_t res           : 17;
	} __attribute__((packed)) dbg;
	struct {
	    u32_t		selector      : 16;
	    u32_t		mbz0          : 14;
	    source_of_tss_e	source        :  2;   /* source_of_tss_e */
	} __attribute__((packed)) task_sw;
	struct {
	    u32_t		cr_num        :  4;
	    access_type_e	access_type   :  2;
	    mem_reg_e		lmsw_op_type  :  1;   /* mem_reg_e */
	    u32_t		mbz0          :  1;
	    gpr_e		mov_cr_gpr    :  4;   /* gpr_e */
	    u32_t		mbz1          :  4;
	    u32_t		lmsw_src_data : 16;
	} __attribute__((packed)) mov_cr;
	struct {
	    u32_t		dr_num        :  3;
	    u32_t		mbz0          :  1;
	    u32_t		dir           :  1;
	    u32_t		mbz1          :  3;
	    gpr_e		mov_dr_gpr    :  4;   /* gpr_e */
	    u32_t		mbz2	      : 20;
	} __attribute__((packed)) mov_dr;
	struct {
	    soa_e		soa	      :  3;   /* soa_e */
	    dir_e		dir           :  1;   /* dir_e */
	    bool		string        :  1;
	    bool		rep           :  1;
	    op_encoding_e	op_encoding   :  1;   /* op_encoding_e */
	    u32_t		mbz0          :  9;
	    u32_t		port_num      : 16;
	} __attribute__((packed)) io;
    };
};


typedef vmcs_field<VMCS_IDX_EINFO_EXIT_REASON, vmcs_ei_reason_t>	vmcsf_ei_reason_t;
typedef vmcs_field<VMCS_IDX_EINFO_INSTR_LEN, u32_t>			vmcsf_ei_instrlen_t;
typedef vmcs_field<VMCS_IDX_EINFO_EXIT_QUAL, vmcs_ei_qual_t>		vmcsf_ei_qual_t;
typedef vmcs_field<VMCS_IDX_EINFO_INTERRUPTION_INFO, vmcs_int_t>	vmcsf_ei_int_t;
typedef vmcs_field<VMCS_IDX_EINFO_INTERRUPTION_ERROR_CODE, u32_t>	vmcsf_ei_intec_t;
typedef vmcs_field<VMCS_IDX_EINFO_IDT_VECTORING_INFO, vmcs_int_t>	vmcsf_ei_idt_t;
typedef vmcs_field<VMCS_IDX_EINFO_IDT_VECTORING_ERROR_CODE, u32_t>	vmcsf_ei_idtec_t;
typedef vmcs_field<VMCS_IDX_EINFO_VM_INSTR_INFO, vmcs_ei_vm_instr_t>	vmcsf_ei_vminstr_t;
typedef vmcs_field<VMCS_IDX_EINFO_VM_INSTR_ERROR, u32_t>		vmcsf_ei_vminstrerr_t;

typedef vmcs_field<VMCS_IDX_EINFO_GUEST_LIN_ADDR, word_t>		vmcsf_ei_gla_t;



/**********************************************************************
 *
 *                  VMX guest state
 *
 **********************************************************************/

/* 16-bit fields */
#define    VMCS_IDX_G_ES_SEL           0x0800
#define    VMCS_IDX_G_CS_SEL           0x0802
#define    VMCS_IDX_G_SS_SEL           0x0804
#define    VMCS_IDX_G_DS_SEL           0x0806
#define    VMCS_IDX_G_FS_SEL           0x0808
#define    VMCS_IDX_G_GS_SEL           0x080a
#define    VMCS_IDX_G_LDTR_SEL         0x080c
#define    VMCS_IDX_G_TR_SEL           0x080e

/* 32-bit fields */
#define    VMCS_IDX_G_ES_LIMIT         0x4800
#define    VMCS_IDX_G_CS_LIMIT         0x4802
#define    VMCS_IDX_G_SS_LIMIT         0x4804
#define    VMCS_IDX_G_DS_LIMIT         0x4806
#define    VMCS_IDX_G_FS_LIMIT         0x4808
#define    VMCS_IDX_G_GS_LIMIT         0x480a
#define    VMCS_IDX_G_LDTR_LIMIT       0x480c
#define    VMCS_IDX_G_TR_LIMIT         0x480e
#define    VMCS_IDX_G_GDTR_LIMIT       0x4810
#define    VMCS_IDX_G_IDTR_LIMIT       0x4812
#define    VMCS_IDX_G_ES_ATTR          0x4814
#define    VMCS_IDX_G_CS_ATTR          0x4816
#define    VMCS_IDX_G_SS_ATTR          0x4818
#define    VMCS_IDX_G_DS_ATTR          0x481a
#define    VMCS_IDX_G_FS_ATTR          0x481c
#define    VMCS_IDX_G_GS_ATTR          0x481e
#define    VMCS_IDX_G_LDTR_ATTR        0x4820
#define    VMCS_IDX_G_TR_ATTR          0x4822
#define    VMCS_IDX_G_INTR_ABILITY_STATE  0x4824
#define    VMCS_IDX_G_ACTIVITY_STATE   0x4826
#define    VMCS_IDX_G_SYSENTER_CS      0x482a

/* 64-bit fields */
#define    VMCS_IDX_G_VMCS_LINK_PTR_F  0x2800
#define    VMCS_IDX_G_VMCS_LINK_PTR_H  0x2801
#define    VMCS_IDX_G_DEBUGCTL_F       0x2802
#define    VMCS_IDX_G_DEBUGCTL_H       0x2803

/* Word size fields */
#define    VMCS_IDX_G_CR0              0x6800
#define    VMCS_IDX_G_CR3              0x6802
#define    VMCS_IDX_G_CR4              0x6804
#define    VMCS_IDX_G_ES_BASE          0x6806
#define    VMCS_IDX_G_CS_BASE          0x6808
#define    VMCS_IDX_G_SS_BASE          0x680a
#define    VMCS_IDX_G_DS_BASE          0x680c
#define    VMCS_IDX_G_FS_BASE          0x680e
#define    VMCS_IDX_G_GS_BASE          0x6810
#define    VMCS_IDX_G_LDTR_BASE        0x6812
#define    VMCS_IDX_G_TR_BASE          0x6814
#define    VMCS_IDX_G_GDTR_BASE        0x6816
#define    VMCS_IDX_G_IDTR_BASE        0x6818
#define    VMCS_IDX_G_DR7              0x681a
#define    VMCS_IDX_G_RSP              0x681c
#define    VMCS_IDX_G_RIP              0x681e
#define    VMCS_IDX_G_RFLAGS           0x6820
#define    VMCS_IDX_G_PEND_DBG_EXCEPT  0x6822
#define    VMCS_IDX_G_SYSENTER_ESP     0x6824
#define    VMCS_IDX_G_SYSENTER_EIP     0x6826

class vmcs_gs_ias_t {
public:
    vmcs_gs_ias_t ()
	{ raw = 0; }

public:
    union {
	u32_t raw;
	struct {
	    u32_t bl_sti        :  1;
	    u32_t bl_movss      :  1;
	    u32_t bl_smi        :  1;
	    u32_t bl_nmi        :  1;
	    u32_t mbz0          : 28;
	} __attribute__((packed));
    } ;
};


class vmcs_gs_as_t {
public:
    vmcs_gs_as_t ()
	{ raw = 0; }

public:
    enum vmcs_gs_as_e {
	active = 0UL, hlt = 1UL, shutdown = 2UL, wf_ipi = 3UL,
    };

public:
    union {
	u32_t raw;
	vmcs_gs_as_e state;
    };
};


class vmcs_gs_pend_dbg_except_t {
public:
    vmcs_gs_pend_dbg_except_t ()
	{ raw = 0; }

public:
    union {
	u64_t raw;
	struct {
	    u64_t bx    :  4;
	    u64_t mbz0  :  8;
	    u64_t br    :  1;
	    u64_t mbz1  :  1;
	    u64_t bs    :  1;
	    u64_t mbz2  : 49;
	} __attribute__((packed));
    };
};


typedef vmcs_field<VMCS_IDX_G_ES_SEL, u16_t>		vmcsf_g_essel_t;
typedef vmcs_field<VMCS_IDX_G_CS_SEL, u16_t>		vmcsf_g_cssel_t;
typedef vmcs_field<VMCS_IDX_G_SS_SEL, u16_t>		vmcsf_g_sssel_t;
typedef vmcs_field<VMCS_IDX_G_DS_SEL, u16_t>		vmcsf_g_dssel_t;
typedef vmcs_field<VMCS_IDX_G_FS_SEL, u16_t>		vmcsf_g_fssel_t;
typedef vmcs_field<VMCS_IDX_G_GS_SEL, u16_t>		vmcsf_g_gssel_t;
typedef vmcs_field<VMCS_IDX_G_TR_SEL, u16_t>		vmcsf_g_trsel_t;
typedef vmcs_field<VMCS_IDX_G_LDTR_SEL, u16_t>		vmcsf_g_ldtrsel_t;

typedef vmcs_field<VMCS_IDX_G_ES_LIMIT, u32_t>		vmcsf_g_eslim_t;
typedef vmcs_field<VMCS_IDX_G_CS_LIMIT, u32_t>		vmcsf_g_cslim_t;
typedef vmcs_field<VMCS_IDX_G_SS_LIMIT, u32_t>		vmcsf_g_sslim_t;
typedef vmcs_field<VMCS_IDX_G_DS_LIMIT, u32_t>		vmcsf_g_dslim_t;
typedef vmcs_field<VMCS_IDX_G_FS_LIMIT, u32_t>		vmcsf_g_fslim_t;
typedef vmcs_field<VMCS_IDX_G_GS_LIMIT, u32_t>		vmcsf_g_gslim_t;
typedef vmcs_field<VMCS_IDX_G_TR_LIMIT, u32_t>		vmcsf_g_trlim_t;
typedef vmcs_field<VMCS_IDX_G_LDTR_LIMIT, u32_t>	vmcsf_g_ldtrlim_t;
typedef vmcs_field<VMCS_IDX_G_GDTR_LIMIT, u32_t>	vmcsf_g_gdtrlim_t;
typedef vmcs_field<VMCS_IDX_G_IDTR_LIMIT, u32_t>	vmcsf_g_idtrlim_t;

typedef vmcs_field<VMCS_IDX_G_ES_ATTR, vmcs_segattr_t>	vmcsf_g_esattr_t;
typedef vmcs_field<VMCS_IDX_G_CS_ATTR, vmcs_segattr_t>	vmcsf_g_csattr_t;
typedef vmcs_field<VMCS_IDX_G_SS_ATTR, vmcs_segattr_t>	vmcsf_g_ssattr_t;
typedef vmcs_field<VMCS_IDX_G_DS_ATTR, vmcs_segattr_t>	vmcsf_g_dsattr_t;
typedef vmcs_field<VMCS_IDX_G_FS_ATTR, vmcs_segattr_t>	vmcsf_g_fsattr_t;
typedef vmcs_field<VMCS_IDX_G_GS_ATTR, vmcs_segattr_t>	vmcsf_g_gsattr_t;
typedef vmcs_field<VMCS_IDX_G_TR_ATTR, vmcs_segattr_t>	vmcsf_g_trattr_t;
typedef vmcs_field<VMCS_IDX_G_LDTR_ATTR, vmcs_segattr_t> vmcsf_g_ldtrattr_t;

typedef vmcs_field<VMCS_IDX_G_ACTIVITY_STATE, vmcs_gs_as_t> vmcsf_g_as_t;
typedef vmcs_field<VMCS_IDX_G_INTR_ABILITY_STATE, vmcs_gs_ias_t> vmcsf_g_ias_t;

typedef vmcs_field<VMCS_IDX_G_CR0, word_t>		vmcsf_g_cr0_t;
typedef vmcs_field<VMCS_IDX_G_CR3, word_t>		vmcsf_g_cr3_t;
typedef vmcs_field<VMCS_IDX_G_CR4, word_t>		vmcsf_g_cr4_t;
typedef vmcs_field<VMCS_IDX_G_DR7, word_t>		vmcsf_g_dr7_t;
typedef vmcs_field<VMCS_IDX_G_RSP, word_t>		vmcsf_g_rsp_t;
typedef vmcs_field<VMCS_IDX_G_RIP, word_t>		vmcsf_g_rip_t;
typedef vmcs_field<VMCS_IDX_G_RFLAGS, word_t>		vmcsf_g_rflags_t;
typedef vmcs_field<VMCS_IDX_G_ES_BASE, word_t>		vmcsf_g_es_base_t;
typedef vmcs_field<VMCS_IDX_G_CS_BASE, word_t>		vmcsf_g_cs_base_t;
typedef vmcs_field<VMCS_IDX_G_SS_BASE, word_t>		vmcsf_g_ss_base_t;
typedef vmcs_field<VMCS_IDX_G_DS_BASE, word_t>		vmcsf_g_ds_base_t;
typedef vmcs_field<VMCS_IDX_G_FS_BASE, word_t>		vmcsf_g_fs_base_t;
typedef vmcs_field<VMCS_IDX_G_GS_BASE, word_t>		vmcsf_g_gs_base_t;
typedef vmcs_field<VMCS_IDX_G_TR_BASE, word_t>		vmcsf_g_tr_base_t;
typedef vmcs_field<VMCS_IDX_G_LDTR_BASE, word_t>	vmcsf_g_ldtr_base_t;
typedef vmcs_field<VMCS_IDX_G_GDTR_BASE, word_t>	vmcsf_g_gdtr_base_t;
typedef vmcs_field<VMCS_IDX_G_IDTR_BASE, word_t>	vmcsf_g_idtr_base_t;

typedef vmcs_field<VMCS_IDX_G_SYSENTER_CS, u32_t>	vmcsf_g_sysenter_cs_t;
typedef vmcs_field<VMCS_IDX_G_SYSENTER_ESP, word_t>	vmcsf_g_sysenter_esp_t;
typedef vmcs_field<VMCS_IDX_G_SYSENTER_EIP, word_t>	vmcsf_g_sysenter_eip_t;

typedef vmcs_field<VMCS_IDX_G_DEBUGCTL_F, u64_t>	vmcsf_g_dbgctl_t;
typedef vmcs_field<VMCS_IDX_G_VMCS_LINK_PTR_F, u64_t>	vmcsf_g_linkptr_t;
typedef vmcs_field<VMCS_IDX_G_PEND_DBG_EXCEPT, vmcs_gs_pend_dbg_except_t> vmcsf_g_pend_dbg_except_t;

/**********************************************************************
 *
 *                  VMX host state
 *
 **********************************************************************/

/* 16-bit fields */
#define    VMCS_IDX_H_ES_SEL           0x0c00
#define    VMCS_IDX_H_CS_SEL           0x0c02
#define    VMCS_IDX_H_SS_SEL           0x0c04
#define    VMCS_IDX_H_DS_SEL           0x0c06
#define    VMCS_IDX_H_FS_SEL           0x0c08
#define    VMCS_IDX_H_GS_SEL           0x0c0a
#define    VMCS_IDX_H_TR_SEL           0x0c0c

/* 32-bit fields */
#define    VMCS_IDX_H_SYSENTER_CS      0x4c00

/* Word size fields */
#define    VMCS_IDX_H_CR0              0x6c00
#define    VMCS_IDX_H_CR3              0x6c02
#define    VMCS_IDX_H_CR4              0x6c04
#define    VMCS_IDX_H_FS_BASE          0x6c06
#define    VMCS_IDX_H_GS_BASE          0x6c08
#define    VMCS_IDX_H_TR_BASE          0x6c0a
#define    VMCS_IDX_H_GDTR_BASE        0x6c0c
#define    VMCS_IDX_H_IDTR_BASE        0x6c0e
#define    VMCS_IDX_H_SYSENTER_ESP     0x6c10
#define    VMCS_IDX_H_SYSENTER_EIP     0x6c12
#define    VMCS_IDX_H_RSP              0x6c14
#define    VMCS_IDX_H_RIP              0x6c16



typedef vmcs_field<VMCS_IDX_H_ES_SEL, u16_t>		vmcsf_h_essel_t;
typedef vmcs_field<VMCS_IDX_H_CS_SEL, u16_t>		vmcsf_h_cssel_t;
typedef vmcs_field<VMCS_IDX_H_SS_SEL, u16_t>		vmcsf_h_sssel_t;
typedef vmcs_field<VMCS_IDX_H_DS_SEL, u16_t>		vmcsf_h_dssel_t;
typedef vmcs_field<VMCS_IDX_H_FS_SEL, u16_t>		vmcsf_h_fssel_t;
typedef vmcs_field<VMCS_IDX_H_GS_SEL, u16_t>		vmcsf_h_gssel_t;
typedef vmcs_field<VMCS_IDX_H_TR_SEL, u16_t>		vmcsf_h_trsel_t;

typedef vmcs_field<VMCS_IDX_H_CR0, word_t>		vmcsf_h_cr0_t;
typedef vmcs_field<VMCS_IDX_H_CR3, word_t>		vmcsf_h_cr3_t;
typedef vmcs_field<VMCS_IDX_H_CR4, word_t>		vmcsf_h_cr4_t;
typedef vmcs_field<VMCS_IDX_H_RSP, word_t>		vmcsf_h_rsp_t;
typedef vmcs_field<VMCS_IDX_H_RIP, word_t>		vmcsf_h_rip_t;
typedef vmcs_field<VMCS_IDX_H_FS_BASE, word_t>		vmcsf_h_fs_base_t;
typedef vmcs_field<VMCS_IDX_H_GS_BASE, word_t>		vmcsf_h_gs_base_t;
typedef vmcs_field<VMCS_IDX_H_TR_BASE, word_t>		vmcsf_h_tr_base_t;
typedef vmcs_field<VMCS_IDX_H_GDTR_BASE, word_t>	vmcsf_h_gdtr_base_t;
typedef vmcs_field<VMCS_IDX_H_IDTR_BASE, word_t>	vmcsf_h_idtr_base_t;

typedef vmcs_field<VMCS_IDX_H_SYSENTER_CS, u32_t>	vmcsf_h_sysenter_cs_t;
typedef vmcs_field<VMCS_IDX_H_SYSENTER_ESP, word_t>	vmcsf_h_sysenter_esp_t;
typedef vmcs_field<VMCS_IDX_H_SYSENTER_EIP, word_t>	vmcsf_h_sysenter_eip_t;



/*******************************************************
 *
 * Guest State Area
 *
 *******************************************************/
class vmcs_gsarea_t {
public:
    void init ();
#if defined(CONFIG_DEBUG)
    void do_vmentry_checks (vmcs_entryctr_t i_entryctr, vmcs_int_t i_iif);
#endif

public:
    /*
     * These variables represent the VMCS fields.
     * Read/Write changes the field in the VMCS.
     * Perhaps slow (access needs about 130 cycles).
     * They may be accessed only if the VMCS is active.
     * The union ensures their "this" pointer is the
     * same as the pointer to the this object (and
     * therefore, the entire VMCS).
     */
    union {
	vmcsf_g_essel_t			es_sel;
	vmcsf_g_cssel_t			cs_sel;
	vmcsf_g_sssel_t			ss_sel;
	vmcsf_g_dssel_t			ds_sel;
	vmcsf_g_fssel_t			fs_sel;
	vmcsf_g_gssel_t			gs_sel;
	vmcsf_g_ldtrsel_t		ldtr_sel;
	vmcsf_g_trsel_t			tr_sel;

	vmcsf_g_es_base_t		es_base;
	vmcsf_g_cs_base_t		cs_base;
	vmcsf_g_ss_base_t		ss_base;
	vmcsf_g_ds_base_t		ds_base;
	vmcsf_g_fs_base_t		fs_base;
	vmcsf_g_gs_base_t		gs_base;
	vmcsf_g_ldtr_base_t		ldtr_base;
	vmcsf_g_tr_base_t		tr_base;
	vmcsf_g_gdtr_base_t		gdtr_base;
	vmcsf_g_idtr_base_t		idtr_base;

	vmcsf_g_eslim_t			es_lim;
	vmcsf_g_cslim_t			cs_lim;
	vmcsf_g_sslim_t			ss_lim;
	vmcsf_g_dslim_t			ds_lim;
	vmcsf_g_fslim_t			fs_lim;
	vmcsf_g_gslim_t			gs_lim;
	vmcsf_g_ldtrlim_t		ldtr_lim;
	vmcsf_g_trlim_t			tr_lim;
	vmcsf_g_gdtrlim_t		gdtr_lim;
	vmcsf_g_idtrlim_t		idtr_lim;

	vmcsf_g_esattr_t		es_attr;
	vmcsf_g_csattr_t		cs_attr;
	vmcsf_g_ssattr_t		ss_attr;
	vmcsf_g_dsattr_t		ds_attr;
	vmcsf_g_fsattr_t		fs_attr;
	vmcsf_g_gsattr_t		gs_attr;
	vmcsf_g_ldtrattr_t		ldtr_attr;
	vmcsf_g_trattr_t		tr_attr;

	vmcsf_g_ias_t			ias;
	vmcsf_g_as_t			as;

	vmcsf_g_linkptr_t		linkptr;
	vmcsf_g_dbgctl_t		dbg_ctl;
	vmcsf_g_cr0_t			cr0;
	vmcsf_g_cr3_t			cr3;
	vmcsf_g_cr4_t			cr4;
	vmcsf_g_dr7_t			dr7;
	vmcsf_g_rsp_t			rsp;
	vmcsf_g_rip_t			rip;
	vmcsf_g_rflags_t		rflags;
	vmcsf_g_pend_dbg_except_t	pend_dbg_except;

	vmcsf_g_sysenter_cs_t		sysenter_cs;
	vmcsf_g_sysenter_esp_t		sysenter_esp;
	vmcsf_g_sysenter_eip_t		sysenter_eip;
    };
};



/*******************************************************
 *
 * Host State Area
 *
 *******************************************************/
class vmcs_hsarea_t {
public:
#if defined(CONFIG_DEBUG)
    void do_vmentry_checks (vmcs_entryctr_t i_entryctr, vmcs_exitctr_t i_exitctr);
#endif

public:
    union {
	vmcsf_h_essel_t		es_sel;
	vmcsf_h_cssel_t		cs_sel;
	vmcsf_h_sssel_t		ss_sel;
	vmcsf_h_dssel_t		ds_sel;
	vmcsf_h_fssel_t		fs_sel;
	vmcsf_h_gssel_t		gs_sel;
	vmcsf_h_trsel_t		tr_sel;

	vmcsf_h_fs_base_t	fs_base;
	vmcsf_h_gs_base_t	gs_base;
	vmcsf_h_tr_base_t	tr_base;
	vmcsf_h_gdtr_base_t	gdtr_base;
	vmcsf_h_idtr_base_t	idtr_base;

	vmcsf_h_sysenter_cs_t	sysenter_cs;
	vmcsf_h_sysenter_esp_t	sysenter_esp;
	vmcsf_h_sysenter_eip_t	sysenter_eip;

	vmcsf_h_rsp_t		rsp;
	vmcsf_h_rip_t		rip;

	vmcsf_h_cr0_t		cr0;
	vmcsf_h_cr3_t		cr3;
	vmcsf_h_cr4_t		cr4;
    };
};


/*******************************************************
 *
 * VM Exec Control Area
 *
 *******************************************************/
class vmcs_exectrarea_t {
public:
    void init ();
#if defined(CONFIG_DEBUG)
    void do_vmentry_checks ();
#endif

public:
    union {
	vmcsf_exectr_pinbased_t		pinbased;
	vmcsf_exectr_cpubased_t		cpubased;

	vmcsf_exectr_excbmp_t		except_bmp;
	vmcsf_exectr_pferrmask_t	pferrmask;
	vmcsf_exectr_pferrmatch_t	pferrmatch;

	vmcsf_exectr_tptrh_t		tprthr;

	vmcsf_excetr_iobmpa_t		iobmpa;
	vmcsf_excetr_iobmpb_t		iobmpb;

	vmcsf_excetr_tscoff_t		tscoff;
	vmcsf_exectr_vapipageaddr_t	vapicaddr;

	vmcsf_excetr_cr0mask_t		cr0mask;
	vmcsf_excetr_cr4mask_t		cr4mask;

	vmcsf_excetr_cr0shadow_t	cr0shadow;
	vmcsf_excetr_cr4shadow_t	cr4shadow;

	vmcsf_exectr_cr3targetcnt_t	cr3targetcnt;
	vmcsf_excetr_cr3val0_t		cr3val0;
	vmcsf_excetr_cr3val1_t		cr3val1;
	vmcsf_excetr_cr3val2_t		cr3val2;
	vmcsf_excetr_cr3val3_t		cr3val3;
    };
};


/*******************************************************
 *
 * VM Exit Control Area
 *
 *******************************************************/
class vmcs_exitctrarea_t {
public:
    void init ();
#if defined(CONFIG_DEBUG)
    void do_vmentry_checks ();
#endif

public:
    union {
	vmcsf_exitctr_t			exitctr;

	vmcsf_exitctr_msr_st_cnt_t	msr_st_cnt;
	vmcsf_exitctr_msr_ld_cnt_t	msr_ld_cnt;

	vmcsf_exitctr_msr_st_addr_t	msr_st_addr;
	vmcsf_exitctr_msr_ld_addr_t	msr_ld_addr;
    };
};


/*******************************************************
 *
 * VM Entry Control
 *
 *******************************************************/
class vmcs_entryctrarea_t {
public:
    void init ();
#if defined(CONFIG_DEBUG)
    void do_vmentry_checks ();
#endif

public:
    union {
	vmcsf_entryctr_t		entryctr;

	vmcsf_entryctr_iif_t		iif;
	vmcsf_entryctr_eec_t		eec;
	vmcsf_entryctr_instrlen_t	instr_len;

	vmcsf_entryctr_msr_ld_cnt_t	msr_ld_cnt;
	vmcsf_entryctr_msr_ld_addr_t	msr_ld_addr;
    };
};


/*******************************************************
 *
 * VM Exit Information
 *
 *******************************************************/
class vmcs_exitinfoarea_t {
public:
    union {
	vmcsf_ei_reason_t	reason;
	vmcsf_ei_qual_t		qual;

	vmcsf_ei_int_t		int_info;
	vmcsf_ei_intec_t	int_ec;
	vmcsf_ei_idt_t		idtvect_info;
	vmcsf_ei_idtec_t	idtvect_ec;

	vmcsf_ei_instrlen_t	instr_len;

	vmcsf_ei_vminstr_t	vm_instr_info;
	vmcsf_ei_vminstrerr_t	vm_instr_err;

	vmcsf_ei_gla_t		linear_addr;
    };
};



/*******************************************************
 *
 * VMCS
 *
 *******************************************************/
class vmcs_t
{
public:
    // Get Object.
    static vmcs_t *alloc_vmcs ();
    static void free_vmcs (vmcs_t *vmcs);

    // Write initial state into the VMCS.
    void init ();

    // Load the VMCS pointer into the current physical processor.
    bool is_loaded () const;
    bool load ();
    bool unload ();

    word_t read_register (word_t idx);
    bool write_register (word_t idx, word_t value);

#if defined(CONFIG_DEBUG)
    void do_vmentry_checks ();
#endif

public:
    union {
	vmcs_gsarea_t gs;
	vmcs_hsarea_t hs;
	vmcs_exectrarea_t exec_ctr;
	vmcs_exitctrarea_t exit_ctr;
	vmcs_entryctrarea_t entry_ctr;
	vmcs_exitinfoarea_t exitinfo;
    };
};


extern vmcs_t *current_vmcs;

INLINE bool vmcs_t::is_loaded () const
{
    return (current_vmcs == this);
}

INLINE bool vmcs_t::load ()
{
    if (EXPECT_TRUE (is_loaded()))
	return true;

    current_vmcs = NULL;
    
    ASSERT (this != 0);
    ASSERT (((word_t) this & ~X86_PAGE_MASK) == 0);

    if (x86_vmsucceed (x86_vmptrld ((word_t) this)))
    {
	current_vmcs = this;
	return true;
    }

    return false;
}

INLINE bool vmcs_t::unload ()
{
    if (is_loaded ())
    {
	current_vmcs = NULL;
	return (x86_vmsucceed (x86_vmclear ((word_t) this)));
    }
    else
	return true;
}


INLINE word_t vmcs_t::read_register (word_t idx)
{
    ASSERT (is_loaded());
    return x86_vmread (idx);
}


INLINE bool vmcs_t::write_register (word_t idx, word_t value)
{
    ASSERT (is_loaded());
    return x86_vmwrite (idx, value);
}



#endif /* !__ARCH__X86__VMX_H__ */
