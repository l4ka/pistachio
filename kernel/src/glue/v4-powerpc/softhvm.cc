/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/softhvm.cc
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
#include <linear_ptab.h>
#include <kdb/tracepoints.h>

#include INC_ARCH(phys.h)
#include INC_ARCH(softhvm.h)
#include INC_ARCH(pgent.h)

#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(kernelinterface.h)

#include INC_GLUE(exception.h)

#define TRACE_EMUL(args...)
#define MAX_INSTR_EMULATE	10

extern ppc_swtlb_t swtlb; /* defined in space-swtlb.cc */

DECLARE_TRACEPOINT(PPC_HVM_EXCEPT_PROG);
DECLARE_TRACEPOINT(PPC_HVM_EXCEPT_DECR);
DECLARE_TRACEPOINT(PPC_HVM_DTLB_MISS);
DECLARE_TRACEPOINT(PPC_HVM_ITLB_MISS);
DECLARE_TRACEPOINT(PPC_HVM_EXCEPT_ISI);
DECLARE_TRACEPOINT(PPC_HVM_EXCEPT_DSI);
DECLARE_TRACEPOINT(PPC_HVM_ALIGN);
DECLARE_TRACEPOINT(PPC_HVM_EMUL_INTERPRET);

INLINE void try_to_debug( except_regs_t *regs, word_t exc_no, word_t dar=0, word_t dsisr=0 )
{
    if( EXPECT_TRUE(get_kip()->kdebug_entry == NULL) )
	return;

    debug_param_t param;

    param.exception = exc_no;
    param.frame = regs;
    param.tcb = get_current_tcb();
    param.space = (get_current_space() ? get_current_space() : get_kernel_space());
    param.dar = dar;
    param.dsisr = dsisr;

    get_kip()->kdebug_entry( (void *)&param );
#ifdef CONFIG_KDB_BREAKIN
    kdebug_check_breakin();
#endif
}

/* assumption: running in context of HVM, IP is guest-virtual */
INLINE bool read_hvm_instruction(word_t ip, word_t &instr, bool speculative = false)
{
    word_t origmsr, newmsr, idx;

    if (EXPECT_FALSE(speculative))
    {
	ppc_mmucr_t::write_search_id(2, 1);
	if (!ppc_tlbsx(ip, idx))
	    return false;
    }

    /* read the instruction */
    asm volatile ("mfmsr %[origmsr]\n"
		  "ori %[newmsr], %[origmsr], %[dts1]\n"
		  "mtmsr %[newmsr]\n"
		  "isync\n"
		  "lwz %[instr], 0(%[ip])\n"
		  "mtmsr %[origmsr]\n"
		  "isync\n"
		  : [instr] "=r" (instr), [origmsr] "=&r" (origmsr), [newmsr] "=&r"(newmsr)
		  : [dts1] "i"(1 << MSR_DS), [ip] "b" (ip));
    return true;
}

NOINLINE bool 
arch_ktcb_t::send_hvm_fault(softhvm_t::exit_reason_e exc, except_regs_t *frame, word_t instr, 
			    word_t param, bool internal)
{
    const int untyped = 4;
    tcb_t *tcb = addr_to_tcb(this);
    acceptor_t acceptor;

    msg_tag_t tag = softhvm_t::fault_tag(exc, untyped, internal);
    tag.x.typed = tcb->append_ctrlxfer_item(tag, untyped + 1);
    tcb->set_tag(tag);
    tcb->set_mr(1, instr);
    tcb->set_mr(2, frame->srr0_ip);
    tcb->set_mr(3, vm->msr);
    tcb->set_mr(4, param);

    acceptor.clear();
    acceptor.x.ctrlxfer = 1;
    tcb->set_br(0, acceptor.raw);

    threadid_t partner = tcb->get_pager();
    tag = tcb->do_ipc (partner, partner, timeout_t::never());

    if (tag.is_error())
	enter_kdebug("hvm fault send failed");

    return !tag.is_error();
}

