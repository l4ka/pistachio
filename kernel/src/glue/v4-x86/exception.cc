/*********************************************************************
 *                
 * Copyright (C) 2007-2008,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/exception.cc
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>
#include <linear_ptab.h>
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(traphandler.h)

DECLARE_TRACEPOINT_DETAIL (EXCEPTION_IPC);
DECLARE_TRACEPOINT (X86_NOMATH);
DECLARE_TRACEPOINT (X86_GP);
DECLARE_TRACEPOINT (X86_SEGRELOAD);
DECLARE_TRACEPOINT (X86_UD);

#if defined(CONFIG_X86_IO_FLEXPAGES)
#include INC_GLUE(io_space.h)
#endif

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
#include INC_GLUE_SA(x32comp/kernelinterface.h)
#endif

bool send_exception_ipc(x86_exceptionframe_t * frame, word_t exception)
{
    tcb_t * current = get_current_tcb();
    if (current->get_exception_handler().is_nilthread())
	return false;

    TRACEPOINT (EXCEPTION_IPC, "exception ipc at %x, %T (%p) -> %T \n", 
		frame->regs[x86_exceptionframe_t::ipreg], current->get_global_id().get_raw(),
		current, current->get_scheduler().get_raw());

    /* setup exception IPC */
    word_t saved_mr[NUM_EXC_REGS-IPC_NUM_SAVED_MRS];
    msg_tag_t tag;
    
    tag.set(0, 2, (word_t) (-5 << 4));

    current->save_state();

    tag.x.untyped += NUM_EXC_REGS - 3;
	
    for (int i = 0; i < NUM_EXC_REGS-IPC_NUM_SAVED_MRS; i++)
	saved_mr[i] = current->get_mr(i+IPC_NUM_SAVED_MRS);
	
    current->set_mr(x86_exc_reg_t::mr(0), exception); 
    for (word_t num=1; num < NUM_EXC_REGS; num++)
	current->set_mr(x86_exc_reg_t::mr(num), frame->regs[x86_exc_reg_t::reg(num)]);
    
    current->set_mr(0, tag.raw);
    tag = current->do_ipc(current->get_exception_handler(), 
			  current->get_exception_handler(), 
			  timeout_t::never());

    if (tag.is_error())
    {
	printf("exception delivery error tag=%x, error code=%x\n", 
	       tag.raw, current->get_error_code());
	
	enter_kdebug("exception delivery error");
    }

    
   
    word_t flags = current->get_user_flags();
	
    for (word_t num=0; num < NUM_EXC_REGS; num++)
	frame->regs[x86_exc_reg_t::reg(num)] = current->get_mr(x86_exc_reg_t::mr(num));
    
    /* mask eflags appropriately */
    current->get_stack_top()[KSTACK_UFLAGS] &= X86_USER_FLAGMASK;
    current->get_stack_top()[KSTACK_UFLAGS] |= (flags & ~X86_USER_FLAGMASK);
	
    for (int i = 0; i < NUM_EXC_REGS-IPC_NUM_SAVED_MRS; i++)
	current->set_mr(i+IPC_NUM_SAVED_MRS, saved_mr[i]);
	
    current->restore_state();  
   
    return !tag.is_error();
}

/**
 * Try handling the faulting instruction by decoding the instruction
 * stream.  If we are able to handle the fault in the kernel (e.g., by
 * reloading segment registers), we do so without involving the
 * user-level exception handler.
 *
 * @param frame		exception frame
 *
 * @return true if kernel handled the fault, false otherwise
 */
