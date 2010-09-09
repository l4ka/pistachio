/*********************************************************************
 *
 * Copyright (C) 2006-2007,  Karlsruhe University
 *
 * File path:     glue/v4-x86/x32/hvm-vmx.cc
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
#include <kdb/tracepoints.h>
#include <debug.h>
#include INC_API(tcb.h)
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(segdesc.h)
#include INC_ARCH_SA(vmx.h)
#include INC_GLUE(hvm.h)
#include INC_GLUE(ipc.h)
#include INC_GLUE(idt.h)


#define GET64_VMCS_LOW(reg)  ((u32_t) vmcs->reg)    
#define GET64_VMCS_HIGH(reg) ((u32_t) (vmcs->reg >> 32))    

#define SET64_VMCS_LOW(reg, val)		\
    do {					\
	u32_t r = vmcs->reg;			\
	r &= 0xffffffff00000000ULL;		\
	r |= val;				\
	vmcs->reg = r;				\
    } while (0);

#define SET64_VMCS_HIGH(reg, val)		\
    do {					\
	u32_t r = vmcs->reg;			\
	r &= 0xffffffffULL;			\
	r |= (u64_t) val << 32;			\
	vmcs->reg = r;				\
    } while (0);



FEATURESTRING ("x86-hvm");

DECLARE_TRACEPOINT(X86_HVM_EXIT);
DECLARE_TRACEPOINT(X86_HVM_EXIT_EXTINT);
DECLARE_TRACEPOINT(X86_HVM_ENTRY);
DECLARE_TRACEPOINT(X86_HVM_ENTRY_EXC);


word_t x32_hvm_vmx_t::host_dr[8];
extern "C" void vmexit_entry_point ();

#if defined(CONFIG_DEBUG)
extern bool x86_kdb_singlestep, x86_kdb_branchstep;
extern word_t x86_kdb_dr_mask;
extern word_t x86_kdb_last_ip;
#endif

/**********************************************************************
 *
 *                        x86 HVM -- VMX-specific functions
 *
 **********************************************************************/


void x32_hvm_vmx_t::init_vmcs ()
{
    tcb_t         *tcb   = addr_to_tcb(this);
    space_t       *space = tcb->get_space ();
    vmcs_hsarea_t *hs    = &(vmcs->hs);
    vmcs_gsarea_t *gs    = &(vmcs->gs);

    ASSERT (space);
    ASSERT (vmcs);

    vmcs->init ();

    hs->cr0	= x86_cr0_read ();
    hs->cr3	= (word_t) space->get_top_pdir_phys ();
    hs->cr4	= x86_cr4_read ();


    hs->rsp	= (word_t) tcb->get_stack_top() - 6 * sizeof (word_t);
    hs->rip	= (word_t) vmexit_entry_point;

    /*
     * Configuration taken from init.cc.
     */

    extern x86_segdesc_t gdt[];

    hs->cs_sel	= X86_KCS;

    hs->ss_sel	= X86_KDS;

    // VMX does not allow user-accessible segments.
    // So we reload them manually on a VM exit.
    hs->ds_sel	= X86_KDS;
    hs->es_sel	= X86_KDS;

    hs->gs_sel	= X86_KDS;
    hs->gs_base	= gdt[X86_KDS >> 3].get_base ();

    hs->fs_sel	= X86_KDS;
    hs->fs_base	= gdt[X86_KDS >> 3].get_base ();

    hs->tr_sel	= X86_TSS;
    hs->tr_base	= gdt[X86_TSS >> 3].get_base ();

    hs->gdtr_base	= (word_t) &gdt;
    hs->idtr_base	= (word_t) &idt;

#if defined(CONFIG_IO_FLEXPAGES)
    hs->sysenter_esp	= ((u32_t) (TSS_MAPPING) + 4);
#else
    extern x86_x32_tss_t tss;
    hs->sysenter_esp	= ((u32_t) (&tss)) + 4;
#endif

    hs->sysenter_eip	= (u32_t) (exc_user_sysipc);
    hs->sysenter_cs	= X86_KCS;


    //bit X in X86_MSR_VMX_CR0_FIXED0 = 1 -> bit must be 1
    //bit X in X86_MSR_VMX_CR0_FIXED1 = 0 -> bit must be 0
    
    
    u64_t cr0_fixed0 = x86_rdmsr (X86_MSR_VMX_CR0_FIXED0);
    u64_t cr0_fixed1 = x86_rdmsr (X86_MSR_VMX_CR0_FIXED1);
    u64_t cr4_fixed0 = x86_rdmsr (X86_MSR_VMX_CR4_FIXED0);
    u64_t cr4_fixed1 = x86_rdmsr (X86_MSR_VMX_CR4_FIXED1);
    
    
    guest_cr0_mask = X86_HVM_CR0_MASK & (cr0_fixed0 ^ cr0_fixed1);
    guest_cr4_mask = X86_HVM_CR4_MASK & (cr4_fixed0 ^ cr4_fixed1);
    // guest_cr4_mask |= X86_CR4_VME;
    
    word_t gcr0 = x86_cr0_read() & ~X86_CR0_WP;
    gcr0 |= cr0_fixed0 & cr0_fixed1;
    gcr0 &= cr0_fixed0 | cr0_fixed1;
    
    word_t gcr4 = x86_cr4_read() & ~X86_CR4_PAE & ~X86_CR4_VMXE;
    gcr4 |= cr4_fixed0 & cr4_fixed1;
    gcr4 &= cr4_fixed0 | cr4_fixed1;

    gs->cr0 = gcr0;
    gs->cr3 = vtlb.get_active_top_pdir();
    gs->cr4 = gcr4;
    gs->dr7 = 0x400;
    
}

#if defined(CONFIG_IO_FLEXPAGES)
void x32_hvm_vmx_t::set_io_pbm (addr_t paddr)
{
    vmcs_exectr_cpubased_t i_cpubased;

    i_cpubased = vmcs->exec_ctr.cpubased;
    i_cpubased.iobitm = 1;
    vmcs->exec_ctr.cpubased = i_cpubased;

    // Set bitmap address.
    vmcs->exec_ctr.iobmpa = (u32_t) paddr;
    vmcs->exec_ctr.iobmpb = (u32_t) paddr + X86_X32_PAGE_SIZE;
}
#endif




