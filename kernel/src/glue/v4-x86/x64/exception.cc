/*********************************************************************
 *                
 * Copyright (C) 2002-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x64/exception.cc
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
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_GLUE(traphandler.h)
#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)


const word_t x86_exc_reg_t::mr2reg[NUM_EXC_REGS][2] = 
{    
    {    19, (word_t) ~0UL			/* EXC	*/},
    {     1, x86_exceptionframe_t::ipreg	/* RIP	*/},
    {     2, x86_exceptionframe_t::breg		/* RBX	*/},
    {     3, x86_exceptionframe_t::r10reg	/* R10	*/},
    {     4, x86_exceptionframe_t::r12reg	/* R12	*/},
    {     5, x86_exceptionframe_t::r13reg	/* R13	*/},
    {     6, x86_exceptionframe_t::r14reg	/* R14	*/},
    {     7, x86_exceptionframe_t::r15reg	/* R15	*/},
    {     8, x86_exceptionframe_t::areg  	/* RAX	*/},
    {     9, x86_exceptionframe_t::creg  	/* RCX	*/},
    {    10, x86_exceptionframe_t::dreg  	/* RDX	*/},
    {    11, x86_exceptionframe_t::Sreg  	/* RSI	*/},
    {    12, x86_exceptionframe_t::Dreg  	/* RDI	*/},
    {    13, x86_exceptionframe_t::Breg  	/* RBP	*/},	
    {    14, x86_exceptionframe_t::r8reg   	/* R8  */},
    {    15, x86_exceptionframe_t::r9reg   	/* R9   */},
    {    16, x86_exceptionframe_t::r11reg  	/* R11	*/},
    {    17, x86_exceptionframe_t::spreg  	/* RSP	*/},
    {    18, x86_exceptionframe_t::freg  	/* RFL	*/},
    {    20, x86_exceptionframe_t::ereg  	/* ERR	*/},
};

#if defined(CONFIG_DEBUG)
const word_t x86_exceptionframe_t::dbgreg[x86_exceptionregs_t::num_dbgregs] = 
{
    x86_exceptionframe_t::areg,  x86_exceptionframe_t::breg,
    x86_exceptionframe_t::creg,  x86_exceptionframe_t::dreg,
    x86_exceptionframe_t::Sreg,  x86_exceptionframe_t::Dreg,
    x86_exceptionframe_t::Breg,  x86_exceptionframe_t::freg,
    x86_exceptionframe_t::r8reg, x86_exceptionframe_t::r9reg,
    x86_exceptionframe_t::r10reg, x86_exceptionframe_t::r11reg,
    x86_exceptionframe_t::r12reg, x86_exceptionframe_t::r13reg,
    x86_exceptionframe_t::r14reg, x86_exceptionframe_t::r15reg,
    x86_exceptionframe_t::csreg, x86_exceptionframe_t::ssreg,
};

const char *x86_exceptionframe_t::name[x86_exceptionregs_t::num_regs] = 
{   "reason",    "r15",	 "r14",	 "r13",	 "r12",	 "r11",	 "r10",	 "r09",	 
    "r08",	 "rdi",	 "rsi",	 "rbp",	 "rdx",	 "rbx",	 "rcx",	 "rax",	 
    "err",	 "rip",	 "cs ",	 "rfl",	 "rsp",	 "ss "	 };    
#endif

void exc_catch_common_wrapper() 					
{							
    __asm__ (						
        ".section .data.x86.exc_common,\"aw\",@progbits		\n\t"
        ".global exc_catch_common				\n\t"
	"\t.type exc_catch_common,@function			\n\t"
	"exc_catch_common:					\n\t"
        "pushq %%rax						\n\t"
	"pushq %%rcx						\n\t"
	"pushq %%rbx						\n\t"
	"pushq %%rdx						\n\t"
	"pushq %%rbp						\n\t"
    	"pushq %%rsi						\n\t"
    	"pushq %%rdi						\n\t"
    	"pushq %%r8						\n\t"
    	"pushq %%r9						\n\t"
    	"pushq %%r10						\n\t"
    	"pushq %%r11						\n\t"
    	"pushq %%r12						\n\t" 
    	"pushq %%r13						\n\t"
    	"pushq %%r14						\n\t" 
    	"pushq %%r15						\n\t"
	"pushq %0			    			\n\t"
	"movq  %%rsp, %%rdi					\n\t"
	"call exc_catch_common_handler				\n\t"		
	"addq  $8, %%rsp					\n\t"		
    	"popq  %%r15						\n\t"		
    	"popq  %%r14						\n\t"		
    	"popq  %%r13						\n\t"		
    	"popq  %%r12						\n\t"		
    	"popq  %%r11						\n\t"		
    	"popq  %%r10						\n\t"		
    	"popq  %%r9						\n\t"		
    	"popq  %%r8						\n\t"		
    	"popq  %%rdi						\n\t"		
    	"popq  %%rsi						\n\t"		
	"popq  %%rbp						\n\t"		
	"popq  %%rdx						\n\t"		
	"popq  %%rbx						\n\t"		
	"popq  %%rcx						\n\t"		
        "popq  %%rax						\n\t"		
	"addq  $8, %%rsp					\n\t"		
	"iretq							\n\t"		
	".previous						\n\t"
	:						
	: "i"(0)					
	);						
}							