static bool handle_faulting_instruction (x86_exceptionframe_t * frame)
{
    tcb_t * current = get_current_tcb ();
    space_t * space = current->get_space ();
    addr_t instr = (addr_t) frame->regs[x86_exceptionframe_t::ipreg];
    u8_t i[4];

    if (!readmem (space, instr, &i[0]))
	return false;

    switch (i[0])
    {
#if defined(CONFIG_X86_SMALL_SPACES)
    case 0xcf:
    {
	/*
	 * CF		iret
	 *
	 * When returning to user-level the instruction pointer
	 * might happen to be outside the small space.  If so, we
	 * must promote the space to a large one.
	 */
	if (! space->is_user_area (instr) &&
	    space->is_small () &&
	    (word_t) current->get_user_ip () > space->smallid ()->size ())
	{
	    space->make_large ();
	    return true;
	}
	break;
    }
#endif

#if defined(CONFIG_X86_IO_FLEXPAGES)
    
    case 0xe4:  /* in  %al,      port imm8  (byte)  */
    case 0xe6:  /* out %al,      port imm8  (byte)  */
    {
	if (!readmem (space, addr_offset(instr, 1), &i[1]))
	    return false;
	return handle_io_pagefault(current, i[1], 0, instr);
    }
    case 0xe5:  /* in  %eax, port imm8  (dword) */
    case 0xe7:  /* out %eax, port imm8  (dword) */
    {
	if (!readmem (space, addr_offset(instr, 1), &i[1]))
	    return false;
	return handle_io_pagefault(current, i[1], 2, instr);
    }
    case 0xec:  /* in  %al,        port %dx (byte)  */
    case 0xee:  /* out %al,        port %dx (byte)  */
    case 0x6c:  /* insb		   port %dx (byte)  */
    case 0x6e:  /* outsb           port %dx (byte)  */
	return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 0, instr);
    case 0xed:  /* in  %eax,   port %dx (dword) */
    case 0xef:  /* out %eax,   port %dx (dword) */
    case 0x6d:  /* insd	       port %dx (dword) */
    case 0x6f:  /* outsd       port %dx (dword) */
	return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 2, instr);
    case 0x66:
    {
	if (!readmem (space, addr_offset(instr, 1), &i[1]))
	    return false;
	/* operand size override prefix */
	switch (i[1])
	{
	case 0xe5:  /* in  %ax, port imm8  (word) */
	case 0xe7:  /* out %ax, port imm8  (word) */
	{
	    if (!readmem (space, addr_offset(instr, 2), &i[2]))
	    return false;
	    return handle_io_pagefault(current, i[2], 1, instr);
	}
	case 0xed:  /* in  %ax, port %dx  (word) */
	case 0xef:  /* out %ax, port %dx  (word) */
	case 0x6d:  /* insw     port %dx  (word) */
	case 0x6f:  /* outsw    port %dx  (word) */
	    return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 1, instr);
	}
    }
    case 0xf3:
    {
	/* rep instruction */
	if (!readmem (space, addr_offset(instr, 1), &i[1]))
	    return false;
        switch (i[1])
	{
        case 0xe4:  /* in  %al,  port imm8  (byte)  */
        case 0xe6:  /* out %al,  port imm8  (byte)  */
        {
	    if (!readmem (space, addr_offset(instr, 2), &i[2]))
		return false;
	    return handle_io_pagefault(current, i[2], 0, instr);
        }
        case 0xe5:  /* in  %eax, port imm8  (dword) */
        case 0xe7:  /* out %eax, port imm8  (dword) */
        {
	    if (!readmem (space, addr_offset(instr, 2), &i[2]))
		return false;
	    return handle_io_pagefault(current, i[2], 2, instr);
        }
        case 0xec:  /* in  %al,    port %dx (byte)  */
        case 0xee:  /* out %al,    port %dx (byte)  */
        case 0x6c:  /* insb        port %dx (byte)  */
        case 0x6e:  /* outsb       port %dx (byte)  */
	    return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 0, instr);
        case 0xed:  /* in  %eax,   port %dx (dword) */
        case 0xef:  /* out %eax,   port %dx (dword) */
        case 0x6d:  /* insd        port %dx (dword) */
        case 0x6f:  /* outsd       port %dx (dword) */
	    return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 2, instr);
        case 0x66:
	{	    
            /* operand size override prefix */
	    if (!readmem (space, addr_offset(instr, 2), &i[2]))
		return false;
            switch (i[2])
            {
            case 0xe5:  /* in  %ax, port imm8  (word) */
            case 0xe7:  /* out %ax, port imm8  (word) */
            {
		if (!readmem (space, addr_offset(instr, 3), &i[3]))
		    return false;
		return handle_io_pagefault(current, i[3], 1, instr);
            }
            case 0xed:  /* in  %ax, port %dx  (word) */
            case 0xef:  /* out %ax, port %dx  (word) */
            case 0x6d:  /* insw	    port %dx  (word) */
            case 0x6f:  /* outsw    port %dx  (word) */
		return handle_io_pagefault(current, frame->regs[x86_exceptionframe_t::dreg] & 0xFFFF, 1, instr);
	    }
	}
	}
    }
