/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/exception.cc
 * Description:   IA-64 exception handling
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
 * $Id: exception.cc,v 1.36 2006/10/19 22:57:39 ud3 Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/tracepoints.h>
#include <kdb/console.h>

#include INC_GLUE(context.h)

#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)

#include INC_ARCH(cr.h)
#include INC_ARCH(instr.h)
#include INC_ARCH(runconv.h)

#include INC_PLAT(system_table.h)
#include INC_PLAT(runtime_services.h)

static char * interruption_names[] = {
    "VHPT Translation",
    "Instruction TLB",
    "Data TLB",
    "Alternate Instruction TLB",
    "Alternate Data TLB",
    "Data Nested TLB",
    "Instruction Key Miss",
    "Data Key Miss",
    "Dirty-Bit",
    "Instruction Access-Bit",
    "Data Access-Bit",
    "Breal Instruction",
    "External Interrupt",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved",
    "Page Not Present",
    "Key Permission",
    "Instruction Access Rights",
    "Data Access Rights",
    "General Exception",
    "Disabled FP-Register",
    "NaT Consumption",
    "Speculation",
    "Reserved 28",
    "Debug",
    "Unaligned Reference",
    "Unsupported Data",
    "Floating-point Fault",
    "Floating-point Trap",
    "Lower-Privilege Transfer Trap",
    "Taken Branch Trap",
    "Single Step Trap",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "IA-32 Exception",
    "IA-32 Intercept",
    "IA-32 Interrupt",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"
};

void ia64_dump_frame (ia64_exception_context_t * frame);

DECLARE_TRACEPOINT (SYSCALL_KERNEL_INTERFACE);

DEFINE_SPINLOCK(kdb_lock);

