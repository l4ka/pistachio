/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2007,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/exception.cc
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
 * $Id: exception.cc,v 1.39 2006/10/19 22:57:39 ud3 Exp $
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


const word_t x86_exc_reg_t::mr2reg[NUM_EXC_REGS][2] = 
{    
    {  3, ~0UL  /* EXC */ }, 
    {  1, x86_exceptionframe_t::ipreg  /* EIP */ }, 
    {  2, x86_exceptionframe_t::freg   /* EFL */ },
    {  4, x86_exceptionframe_t::ereg   /* ERR */},  
    {  5, x86_exceptionframe_t::Dreg   /* EDI */},  
    {  6, x86_exceptionframe_t::Sreg   /* ESI */},  
    {  7, x86_exceptionframe_t::Breg   /* EBP */}, 
    {  8, x86_exceptionframe_t::spreg  /* ESP */},  
    {  9, x86_exceptionframe_t::breg   /* EBX */},  
    { 10, x86_exceptionframe_t::dreg   /* EDX */}, 
    { 11, x86_exceptionframe_t::creg   /* ECX */}, 
    { 12, x86_exceptionframe_t::areg   /* EAX */ }
};


#if defined(CONFIG_DEBUG)
const word_t x86_exceptionframe_t::dbgreg[x86_exceptionframe_t::num_dbgregs] = 
{
    x86_exceptionframe_t::areg,  x86_exceptionframe_t::breg,
    x86_exceptionframe_t::creg,  x86_exceptionframe_t::dreg,
    x86_exceptionframe_t::Sreg,  x86_exceptionframe_t::Dreg,
    x86_exceptionframe_t::Breg,  x86_exceptionframe_t::freg,
    x86_exceptionframe_t::csreg, x86_exceptionframe_t::ssreg,
    x86_exceptionframe_t::dsreg, x86_exceptionframe_t::esreg,
};
    
const char *x86_exceptionframe_t::name[x86_exceptionframe_t::num_regs] = 
{  "rsn", "es ", "ds ", "edi",	"esi", "ebp",    0, "ebx",		
   "edx", "ecx", "eax", "err", "eip", "cs ", "efl", "esp", "ss "		
};    
#endif


void exc_catch_common_wrapper() 
{							
    __asm__ (						
        ".section .data.x86.exc_common,\"aw\",@progbits		\n\t"
	".global exc_catch_common				\n\t"
	"\t.type exc_catch_common,@function			\n\t"
	"exc_catch_common:					\n\t"
	"pusha							\n\t"
	"push	%%ds						\n\t"
	"push	%%es						\n\t"
	"push	%0						\n\t"
	"push	%%esp						\n\t"
	"call  exc_catch_common_handler				\n\t"		
	"addl  $8, %%esp					\n\t"		
	"popl	%%es						\n\t"
	"popl	%%ds						\n\t"
	"popa							\n\t"
	"addl	$4, %%esp					\n\t"
	"iret							\n\t"		
	".previous						\n\t"
	:						
	: "i"(0)					
	);						
}							
