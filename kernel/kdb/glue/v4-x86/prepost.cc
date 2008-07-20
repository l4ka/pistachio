/*********************************************************************
 *                
 * Copyright (C) 2002-2008,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-x86/prepost.cc
 * Description:   IA-32 specific handlers for KDB entry and exit
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
 * $Id: prepost.cc,v 1.23 2006/12/05 15:23:15 skoglund Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/init.h>
#include <kdb/console.h>
#include <kdb/tracepoints.h>

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)
#include INC_ARCH(apic.h)	
#include INC_ARCH(traps.h)	
#include INC_ARCH(trapgate.h)	
#include INC_GLUE(cpu.h)	
#include INC_GLUE(schedule.h)	

extern space_t *current_disas_space;

#if defined(CONFIG_KDB_DISAS)
extern "C" int disas(addr_t pc);
INLINE void disas_addr (addr_t ip, const char * str = "")
{
    printf ("%p    ", ip);
    current_disas_space =  kdb.kdb_current->get_space();
    disas (ip);
    printf ("\n");
}
#else
INLINE void disas_addr (addr_t ip, const char * str = "")
{
    printf ("ip=%p -- %s\n", ip, str);
}
#endif

#if defined(CONFIG_TRACEPOINTS)
extern bool x86_breakpoint_cpumask;
extern bool x86_breakpoint_cpumask_kdb;
DECLARE_TRACEPOINT(X86_BREAKPOINT);
#endif

#if defined(CONFIG_SMP)
atomic_t kdb_current_cpu;

KDEBUG_INIT(kdb_prepost_init);
void kdb_prepost_init()
{
    kdb_current_cpu = CONFIG_SMP_MAX_CPUS;
}
#endif



bool kdb_t::pre() 
{
    cpuid_t cpu = get_current_cpu();
    bool enter_kernel_debugger = true;
    debug_param_t * param = (debug_param_t*) kdb_param;
    x86_exceptionframe_t* f = param->frame;
    kdb_current = param->tcb;

#if defined(CONFIG_SMP)
    while (!kdb_current_cpu.cmpxchg(CONFIG_SMP_MAX_CPUS, current_cpu))
    {
	/* Execute a dummy iret to receive NMIs again, then sleep */
	x86_iret_self();
	x86_sleep_uninterruptible();
	
	/* 
	 * if we wanted to enter KDB, we have come here via any other
	 * exception than NMI. Otherwise we're just any of the unrelated cpus
	 * having been stopped via broadcast NMI.
	 */
	if (param->exception == X86_EXC_NMI)
	{
	    if (kdb_current_cpu == cpu)
	    {
		printf("--- Switched to CPU %d ---\n", cpu);
		return true;
	    }
	    else if (kdb_current_cpu == CONFIG_SMP_MAX_CPUS)
		return false;
	}
    }
    
    if (param->exception != X86_EXC_NMI)
    {
	local_apic_t<APIC_MAPPINGS_START> local_apic;
	local_apic.broadcast_nmi();
    }
    
