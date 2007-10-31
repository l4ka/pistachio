/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-amd64/prepost.cc
 * Description:   X86-64 specific handlers for KDB entry and exit
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
 * $Id: prepost.cc,v 1.8 2006/10/21 00:33:34 reichelt Exp $ 
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/console.h>

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)

#include INC_ARCHX(x86,traps.h)	/* for exception numbers	*/
#include INC_ARCH(trapgate.h)	/* for amd64_exceptionframe_t	*/
#include INC_PLAT(nmi.h)	/* for nmi_t			*/

extern "C" int disas(addr_t pc);


bool kdb_t::pre()
{
    debug_param_t * param = (debug_param_t*)kdb_param;
    x86_exceptionframe_t* f = param->frame;
    kdb_current = param->tcb;

    space_t * space = kdb.kdb_current->get_space();
    if (!space) space = get_kernel_space();

    addr_t addr = (addr_t) (f->rip);

    switch (param->exception)
    {
    case X86_EXC_DEBUG:	/* single step, hw breakpoints */
	printf("--- Breakpoint ---\n");
	printf("Addr=%16x, Dumping frame:\n", f->rip);
	printf("\tRAX %16x", f->rax);
	printf("\t R8 %16x\n", f->r8 );
	printf("\tRCX %16x", f->rcx);
	printf("\t R9 %16x\n", f->r9 );
	printf("\tRDX %16x", f->rdx);
	printf("\tR10 %16x\n", f->r10);
	printf("\tRBX %16x", f->rbx);
	printf("\tR11 %16x\n", f->r11);
	printf("\tRSP %16x", f->rsp);
	printf("\tR12 %16x\n", f->r12);
	printf("\tRBP %16x", f->rbp);
	printf("\tR13 %16x\n", f->r13);
	printf("\tRSI %16x", f->rsi);
	printf("\tR14 %16x\n", f->r14);
	printf("\tRDI %16x", f->rdi);
	printf("\tR15 %16x\n", f->r15);
	printf("\tRIP %16x\n", f->rip);
	printf("\tERR %16x", f->error);
	printf("\tRFL %16x\n", f->rflags);
	printf("\t CS %16x", f->cs);
	printf("\t SS %16x\n", f->ss);	    

	if (f->rflags & (1 << 8))
	{
#if defined(CONFIG_KDB_DISAS)
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
	    if (!(space->is_compatibility_mode() && space->is_user_area(addr)))
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */
	    {
		disas(addr);
		printf("\n");
	    }
#endif
	    f->rflags &= ~(1 << 8);
	}

	break;


    case X86_EXC_NMI:		/* well, the name says enough	*/
	printf("--- NMI ---\n");
	break;
	
    case X86_EXC_BREAKPOINT: /* int3 */
    {
	unsigned char c;
	if (! readmem(space, addr, &c) )
	    break;
	
	bool enter_kernel_debugger = true;

	if (c == 0x90)
	{
	    addr = addr_offset(addr, 1);
	    enter_kernel_debugger = false;
	}
	
	if (!readmem(space, addr, &c) )
	    break;
	
	if (c == 0xeb)
	{
	    u16_t opcode = space->get_from_user(addr_offset(addr, 2));
	    addr_t user_addr = NULL;

	    /* jmp rel */
	    if ((u8_t) opcode == 0xb8)
	    {
#if defined(CONFIG_AMD64_COMPATIBILITY_MODE)
		if (space->is_compatibility_mode() && space->is_user_area(addr))
		    /* movl addr32, %eax */
		    user_addr = (addr_t) (u32_t) (space->get_from_user(addr_offset(addr, 3)));
		else
#endif /* defined(CONFIG_AMD64_COMPATIBILITY_MODE) */
		    /* movq addr64, %rax */
		    user_addr = (addr_t) (space->get_from_user(addr_offset(addr, 3)));
	    }
	    else if (opcode == 0xc748)
		/* movq addr32, %rax */
		user_addr = (addr_t) (s32_t) (space->get_from_user(addr_offset(addr, 5)));

	    if (user_addr)
		printf("--- \"%s\" ---\n"
		       "--------------------------------- (rip=%8p, rsp=%8p) ---\n", 
		       user_addr, f->rip - 1, f->rsp);
	}
	else if (c == 0x3c)
	{
	    // putc / getc
	    if (!readmem (space, addr_offset(addr, 1), &c))
		break;
	    switch(c)
	    {
	    case 0x0:
		putc(f->rax); return false;
	    case 0x1:
	    {
		addr_t user_addr = (addr_t) f->rax;
		while (readmem (space, user_addr, &c) && (c != 0))
		{
		    putc (c);
		    user_addr = addr_offset (user_addr, 1);
		}
		if (c != 0)
		    printf ("[string not completely mapped]");
		return false;
	    }
 	    case 0xd:
	    case 0x8:
	    	f->rax = getc(); return false;
	    default:
		printf("kdb: unknown opcode: int3, cmpb %d\n", space->get_from_user(addr_offset(addr, 1)));
	    }
	}
	else
	    printf("%x, %x\n", space->get_from_user(addr), space->get_from_user(addr_offset(addr, 1)));

	return enter_kernel_debugger;

    }; break;

    default:
	printf("--- KD# unknown reason ---\n");
	break;
    } /* switch */

    return true;
};




void kdb_t::post()
{
    debug_param_t * param = (debug_param_t*)kdb.kdb_param;
    x86_exceptionframe_t* f = param->frame;
    
    switch (param->exception)
    {
    case X86_EXC_DEBUG:
	/* Set RF in RFLAGS. This will disable breakpoints for one
	   instruction. The processor will reset it afterwards. */
	f->rflags |= (1 << 16);
	break;

    case X86_EXC_NMI:
	nmi_t().unmask();
	break;

    } /* switch */
    return;
};