#endif

    case 0x0f: /* two-byte instruction prefix */
    {
	if (!readmem (space, addr_offset(instr, 1), &i[1]))
	    return false;
	switch( i[1] )
	{
	case 0x30:
    	    /* wrmsr */
	    if ( is_privileged_space(space) ) {
		x86_wrmsr (frame->regs[x86_exceptionframe_t::creg], 
			   ((u64_t)(frame->regs[x86_exceptionframe_t::areg])) | 
			   ((u64_t)(frame->regs[x86_exceptionframe_t::dreg])) << 32);
		frame->regs[x86_exceptionframe_t::ipreg] += 2;
		return true;
	    } break;

	case 0x32:
	    /* rdmsr */
	    if ( is_privileged_space(space) ) {
		u64_t val = x86_rdmsr (frame->regs[x86_exceptionframe_t::creg]);
		frame->regs[x86_exceptionframe_t::areg] = (u32_t) val;
		frame->regs[x86_exceptionframe_t::dreg] = (u32_t)(val >> 32);
		frame->regs[x86_exceptionframe_t::ipreg] += 2;
		return true;
	    } break;
	    
	case 0xa1:
	case 0xa9:
	    goto pop_seg;
	}
	break;
    } /* two-byte prefix */

    case 0x8e:
    case 0x07:
    case 0x17:
    case 0x1f:
    pop_seg:
	/*
	 * 8E /r	mov %reg, %segreg
	 * 07		pop %es
	 * 17		pop %ss
	 * 1F		pop %ds
	 * 0F A1	pop %fs
	 * 0F A9	pop %gs
	 *
	 * Segment descriptor is being written with an invalid value.
	 * Reset all descriptors with the correct value and skip the
	 * instruction.
	 */

	if (! space->is_user_area (instr))
	    // Assume that kernel knows what it is doing.
	    break;

	TRACEPOINT (X86_SEGRELOAD, "segment register reload");
	reload_user_segregs ();
#if defined(CONFIG_SUBARCH_X32)
	frame->ds = frame->es = X86_UDS;
#endif
	frame->regs[x86_exceptionframe_t::ipreg]++;
	
	if (i[0] == 0x8e || i[0] == 0x0f)
	    frame->regs[x86_exceptionframe_t::ipreg]++;
	
	return true;
    }

#if defined(CONFIG_X86_SMALL_SPACES)
    /*
     * A GP(0) or SS(0) might indicate that a small address space
     * tries to access memory outside of the small space boundary.
     * Try to promote space to a large one instead of sending
     * exception IPC.
     */
    if ((frame->reason == X86_EXC_STACKSEG_FAULT ||
	 frame->reason == X86_EXC_GENERAL_PROTECTION) &&
	frame->error == 0 && space->is_small ())
    {
	space->make_large ();
	return true;
    }
#endif

    return false;
}

extern "C" void sysexit_tramp (void);
extern "C" void sysexit_tramp_end (void);
extern "C" void reenter_sysexit (void);

X86_EXCWITH_ERRORCODE(exc_gp, X86_EXC_GENERAL_PROTECTION)
{
    if (kdebug_check_interrupt())
	return;

    
    TRACEPOINT (X86_GP, "general protection fault @ %p, error: %x\n", 
		frame->regs[x86_exceptionframe_t::ipreg], frame->error);

#if defined(CONFIG_X86_SMALL_SPACES) && defined(CONFIG_X86_SYSENTER)
    /*
     * Check if we caught an exception in the sysexit trampoline.
     */
    tcb_t * current = get_current_tcb ();
    addr_t user_eip = current->get_user_ip ();

    if (user_eip >= (addr_t) sysexit_tramp &&
	user_eip <  (addr_t) sysexit_tramp_end &&
	current->get_space ()->is_small ())
    {
	/*
	 * If we faulted at the LRET instruction or otherwise was
	 * interrupted during the sysexit trampoline (i.e., still in
	 * user level) we must IRET to the kernel due to the user
	 * space code segment limitation.  We must also disable
	 * interrupts since we can not be allowed to be preempted in
	 * the reenter-trampoline.
	 */
	frame->cs = X86_KCS;
	frame->eflags &= ~X86_FLAGS_IF;
	frame->ecx = (word_t) current->get_user_sp ();
	frame->eip = (word_t) reenter_sysexit;
	return;
    }
#endif

#if defined(CONFIG_SUBARCH_X32)
    /*
     * A GP(0) could mean that we have a segment register with zero
     * contents.  If so, just reload all segment register selectors
     * with appropriate values.
     */

    if (frame->error == 0)
    {
	word_t fs, gs;
	asm ("	mov	%%fs, %w0	\n"
	     "	mov	%%gs, %w1	\n"
	     :"=r"(fs), "=r"(gs));

	if ((frame->ds & 0xffff) == 0 || (frame->es & 0xffff) == 0 ||
	    fs == 0 || gs == 0 )
	{
	    TRACEPOINT (X86_SEGRELOAD, "segment register reload");
	    reload_user_segregs ();
	    frame->ds = frame->es =
		(frame->cs & 0xffff) == X86_UCS ? X86_UDS : X86_KDS;
	    return;
	}
    }
#endif
    
    /*
     * In some cases we handle the faulting instruction without
     * involving the user-level exception handler.
     */
    if (handle_faulting_instruction (frame))
	return;


    if (send_exception_ipc(frame, X86_EXC_GENERAL_PROTECTION))
	return;

#ifdef CONFIG_KDB
    frame->dump();
    word_t ds = 0 , es = 0, fs = 0, gs = 0;

    __asm__ (
        "mov %%ds, %0   \n" 
        "mov %%es, %1	\n" 
        "mov %%fs, %2   \n" 
        "mov %%gs, %3   \n" 
        :
        : "r"(ds), "r"(es), "r"(fs),"r"(gs)
        );
    TRACE("\t DS %16x", ds);
    TRACE("\t ES %16x\n", es);
    TRACE("\t FS %16x", fs);
    TRACE("\t GS %16x\n", gs);

    enter_kdebug("#GP");
#endif

    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();
}