NOINLINE bool
arch_ktcb_t::send_hvm_pagefault(softhvm_t::exit_reason_e exc, except_regs_t *frame,
                                word_t addr, word_t instr,
				                word_t tlb0, word_t tlb1, word_t tlb2, u8_t pid, u8_t idx,
				                bool read, bool write, bool execute)
{
    const int untyped = 8;
    tcb_t *tcb = addr_to_tcb(this);
    acceptor_t acceptor = 0;

    msg_tag_t tag = softhvm_t::pagefault_tag(exc, untyped, read, write, execute);
    tag.x.typed = tcb->append_ctrlxfer_item(tag, untyped + 1);
    tcb->set_tag(tag);
    tcb->set_mr(1, addr);
    tcb->set_mr(2, frame->srr0_ip);
    tcb->set_mr(3, instr);
    tcb->set_mr(4, vm->msr);
    tcb->set_mr(5, tlb0);
    tcb->set_mr(6, tlb1);
    tcb->set_mr(7, tlb2);
    tcb->set_mr(8, (word_t) pid << 8 | idx);

    acceptor.set_rcv_window(fpage_t::complete_mem());
    acceptor.x.ctrlxfer = 1;
    tcb->set_br(0, acceptor.raw);

    threadid_t partner = tcb->get_pager();
    tag = tcb->do_ipc (partner, partner, timeout_t::never());

    if (tag.is_error())
	enter_kdebug("hvm pagefault send failed");

    return !tag.is_error();
}

static void check_tlb( ppc_softhvm_t *vm )
{
#if 0
    if (vm->htlb_dirty)
	enter_kdebug("TLB still dirty?");

    for (word_t idx = 0; idx < PPC_MAX_TLB_ENTRIES; idx++)
    {
	ppc_tlb0_t tlb0;
	tlb0.read(idx);
	if (tlb0.is_valid() && tlb0.trans_space == 1)
	{
	    bool found = false;
	    for (int entry = 0; entry < PPC_MAX_TLB_ENTRIES; entry++)
		if (vm->tlb[entry].vaddr_in_entry(tlb0.get_vaddr(), vm->pid))
		    found = true;
	    if (vm->shadow_tlb.vaddr_in_entry(tlb0.get_vaddr(), vm->pid))
		found = true;
	    if (!found)
	    {
		printf("invalid host-guest mapping found in TLB: %d\n", idx);
		enter_kdebug("invalid host mapping");
	    }
	}
    }
#endif
}

static void update_tlb_hvm( ppc_softhvm_t * vm )
{
    if (EXPECT_FALSE(vm->htlb_dirty))
    {
	space_t *space = get_current_space();
	space->flush_tlb_hvm(space, vm->tlb_dirty_start, vm->tlb_dirty_end);
	vm->htlb_dirty = false;
    }
}

NOINLINE void space_t::flush_tlb_hvm( space_t *curspace, word_t start, word_t end )
{
    //TRACEF("flush %x - %x\n", start, end);
    for (word_t idx = 0; idx < swtlb.high_water; idx++)
    {
	ppc_tlb0_t tlb0;
	tlb0.read(idx);

	if (!tlb0.is_valid())
	    continue;

	if ( (tlb0.trans_space == 1) &&
	     !((tlb0.get_vaddr() >= end) ||
	       (tlb0.get_vaddr() + tlb0.get_size() - 1) <= start) )
	{
	    ppc_tlb0_t::invalid().write(idx);
	    swtlb.set_free(idx);
	}
    }
    isync();
}

