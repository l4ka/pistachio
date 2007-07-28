/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     kdb/glue/v4-ia32/prepost.cc
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
#include <kdb/console.h>

#include <debug.h>
#include <linear_ptab.h>
#include INC_API(tcb.h)

#include INC_ARCH(traps.h)	/* for IA32 exception numbers	*/
#include INC_ARCH(trapgate.h)	/* for ia32_exceptionframe_t	*/

#include INC_PLAT(nmi.h)
extern space_t *current_disas_space;

#if defined(CONFIG_KDB_DISAS)
extern "C" int disas(addr_t pc);
INLINE void disas_addr (addr_t ip, char * str = "")
{
    printf ("%p    ", ip);
    current_disas_space =  kdb.kdb_current->get_space();
    disas (ip);
    printf ("\n");
}
#else
INLINE void disas_addr (addr_t ip, char * str = "")
{
    printf ("eip=%p -- %s\n", ip, str);
}
#endif

static spinlock_t kdb_lock;

bool kdb_t::pre() 
{
    bool enter_kernel_debugger = true;

    ia32_exceptionframe_t* f = (ia32_exceptionframe_t*) kdb_param;

    switch (f->reason)
    {
    case IA32_EXC_DEBUG:	/* single step, hw breakpoints */
    {
	/* Debug exception */
	if (f->eflags & (1 << 8))
	{
	    extern u32_t ia32_last_eip;
#if defined(CONFIG_CPU_IA32_I686) || defined(CONFIG_CPU_IA32_P4)
	    extern bool ia32_single_step_on_branches;
	    if (ia32_single_step_on_branches)
	    {
		addr_t last_branch_ip;
		x86_wrmsr (IA32_DEBUGCTL, 0);
		ia32_single_step_on_branches = false;
#if defined(CONFIG_CPU_IA32_I686)
		last_branch_ip = (addr_t) (word_t)
		    x86_rdmsr (IA32_LASTBRANCHFROMIP);
#else
		last_branch_ip = (addr_t) (word_t)
		    (x86_rdmsr (IA32_LASTBRANCH_0 +
				 x86_rdmsr (IA32_LASTBRANCH_TOS)) >> 32);
#endif
		disas_addr (last_branch_ip, "branch to");
		ia32_last_eip = f->eip;
	    }
#endif
	    if (ia32_last_eip != ~0U)
		disas_addr ((addr_t) ia32_last_eip);
	    f->eflags &= ~((1 << 8) + (1 << 16));	/* !RF + !TF */
	    ia32_last_eip = ~0U;
	}
	else
	{
	    printf("--- Debug Exception ---\n");
	}
	break;
    }

    case IA32_EXC_NMI:		/* well, the name says enough	*/
	printf("--- NMI ---\n");
	break;
	
    case IA32_EXC_BREAKPOINT: /* int3 */
    {
	space_t * space = kdb.kdb_current->get_space();
	if (!space) space = get_kernel_space();

	addr_t addr = (addr_t)(f->eip);

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

	    if (c == 0xb8)
	    {
		printf("--- \"");

		/* mov addr, %eax */
		addr_t user_addr;
		if (! readmem (space, addr_offset(addr, 3),
			       (word_t *) (void *) &user_addr))
		    printf("[string address not mapped]");
		else
		{
		    while (readmem(space, user_addr, &c) && (c != 0))
		    {
			putc(c);
			user_addr = addr_offset(user_addr, 1);
		    }
		    if (c != 0)
			printf ("[string not completely mapped]");
		}

		printf("\" ---\n"
		       "--------------------------------- (eip=%p, esp=%p) ---\n", 
		       f->eip - 1, f->esp);
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
	    if (!readmem (space, addr_offset(addr, 1), &c))
		break;

	    switch (c)
	    {
	    case 0x0:
		//
		// KDB_PrintChar()
		//
		putc (f->eax);
		return false;

	    case 0x1:
	    {
		//
		// KDB_PrintString()
		//
		addr_t user_addr = (addr_t) f->eax;
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
		//
		// KDB_ReadChar_Blocked()
		//
	    	f->eax = getc (true);
		return false;

	    case 0x8:
		//
		// KDB_ReadChar()
		//
	    	f->eax = getc (false);
		return false;

	    default:
		printf("kdb: unknown opcode: int3, cmpb %d\n",
		       space->get_from_user(addr_offset(addr, 1)));
	    }
	}
    }
    break;

    default:
	printf("--- KD# unknown reason ---\n");
	break;
    } /* switch */

    if (enter_kernel_debugger)
	kdb_lock.lock();

    return enter_kernel_debugger;
};




void kdb_t::post() {

    ia32_exceptionframe_t* f = (ia32_exceptionframe_t*) kdb_param;
    
    switch (f->reason)
    {
    case IA32_EXC_DEBUG:
	/* Set RF in EFLAGS. This will disable breakpoints for one
	   instruction. The processor will reset it afterwards. */
	f->eflags |= (1 << 16);
	break;

    case IA32_EXC_NMI:
	nmi_t().unmask();
	break;

    } /* switch */

    kdb_lock.unlock();

    return;
};