static void enter_debugger (ia64_exception_context_t * frame)
{
    /*
     * Create an own TCB for the kernel debugger, including stacks.
     */
    static whole_tcb_t __kdb_tcb 
	__attribute__ ((aligned (sizeof (whole_tcb_t))));
    tcb_t * kdb_tcb = (tcb_t *) &__kdb_tcb;


    /*
     * Make sure only one processor enters the kernel debugger at a
     * time.  Needed since we share the kernel debuger TCB among all
     * processors.
     */
    kdb_lock.lock ();

    /*
     * Make sure that current stack pointer does not point into the
     * middle of the exception frame.
     */
    tcb_t * current = addr_to_tcb (frame);
    current->stack = (word_t *)
	((word_t) frame - sizeof (ia64_switch_context_t));

    /*
     * Store various info in the kernel debugger TCB so that we can
     * use catch exceptions on it and also invoke tcb_t specific
     * methods (like get_current_space()) on it.
     */
    kdb_tcb->set_space (get_current_space ());
    kdb_tcb->get_arch ()->phys_addr = virt_to_phys ((addr_t) kdb_tcb);

    asm ("	;;						\n"
	 "	// Store arguments into non-stacked registers	\n"
	 "	mov	r14 = %[stack]				\n"
	 "	mov	r15 = %[regstack]			\n"
	 "	mov	b6  = %[entry]				\n"
	 "	mov	r16 = %[frame]				\n"
	 "							\n"
	 "	// Switch physical TCB location			\n"
	 "	mov	r19 = "MKSTR(r_PHYS_TCB_ADDR)" ;;	\n"
	 "	mov	"MKSTR(r_PHYS_TCB_ADDR)" = %[tcb_phys]	\n"
	 "							\n"
	 "	// Store stuff which is not clobbered by asm	\n"
	 "	mov	r17 = ar.pfs				\n"
	 "	mov	r18 = rp				\n"
	 "							\n"
	 "	// Make a call so that we can get CFM		\n"
	 "	br.call.sptk.many rp = 1f			\n"
	 "							\n"
	 "	// Store current SP and RSE settings		\n"
	 "1:	alloc	loc0 = ar.pfs,0,8,1,0 ;;		\n"
	 "	mov	loc1 = sp				\n"
	 "	mov	loc2 = ar.rsc				\n"
	 "	mov	loc3 = ar.bsp				\n"
	 "	;;						\n"
	 "	flushrs						\n"
	 "	mov	ar.rsc = 0 ;;				\n"
	 "	mov	loc4 = ar.rnat				\n"
	 "	mov	loc5 = r17				\n"
	 "	mov	loc6 = r18				\n"
	 "	mov	loc7 = r19				\n"
	 "	add	r20 = %[off_rnat], r16			\n"
	 "	add	r21 = %[off_bspst], r16			\n"
	 "	invala						\n"
	 "							\n"
	 "	// Switch to debuggers SP and RSP		\n"
	 "	mov	ar.bspstore = r15 ;;			\n"
	 "	mov	ar.rnat = 0				\n"
	 "	mov	sp   = r14				\n"
	 "							\n"
	 "	// Invoke kernel debugger			\n"
	 "	mov	out0 = r16				\n"
	 "	st8	[r20] = loc4	      // ar.rnat (kern)	\n"
	 "	st8	[r21] = loc3	      // ar.bsp (kern)	\n"
	 "	br.call.sptk.many rp = b6			\n"
	 "							\n"
	 "	// Restore old SP and RSE settings		\n"
	 "	mov	ar.pfs = loc0				\n"
	 "	mov	sp  = loc1				\n"
	 "	;;						\n"
	 "	flushrs						\n"
	 "	mov	ar.rsc = 0 ;;				\n"
	 "	invala						\n"
	 "	mov	ar.bspstore = loc3 ;;			\n"
	 "	mov	ar.rnat = loc4				\n"
	 "	movl	r14 = 1f ;;				\n"
	 "	mov	rp  = r14 ;;				\n"
	 "	mov	r17 = loc5				\n"
	 "	mov	r18 = loc6				\n"
	 "	mov	r19 = loc7				\n"
	 "							\n"
	 "	// Do return to reset CFM to old value		\n"
	 "	br.ret.sptk.many rp				\n"
	 "							\n"
	 "	// Restore non-clobbered registers		\n"
	 "1:	mov	rp  = r18				\n"
	 "	mov	ar.pfs = r17 ;;				\n"
	 "	mov	r14 = pmc[r0] ;;			\n"	
	 "	dep	r14 = 0,r14,0,1 ;;			\n"
	 "	mov	pmc[r0] = r14 ;;			\n"
	 "							\n"
	 "	// Switch back to original TCB location		\n"
	 "	mov	"MKSTR(r_PHYS_TCB_ADDR)" = r19		\n"
	 :
	 :
	 [tcb_phys]	"r" (virt_to_phys (kdb_tcb)),
	 [stack]	"r" (kdb_tcb->get_stack_top ()),
	 [regstack]	"r" (kdb_tcb->get_reg_stack_bottom ()),
	 [entry]	"r" (((word_t *) get_kip ()->kdebug_entry)[0]),
	 [frame]	"r" (frame),
	 [off_rnat]	"i" (offsetof (ia64_exception_context_t, rnat_kern)),
	 [off_bspst]	"i" (offsetof (ia64_exception_context_t,bspstore_kern))
	 :
	 CALLER_SAVED_REGS, CALLEE_SAVED_REGS, "memory");

    kdb_lock.unlock();
}


extern "C" void handle_exception (word_t num, ia64_exception_context_t * frame)
{
    addr_t iip = frame->iip;
    psr_t ipsr = frame->ipsr;
    cr_isr_t isr = frame->isr;

    printf ("Interruption %d (%s) @ %p:  ipsr=%p  isr=%p\n",
	    num, interruption_names[num],
	    addr_offset (iip, isr.instruction_slot * 6),
	    ipsr.raw, isr.raw);

    enter_debugger (frame);
}