NOINLINE bool 
space_t::handle_hvm_tlb_miss(ppc_softhvm_t *vm, ppc_softhvm_t::tlb_t *tlbentry, word_t gvaddr, paddr_t &gpaddr)
{
    TRACE_EMUL("Inserting GV-GP TLB entry from shadow TLB: %08x %08x %08x pid=%x\n", 
	       tlbentry->tlb0.raw, tlbentry->tlb1.raw, tlbentry->tlb2.raw, tlbentry->pid);
    
    ppc_tlb1_t tlb1(tlbentry->phys_tlb1);

    size_t gsize = tlbentry->tlb0.get_log2size();
    gpaddr = tlbentry->tlb1.get_paddr() | (gvaddr & (tlbentry->tlb0.get_size() - 1));

    if (gpaddr >= USER_AREA_END)
	return false;

    TRACE_EMUL("GVA:%lx GPA:%lx.%lx\n", gvaddr, static_cast<word_t>(gpaddr >> 32), static_cast<word_t>(gpaddr));

    pgent_t *pg;
    pgent_t::pgsize_e pgsize;
    if (!this->lookup_mapping ((addr_t) gpaddr, &pg, &pgsize))
	return false;
    
    size_t hsize = page_shift (pgsize);
    paddr_t hpaddr = pg->address (this, pgsize) | (gpaddr & ((1ull << hsize) - 1));
	
    /* we have a valid entry in the TLB and in the ptab --> we
     * can create a TLB entry */
    TRACE_EMUL("mapping found (%p): %p -> %x.%08x (%x)\n",
	       pg, gpaddr, (word_t)(hpaddr >> 32), (word_t)hpaddr, pgsize);
    TRACE_EMUL("[%c%c%c], cache=%x, erpn=%x\n", 
	       pg->map.read ? 'R' : ' ', pg->map.write ? 'W' : ' ',
	       pg->map.execute ? 'X' : ' ', pg->map.caching, pg->map.erpn);
    
    size_t size = min (gsize, hsize);
    while (!ppc_tlb0_t::is_valid_pagesize (size))
        size--;

    ppc_tlb0_t tlb0 (gvaddr & ~((1ul << size) - 1), size);
    tlb0.trans_space = 1;

    tlb1.init_paddr (hpaddr & ~((1ull << size) - 1));

    ppc_tlb2_t tlb2;
    tlb2.raw = tlbentry->tlb2.raw;

    if (tlb2.is_accessible()) // don't bother if no access rights are set...
    {
	/* we support three protection modes right now:
	 *   user and kernel have access rights: pid0
	 *   only user: pid1
	 *   only kernel: pid2
	 */

	word_t pid = 0; // kernel+user accessible

	if (!tlb2.is_user_accessible())
	    pid = 2;
	else if (!tlb2.is_kernel_accessible())
	    pid = 1;

	// move kernel access permissions into user part
	tlb2.raw |= (tlb2.raw & 0x7) << 3;

	// fix up cache attributes
#warning fix cache attribute setting
	tlb2.raw &= 0x3f;
	tlb2.mem_coherency = 1;
	tlb2.wt_l1 = 1;
	tlb2.user2 = 1;

	word_t hwtlb_index = swtlb.allocate();

	TRACE_EMUL("inserting TLB entry %d: %08x, %08x, %08x, pid=%d (%x)\n",
		   hwtlb_index, tlb0.raw, tlb1.raw, tlb2.raw, pid, 
		   (&vm->shadow_tlb == tlbentry) ? 255 : (tlbentry - &vm->tlb[0]) / sizeof(ppc_softhvm_t::tlb_t));

	ppc_mmucr_t::write_search_id(pid);
	tlb0.write(hwtlb_index);
	tlb1.write(hwtlb_index);
	tlb2.write(hwtlb_index);
	isync();

	/* flush icache to avoid alias problems after PID change */
	if (tlb2.user_execute)
	    asm volatile ("iccci 0,0" : : : "memory");

	/* mark entry dirty in VTLB */
	tlbentry->touch(hwtlb_index);
    }
    return true;
}