#endif

    switch (param->exception)
    {
    case X86_EXC_DEBUG:	/* single step, hw breakpoints */
    {
	/* breakpoint exception */
	if (f->regs[x86_exceptionframe_t::freg] & (1 << 8))
	{
	    extern word_t x86_last_ip;
#if defined(CONFIG_CPU_X86_I686) || defined(CONFIG_CPU_X86_P4) 
	    extern bool x86_single_step_on_branches;
	    if (x86_single_step_on_branches)
	    {
		addr_t last_branch_ip;
		x86_wrmsr (X86_MSR_DEBUGCTL, 0);
		x86_single_step_on_branches = false;
#if defined(CONFIG_CPU_X86_I686)
		last_branch_ip = (addr_t) (word_t)
		    x86_rdmsr (X86_MSR_LASTBRANCHFROMIP);
#else
		word_t last_branch_tos = x86_rdmsr (X86_MSR_LASTBRANCH_TOS);
		ASSERT(last_branch_tos < 16);
		word_t lbipmsr;
		u32_t model, dummy;
		x86_cpuid(1, &model, &dummy, &dummy, &dummy);
		if (((model >> 4) & 0xF) > 2)
		    lbipmsr = X86_MSR_LASTBRANCH_FROM;
		else
		    lbipmsr = X86_MSR_LASTBRANCH_FROM_OLD;
		lbipmsr+= last_branch_tos;
		last_branch_ip = (addr_t) (word_t) x86_rdmsr(lbipmsr);
#endif
		disas_addr (last_branch_ip, "branch to");
		x86_last_ip = f->regs[x86_exceptionframe_t::ipreg];
	    }
#endif
	    if (x86_last_ip != ~0U)
		disas_addr ((addr_t) x86_last_ip);
	    f->regs[x86_exceptionframe_t::freg] &= ~((1 << 8) + (1 << 16));	/* !RF + !TF */
	    x86_last_ip = ~0U;
	}
	else
	{
#if defined(CONFIG_TRACEPOINTS)
	    // TCB, address, content
	    word_t db7, db6, dbnum = 0, db = 0;
	    __asm__ ("mov %%db7,%0": "=r"(db7));
	    __asm__ ("mov %%db6,%0": "=r"(db6));
	    
	    if (db6 & 8)
	    {
		dbnum = 3;
		__asm__ ("mov %%db3,%0": "=r"(db));
	    }
	    else if (db6 & 4)
	    {
		dbnum = 2;
		__asm__ ("mov %%db2,%0": "=r"(db));

	    }
	    else if (db6 & 2)
	    {
		dbnum = 1;
		__asm__ ("mov %%db1,%0": "=r"(db));

	    }
	    else if (db6 & 1)
	    {
		dbnum = 0;
		__asm__ ("mov %%db0,%0": "=r"(db));

	    }
	    else 
	    {
		printf("--- Debug Exception NO DR---\n");
	    }
	    space_t *space = kdb.kdb_current->get_space();
	    if (!space)
		space = get_kernel_space();
	    

	    word_t content;
	    ENABLE_TRACEPOINT(X86_BREAKPOINT, x86_breakpoint_cpumask, 0);

	    if  (! readmem(space, (addr_t) db, &content) )
		TRACEPOINT(X86_BREAKPOINT, "breakpoint dr%d ip: %x addr %x content ########", 
			   dbnum, f->regs[x86_exceptionframe_t::ipreg], db);
	    else
		TRACEPOINT(X86_BREAKPOINT, "breakpoint dr%d ip: %x addr %x content %x", 
			   dbnum, f->regs[x86_exceptionframe_t::ipreg], db, content);
	    
	    enter_kernel_debugger = ((x86_breakpoint_cpumask_kdb & (1 << cpu)) != 0);
#else
	    printf("--- Debug Exception ---\n");
#endif
	}
    }
    break;
    case X86_EXC_NMI:		/* well, the name says enough	*/
#if !defined(CONFIG_SMP)
    {
	printf("--- NMI  ---\n");
    } 
#else
    /* 
     * if we wanted to enter KDB, we have come here via any other
     * exception than NMI. Otherwise we're just any of the unrelated cpus
     * having been stopped via broadcast NMI. This case here occurs if we have
     * received a broadcast after a different CPU has exited KDB.
     */
    enter_kernel_debugger = false;
#endif
    break;
    case X86_EXC_BREAKPOINT: /* int3 */
    {
	space_t * space = kdb.kdb_current->get_space();
	if (!space) space = get_kernel_space();

	addr_t addr = (addr_t)(f->regs[x86_exceptionframe_t::ipreg]);
	
	unsigned char c;
	
	if (! readmem(space, addr, &c) )
	    break;

	if (c == 0x90)
	{
	    addr = addr_offset(addr, 1);
	    enter_kernel_debugger = false;
	}

	if (! readmem(space, addr, &c) )
	    break;

	/*
	 * KDB_Enter() is implemented as:
	 *
	 *	int	$3
	 *	jmp	1f
	 *	mov	$2f, %eax
	 *   1:
	 *
	 *	.rodata
	 *   2: .ascii	"output text"
	 *	.byte	0
	 *
	 */
	if (c == 0xeb) /* jmp rel */
	{
	    if (! readmem(space, addr_offset(addr, 2), &c) )
		break;

	    addr_t user_addr = NULL;
	    bool mapped = false;
	    if (c == 0xb8)
	    {

#if defined(CONFIG_X86_COMPATIBILITY_MODE)
		if (space->is_compatibility_mode() && space->is_user_area(addr))
		    /* movl addr32, %eax */
		    mapped = readmem (space, addr_offset(addr, 3), (u32_t *) &user_addr);
		else
#endif 
		    /* mov addr, AREG  */
		    mapped = readmem (space, addr_offset(addr, 3), (word_t *) (addr_t) &user_addr);
	    }
	    else if (c == 0x48)
	    {
		if (! readmem(space, addr_offset(addr, 3), &c) )
		    break;
		
		if (c == 0xc7)
		{
		    /* movq addr32, %rax */
		    s32_t suser_addr = 0;
		    mapped = readmem (space, addr_offset(addr, 5), (s32_t *) &suser_addr);
		    user_addr = (addr_t) suser_addr;
		}
		
	    }
	    if (user_addr)
	    {
		
		printf("--- \"");

		if (!mapped)
		    printf("[string address not mapped]");
		else
		{
		    while (readmem(space, user_addr, &c) && (c != 0))
		    {
			putc(c);
			user_addr = addr_offset(user_addr, 1);
		    }
		    
		    if (c != 0)
			printf("[string not completely mapped]");
		    
		    printf("\" ---\n"
				    "--------------------------------- (eip=%p, esp=%p) ---\n", 
				    f->regs[x86_exceptionframe_t::ipreg] - 1, 
				    f->regs[x86_exceptionframe_t::spreg]);
		}
	    }
	}
	/*
	 * Other kdebug operations are implemented as follows:
	 *
	 *	int	$3
	 *	cmpb	<op>, %al
	 *
	 */
	else if (c == 0x3c) /* cmpb */
	{
	    enter_kernel_debugger = false;
	    
	    if (!readmem (space, addr_offset(addr, 1), &c))
		break;

	    switch (c)
	    {
	    case 0x0:
		//
		// KDB_PrintChar()
		//
		putc(f->regs[x86_exceptionframe_t::areg]);
		break;

	    case 0x1:
	    {
		//
		// KDB_PrintString()
		//
		addr_t user_addr = (addr_t) f->regs[x86_exceptionframe_t::areg];
		while (readmem (space, user_addr, &c) && (c != 0))
		{
		    putc(c);
		    user_addr = addr_offset (user_addr, 1);
		}
		if (c != 0)
		    printf("[string not completely mapped]");
		break;
	    }

	    case 0xd:
		//
		// KDB_ReadChar_Blocked()
		//
	    	f->regs[x86_exceptionframe_t::areg] = getc (true); 
		break;
		
	    case 0x8:
		//
		// KDB_ReadChar()
		//
	    	f->regs[x86_exceptionframe_t::areg] = getc (false);
		break;

	    default:
		enter_kernel_debugger = true;
		printf("kdb: unknown opcode: int3, cmpb %d\n",
				space->get_from_user(addr_offset(addr, 1)));
		break;
	    }
	}
	else
	{
	    printf("kdb: unknown kdb op: %x ip %x\n", 
			    c, f->regs[x86_exceptionframe_t::ipreg]);
	}
    }
    break;
    default:
    {
	printf("--- KD# unknown reason %d ip %x ---\n", 
	       f->reason, f->regs[x86_exceptionframe_t::ipreg]);
	break;
    } /* switch */
    }
  
    return enter_kernel_debugger;
};



void kdb_t::post() {

    debug_param_t * param = (debug_param_t*)kdb_param;
    
    if (param->exception == X86_EXC_DEBUG)
    {
	/* Set RF in EFLAGS. This will disable breakpoints for one
	   instruction. The processor will reset it afterwards. */
	param->frame->regs[x86_exceptionframe_t::freg] |= (1 << 16);

    } /* switch */

#if defined(CONFIG_SMP) 

    if (kdb_current_cpu == get_current_cpu())
    {
	kdb_current_cpu = CONFIG_SMP_MAX_CPUS;
	
	local_apic_t<APIC_MAPPINGS_START> local_apic;
	local_apic.broadcast_nmi();
	
    }
#endif
};