void x32_hvm_vmx_t::save_guest_drs()
{
    X86_GET_DR(0, guest_dr[0]); X86_GET_DR(1, guest_dr[1]);
    X86_GET_DR(2, guest_dr[2]); X86_GET_DR(3, guest_dr[3]);
    X86_GET_DR(6, guest_dr[6]); guest_dr[7] = vmcs->gs.dr7;

    X86_SET_DR(0, host_dr[0]); X86_SET_DR(1, host_dr[1]);
    X86_SET_DR(2, host_dr[2]); X86_SET_DR(3, host_dr[3]);
    X86_SET_DR(6, host_dr[6]); X86_SET_DR(7, host_dr[7]);

}

void x32_hvm_vmx_t::restore_guest_drs()
{
    X86_GET_DR(0, host_dr[0]); X86_GET_DR(1, host_dr[1]);
    X86_GET_DR(2, host_dr[2]); X86_GET_DR(3, host_dr[3]);
    X86_GET_DR(6, host_dr[6]); X86_GET_DR(7, host_dr[7]);

    //x86_set_kdb_dr(0, x86_bp_instr, 0xfe05b, true, true);
    
#if defined(CONFIG_DEBUG)
    if (x86_kdb_singlestep || (x86_kdb_branchstep ^ kdb_branchstep))
    {

	vmcs_gs_ias_t ias = vmcs->gs.ias;
	vmcs_gs_as_t  as  = vmcs->gs.as;
	    
	if (ias.bl_sti == 1 || ias.bl_movss == 1 || as.state == vmcs_gs_as_t::hlt)
	{
	    vmcs_gs_pend_dbg_except_t dbge = vmcs->gs.pend_dbg_except;
	    dbge.bs = 1;
	    vmcs->gs.pend_dbg_except = dbge;
	}
	
	x86_kdb_last_ip = vmcs->read_register (VMCS_IDX_G_CS_BASE) + 
	    get_user_frame(addr_to_tcb(this))->regs[x86_exceptionframe_t::ipreg];

    }

    if (x86_kdb_branchstep ^ kdb_branchstep)
    {
	if (x86_kdb_branchstep)
	{
	    printf("enable L4-KDB branchstepping %x\n");
	    word_t dbg_ctl = GET64_VMCS_LOW(gs.dbg_ctl);
	    SET64_VMCS_LOW(gs.dbg_ctl, dbg_ctl | 0x3);
	}
	else 
	{
	    printf("disable L4-KDB branchstepping %x\n");
	    word_t dbg_ctl = GET64_VMCS_LOW(gs.dbg_ctl);
	    SET64_VMCS_LOW(gs.dbg_ctl, dbg_ctl & ~0x3);
	}
	kdb_branchstep = x86_kdb_branchstep;
    }
    
    word_t kdb_drs = x86_kdb_dr_mask ^ kdb_dr_mask;
    if (kdb_drs)
    {
	word_t on = kdb_drs & x86_kdb_dr_mask;
	word_t off = kdb_drs & ~x86_kdb_dr_mask;
	kdb_dr_mask |= on;
	kdb_dr_mask &= ~off;
	
	for (word_t dr=x86_lsb(off); off!=0; 
	     off>>=x86_lsb(off)+1, dr+=x86_lsb(off)+1)
	{
	    guest_dr[7] &=  ~(2 << (dr * 2)); /* disable */
	    guest_dr[7] &= ~(0x000F0000 << (dr * 4));
	}
	
	for (word_t dr=x86_lsb(on); on!=0; 
	     on>>=x86_lsb(on)+1, dr+=x86_lsb(on)+1)
	{
	    guest_dr[dr] = x86_dr_read(dr);
	    /* Copy type from host dr7 */
	    word_t type = (host_dr[7] >> (dr * 4)) & 0x30000;
	    guest_dr[7] |=  (2 << (dr * 2)); /* enable */
	    guest_dr[7] &= ~(0x000F0000 << (dr * 4));
	    guest_dr[7] |= (type << (dr * 4));
	}
	
	if (!kdb_dr_mask)
	{
	    // Redisable VMM DRs if needed	    
	    if (!flags.movdr)
	    {
		vmcs_exectr_cpubased_t cb = vmcs->exec_ctr.cpubased;
		cb.movdr = 0;
		vmcs->exec_ctr.cpubased = cb;
	    }
	    if (!flags.db || !flags.bp)
	    {
		vmcs_exectr_excbmp_t ebmp = vmcs->exec_ctr.except_bmp;
		ebmp.db = flags.db;
		ebmp.bp = flags.bp;
		vmcs->exec_ctr.except_bmp = ebmp;
	    }
	}
    }
#endif
    
    X86_SET_DR(0, guest_dr[0]); X86_SET_DR(1, guest_dr[1]);
    X86_SET_DR(2, guest_dr[2]); X86_SET_DR(3, guest_dr[3]);
    X86_SET_DR(6, guest_dr[6]); vmcs->gs.dr7 = guest_dr[7];
}

extern void do_enter_kdebug(x86_exceptionframe_t *frame, const word_t exception);
INLINE bool x32_hvm_vmx_t::handle_debug_exit (vmcs_ei_qual_t qual)
{
#if defined(CONFIG_DEBUG)
    if (qual.dbg.bs)
    {
	if (x86_kdb_branchstep)
	{
	    word_t dbg_ctl = GET64_VMCS_LOW(gs.dbg_ctl);
	    SET64_VMCS_LOW(gs.dbg_ctl, dbg_ctl & ~0x3);
	    X86_SET_DR(6, qual.raw);
	    do_enter_kdebug(get_user_frame(addr_to_tcb(this)), X86_EXC_DEBUG);
	    return true;
	}
    
	if (x86_kdb_singlestep)		 
	{
	    vmcs_gs_ias_t ias = vmcs->gs.ias;
	    
	    ias.bl_sti = ias.bl_movss = 0;
	    vmcs->gs.ias = ias;
	    
	    vmcs_gs_as_t  as  = vmcs->gs.as;
	    ASSERT(as.state != vmcs_gs_as_t::hlt);
	    
	    X86_SET_DR(6, qual.raw);
	    do_enter_kdebug(get_user_frame(addr_to_tcb(this)), X86_EXC_DEBUG);
	    return true;
	}
    }
    if (qual.dbg.bx)
    {
	X86_SET_DR(6, qual.raw);
	do_enter_kdebug(get_user_frame(addr_to_tcb(this)), X86_EXC_DEBUG);
	return true;
    }
    
    printf("HVM dbg exit no l4");
    enter_kdebug("UNTESTED");
#endif
    return false;
}