EXCDEF( hvm_dtlb_miss_handler )
{
    tcb_t *tcb = get_current_tcb();
    ppc_softhvm_t *vm = tcb->get_arch()->vm;
    word_t dear = ppc_get_spr(SPR_DEAR);
    ppc_esr_t esr; esr.read();
    paddr_t gpaddr;

    TRACEPOINT(PPC_HVM_DTLB_MISS,
	       "HVM DTLB MISS: IP: %08x, DEAR: %lx",
	       srr0, dear);

    if (ppc_is_kernel_mode(srr1))
    {
        if (!get_kernel_space()->handle_tlb_miss((addr_t)dear, (addr_t)dear, false, true))
            panic("kernel accessed unmapped device @ %08x, IP %08x\n", dear, srr0);
        return_except();
    }
    
    ppc_softhvm_t::tlb_t *tlbentry = NULL;
    int tlbidx = -1;

    if (vm->in_shadow_tlb(dear))
	tlbentry = &vm->shadow_tlb;
    else if ( (tlbidx = vm->find_tlb_entry (dear)) != -1 )
	tlbentry = &vm->tlb[tlbidx];

    if (!tlbentry)
    {
	vm->dear = dear;
	vm->esr = esr.raw;      // XXX: Interpret this correctly!
	vm->raise_exception(ppc_softhvm_t::exc_data_tlb, frame);
    }
    else
    {
	if (tcb->get_space()->handle_hvm_tlb_miss(vm, tlbentry, dear, gpaddr))
	{
	    if (tlbidx != -1)
	    {
		vm->replace_shadow_tlb(tlbentry, tlbidx);
		update_tlb_hvm(vm);
	    }
	}
	else
	{
	    word_t instr;
	    read_hvm_instruction(frame->srr0_ip, instr);

	    tcb->get_arch()->send_hvm_pagefault(
		softhvm_t::er_tlb, frame, dear, instr,
		tlbentry->tlb0.raw, tlbentry->tlb1.raw, tlbentry->tlb2.raw,
		tlbentry->pid, tlbidx, esr.x.store == 0, esr.x.store != 0, false);
	}
    }

    vm->handle_pending_events(frame);
    check_tlb(vm);
    return_except();
}

EXCDEF( hvm_itlb_miss_handler )
{
    tcb_t *tcb = get_current_tcb();
    ppc_softhvm_t *vm = tcb->get_arch()->vm;
    paddr_t gpaddr;

    TRACEPOINT(PPC_HVM_ITLB_MISS,
	       "HVM ITLB MISS: IP: %08x, LR: %08x",
	       srr0, frame->lr);

    ASSERT(!ppc_is_kernel_mode(srr1));

    ppc_softhvm_t::tlb_t *tlbentry = NULL;
    int tlbidx = -1;

    if (vm->in_shadow_tlb(srr0))
	tlbentry = &vm->shadow_tlb;
    else if ( (tlbidx = vm->find_tlb_entry (srr0)) != -1 )
	tlbentry = &vm->tlb[tlbidx];

    if (!tlbentry)
    {
	vm->raise_exception(ppc_softhvm_t::exc_instr_tlb, frame);
    }
    else 
    {
	if (tcb->get_space()->handle_hvm_tlb_miss(vm, tlbentry, srr0, gpaddr))
	{
	    if (tlbidx != -1)
	    {
		vm->replace_shadow_tlb(tlbentry, tlbidx);
		update_tlb_hvm(vm);
	    }
	}
	else
	{
	    tcb->get_arch()->send_hvm_pagefault(
		softhvm_t::er_tlb, frame, srr0, 0,
		tlbentry->tlb0.raw, tlbentry->tlb1.raw, tlbentry->tlb2.raw,
		tlbentry->pid, tlbidx, false, false, true);
	}
    }

    vm->handle_pending_events(frame);
    check_tlb(vm);
    return_except();
}

