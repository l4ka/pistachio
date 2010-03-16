/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     src/arch/powerpc/softhvm.cc
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
#include <kdb/tracepoints.h>
#include INC_ARCH(softhvm.h)

#define TRACE_EMUL(args...)

DECLARE_TRACEPOINT(PPC_HVM_EMUL_MFMSR);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_MTMSR);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_MFSPR);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_MTSPR);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_TLBWE);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_TLBRE);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_TLBSX);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_WRTEE);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_WRTEEI);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_RFI);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_TWI);

void ppc_softhvm_t::init()
{
    // clear the VTLB shadows
    for (int i = 0; i < PPC_MAX_TLB_ENTRIES; i++)
	tlb[i].phys_tlb1.raw = ~0U;

    // disable all timers
    dec_base = ~0ULL;
    watchdog_base = ~0ULL;
    fixed_interval_base = ~0ULL;
}

NOINLINE void ppc_softhvm_t::raise_exception(int num, except_regs_t *regs)
{
    TRACE_EMUL("raise exception %d\n", num);
    ASSERT(num < 16);

    srr0 = regs->srr0_ip;
    srr1 = msr;
    set_msr(msr & ((1 << MSR_CE) | (1 << MSR_ME) | (1 << MSR_DE)));
    regs->srr0_ip = get_ivor_ip(num);
    
}

NOINLINE void ppc_softhvm_t::raise_crit_interrupt(int num, except_regs_t *regs)
{
    TRACE_EMUL("raise critical interrupt %d\n", num);
    ASSERT(num < 16);
    csrr0 = regs->srr0_ip;
    csrr1 = msr;
    set_msr(msr & (1 << MSR_ME));
    regs->srr0_ip = get_ivor_ip(num);
}

NOINLINE void ppc_softhvm_t::raise_mcheck_interrupt(int num, except_regs_t *regs)
{
    TRACE_EMUL("raise mcheck interrupt %d\n", num);
    ASSERT(num < 16);
    mcsrr0 = regs->srr0_ip;
    mcsrr1 = msr;
    set_msr(0);
    regs->srr0_ip = get_ivor_ip(num);
}

NOINLINE void ppc_softhvm_t::set_msr(word_t val)
{ 
    word_t oldmsr = msr;
    msr = val;
    
    if (msr & (1 << MSR_IS | 1 << MSR_DS))
	UNIMPLEMENTED();

    if ((oldmsr & (1 << MSR_PR)) != (msr & (1 << MSR_PR)))
        ppc_set_pid(get_pid_for_msr());     
}

inline bool ppc_softhvm_t::mfmsr(ppc_instr_t instr, except_regs_t *regs)
{
    TRACEPOINT(PPC_HVM_EMUL_MFMSR, "mfmsr r%d, %08x", instr.rt(), msr);

    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	regs->set_register(instr.rt(), msr);
	regs->srr0_ip += sizeof(instr);
    }
    return true;
}

inline bool ppc_softhvm_t::mtmsr(ppc_instr_t instr, except_regs_t *regs)
{
    TRACEPOINT(PPC_HVM_EMUL_MTMSR, "mtmsr %08x -> r%d", msr, instr.rt());

    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	msr = regs->get_register(instr.rt());
	regs->srr0_ip += sizeof(instr);
    }
    return true;
}