INLINE bool x32_hvm_vmx_t::handle_nomath_exit()
{
    /* FPU access logic:
     * guest NE bit = 1 -> VMM has disabled FPU
     * virtual guest TS bit = 1 -> VMM wants NM exception
     * virtual guest TS bit = 0 -> L4 virtualizes FPU
     */
    
    if (flags.fpu_ts_bit)
	return false;
    
    word_t gcr0 = vmcs->gs.cr0;

    if (gcr0 & X86_CR0_EM)
	return false;
	
    ASSERT(x86_cr0_read() & X86_CR0_TS);
    tcb_t *tcb = addr_to_tcb (this);
    tcb->resources.x86_no_math_exception (tcb);
    return true;
}

INLINE bool x32_hvm_vmx_t::handle_pagefault_exit (vmcs_ei_qual_t qual)
{
    // Check if this is just a VTLB miss, not a real page fault.
    return  vtlb.handle_vtlb_miss ((addr_t) qual.faddr, exc.exit_eec);
}



INLINE void x32_hvm_vmx_t::handle_invlpg_exit (vmcs_ei_qual_t qual)
{
    vtlb.flush_gvirt ((addr_t) qual.faddr);
}

INLINE void x32_hvm_vmx_t::handle_invd_exit ()
{
    x86_wbinvd();
}


/**********************************************************************
 *
 *                        x86 HVM -- implementation for VMX
 *
 **********************************************************************/

bool arch_hvm_ktcb_t::enable_hvm ()
{
    if (hvm_enabled) 
        return true;
    
   
    if (!x86_x32_vmx_t::is_enabled ())
	return false;


    tcb_t   *tcb   = addr_to_tcb(this);
    space_t *space = tcb->get_space ();
    ASSERT (space);

    if (!vtlb.alloc(space))
	return false;

    vmcs = vmcs_t::alloc_vmcs ();
    if (!vmcs || !vmcs->load ())
    {
	vtlb.free ();
	return false;
    }

#if defined(CONFIG_DEBUG)
    kdb_dr_mask = 0;
    kdb_branchstep = false;
#endif

    for (word_t i=0; i < 7; i++)
	guest_dr[i] = 0;
    
    guest_cr2 = guest_cr0_mask = guest_cr4_mask = 0;
    
    for (word_t i=0; i < 7; i++)
	((word_t *) &exc)[i] = 0;
    
    flags.injecting = flags.cr2_write = flags.fpu_ts_bit = 0;
    
    init_vmcs ();
	
#if defined(CONFIG_IO_FLEXPAGES)
    set_io_pbm (virt_to_phys (space->get_io_bitmap ()));
#endif
    hvm_enabled = true;

    return true;
}

void arch_hvm_ktcb_t::disable_hvm ()
{
    if (!hvm_enabled) 
        return;
    
    hvm_enabled = false;

    if (vmcs)
    {
	vmcs_t::free_vmcs (vmcs);
	vmcs = NULL;
    }
    vtlb.free ();
}


word_t arch_hvm_ktcb_t::get_x86_hvm_cregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;    
    
    /* transfer from frame to dst */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value;
        switch (reg)
        {
        case ctrlxfer_item_t::creg_cr0:
            value = vmcs->gs.cr0;
            break;
        case ctrlxfer_item_t::creg_cr0_rd:
            value = vmcs->exec_ctr.cr0shadow;
            break;
        case ctrlxfer_item_t::creg_cr0_msk:
            value = vmcs->exec_ctr.cr0mask;
            break;
        case ctrlxfer_item_t::creg_cr2:
            value = guest_cr2;
            break;
        case ctrlxfer_item_t::creg_cr3:
            value = (word_t) vtlb.get_guest_top_pdir();
            break;
        case ctrlxfer_item_t::creg_cr4:
            value =  vmcs->gs.cr4;
            break;
        case ctrlxfer_item_t::creg_cr4_rd:
            value = vmcs->exec_ctr.cr4shadow;
            break;
        case ctrlxfer_item_t::creg_cr4_msk:
            value = vmcs->exec_ctr.cr4mask;
            break;
        default:
            value = 0;
            printf("x86-hvm: unsupported read from exception reg %d\n", reg);
            enter_kdebug("hvm write");
            break;
        }

        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, 0, ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);

        dst->set_mr(dst_mr++, value);
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_cregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    
    /* transfer from src to frame */
    word_t num = 0;
    
 
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value =src->get_mr(src_mr++);
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, 0, ctrlxfer_item_t::get_hwregname(id, reg),
                                value);
        
        switch (reg)
        {
        case ctrlxfer_item_t::creg_cr0:
        {
            word_t cr0 = vmcs->gs.cr0;
	
            vtlb.set_pe(value & X86_CR0_PG);
            vtlb.set_wp(value & X86_CR0_WP);
            vtlb.flush_gvirt ();
	
            vmcs->gs.cr0 = (value & guest_cr0_mask) | (cr0 & ~guest_cr0_mask);
        }
        break;
        case ctrlxfer_item_t::creg_cr0_rd:
            vmcs->exec_ctr.cr0shadow = value;
            break;
        case ctrlxfer_item_t::creg_cr0_msk:
            vmcs->exec_ctr.cr0mask = value | ~guest_cr0_mask;
            break;
        case ctrlxfer_item_t::creg_cr2:
            guest_cr2 = value;
            flags.cr2_write = true;
            break;
        case ctrlxfer_item_t::creg_cr3:
            vtlb.set_guest_top_pdir((pgent_t*)value);
            vtlb.flush_gvirt ();
            break;
        case ctrlxfer_item_t::creg_cr4:
        {
            word_t cr4 = vmcs->gs.cr4;
            vmcs->gs.cr4 = (value & guest_cr4_mask) | (cr4 & ~guest_cr4_mask);
            vtlb.set_pg(value & X86_CR4_PGE);
            vtlb.flush_gvirt ();
        }
        break;
        case ctrlxfer_item_t::creg_cr4_rd:
            vmcs->exec_ctr.cr4shadow = value;
            break;
        case ctrlxfer_item_t::creg_cr4_msk:
            vmcs->exec_ctr.cr4mask = value | ~guest_cr4_mask;
            vtlb.flush_gvirt ();
            break;
        default:
            printf("x86-hvm: unsupported write to exception reg %d value %x\n", reg, value);
            enter_kdebug("hvm write");
            break;
        }
    }
    return num;
}

