/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     arch/powerpc/softhvm.h
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
#pragma once

#include "msr.h"
#include "swtlb.h"
#include "frame.h"

#define LAZY_TLB

class ppc_instr_t {
public:
    ppc_instr_t()
	{ }
    ppc_instr_t(word_t val)
	{ raw = val; }

    /* acessor functions */
    int get_primary()
	{ return this->xform.primary; }
    int get_secondary()
	{ return this->xform.xo; }
    word_t ra() const
	{ return xform.ra; }
    word_t rb() const
	{ return xform.rb; }
    word_t rt() const
	{ return xform.rt; }
    word_t rf() const
	{ return xform.rb << 5 | xform.ra; }
    s16_t d() const
	{ return dform.d; }

    union {
	word_t raw;
	struct {
	    word_t primary : 6;
	    word_t li : 26;
	} iform;
	struct {
	    word_t primary : 6;
	    word_t bo : 5;
	    word_t bi : 5;
	    word_t bd : 14;
	    word_t aa : 1;
	    word_t lk : 1;
	} bform;
	struct {
	    word_t primary : 6;
	    word_t     : 24;
	    word_t one : 1;
	    word_t     : 1;
	} scform;
	struct {
	    word_t primary : 6;
	    word_t rt : 5;
	    word_t ra : 5;
	    word_t d : 16;
	} dform;
	struct {
	    word_t primary : 6;
	    word_t rt : 5;
	    word_t ra : 5;
	    word_t rb : 5;
	    word_t xo : 10;
	    word_t rc : 1;
	} xform;
	struct {
	    word_t primary : 6;
	    word_t bt : 5;
	    word_t ba : 5;
	    word_t bb : 5;
	    word_t xo : 10;
	    word_t lk : 1;
	} xlform;
	struct {
	    word_t primary : 6;
	    word_t rt : 5;
	    word_t rf : 10;
	    word_t xo : 10;
	    word_t    : 1;
	} xfxform;
	struct {
	    word_t primary : 6;
	    word_t rt : 5;
	    word_t ra : 5;
	    word_t rb : 5;
	    word_t oe : 1;
	    word_t xo : 9;
	    word_t rc : 1;
	} xoform;
	struct {
	    word_t primary : 6;
	    word_t rs : 5;
	    word_t ra : 5;
	    word_t rb : 5;
	    word_t mb : 5;
	    word_t me : 5;
	    word_t rc : 1;
	} mform;
    } __attribute((packed));
};

class ppc_softhvm_t
{
public:
    enum {
	exc_critical_input = 0,
	exc_machine_check = 1,
	exc_data_storage = 2,
	exc_instr_storage = 3,
	exc_ext_input = 4,
	exc_alignment = 5,
	exc_program = 6,
	exc_fpu_unavail = 7,
	exc_system_call = 8,
	exc_aux_unavail = 9,
	exc_decrementer = 10,
	exc_fixed_interval = 11,
	exc_watchdog = 12,
	exc_data_tlb = 13,
	exc_instr_tlb = 14,
	exc_debug = 15,
    };

    enum {
	evt_machine_check,
	evt_debug,
	evt_critical_input,
	evt_external_input,
	evt_last,
    };

    /* supervisor */
    word_t msr;		// machine status
    word_t pvr;		// proc version
    word_t pir;		// proc id
    word_t ccr0;	// core config reg 0
    word_t ccr1;	// core config reg 1
    word_t rstcfg;	// reset config
    word_t sprg[8];	// spr
    word_t esr;		// exc syndrome
    word_t mcsr;	// machine check syndrome
    word_t dear;	// data exception address
    word_t srr0, srr1;	// save/restore
    word_t csrr0, csrr1;// critical save/restore
    word_t mcsrr0, mcsrr1; // machine check save/restore
    word_t ivpr;	// int vector prefix
    word_t ivor[16];	// int vector offset
    word_t tbu, tbl;	// time base
    ppc_tcr_t tcr;	// timer ctrl
    ppc_tsr_t tsr;	// timer status
    word_t dec;		// decrementer
    word_t decar;	// decrememter auto-reload
    word_t ivlim;	// instruction cache victim limit
    word_t inv[4];	// instruction cache normal victim
    word_t itv[4];	// instruction cache transient victim
    word_t dvlim;	// data cache victim limit
    word_t dnv[4];	// data cache normal victim
    word_t dtv[4];	// data cache transient victim
    word_t pid;		// process id
    ppc_mmucr_t mmucr;	// mmu control
    word_t dbsr;	// debug status
    word_t dbdr;	// debug data
    word_t dbcr[3];	// debug control
    word_t dac[2];	// data address compare
    word_t dvc[2];	// data value compare
    word_t iac[4];	// instruction address compare
    word_t icdbdr;	// instruction cache debug data
    word_t icdbt[2];	// instruction cache debug tag
    word_t dcdbt[2];	// data cache debug tag

    word_t event_inject;	// injection of events

    class tlb_t {
    public:
        /* WARNING: If you change class layout, adopt ctrlxfer_{get,set} ! */
        ppc_tlb0_t tlb0;
        ppc_tlb1_t tlb1;
        ppc_tlb2_t tlb2;
        word_t     pid;
        ppc_tlb1_t phys_tlb1; /* phys shadow */
        
        bool       touched;	/* true if entry was ever fetched in host TLB */
        

	bool vaddr_in_entry(word_t vaddr, u8_t searchpid)
	    {
		return tlb0.is_valid() &&
		    tlb0.is_vaddr_covered(vaddr) &&
		    (pid == 0 || pid == searchpid);
	    }