EXCDEF( hvm_dsi_handler )
{
    /* just reflect the exception back to the guest, the VTLB should
     * take care of this */

    ppc_esr_t esr; esr.read();
    word_t dear = ppc_get_spr(SPR_DEAR);

    TRACEPOINT(PPC_HVM_EXCEPT_DSI,
	       "HVM DSI EXCEPT: IP: %08x, LR: %08x, DEAR: %08x, ESR: %08x",
	       srr0, frame->lr, dear, esr.raw);

    ASSERT(!ppc_is_kernel_mode(frame->srr1_flags));

    ppc_softhvm_t *vm = get_current_tcb()->get_arch()->vm;

    vm->esr = esr.raw;
    vm->dear = dear;
    vm->raise_exception(ppc_softhvm_t::exc_data_storage, frame);
    check_tlb(vm);
 
    return_except();
}

EXCDEF( hvm_isi_handler )
{
    /* just reflect the exception back to the guest, the VTLB should
     * take care of this */

    ppc_esr_t esr; esr.read();

    TRACEPOINT(PPC_HVM_EXCEPT_ISI,
	       "HVM ISI EXCEPT: IP: %08x, LR: %08x, ESR: %08x",
	       srr0, frame->lr, esr.raw);

    ASSERT(!ppc_is_kernel_mode(frame->srr1_flags));
    
    ppc_softhvm_t *vm = get_current_tcb()->get_arch()->vm;

    vm->esr = esr.raw;
    vm->raise_exception(ppc_softhvm_t::exc_instr_storage, frame);
    check_tlb(vm);
 
    return_except();
}

EXCDEF( hvm_alignment_handler )
{
    ppc_esr_t esr; esr.read();
    word_t dear = ppc_get_spr(SPR_DEAR);

    TRACEPOINT(PPC_HVM_ALIGN,
	       "HVM ALIGNMENT: IP: %08x, LR: %08x, DEAR: %08x, ESR: %08x",
	       srr0, frame->lr, dear, esr.raw);
    
    ASSERT(!ppc_is_kernel_mode(frame->srr1_flags));

    ppc_softhvm_t *vm = get_current_tcb()->get_arch()->vm;

    vm->esr = esr.raw;
    vm->dear = dear;
    vm->raise_exception(ppc_softhvm_t::exc_alignment, frame);
    check_tlb(vm);
 
    return_except();
}

EXCDEF( hvm_program_handler )
{
    if( ppc_is_kernel_mode(frame->srr1_flags) )
    {
	if( get_kip()->kdebug_entry )
	{
	    // If the debugger exists, let it have the first try at handling
	    // the exception.
	    word_t start_ip = frame->srr0_ip;
	    word_t start_flags = frame->srr1_flags;
	    
	    try_to_debug( frame, EXCEPT_ID(PROGRAM) );
	    
	    if( (frame->srr0_ip != start_ip) || (frame->srr1_flags != start_flags) )
		return_except();	// The kernel debugger handled the exception.
	}
	else
	    panic( "program exception in kernel thread.\n" );
    }
    else
    {
	ppc_esr_t esr; esr.read();
	ppc_softhvm_t *vm = get_current_tcb()->get_arch()->vm;

	TRACEPOINT(PPC_HVM_EXCEPT_PROG, 
		   "PROG EXC: IP %p, MSR %08x, ESR %08x", 
		   frame->srr0_ip, frame->srr1_flags, esr.raw);

	vm->update_timers(ppc_get_timebase());

	if (esr.x.privileged_instr)
	{
	    word_t instr;
	    word_t oldip = frame->srr0_ip;

	    read_hvm_instruction(frame->srr0_ip, instr);

	    TRACE_EMUL("Faulting instruction: %08x @ %08x\n", instr, frame->srr0_ip);

	    if (vm->emulate_instruction(instr, frame))
	    {
#if 0
		/* we put an upper bound on emulation to avoid malicious
		 * tasks to run a DOS with interrupts disabled */
		for (unsigned num = 0; num < MAX_INSTR_EMULATE; num++)
		{
		    /* only emulate privileged code */
		    if (vm->is_user())
			break;

		    /* speculative fetch if TLB is dirty or next instruction
		     * is in different 1k page (i.e., minimal page size) */
		    bool speculative = vm->htlb_dirty | ((frame->srr0_ip & ~0x3ff) != (oldip & ~0x3ff));
		    oldip = frame->srr0_ip;
		    update_tlb_hvm(vm);
		    
		    if (!read_hvm_instruction(frame->srr0_ip, instr, speculative) || 
			!vm->emulate_instruction(instr, frame))
			break;

		    TRACEPOINT(PPC_HVM_EMUL_INTERPRET, "instr emulation opcode %08x, IP %08x", 
			       instr, frame->srr0_ip);
		}
#endif
		update_tlb_hvm(vm);
	    }
	    else
	    {
		//TRACEF("send exception IPC\n");
		get_current_tcb()->get_arch()->
		    send_hvm_fault(softhvm_t::er_program, frame, instr, 0, false);
	    }
	}
	else
	{
	    vm->raise_exception(ppc_softhvm_t::exc_program, frame);
	    vm->esr = esr.raw;

	    if (esr.x.floating_point)
	    {
		enter_kdebug("fpu emulation");
	    } 
	    else if (!esr.x.trap)
	    {
		/* reflect exception? */
		TRACEF("program exception @ %p, ESR: %08x\n", frame->srr0_ip, esr.raw);
		enter_kdebug("prog except");
	    }
	}
	vm->handle_pending_events(frame);
	check_tlb(vm);
    }

    return_except();
}