word_t arch_hvm_ktcb_t::get_x86_hvm_dregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    
    /* transfer from frame to dst */
    const word_t *hwreg = ctrlxfer_item_t::hwregs[id];
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {   
        word_t value = guest_dr[hwreg[reg]];
        if (hwreg[reg] > 7 || hwreg[reg] == 4 || hwreg[reg] == 5)
        {
            printf("x86-hvm: invalid access to dreg %d reg %d\n", hwreg[reg], reg);
            continue;
        }
            
        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, hwreg[reg], ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);
        dst->set_mr(dst_mr++, value);;
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_dregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    
    /* transfer from src to frame */
    const word_t *hwreg = ctrlxfer_item_t::hwregs[id];
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        if (hwreg[reg] > 7 || hwreg[reg] == 4 || hwreg[reg] == 5)
        {
            printf("x86-hvm: invalid access to dreg %d reg %d\n", hwreg[reg], reg);
            continue;
        }
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, hwreg[reg], ctrlxfer_item_t::get_hwregname(id, reg),
                                src->get_mr(src_mr));
        
        guest_dr[hwreg[reg]] = src->get_mr(src_mr++);
    }
    return num;
}


word_t arch_hvm_ktcb_t::get_x86_hvm_segregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    
    /* transfer from frame to dst */
    const word_t *hwreg = ctrlxfer_item_t::hwregs[id];
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value = vmcs->read_register(hwreg[reg]);
        
        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, hwreg[reg], ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);;
        dst->set_mr(dst_mr++, value);
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_segregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from src to frame */
    const word_t *hwreg = ctrlxfer_item_t::hwregs[id];
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, hwreg[reg], ctrlxfer_item_t::get_hwregname(id, reg),
                                src->get_mr(src_mr));
        vmcs->write_register(hwreg[reg], src->get_mr(src_mr++));
    }
    return num;
}


word_t arch_hvm_ktcb_t::get_x86_hvm_nonregexc(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from frame to dst */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value;
        
        switch (reg)
        {
        case ctrlxfer_item_t::activity_state:
        {
            vmcs_gs_as_t as = vmcs->gs.as;
            value = as.raw;
        }
        break;
        case ctrlxfer_item_t::interruptibility_state:
        {	
            vmcs_gs_ias_t ias = vmcs->gs.ias;
            value = ias.raw;
        }
        break;
        case ctrlxfer_item_t::pending_debug_exc:
        {
            vmcs_gs_pend_dbg_except_t dbge = vmcs->gs.pend_dbg_except;
            value = dbge.raw;
        }
        break;
        case ctrlxfer_item_t::entry_info:
            value = exc.entry_info.raw;
            break;
        case ctrlxfer_item_t::entry_eec:
            value = exc.entry_eec;
            break;
        case ctrlxfer_item_t::entry_ilen:
            value = exc.entry_ilen;
            break;
        case ctrlxfer_item_t::exit_info:
            value = exc.exit_info.raw;
            break;
        case ctrlxfer_item_t::exit_eec:
            value = exc.exit_eec;
            break;
        case ctrlxfer_item_t::idt_info:
            value = exc.idt_info.raw;
            break;
        case ctrlxfer_item_t::idt_eec:
            value = exc.idt_eec;
            break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported read from nonreg/exc %d\n", reg);
            enter_kdebug("hvm read");
            break;
        }
        }
        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, 0, ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);
    

        dst->set_mr(dst_mr++, value);
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_nonregexc(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from src to frame */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value = src->get_mr(src_mr++);
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, 0, ctrlxfer_item_t::get_hwregname(id, reg),
                                value);
        switch (reg)
        {
        case ctrlxfer_item_t::activity_state:
        {
            vmcs_gs_as_t as;
            as.raw = value;
            vmcs->gs.as = as;
        }
        break;
        case ctrlxfer_item_t::interruptibility_state:
        {	
            vmcs_gs_ias_t ias;
            ias.raw = value;
            vmcs->gs.ias = ias;
        }
        break;
        case ctrlxfer_item_t::pending_debug_exc:
        {
            vmcs_gs_pend_dbg_except_t dbge;
            dbge.raw = value;
            vmcs->gs.pend_dbg_except = dbge;
        }
        break;
        case ctrlxfer_item_t::entry_info:
            exc.entry_info.raw = value;
            flags.injecting = true;
            break;
        case ctrlxfer_item_t::entry_eec:
            exc.entry_eec = value;
            flags.injecting = true;
            break;
        case ctrlxfer_item_t::entry_ilen:
            exc.entry_ilen = value;
            flags.injecting = true;
            break;
        case ctrlxfer_item_t::exit_info:
        case ctrlxfer_item_t::exit_eec:
        case ctrlxfer_item_t::idt_info:
        case ctrlxfer_item_t::idt_eec:
            // Read-only
            break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported write to nonreg/exc %d value %x\n", reg, value);
            enter_kdebug("hvm write");
            break;
        }
        }

    }
    return num;
}