	void touch(unsigned index)
	    { touched = true; }
        
        word_t ctrlxfer_get(word_t reg) { return ((word_t *) &tlb0)[reg]; }
        void ctrlxfer_set(word_t reg, word_t val) { ((word_t *) &tlb0)[reg] = val; }

    } __attribute__((packed));

    /* TLB */
    tlb_t tlb[PPC_MAX_TLB_ENTRIES];
    tlb_t shadow_tlb;
    u8_t shadow_ref;

    bool htlb_dirty;
    word_t tlb_dirty_start;
    word_t tlb_dirty_end;

    /* Timer facilities */
    u64_t dec_base;
    u64_t watchdog_base;
    u64_t fixed_interval_base;

private:
    bool mfmsr(ppc_instr_t instr, except_regs_t *regs);
    bool mtmsr(ppc_instr_t instr, except_regs_t *regs);
    bool mfspr(ppc_instr_t instr, except_regs_t *regs);
    bool mtspr(ppc_instr_t instr, except_regs_t *regs);
    bool tlbre(ppc_instr_t instr, except_regs_t *regs);
    bool tlbsx(ppc_instr_t instr, except_regs_t *regs);
    bool tlbsync(ppc_instr_t instr, except_regs_t *regs);
    bool tlbwe(ppc_instr_t instr, except_regs_t *regs);
    bool tw(ppc_instr_t instr, except_regs_t *regs);
    bool wrtee(ppc_instr_t instr, except_regs_t *regs);
    bool wrteei(ppc_instr_t instr, except_regs_t *regs);
    bool rfi(ppc_instr_t instr, except_regs_t *regs, word_t srr0, word_t srr1);
    bool twi(ppc_instr_t instr, except_regs_t *regs);

    bool set_esr(word_t val, except_regs_t *regs);
    void set_msr(word_t val);
    word_t *get_spr(int spr, bool read);

    void inject_pending_events(except_regs_t *regs);
    void update_decrementer();

public:
    bool is_user()
	{ return msr & (1U << MSR_PR); }

    /* emulation functions */
    bool in_shadow_tlb(word_t vaddr)
	{
	    return shadow_tlb.vaddr_in_entry(vaddr, pid);
	}

    void invalidate_shadow_tlb()
	{
	    if (!shadow_tlb.tlb0.is_valid())
		return;

	    if (tlb[shadow_ref].tlb0.raw != shadow_tlb.tlb0.raw ||
		tlb[shadow_ref].tlb1.raw != shadow_tlb.tlb1.raw ||
		tlb[shadow_ref].tlb2.raw != shadow_tlb.tlb2.raw)
	    {
#if 0
		if (shadow_tlb.touched)
		    TRACEF("invalidate shadow TLB and flush (entry: %d, %08x, %08x, %08x, dirty: %d)\n", 
			   shadow_ref, shadow_tlb.tlb0.raw, shadow_tlb.tlb1.raw, shadow_tlb.tlb2.raw, shadow_tlb.touched);
#endif
		update_tlb_dirty(&shadow_tlb);
	    }
	    shadow_tlb.tlb0 = ppc_tlb0_t::invalid();
	}

    void replace_shadow_tlb(tlb_t *tlbentry, int idx)
	{
	    update_tlb_dirty(&shadow_tlb);
#if 0	    
	    shadow_ref = idx;
	    shadow_tlb = *tlbentry;
#else
	    shadow_tlb.tlb0 = ppc_tlb0_t::invalid();
#endif
	    shadow_tlb.touched = false;
	}

    void update_tlb_dirty(tlb_t *tlb)
	{
	    if (tlb->touched)
	    {
		htlb_dirty = true;
		tlb_dirty_start = tlb->tlb0.get_vaddr();
		tlb_dirty_end = tlb_dirty_start + tlb->tlb0.get_size() - 1;
		tlb->touched = false;
	    }
	}
    
    void clear_tlb_dirty()
	{
	    shadow_tlb.touched = false;
	    for (int i = 0; i < PPC_MAX_TLB_ENTRIES; i++)
		tlb[i].touched = false;
	}

    int find_tlb_entry(word_t vaddr)
	{
	    for (int i = 0; i < PPC_MAX_TLB_ENTRIES; i++) 
		if (tlb[i].vaddr_in_entry(vaddr, pid))
		    return i;
	    return -1;
	}

    word_t get_ivor_ip(int index)
	{
	    return ivpr | ivor[index];
	}

    void raise_exception(int num, except_regs_t *regs);
    void raise_noncrit_interrupt(int num, except_regs_t *regs);
    void raise_crit_interrupt(int num, except_regs_t *regs);
    void raise_mcheck_interrupt(int num, except_regs_t *regs);

    bool emulate_instruction(word_t instr, except_regs_t *regs);
    void update_timers(u64_t time);
    void handle_pending_events(except_regs_t *regs)
	{
	    if ((event_inject & (1 << evt_last) - 1) || tsr.pending_irqs())
		inject_pending_events(regs);
	}

    void load_guest_sprs()
	{
	    ppc_set_spr(SPR_SPRG4, sprg[4]);
	    ppc_set_spr(SPR_SPRG5, sprg[5]);
	    ppc_set_spr(SPR_SPRG6, sprg[6]);
	    ppc_set_spr(SPR_SPRG7, sprg[7]);
	}

    word_t get_pid_for_msr()
	{ return ppc_is_kernel_mode(msr) ? 2 : 1; }

    void init();
};