word_t *ppc_softhvm_t::get_spr(int spr, bool read)
{
    switch(spr) {
    case 0x16: return &this->dec;
    case 0x1a: return &this->srr0;
    case 0x1b: return &this->srr1;
    case 0x30: return &this->pid;
    case 0x36: return read ? NULL : &this->decar;
    case 0x3a: return &this->csrr0;
    case 0x3b: return &this->csrr1;
    case 0x3d: return &this->dear;
    case 0x3e: return &this->esr;
    case 0x3f: return &this->ivpr;
    case 0x110 ... 0x117: return &this->sprg[spr - 0x110];
    case 0x11c: return read ? NULL : &this->tbl;
    case 0x11d: return read ? NULL : &this->tbu;
    case 0x11e: return read ? &this->pir : NULL;
    case 0x11f: return read ? &this->pvr : NULL;
    case 0x130: return read ? &this->dbsr : NULL;
    case 0x134 ... 0x136: return &this->dbcr[spr - 0x134];
    case 0x138 ... 0x13b: return &this->iac[spr - 0x138];
    case 0x13c ... 0x13d: return &this->dac[spr - 0x13c];
    case 0x13e ... 0x13f: return &this->dvc[spr - 0x13e];
    case 0x150: return &this->tsr.raw;
    case 0x154: return &this->tcr.raw;
    case 0x190 ... 0x19f: return &this->ivor[spr - 0x190];
    case 0x23a: return &this->mcsrr0;
    case 0x23b: return &this->mcsrr1;
    case 0x23c: return &this->mcsr;
    case 0x370 ... 0x373: return &this->inv[spr - 0x370];
    case 0x374 ... 0x377: return &this->itv[spr - 0x374];
    case 0x378: return &this->ccr1;
    case 0x390 ... 0x393: return &this->dnv[spr - 0x390];
    case 0x394 ... 0x397: return &this->dtv[spr - 0x394];
    case 0x398: return &this->dvlim;
    case 0x399: return &this->ivlim;
    case 0x39b: return read ? &this->rstcfg : NULL;
    case 0x39c ... 0x39d: return read ? &this->dcdbt[spr - 0x39c] : NULL;
    case 0x39e ... 0x39f: return read ? &this->icdbt[spr - 0x39c] : NULL;
    case 0x3b2: return &this->mmucr.raw;
    case 0x3b3: return &this->ccr0;
    case 0x3d3: return &this->icdbdr;
    case 0x3f3: return &this->dbdr;
    default: return NULL;
    }
}

inline bool ppc_softhvm_t::mfspr(ppc_instr_t instr, except_regs_t *regs)
{
    unsigned spridx = instr.rf();

    TRACEPOINT(PPC_HVM_EMUL_MFSPR, "mfspr %u/%x -> reg:%u (IP %08x)", 
	       spridx, spridx, instr.rt(), regs->srr0_ip);

    /* we assume that accesses to user SPRs don't come here */
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	word_t *spr = get_spr(spridx, true);
	if (!spr)
	    return false;

	regs->set_register(instr.rt(), *spr);
	regs->srr0_ip += sizeof(instr);
    }
    return true;
}

inline bool ppc_softhvm_t::mtspr(ppc_instr_t instr, except_regs_t *regs)
{
    unsigned spridx = instr.rf();

    TRACEPOINT(PPC_HVM_EMUL_MTSPR, "mtspr %u/%x <- reg:%u (IP %08x)", 
	       spridx, spridx, instr.rt(), regs->srr0_ip);
    
    /* we assume that accesses to user SPRs don't come here */
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	word_t *spr = get_spr(spridx, false);
	if (!spr)
	    return false;

	word_t val = regs->get_register(instr.rt());

	// handle write-clear special cases
	switch (spridx)
	{
	case 0x16:
	    dec_base = ppc_get_timebase();
	    decar = val;
	    break;
	case 0x130:
	case 0x150:
	case 0x23c: *spr = *spr & ~val; break;
	case 0x30: 
	    TRACE_EMUL("setting PID to %x\n", val);
	    clear_tlb_dirty();
	    htlb_dirty = true; // pid change--flush host TLB
	    tlb_dirty_start = 0;
	    tlb_dirty_end = ~0U;
	    /* fall through */
	default: *spr = val;
	}

	regs->srr0_ip += sizeof(instr);
	load_guest_sprs();
    }
    return true;
}

inline bool ppc_softhvm_t::tlbre(ppc_instr_t instr, except_regs_t *regs)
{
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	word_t val = 0;
	int idx = regs->get_register(instr.ra());

	if (idx < PPC_MAX_TLB_ENTRIES)
	    switch(instr.rb())
	    {
	    case 0:
		val = tlb[idx].tlb0.raw;
		mmucr.search_id = tlb[idx].pid;
		mmucr.search_translation_space = 0; // XXX
		break;
	    case 1: val = tlb[idx].tlb1.raw; break;
	    case 2: val = tlb[idx].tlb2.raw; break;
	    }
	regs->set_register(instr.rt(), val);
	regs->srr0_ip += sizeof(instr);

	TRACEPOINT(PPC_HVM_EMUL_TLBRE, "tlbre [%02d:%d] val=%08x", idx, instr.rb(), val); 
    }
    return true;
}