word_t arch_hvm_ktcb_t::get_x86_hvm_execctrl(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from frame to dst */
    
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value;
        switch (reg)
        {
        case ctrlxfer_item_t::pin_exec_ctrl:
        {
            vmcs_exectr_pinbased_t pb = vmcs->exec_ctr.pinbased;
            value = pb.raw;
        }
        break;
        case ctrlxfer_item_t::cpu_exec_ctrl:
        {	
            vmcs_exectr_cpubased_t cb = vmcs->exec_ctr.cpubased;
            value = cb.raw;
        }
        break;
        case ctrlxfer_item_t::exc_bitmap:
        {
            vmcs_exectr_excbmp_t ebmp = vmcs->exec_ctr.except_bmp;
            value = ebmp.raw;
        }
        break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported read from execctrl reg %d\n", reg);
            enter_kdebug("hvm read");
            break;
        }
        }
        
        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, 0, ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);

        dst->set_mr(dst_mr++, value);
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_execctrl(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from src to frame */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value = src->get_mr(src_mr++);
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, 0, ctrlxfer_item_t::get_hwregname(id, reg), 
                                value);
        
        switch (reg)
        {
        case ctrlxfer_item_t::pin_exec_ctrl:
        {
            vmcs_exectr_pinbased_t pb;
            pb.raw = value;
            vmcs->exec_ctr.pinbased = pb;
        }
        break;
        case ctrlxfer_item_t::cpu_exec_ctrl:
        {	
            vmcs_exectr_cpubased_t cb;
            cb.raw = value;
	
#if defined(CONFIG_X86_IO_FLEXPAGES)
            // Don't allow IO passthrough
            cb.io = 1;
#endif
#if defined(CONFIG_DEBUG)
            // Logic to merge KDB and VMM DRs
            if (cb.movdr == 0)
            {
                flags.movdr = 0;
                cb.movdr = (kdb_dr_mask != 0);
            }
#endif	
            if (cb.iobitm || cb.msrbitm)
                UNIMPLEMENTED();
	
            //Don't allow invlpg or hlt passthrough
            cb.invlpg = 1;
            cb.hlt = 1;
	
            vmcs->exec_ctr.cpubased = cb;
        }
        break;
        case ctrlxfer_item_t::exc_bitmap:
        {
            vmcs_exectr_excbmp_t ebmp;
            ebmp.raw = value;
#if defined(CONFIG_DEBUG)
            // Logic to merge KDB and VMM DRs
            if (ebmp.db == 0 || ebmp.bp == 0)
            {
                flags.db = ebmp.db;
                flags.bp = ebmp.bp;
                ebmp.db = ebmp.bp = (kdb_dr_mask != 0);
            }
#endif	
            vmcs->exec_ctr.except_bmp = ebmp;
        }
        break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported write to execctrl reg %d value %x\n", reg, value);
            enter_kdebug("hvm write");
            break;
        }
        }
    }
    return num;
}


word_t arch_hvm_ktcb_t::get_x86_hvm_otherregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from frame to dst */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value;
        switch (reg)
        {
        case ctrlxfer_item_t::sysenter_cs_msr:
            value = vmcs->gs.sysenter_cs;
            break;
        case ctrlxfer_item_t::sysenter_eip_msr:
            value = vmcs->gs.sysenter_eip;
            break;
        case ctrlxfer_item_t::sysenter_esp_msr:
            value = vmcs->gs.sysenter_esp;
            break;
        case ctrlxfer_item_t::debugctl_msr_low:
            value = GET64_VMCS_LOW(gs.dbg_ctl);
            break;
            break;
        case ctrlxfer_item_t::debugctl_msr_high:
            value = GET64_VMCS_HIGH(gs.dbg_ctl);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_low:
            value = GET64_VMCS_LOW(exec_ctr.tscoff);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_high:
            value = GET64_VMCS_HIGH(exec_ctr.tscoff);
            break;
        case ctrlxfer_item_t::vapic_address:
            value = vmcs->exec_ctr.vapicaddr;
            break;
        case ctrlxfer_item_t::tpr_threshold:
        {
            vmcs_exectr_tprth_t i_tprth;
            i_tprth = vmcs->exec_ctr.tprthr;
            value = i_tprth.raw;
        }
        break;
        default:
            value = 0;
            printf ("x86-hvm: unsupported read from guest state reg %d\n", reg);
            enter_kdebug("hvm msr read");
        }
        TRACE_CTRLXFER_DETAILS( "\t (f%06d/%06d/%8s->m%06d): %08x", 
                                reg, 0, ctrlxfer_item_t::get_hwregname(id, reg), 
                                dst_mr, value);
        dst->set_mr(dst_mr++, value);
    }
    return num;
}

word_t arch_hvm_ktcb_t::set_x86_hvm_otherregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    /* transfer from src to frame */
    word_t num = 0;
    
    for (word_t reg=lsb(mask); mask!=0; mask>>=lsb(mask)+1,reg+=lsb(mask)+1,num++)
    {
        word_t value = src->get_mr(src_mr++);
        
        TRACE_CTRLXFER_DETAILS( "\t (m%06d->f%06d/%06d/%8s): %08x", 
                                src_mr, reg, 0, ctrlxfer_item_t::get_hwregname(id, reg),
                                value);
        
        switch (reg)
        {
        case ctrlxfer_item_t::sysenter_cs_msr:
            vmcs->gs.sysenter_cs = value;
            break;
        case ctrlxfer_item_t::sysenter_eip_msr:
            vmcs->gs.sysenter_eip = value;
            break;
        case ctrlxfer_item_t::sysenter_esp_msr:
            vmcs->gs.sysenter_esp = value;
            break;
        case ctrlxfer_item_t::debugctl_msr_low:
            SET64_VMCS_LOW(gs.dbg_ctl, value);
            break;
            break;
        case ctrlxfer_item_t::debugctl_msr_high:
            SET64_VMCS_HIGH(gs.dbg_ctl, value);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_low:
            SET64_VMCS_LOW(exec_ctr.tscoff, value);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_high:
            SET64_VMCS_HIGH(exec_ctr.tscoff, value);
            break;
        case ctrlxfer_item_t::vapic_address:
            vmcs->exec_ctr.vapicaddr = value;
            break;
        case ctrlxfer_item_t::tpr_threshold:
        {
            vmcs_exectr_tprth_t i_tprth;
            i_tprth.th = value;
            vmcs->exec_ctr.tprthr =  i_tprth;
        }
        break;
        default:
            printf("x86-hvm: unsupported write to guest state reg %d value %x\n", reg, value);
            enter_kdebug("hvm msrreg write");
            break;
        }
    }
    return num;
}