extern "C" void handle_break (ia64_exception_context_t * frame)
{
    tcb_t * current = addr_to_tcb (frame);
    space_t * space = current->get_space ();

    extern word_t _initial_register_stack;
    extern word_t _initial_stack;

    // Sanity check
    if (! space->is_tcb_area (frame) &&
	addr_to_tcb (frame) != get_idle_tcb () &&
	((word_t *) frame < &_initial_register_stack ||
	 (word_t *) frame > &_initial_stack))
    {
	printf ("handle_break(): frame not in TCB area (%p)\n", frame);
	ia64_dump_frame (frame);

	printf ("\nPress any key to reboot!\n");
	getc ();

	efi_runtime_services->reset_system
	    (efi_runtime_services_t::warm,
	     efi_runtime_services_t::success,
	     0, NULL);

	/* NOTREACHED */
	for (;;);
    }
    
    ia64_instr_t i0, i1, i2;
    ia64_bundle_t bundle;

    if (space->is_user_area (frame->iip))
    {
	// Read user-level instruction.  Instruction may not be in
	// DTLB, so we read it using the physical address instead.
	bool readable = 
	    space->readmem (frame->iip, &bundle.raw64[0]) &&
	    space->readmem (addr_offset (frame->iip, sizeof (word_t)),
			    &bundle.raw64[1]);
	if (! readable)
	{
	    // Should not happen
	    enter_kdebug ("break: unreadable user-memory");
	    return;
	}
    }
    else
    {
	// Instruction in kernel area.  This area is mapped by TRs.
	bundle.raw64[0] = ((word_t *) frame->iip)[0];
	bundle.raw64[1] = ((word_t *) frame->iip)[1];

    }

    i0 = bundle.slot (0);
    i1 = bundle.slot (1);
    i2 = bundle.slot (2);

    /*
     * Check for KernelInterface() syscall.  The KernelInterface()
     * syscall has the following format.
     *
     *   { .mlx
     *   (qp)  break.m	0x1face
     *   (qp)  movl	r0 = 0x0
     *	       ;;
     *   }
     */
    if (bundle.get_template () == ia64_bundle_t::mlx_s3 &&
	i0.m_nop.is_break () && i0.m_nop.immediate () == 0x1face &&
	i2.x_movl.is_movl () && i2.x_movl.reg () == 0 &&
	i2.x_movl.immediate (i1) == 0)
    {
	// Fill in return values

	frame->r8  = (u64_t) space->get_kip_page_area ().get_base ();
	frame->r9  = get_kip ()->api_version;
	frame->r10 = get_kip ()->api_flags;
	frame->r11 = (4 << 24) | (1 << 16);
	frame->unat &= ~(0xf << 8);

	TRACEPOINT (SYSCALL_KERNEL_INTERFACE,
		    printf ("SYS_KERNEL_INTERFACE: [%p @ %p]\n",
			    get_current_tcb (), frame->iip));

	// Skip past instruction bundle

	frame->iip = addr_offset (frame->iip, 16);
	frame->ipsr.ri = 0;
    }

    /*
     * Check if break instruction is a kdebug operation.  Kdebug
     * operations have the following format:
     *
     *   { .mlx
     *   (qp)  break.m	<type>
     *   (qp)  movl	r0 = <arg>
     *	       ;;
     *   }
     */
    else if (bundle.get_template () == ia64_bundle_t::mlx_s3 &&
	     i0.m_nop.is_break () &&
	     i2.x_movl.is_movl () && i2.x_movl.reg () == 0)
    {
	enter_debugger (frame);

	// Skip past instruction bundle

	frame->iip = addr_offset (frame->iip, 16);
	frame->ipsr.ri = 0;
    }

    else
    {
	printf ("break 0x%x @ %p (frame=%p)\n",
		frame->iim, frame->iip, frame);

	frame->exception_num = 0; // Avoid instruction decoding
	enter_debugger (frame);
    }
}


extern "C" void handle_debug_event (ia64_exception_context_t * frame)
{
    enter_debugger (frame);
}

extern "C" void handle_int (word_t vector, ia64_exception_context_t * frame)
{
    printf ("Interrupt --- vector: %p,  frame: %p\n", vector, frame);
    enter_debugger (frame);
}