inline bool ppc_softhvm_t::tlbsx(ppc_instr_t instr, except_regs_t *regs)
{
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	if (instr.xform.rc)
	{
	    regs->cr &= ~(0xf << 28);
	    if (regs->xer & 80000000) regs->cr |= (1 << 28); // XER OV
	}
	word_t eaddr = instr.ra() == 0 ? 0 : regs->get_register(instr.ra());
	eaddr += regs->get_register(instr.rb());
	int idx = find_tlb_entry(eaddr);
	if (idx >= 0) 
	{
	    regs->set_register(instr.rt(), idx);
	    regs->cr |= (2 << 28);
	}
	regs->srr0_ip += sizeof(instr);
	TRACEPOINT(PPC_HVM_EMUL_TLBSX, "tlbsx %x -> %s, idx: %d", 
		   eaddr, idx >= 0 ? "found" : "not found", idx);
    }
    return true;
}

inline bool ppc_softhvm_t::tlbsync(ppc_instr_t instr, except_regs_t *regs)
{
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	/* no-op on 440 */
	regs->srr0_ip += sizeof(instr);
    }
    return true;
}

inline bool ppc_softhvm_t::tlbwe(ppc_instr_t instr, except_regs_t *regs)
{
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	int idx = regs->get_register(instr.ra());
	if (idx < PPC_MAX_TLB_ENTRIES)
	{
	    update_tlb_dirty(&tlb[idx]);
	    word_t val = regs->get_register(instr.rt());
	    switch(instr.rb())
	    {
	    case 0:
		tlb[idx].tlb0.raw = val;
		tlb[idx].pid = mmucr.search_id;
		break;
	    case 1: tlb[idx].tlb1.raw = val; break;
	    case 2: tlb[idx].tlb2.raw = val; break;
	    }
	    TRACEPOINT(PPC_HVM_EMUL_TLBWE, "tlbwe entry=[%02d:%d], val=%08x\n", idx, instr.rb(), val);
	}
	regs->srr0_ip += sizeof(instr);
    }
    return true;
}

bool ppc_softhvm_t::set_esr(word_t val, except_regs_t *regs)
{
    TRACE_EMUL("set_esr: %x (%d)\n", val & (1 << MSR_EE) ? 1 : 0);
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	const word_t mask = (1U << MSR_EE);
	set_msr(msr & ~mask | (val & mask));
	regs->srr0_ip += sizeof(ppc_instr_t);
    }
    return true;
}

inline bool ppc_softhvm_t::wrtee(ppc_instr_t instr, except_regs_t *regs)
{
    TRACEPOINT(PPC_HVM_EMUL_WRTEE, "wrtee %08x", regs->get_register(instr.rt()));
    return set_esr(regs->get_register(instr.xfxform.rt), regs);
}

inline bool ppc_softhvm_t::wrteei(ppc_instr_t instr, except_regs_t *regs)
{
    TRACEPOINT(PPC_HVM_EMUL_WRTEEI, "wrteei");
    return set_esr(instr.raw, regs);
}

inline bool ppc_softhvm_t::rfi(ppc_instr_t instr, except_regs_t *regs, 
			       word_t srr0, word_t srr1)
{
    TRACEPOINT(PPC_HVM_EMUL_RFI, "rfi -> srr0=%08x, srr1=%08x", srr0, srr1);
    if (is_user())
	raise_exception(exc_program, regs);
    else
    {
	invalidate_shadow_tlb();
	regs->srr0_ip = srr0;
	set_msr(srr1);
    }
    return true;
}

inline bool ppc_softhvm_t::twi(ppc_instr_t instr, except_regs_t *regs)
{
    TRACEPOINT(PPC_HVM_EMUL_TWI, "twi");

    ppc_esr_t newesr;
    newesr.raw = 0;
    newesr.x.trap = 1;

    raise_exception (exc_program, regs);
    this->esr = newesr.raw;

    return true;
}