#if defined(CONFIG_DEBUG)
word_t arch_hvm_ktcb_t::get_x86_hvm_ctrlxfer_reg(word_t id, word_t reg) 
{
    if (!hvm_enabled || !load_vmcs()) return 0;
    
    word_t value = 0;
    const word_t *hwreg = ctrlxfer_item_t::hwregs[id];

    switch (id)
    {
    case ctrlxfer_item_t::id_cregs:
        switch (reg)
        {
        case ctrlxfer_item_t::creg_cr0:
            value = vmcs->gs.cr0;
            break;
        case ctrlxfer_item_t::creg_cr0_rd:
            value = vmcs->exec_ctr.cr0shadow;
            break;
        case ctrlxfer_item_t::creg_cr0_msk:
            value = vmcs->exec_ctr.cr0mask;
            break;
        case ctrlxfer_item_t::creg_cr2:
            value = guest_cr2;
            break;
        case ctrlxfer_item_t::creg_cr3:
            value = (word_t) vtlb.get_guest_top_pdir();
            break;
        case ctrlxfer_item_t::creg_cr4:
            value =  vmcs->gs.cr4;
            break;
        case ctrlxfer_item_t::creg_cr4_rd:
            value = vmcs->exec_ctr.cr4shadow;
            break;
        case ctrlxfer_item_t::creg_cr4_msk:
            value = vmcs->exec_ctr.cr4mask;
            break;
        default:
            value = 0;
            printf("x86-hvm: unsupported reg to exception reg %d value %x\n", reg, value);
            enter_kdebug("hvm write");
            break;
        }
        break;
    case ctrlxfer_item_t::id_dregs:
        value = guest_dr[hwreg[reg]];
        break;
    case ctrlxfer_item_t::id_csregs:
    case ctrlxfer_item_t::id_ssregs:
    case ctrlxfer_item_t::id_dsregs:
    case ctrlxfer_item_t::id_esregs:
    case ctrlxfer_item_t::id_fsregs:
    case ctrlxfer_item_t::id_gsregs:
    case ctrlxfer_item_t::id_trregs:
    case ctrlxfer_item_t::id_ldtrregs:
    case ctrlxfer_item_t::id_idtrregs:
    case ctrlxfer_item_t::id_gdtrregs:
        /* transfer from frame to dst */
        value = vmcs->read_register(hwreg[reg]);
        break;
    case ctrlxfer_item_t::id_nonregexc:
        switch (reg)
        {
        case ctrlxfer_item_t::activity_state:
        {
            vmcs_gs_as_t as = vmcs->gs.as;
            value = as.raw;
        }
        break;
        case ctrlxfer_item_t::interruptibility_state:
        {	
            vmcs_gs_ias_t ias = vmcs->gs.ias;
            value = ias.raw;
        }
        break;
        case ctrlxfer_item_t::pending_debug_exc:
        {
            vmcs_gs_pend_dbg_except_t dbge = vmcs->gs.pend_dbg_except;
            value = dbge.raw;
        }
        break;
        case ctrlxfer_item_t::entry_info:
            value = exc.entry_info.raw;
            break;
        case ctrlxfer_item_t::entry_eec:
            value = exc.entry_eec;
            break;
        case ctrlxfer_item_t::entry_ilen:
            value = exc.entry_ilen;
            break;
        case ctrlxfer_item_t::exit_info:
            value = exc.exit_info.raw;
            break;
        case ctrlxfer_item_t::exit_eec:
            value = exc.exit_eec;
            break;
        case ctrlxfer_item_t::idt_info:
            value = exc.idt_info.raw;
            break;
        case ctrlxfer_item_t::idt_eec:
            value = exc.idt_eec;
            break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported read from nonreg/exc %d\n", reg);
            enter_kdebug("hvm read");
            break;
        }
        }
        break;
    case ctrlxfer_item_t::id_execctrl:
        switch (reg)
        {
        case ctrlxfer_item_t::pin_exec_ctrl:
        {
            vmcs_exectr_pinbased_t pb = vmcs->exec_ctr.pinbased;
            value = pb.raw;
        }
        break;
        case ctrlxfer_item_t::cpu_exec_ctrl:
        {	
            vmcs_exectr_cpubased_t cb = vmcs->exec_ctr.cpubased;
            value = cb.raw;
        }
        break;
        case ctrlxfer_item_t::exc_bitmap:
        {
            vmcs_exectr_excbmp_t ebmp = vmcs->exec_ctr.except_bmp;
            value = ebmp.raw;
        }
        break;
        default:
        {
            value = 0;
            printf("x86-hvm: unsupported read from execctrl reg %d\n", reg);
            enter_kdebug("hvm read");
            break;
        }
        }
        break;
    case ctrlxfer_item_t::id_otherregs:
        switch (reg)
        {
        case ctrlxfer_item_t::sysenter_cs_msr:
            vmcs->gs.sysenter_cs = value;
            break;
        case ctrlxfer_item_t::sysenter_eip_msr:
            vmcs->gs.sysenter_eip = value;
            break;
        case ctrlxfer_item_t::sysenter_esp_msr:
            vmcs->gs.sysenter_esp = value;
            break;
        case ctrlxfer_item_t::debugctl_msr_low:
            SET64_VMCS_LOW(gs.dbg_ctl, value);
            break;
            break;
        case ctrlxfer_item_t::debugctl_msr_high:
            SET64_VMCS_HIGH(gs.dbg_ctl, value);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_low:
            SET64_VMCS_LOW(exec_ctr.tscoff, value);
            break;
        case ctrlxfer_item_t::rdtsc_ofs_high:
            SET64_VMCS_HIGH(exec_ctr.tscoff, value);
            break;
        case ctrlxfer_item_t::vapic_address:
            vmcs->exec_ctr.vapicaddr = value;
            break;
        case ctrlxfer_item_t::tpr_threshold:
        {
            vmcs_exectr_tprth_t i_tprth;
            i_tprth.th = value;
            vmcs->exec_ctr.tprthr =  i_tprth;
        }
        break;
        default:
            printf("unsupported hvm ctrlxfer item read id %d reg %d\n", id, reg);
            enter_kdebug("hvm ctrlxfer read");
            break;
        }
    }
    return value;
}
#endif
word_t saved_rfl;