X86_EXCNO_ERRORCODE(exc_invalid_opcode, X86_EXC_INVALIDOPCODE)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();
    addr_t addr = (addr_t) frame->regs[x86_exceptionframe_t::ipreg];

    TRACEPOINT (X86_UD, "x86_ud at %x (%x) (current=%x)", addr, space->get_from_user(addr), current);
    
    /* instruction emulation */
    switch( (u8_t) space->get_from_user(addr))
    {
    case 0xf0: /* lock prefix */
 	if ( (u8_t) space->get_from_user(addr_offset(addr, 1)) == 0x90)
 	{
	    /* lock; nop */
	    frame->regs[x86_exceptionframe_t::areg] = (word_t)space->get_kip_page_area().get_base();
 	    frame->regs[x86_exceptionframe_t::creg] = get_kip()->api_version;
	    frame->regs[x86_exceptionframe_t::Sreg] = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();
#if defined(CONFIG_X86_COMPATIBILITY_MODE)
 	    if (space->is_compatibility_mode())
 	    {
		/* srXXX: Hack: Update system and user base in 32-bit KIP.
		   This is necessary because they are not set in the initialization phase. */
		x32::get_kip()->thread_info.set_system_base(get_kip()->thread_info.get_system_base());
		x32::get_kip()->thread_info.set_user_base(get_kip()->thread_info.get_user_base());
		frame->regs[x86_exceptionframe_t::dreg] = x32::get_kip()->api_flags;
		frame->regs[x86_exceptionframe_t::ipreg] += 2;
		return;
 	    }
#endif /* defined(CONFIG_X86_COMPATIBILITY_MODE) */
	    frame->regs[x86_exceptionframe_t::dreg] = get_kip()->api_flags;
	    frame->regs[x86_exceptionframe_t::ipreg] += 2;
	    return;
 	}
      
    default:
	printf ("%p: invalid opcode at IP %p\n", current, addr);
 	enter_kdebug("invalid opcode");
    }
    
    if (send_exception_ipc(frame, X86_EXC_INVALIDOPCODE))
	return;
    
    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();


}



X86_EXCNO_ERRORCODE(exc_nomath_coproc, X86_EXC_NOMATH_COPROC)
{
    tcb_t * current = get_current_tcb();

    TRACEPOINT(X86_NOMATH, "X86_NOMATH %t @ %p\n", 
	       current, frame->regs[x86_exceptionframe_t::ipreg]);

    current->resources.x86_no_math_exception(current);
}


u64_t exc_catch_all[IDT_SIZE] UNIT("x86.exc_all");
extern "C" void exc_catch_common_handler(x86_exceptionframe_t *frame)
{

    word_t exc  = (frame->error - 5 - (word_t) exc_catch_all) / 8;
    if (send_exception_ipc(frame, exc))
	return;

    
#if defined(CONFIG_IOAPIC)
    if (exc == 15)
    {
	TRACE("Ignoring spurious APIC interrupt\n");
	return;
    }
#endif
    
    printf("Unhandled exception %d\n", exc);
    
    enter_kdebug("Exception caught");
    
    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();

}