bool ppc_softhvm_t::emulate_instruction(word_t opcode, except_regs_t *regs)
{
    ppc_instr_t instr(opcode);

    TRACE_EMUL("emulate instruction (%08x): prim: %d, sec: %d, sz: %d\n",
	       instr.raw, instr.get_primary(), instr.get_secondary(), sizeof(instr));

    switch(instr.get_primary())
    {
    case 31:
	switch(instr.get_secondary()) {
	case  83: return mfmsr(instr, regs);
	case 339: return mfspr(instr, regs);
	case 146: return mtmsr(instr, regs);
	case 467: return mtspr(instr, regs);
	case 946: return tlbre(instr, regs);
	case 914: return tlbsx(instr, regs);
	case 566: return tlbsync(instr, regs);
	case 978: return tlbwe(instr, regs);
	case 131: return wrtee(instr, regs);
	case 163: return wrteei(instr, regs);
	case 966: asm volatile ("iccci 0,0" : : : "memory"); return false;
	};
	break;

    case 19:
	switch(instr.get_secondary()) {
	case 38: return rfi(instr, regs, this->mcsrr0, this->mcsrr1);
	case 50: return rfi(instr, regs, this->srr0, this->srr1);
	case 51: return rfi(instr, regs, this->csrr0, this->csrr1);
	};
	break;

    case 3: return twi(instr, regs);
    }
    return false;
}

void ppc_softhvm_t::update_timers(u64_t time)
{
    if (dec_base <= time - decar)
    {
	//enter_kdebug("decrementer underflow");
	tsr.decrementer_irq_status = 1;
	if (tcr.auto_reload)
	    dec_base += decar;
	else
	    dec_base = ~0ULL;	// disable dec
    }
    dec = (dec_base <= time - decar) ? time - dec_base : 0;

    if (watchdog_base <= time - tcr.get_watchdog_period())
    {
	enter_kdebug("watchdog timer");
	watchdog_base += tcr.get_watchdog_period();
	tsr.watchdog_irq_status = 1;
    }

    if (fixed_interval_base <= time - tcr.get_fixed_interval_period())
    {
	enter_kdebug("fixed interval timer");
	fixed_interval_base += tcr.get_fixed_interval_period();
	tsr.fixed_interval_irq_status = 1;
    }
}

void ppc_softhvm_t::inject_pending_events(except_regs_t *regs)
{
    if ( (event_inject & (1 << evt_machine_check)) &&
	 (msr & (1 << MSR_ME)) )
    {
	raise_mcheck_interrupt(exc_critical_input, regs);
    } 
    else if ( (event_inject & (1 << evt_debug)) &&
	      (msr & (1 << MSR_DE)) )
    {
	raise_crit_interrupt(exc_critical_input, regs);
    }
    else if ( (event_inject & (1 << evt_critical_input)) &&
	      (msr & (1 << MSR_CE)) )
    {
	raise_crit_interrupt(exc_critical_input, regs);
    }
    else if ( tcr.watchdog_irq_enable && 
	      tsr.watchdog_irq_status &&
	      (msr & (1 << MSR_CE)) )
    {
	/* XXX: add reset state machine */
	raise_crit_interrupt(exc_watchdog, regs);
    }
    else if ( (event_inject & (1 << evt_external_input)) &&
	      (msr & (1 << MSR_EE)) )
    {
	raise_exception(exc_ext_input, regs);
    }
    else if ( tcr.fixed_interval_irq_enable && 
	      tsr.fixed_interval_irq_status &&
	      (msr & (1 << MSR_EE)) )
    {
	enter_kdebug("raise fixed interval IRQ");
	raise_exception(exc_fixed_interval, regs);
    }
    else if ( tcr.dec_irq_enable && 
	      tsr.decrementer_irq_status &&
	      (msr & (1 << MSR_EE)) )
    {
	//TRACEF("raise decrementer IRQ (%x)\n", static_cast<word_t>(dec_base));
	raise_exception(exc_decrementer, regs);
    }
}