void arch_hvm_ktcb_t::resume_hvm ()
{
    tcb_t *tcb		= addr_to_tcb(this);
    x86_exceptionframe_t *frame	= get_user_frame(tcb);
    
    ASSERT(hvm_enabled);
    ASSERT(vmcs);
    vmcs->load ();
    ASSERT(vmcs->is_loaded());
	   
    vmcs->gs.rflags = frame->eflags;
    vmcs->gs.rip = frame->eip;
    vmcs->gs.rsp = frame->esp;

    restore_guest_drs();
   
    /* Set FPU access permissions:
     * virtual guest TS bit = 1 -> gcr0 = 1
     * virtual guest TS bit = 0 -> gcr0 = hcr0
     */
    word_t gcr0 = vmcs->gs.cr0;

    if (flags.fpu_ts_bit)
	gcr0 |= X86_CR0_TS;
    else
	gcr0 = (gcr0 & ~X86_CR0_TS) | (x86_cr0_read() & X86_CR0_TS);

    vmcs->gs.cr0 = gcr0;
    vmcs->gs.cr3 = vtlb.get_active_top_pdir();

    if (1 || flags.cr2_write)
    {
	x86_cr2_write (this->guest_cr2);
	flags.cr2_write = false;
    }
    
    if (flags.injecting)
    {
	vmcs->entry_ctr.iif = exc.entry_info;
	vmcs->entry_ctr.eec = exc.entry_eec;
	vmcs->entry_ctr.instr_len = exc.entry_ilen;
	
	TRACEPOINT(X86_HVM_ENTRY_EXC, 
		   "x86-hvm: inject exception %x (type %d vec %d eecv %c), eec %d, ilen %d", 
		   exc.entry_info.raw, exc.entry_info.type, exc.entry_info.vector,
		   exc.entry_info.err_code_valid ? 'y' : 'n', exc.entry_eec, exc.entry_ilen);
    }
    else
	ASSERT(!exc.entry_info.valid);
    
#if defined(CONFIG_DEBUG)
    vmcs->do_vmentry_checks ();
#endif
    UNUSED vmcs_int_t iif = ((vmcs_int_t)vmcs->entry_ctr.iif);
    UNUSED word_t eec = vmcs->entry_ctr.eec;
    UNUSED word_t ilen = vmcs->entry_ctr.instr_len;
    UNUSED word_t rfl = vmcs->gs.rflags;
    UNUSED word_t csb = vmcs->read_register (VMCS_IDX_G_CS_BASE);
    UNUSED word_t rip = vmcs->gs.rip;
    UNUSED vmcs_gs_pend_dbg_except_t dbge = vmcs->gs.pend_dbg_except;
    
    if ((rfl != saved_rfl) && rfl == 0x46)
	printf("x86-hvm: entry old_fl %x new_fl %x\n",  saved_rfl, rfl);
	
    TRACEPOINT (X86_HVM_ENTRY, "x86-hvm: entry csbase %x ip %x fl %x iif %x eec %x ilen %x dbge %x",
		csb, rip, rfl, iif.raw, eec, ilen, dbge.raw);
}


