/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-amd64/exception.cc
 * Description:   exception handling
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
 * $Id: exception.cc,v 1.14 2006/10/21 00:46:39 reichelt Exp $ 
 *                
 ********************************************************************/

#include <debug.h>
#include <linear_ptab.h>
#include <kdb/tracepoints.h>
#include INC_ARCHX(x86,traps.h)
#include INC_ARCH(trapgate.h)
#include INC_GLUE(traphandler.h)
#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)

#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
#include INC_GLUE(ia32/kernelinterface.h)
#endif

#if defined(CONFIG_IO_FLEXPAGES)
#include INC_PLAT(io_space.h)
DECLARE_TRACEPOINT (AMD64_IO_PAGEFAULT);
#endif

DECLARE_TRACEPOINT (AMD64_GP);
DECLARE_TRACEPOINT (AMD64_UD);
DECLARE_TRACEPOINT (AMD64_NOMATH);
DECLARE_TRACEPOINT (AMD64_SEGRELOAD);


static bool __attribute__((__noinline__)) send_exception_ipc(amd64_exceptionframe_t *frame, word_t exception_number )
{
    tcb_t * current = get_current_tcb();
    if (current->get_exception_handler().is_nilthread())
 	return false;

    /* setup exception IPC */
    word_t saved_mr[21];
    msg_tag_t tag;

    // save message registers 
    for (int i = 0; i < 20; i++)
 	saved_mr[i] = current->get_mr(i);

    word_t saved_br0 = current->get_br(0);

    current->saved_partner = current->get_partner();

    tag.set(0, 20, -5 << 4);
    current->set_mr(0,  tag.raw);
    current->set_mr(1,  frame->rip);
    current->set_mr(2,  frame->rbx);
    current->set_mr(3, frame->r10);
    current->set_mr(4,  frame->r12);
    current->set_mr(5,  frame->r13);
    current->set_mr(6,  frame->r14);
    current->set_mr(7,  frame->r15);
    current->set_mr(8, frame->rax);
    current->set_mr(9, frame->rcx);
    current->set_mr(10, frame->rdx);
    current->set_mr(11, frame->rsi);
    current->set_mr(12, frame->rdi);
    current->set_mr(13, frame->rbp);
    current->set_mr(14, frame->r8);
    current->set_mr(15, frame->r9);
    current->set_mr(16,  frame->r11);
    current->set_mr(17, frame->rsp);
    current->set_mr(18, frame->rflags);
    current->set_mr(19,  exception_number);
    current->set_mr(20,  frame->error);

    tag = current->do_ipc(current->get_exception_handler(), 
			  current->get_exception_handler(), 
			  timeout_t::never());

    if (!tag.is_error())
    {
	frame->rip = current->get_mr(1);
	frame->rbx = current->get_mr(2);
	frame->r10 = current->get_mr(3);
	frame->r12 = current->get_mr(4);
	frame->r13 = current->get_mr(5);
	frame->r14 = current->get_mr(6);
	frame->r15 = current->get_mr(7);
	frame->rax = current->get_mr(8);
	frame->rcx = current->get_mr(9);
	frame->rdx = current->get_mr(10);
	frame->rsi = current->get_mr(11);
	frame->rdi = current->get_mr(12);
	frame->rbp = current->get_mr(13);
	frame->r8 = current->get_mr(14);
	frame->r9 = current->get_mr(15);
	frame->r11 = current->get_mr(16);
	frame->rsp = current->get_mr(17);
	current->set_user_flags(current->get_mr(18));
    }
    else
    {
 	enter_kdebug("Exception delivery error");
    }

    for (int i = 0; i < 20; i++)
 	current->set_mr(i, saved_mr[i]);
    
    current->set_br(0, saved_br0);
    current->set_partner(current->saved_partner);
    current->saved_partner = NILTHREAD;

    return !tag.is_error();
}