EXCDEF( hvm_fp_unavail_handler )
{
    tcb_t *current_tcb = get_current_tcb();

    /* FPU can be unavailable for 2 reasons: guest disabled it or it
     * is not restored */
    ppc_softhvm_t *vm = current_tcb->get_arch()->vm;
    if (!(vm->msr & (1 << MSR_FP)))
    {
	vm->raise_exception(ppc_softhvm_t::exc_fpu_unavail, frame);
    }
    else
    {
	/* FPU is enabled in guest but not in host--enable it */
	current_tcb->resources.fpu_unavail_exception( current_tcb );
    }
    return_except();
}

EXCDEF( hvm_syscall_handler )
{
    get_current_tcb()->get_arch()->vm->
	raise_exception(ppc_softhvm_t::exc_system_call, frame);
    return_except();
}

EXCDEF( hvm_debug_handler )
{
    get_current_tcb()->get_arch()->vm->
	raise_exception(ppc_softhvm_t::exc_debug, frame);
    return_except();
}

EXCDEF( hvm_decrementer_handler )
{
    /* Don't go back to sleep if the thread was in power savings mode.
     * We will only see timer interrupts from user mode, or from the sleep
     * function in kernel mode.
     */
    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) ) {
	srr1 = processor_wake( srr1 );
	frame->srr1_flags = srr1;
    }

    TRACEPOINT(PPC_HVM_EXCEPT_DECR, 
	       "Decrementer Intr: IP=%p, MSR %08x", srr0, srr1);

    // BookE uses auto-reload decrementer; just ack
    ppc_tsr_t::dec_irq().write();
    get_current_scheduler()->handle_timer_interrupt();

    // tick the VM and fire necessary interrupts
    ppc_softhvm_t *vm = get_current_tcb()->get_arch()->vm;
    vm->update_timers(ppc_get_timebase());
    vm->handle_pending_events(frame);

    return_except();
}


FEATURESTRING("powerpc-hvm");

DECLARE_KMEM_GROUP(kmem_hvm);
void arch_ktcb_t::init_hvm(tcb_t *tcb)
{
    const int allocsize = ((sizeof(ppc_softhvm_t) / KMEM_CHUNKSIZE) + 1) * KMEM_CHUNKSIZE;
    vm = (ppc_softhvm_t*)kmem.alloc(kmem_hvm, allocsize);
    tcb->resource_bits += SOFTHVM;
    vm->init();
}