INLINE void arch_hvm_ktcb_t::handle_hvm_exit ()
{
    tcb_t *tcb	= addr_to_tcb(this);
    vmcs_ei_reason_t reason = vmcs->exitinfo.reason;
    vmcs_ei_reason_t::basic_reason_e basic_reason = reason.basic_reason;
    vmcs_ei_qual_t qual = vmcs->exitinfo.qual;
    word_t ilen= vmcs->exitinfo.instr_len;
    word_t ia_info = 0;
    bool handled = false;

    x86_exceptionframe_t *frame	= get_user_frame(tcb);
    frame->eip = vmcs->gs.rip;
    frame->esp = vmcs->gs.rsp;
    frame->eflags = vmcs->gs.rflags;
    saved_rfl = frame->eflags;
    
    UNUSED word_t csb = vmcs->read_register (VMCS_IDX_G_CS_BASE);

    exc.exit_info.raw = exc.exit_eec = 0;
    exc.idt_info = vmcs->exitinfo.idtvect_info;
    exc.idt_eec = vmcs->exitinfo.idtvect_ec;

    guest_cr2 = x86_cr2_read();

    if (flags.injecting)
    {
	/* VMX clears valid bit on entry, mirror fields */
	exc.entry_info = vmcs->entry_ctr.iif;
	exc.entry_eec = vmcs->entry_ctr.eec;
	exc.entry_ilen = vmcs->entry_ctr.instr_len;
	flags.injecting = false;
    }
    
    save_guest_drs();

    if(reason.is_entry_fail)
    {
	enter_kdebug("vmexit due to entry failed");
    }
    
    switch (basic_reason)
    {
    case vmcs_ei_reason_t::be_exp_nmi:	// Exception or NMI.
    {
	exc.exit_info = vmcs->exitinfo.int_info;
	exc.exit_eec = vmcs->exitinfo.int_ec;
	
	switch (exc.exit_info.vector)
	{
	case X86_EXC_DEBUG:
	    TRACEPOINT (X86_HVM_EXIT, "x86-hvm: breakpoint exit %d qual %x  cs %x ip %x",
			basic_reason, qual.raw,  csb, frame->eip); 
	    handled = handle_debug_exit(qual);
	    break;
	case X86_EXC_NOMATH_COPROC:	// No math exception.
	    TRACEPOINT (X86_HVM_EXIT, "x86-hvm: nomath exit %d qual %x  cs %x ip %x",
			basic_reason, qual.raw,  csb, frame->eip); 
	    handled = handle_nomath_exit ();
	    break;
	case X86_EXC_PAGEFAULT:		// Page fault.
	    if (qual.raw == 0x88)
		ENABLE_TRACEPOINT(X86_HVM_EXIT, ~0UL, ~0UL);
                
	    TRACEPOINT (X86_HVM_EXIT, "x86-hvm: pf exit %d qual %x  eec %d cs %x ip %x",
			basic_reason, qual.raw,  exc.exit_eec, csb, frame->eip); 
	    handled = handle_pagefault_exit (qual);
	    break;
	default:
	    TRACEPOINT (X86_HVM_EXIT, "x86-hvm: exc exit %d (exc %d:%x) vec %d qual %x  cs %x ip %x",
			basic_reason, exc.exit_info.err_code_valid, exc.exit_eec, exc.exit_info.vector,
			qual.raw,  csb, frame->eip);
	    break;
	}
	break;
    }
    case vmcs_ei_reason_t::be_ext_int:	// External interrupt.
    {
	TRACEPOINT (X86_HVM_EXIT_EXTINT, "x86-hvm:  reason %d (extint) cs %x ip %x",
		    reason.basic_reason, csb, frame->eip);

	// Allow ourselves to be preempted on an external interrupt.
	asm (
	    "	sti	\n"
	    "	nop	\n"
	    "	cli	\n"
	    );

	vmcs->load ();
	handled = true;
    }
    break;
    case vmcs_ei_reason_t::be_iw:	// Interrupt Window.
	TRACEPOINT(X86_HVM_EXIT, "x86-hvm: interrupt window exit: eflags %x cs %x ip %x", 
		   (word_t) frame->eflags,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_dr:
    case vmcs_ei_reason_t::be_cr:
	ia_info = vmcs->exitinfo.linear_addr;
	    
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: cr/dr exit: %d (%cr%c) qual %x,  cs %x ip %x", 
		    basic_reason,  (basic_reason == vmcs_ei_reason_t::be_cr ? 'c' : 'd'),
		    qual.mov_cr.cr_num + '0', qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_tasksw:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: tasksw exit: %d qual %x,  cs %x ip %x", 
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_io:
	ia_info = vmcs->exitinfo.linear_addr;

	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: i/o exit: qual %x (%c, p %x, st %d sz %d, r %d i %d) cs %x ip %x",
		    qual.raw, (qual.io.dir == vmcs_ei_qual_t::out ? 'o' : 'i'),
		    qual.io.port_num, qual.io.string, (qual.io.soa + 1) * 8, 
		    qual.io.rep, qual.io.op_encoding, csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_rdmsr:
    case vmcs_ei_reason_t::be_wrmsr:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: msr exit %d (msr %x) qual %x  cs %x ip %x",
		    basic_reason, frame->ecx, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_vmcall:
    case vmcs_ei_reason_t::be_vmclear ... vmcs_ei_reason_t::be_vmxon:
    {
	vmcs_ei_vm_instr_t vm_instr_info = vmcs->exitinfo.vm_instr_info;
	ia_info = vm_instr_info.raw;
	
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: vmx exit %d (vmcall/vmclear...vmxon) qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
    }
    break;
    case vmcs_ei_reason_t::be_cpuid:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: cpuid exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_hlt:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: hlt exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_invd:	// Invalidate Caches
	handle_invd_exit();
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: invd exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
	
    case vmcs_ei_reason_t::be_rdpmc:
    case vmcs_ei_reason_t::be_rdtsc:
    case vmcs_ei_reason_t::be_rsm:
    case vmcs_ei_reason_t::be_monitor:
    case vmcs_ei_reason_t::be_mwait:
    case vmcs_ei_reason_t::be_pause:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: cpuid/hlt/invd/rdpmc/rdtsc exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_invlpg:	// Invalidate TLB Entry.
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: invlpg exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	handle_invlpg_exit (qual);
	break;
    case vmcs_ei_reason_t::be_entry_invg: // Invalid Guest State
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: invalid gueststate exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    case vmcs_ei_reason_t::be_entry_msrld:	// Invalid MSR
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: invalid msr load exit %d qual %x  cs %x ip %x",
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    default:
	TRACEPOINT (X86_HVM_EXIT, "x86-hvm: exit %d qual %x  cs %x ip %x",  
		    basic_reason, qual.raw,  csb, frame->eip);
	break;
    }
    
    if (!handled || exc.idt_info.valid)
        send_hvm_fault((word_t) basic_reason, qual.raw, ilen, ia_info, handled);
    
    resume_hvm();

}



NORETURN void arch_hvm_ktcb_t::enter_hvm_loop ()
{
    tcb_t *tcb = addr_to_tcb(this);
    msg_tag_t msgtag = tcb->get_tag();
    if(msgtag.get_untyped())
	printf("Received untyped items in startup reply\n");
    
    ASSERT(hvm_enabled);
    resume_hvm();
    
    // First entry into the VM needs to be a vmlaunch.
    x86_x32_vmx_t::vmlaunch (get_user_frame(tcb)); 
}



NORETURN void do_handle_vmexit ()
{
#if defined(CONFIG_IO_FLEXPAGES)
    /*
     * VT predefined the size of the TSS segment to 67h bytes
     *  this excludes the IOPBMP.
     *
     * To use the IOPBMP, we define an other TSS to be loaded at
     * the VM-Exit. (X86_TSS_VMX)
     *
     * Here we switch to the L4 TSS, which handles IOBMPs.
     */
    extern x86_x32_segdesc_t gdt[];

    gdt[X86_TSS >> 3].set_sys((u32_t) TSS_MAPPING, sizeof (x86_x32_tss_t) - 1,
			       0, x86_x32_segdesc_t::tss);

    asm (
	"	ltr  %%ax	\n"
	:
	: "a" (X86_TSS));
#endif

    
    
    tcb_t *current		= get_current_tcb ();
    
    // Handle VM exit
    current->get_arch()->handle_hvm_exit();

    x86_mmu_t::flush_tlb(true);

    // Reenter the VM.
    x86_x32_vmx_t::vmresume (get_user_frame(current));
}


void vmexit_entry_point_wrapper ()
{
    word_t *do_handle_vmexit_ptr = (word_t *) do_handle_vmexit;
    asm (
	".globl vmexit_entry_point	\n"
	".type vmexit_entry_point,@function \n"
	"vmexit_entry_point:		\n"
	"	push %%eax		\n"
	"	push %%ecx		\n"
	"	push %%edx		\n"
	"	push %%ebx		\n"
	"	pushf			\n"
	"	push %%ebp		\n"
	"	push %%esi		\n"
	"	push %%edi		\n"
	// ds, es, reason
	"	subl  $12, %%esp	\n"

	/*****************************************
	 * Restore kernel context.
	 *****************************************/

	// Load segment selectors.
	// Selectors set in VMCS host state cannot be user-accessible.

	// DS, ES.
#if !defined(CONFIG_X86_X32_SMALL_SPACES)
	"	mov %0, %%bx		\n"
	"	mov %%bx, %%ds		\n"
	"	mov %%bx, %%es		\n"
	"	mov %%bx, %%fs		\n"
#endif
	// GS.
	"	mov %1, %%bx		\n"
	"	mov %%bx, %%gs		\n"

	// EFLAGS.
	//  VM-Exit has a cleared eflags (except bit 1, which is always 1)
	"       pushl %2		\n"
	"	popfl			\n"

	// Call do_handle_vmexit.
	"	jmp %3			\n"
	:
	: "i" (X86_UDS),					// %0
	  "i" (X86_UTCBS),					// %1
	  "i" (X86_KERNEL_FLAGS),				// %2
	  "m" (*do_handle_vmexit_ptr)                           // %3
	);
}