X86_EXCNO_ERRORCODE(exc_catch_diverr, -1)
{
    TRACE("Divide by Zero Exception\n");
    
    if (send_exception_ipc(frame, X86_EXC_DIVIDE_ERROR))
	return;
    
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_overflow, -1)
{
    TRACE("Overflow Exception\n");
    
    if (send_exception_ipc(frame, X86_EXC_OVERFLOW))
	return;
    
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_boundrange, -1)
{
    TRACE("Bound Range Exception\n");
    if (send_exception_ipc(frame, X86_EXC_BOUNDRANGE))
	return;

    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_doublefault, -1)
{
    TRACE("Doublefault Exception\n");
    if (send_exception_ipc(frame, X86_EXC_DOUBLEFAULT))
	return;
    
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_overrun, -1)
{
    TRACE("Overrun Exception\n");
    if (send_exception_ipc(frame, X86_EXC_COPSEG_OVERRUN))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_invtss, -1)
{
    TRACE("Invalid TSS Exception\n");
    if (send_exception_ipc(frame, X86_EXC_INVALID_TSS))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_segnotpr, -1)
{
    TRACE("Segment not present Exception\n");
    if (send_exception_ipc(frame, X86_EXC_SEGMENT_NOT_PRESENT))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCWITH_ERRORCODE(exc_catch_ss_fault, -1)
{
    TRACE("Stack Segment fault Exception\n");
    if (send_exception_ipc(frame, X86_EXC_STACKSEG_FAULT))
	return;
    x86_dump_frame(frame);
enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_ac, -1)
{
    TRACE("Alignment Check Exception\n");
    if (send_exception_ipc(frame, X86_EXC_ALIGNEMENT_CHECK))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  
X86_EXCNO_ERRORCODE(exc_catch_mc, -1)
{
    TRACE("Machine Check Exception\n");
    if (send_exception_ipc(frame, X86_EXC_MACHINE_CHECK))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
}				  

X86_EXCNO_ERRORCODE(exc_invalid_opcode, X86_EXC_INVALIDOPCODE)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();
    addr_t addr = (addr_t) frame->rip;

    TRACEPOINT (AMD64_UD, "amd64_ud at %x (current=%x)", addr, current);

    /* instruction emulation */
    switch( (u8_t) space->get_from_user(addr))
    {
    case 0xf0: /* lock prefix */
 	if ( (u8_t) space->get_from_user(addr_offset(addr, 1)) == 0x90)
 	{
 	    /* lock; nop */
	    //TRACEF("lock;nop: kip is at %p\n", (u64_t)space->get_kip_page_area().get_base());
 	    frame->rax = (u64_t)space->get_kip_page_area().get_base();
 	    frame->rcx = get_kip()->api_version;
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
 	    if (space->is_compatibility_mode())
 	    {
		/* srXXX: Hack: Update system and user base in 32-bit KIP.
		   This is necessary because they are not set in the initialization phase. */
		ia32::get_kip()->thread_info.set_system_base(get_kip()->thread_info.get_system_base());
		ia32::get_kip()->thread_info.set_user_base(get_kip()->thread_info.get_user_base());
 		frame->rdx = ia32::get_kip()->api_flags;
 	    }
 	    else
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */
 		frame->rdx = get_kip()->api_flags;
 	    frame->rsi = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();
 	    frame->rip+= 2;
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

#if 0
static inline u8_t read_data (space_t * s, addr_t a)
{
    if (s->is_user_area (a))
	return s->get_from_user (a);
    else
	return *(u8_t *) a;
}
#endif

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
static bool handle_faulting_instruction (amd64_exceptionframe_t * frame)
{
    tcb_t * current = get_current_tcb ();
    space_t * space = current->get_space ();
    addr_t instr = (addr_t) frame->rip;
    u8_t i[4];

    if (!readmem (space, instr, &i[0]))
	return false;

    switch (i[0])
    {

#if defined(CONFIG_IO_FLEXPAGES)

#define HANDLE_IO_PAGEFAULT(port, size)							\
	tcb_t *tcb = get_current_tcb();							\
	TRACEPOINT (AMD64_IO_PAGEFAULT, "IO-Pagefault @ %x [size %x] (current=%T)",	\
		    (u32_t)port, (u32_t) size, TID(tcb->get_global_id()));		\
	handle_io_pagefault(tcb, port, size, (addr_t)frame->rip);			\
	return true;
    
    
    case 0xe4:  /* in  %al,      port imm8  (byte)  */
    case 0xe6:  /* out %al,      port imm8  (byte)  */
    {
	if (!readmem (space, instr, &i[1]))
	    return false;
	HANDLE_IO_PAGEFAULT(i[1], 0);
    }
    case 0xe5:  /* in  %eax, port imm8  (dword) */
    case 0xe7:  /* out %eax, port imm8  (dword) */
    {
	if (!readmem (space, instr, &i[1]))
	    return false;
	HANDLE_IO_PAGEFAULT(i[1], 2);
    }
    case 0xec:  /* in  %al,        port %dx (byte)  */
    case 0xee:  /* out %al,        port %dx (byte)  */
    case 0x6c:  /* insb		   port %dx (byte)  */
    case 0x6e:  /* outsb           port %dx (byte)  */
    {
	HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 0);
    }
    case 0xed:  /* in  %eax,   port %dx (dword) */
    case 0xef:  /* out %eax,   port %dx (dword) */
    case 0x6d:  /* insd	       port %dx (dword) */
    case 0x6f:  /* outsd       port %dx (dword) */
    {
	HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 2);
    }
    case 0x66:
    {
	/* operand size override prefix */
	if (!readmem (space, instr, &i[1]))
	    return false;
	switch ((u16_t) i[1])
	{
	case 0xe5:  /* in  %ax, port imm8  (word) */
	case 0xe7:  /* out %ax, port imm8  (word) */
	{
	if (!readmem (space, instr, &i[2]))
	    return false;
	    HANDLE_IO_PAGEFAULT(i[2], 1);
	}
	case 0xed:  /* in  %ax, port %dx  (word) */
	case 0xef:  /* out %ax, port %dx  (word) */
	case 0x6d:  /* insw     port %dx  (word) */
	case 0x6f:  /* outsw    port %dx  (word) */
	{
	    HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 1);
	}
	}
    }
    case 0xf3:
    {
	/* rep instruction */
	if (!readmem (space, instr, &i[1]))
	    return false;
        switch (i[1])
	{
        case 0xe4:  /* in  %al,  port imm8  (byte)  */
        case 0xe6:  /* out %al,  port imm8  (byte)  */
        {
	    if (!readmem (space, instr, &i[2]))
		return false;
	    HANDLE_IO_PAGEFAULT(i[2], 0);
        }
        case 0xe5:  /* in  %eax, port imm8  (dword) */
        case 0xe7:  /* out %eax, port imm8  (dword) */
        {
	    if (!readmem (space, instr, &i[2]))
		return false;
	    HANDLE_IO_PAGEFAULT(i[2], 2);
        }
        case 0xec:  /* in  %al,    port %dx (byte)  */
        case 0xee:  /* out %al,    port %dx (byte)  */
        case 0x6c:  /* insb        port %dx (byte)  */
        case 0x6e:  /* outsb       port %dx (byte)  */
        {
	    HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 0);
        }
        case 0xed:  /* in  %eax,   port %dx (dword) */
        case 0xef:  /* out %eax,   port %dx (dword) */
        case 0x6d:  /* insd        port %dx (dword) */
        case 0x6f:  /* outsd       port %dx (dword) */
        {
	    HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 2);
        }
        case 0x66:
	{	    
            /* operand size override prefix */
	    if (!readmem (space, instr, &i[2]))
		return false;
            switch (i[2])
            {
            case 0xe5:  /* in  %ax, port imm8  (word) */
            case 0xe7:  /* out %ax, port imm8  (word) */
            {
		if (!readmem (space, instr, &i[3]))
		    return false;
		HANDLE_IO_PAGEFAULT(i[3], 1);
            }
            case 0xed:  /* in  %ax, port %dx  (word) */
            case 0xef:  /* out %ax, port %dx  (word) */
            case 0x6d:  /* insw	    port %dx  (word) */
            case 0x6f:  /* outsw    port %dx  (word) */
            {
		HANDLE_IO_PAGEFAULT(frame->rdx & 0xFFFF, 1);
	    }
	    }
	}
	}
    }
#endif

    case 0x0f: /* two-byte instruction prefix */
    {
	if (!readmem (space, instr, &i[1]))
	    return false;
	switch( i[1] )
	{
	case 0x30:
    	    /* wrmsr */
	    if ( is_privileged_space(space) ) {
		x86_wrmsr ((u32_t) frame->rcx, (frame->rdx << 32) |  (frame->rax & 0xffffffff));
		frame->rip += 2;
		return true;
	    } break;

	case 0x32:
	    /* rdmsr */
	    if ( is_privileged_space(space) ) {
		u64_t val = x86_rdmsr ((u32_t) frame->rcx);
		frame->rax = val & 0xffffffff;
		frame->rdx = val >> 32;
		frame->rip += 2;
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

	TRACEPOINT (AMD64_SEGRELOAD, "segment register reload");
	reload_user_segregs ();
	frame->rip++;
	if (i[0] == 0x8e || i[0] == 0x0f)
	    frame->rip++;
	return true;
    }

    return false;
}


X86_EXCWITH_ERRORCODE(exc_gp, X86_EXC_GENERAL_PROTECTION)
{
    TRACEPOINT (AMD64_GP, "general protection fault @ %p, error: %x\n", frame->rip, frame->error);

    /*
     * In some cases we handle the faulting instruction without
     * involving the user-level exception handler.
     */
    if (handle_faulting_instruction (frame))
	return;


    word_t ds = 0 , es = 0, fs = 0, gs = 0;
    
    if (send_exception_ipc(frame, X86_EXC_GENERAL_PROTECTION))
	return;

    TRACE("GP exception\n");
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");
    
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
}

X86_EXCNO_ERRORCODE(exc_nomath_coproc, X86_EXC_NOMATH_COPROC)
{
    tcb_t * current = get_current_tcb();

    TRACEPOINT(AMD64_NOMATH, "AMD64_NOMATH %t @ %p\n", current, frame->rip);

    current->resources.x86_no_math_exception(current);
}

X86_EXCNO_ERRORCODE(exc_fpu_fault, X86_EXC_FPU_FAULT)
{
    TRACEF("FPU exception\n");
    if (send_exception_ipc(frame, X86_EXC_FPU_FAULT))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");    

}

X86_EXCNO_ERRORCODE(exc_simd_fault, X86_EXC_SIMD_FAULT)
{
    TRACEF("SIMD exception\n");
    if (send_exception_ipc(frame, X86_EXC_SIMD_FAULT))
	return;
    x86_dump_frame(frame);
    enter_kdebug("unhandled exception");    

}



word_t exc_catch_all[IDT_SIZE];
extern "C" void exc_catch_common_handler(amd64_exceptionframe_t frame){

    word_t irq  = (frame.error - 5 - (word_t) exc_catch_all) / 8;
    TRACE("Invalid jump to IDT Entry No %d - Bogus Interrupt?\n", irq);
    //enter_kdebug("Bogus Interrupt");
}

void exc_catch_common_wrapper() 					
{							
    __asm__ (						
        ".section .data.amd64.exc_common	\n"
        ".global exc_catch_common		\n"
	"\t.type exc_catch_common,@function	\n"
	"exc_catch_common:			\n"
        "pushq %%rax				\n"
	"pushq %%rcx				\n"
	"pushq %%rbx				\n"
	"pushq %%rdx				\n"
	"pushq %%rbp				\n"
    	"pushq %%rsi				\n"
    	"pushq %%rdi				\n"
    	"pushq %%r8				\n"
    	"pushq %%r9				\n"
    	"pushq %%r10				\n"
    	"pushq %%r11				\n"
    	"pushq %%r12				\n" 
    	"pushq %%r13				\n"
    	"pushq %%r14				\n" 
    	"pushq %%r15				\n"
	"pushq %0			    	\n"
	"call exc_catch_common_handler		\n"		
	"addq  $8, %%rsp			\n"		
    	"popq  %%r15				\n"		
    	"popq  %%r14				\n"		
    	"popq  %%r13				\n"		
    	"popq  %%r12				\n"		
    	"popq  %%r11				\n"		
    	"popq  %%r10				\n"		
    	"popq  %%r9				\n"		
    	"popq  %%r8				\n"		
    	"popq  %%rdi				\n"		
    	"popq  %%rsi				\n"		
	"popq  %%rbp				\n"		
	"popq  %%rdx				\n"		
	"popq  %%rbx				\n"		
	"popq  %%rcx				\n"		
        "popq  %%rax				\n"		
	"addq  $8, %%rsp			\n"		
	"iretq					\n"		
	".previous				\n"
	:						
	: "i"(0)					
	);						
}							
