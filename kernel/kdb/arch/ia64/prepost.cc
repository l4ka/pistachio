/*********************************************************************
 *                
 * Copyright (C) 2002-2004,  Karlsruhe University
 *                
 * File path:     kdb/arch/ia64/prepost.cc
 * Description:   Entry and exit stubs for kernel debugger
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
 * $Id: prepost.cc,v 1.25 2004/06/02 08:41:42 sgoetz Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/console.h>
#include <linear_ptab.h>

#include INC_ARCH(instr.h)
#include INC_GLUE(context.h)
#include INC_GLUE(registers.h)
#include INC_API(tcb.h)


#if defined(CONFIG_KDB_DISAS)

extern "C" int disas (addr_t ip);
#define DISAS(ip) do {				\
    printf ("%p: ", ip);			\
    disas (ip);					\
    printf ("\n");				\
} while (0)
#define BSRC(ip) printf ("src - ")
#define BDST(ip) printf ("dst - ")

#else /* !CONFIG_KDB_DISAS */
#define DISAS(ip)
#define BSRC(ip) printf ("Branch %p ===> ", ip)
#define BDST(ip) printf ("%p\n", ip)
#endif


bool kdb_t::pre (void)
{
    ia64_exception_context_t * frame = (ia64_exception_context_t *) kdb_param;
    tcb_t * current = addr_to_tcb (frame);
    space_t * space = current->get_space ();

    ia64_instr_t i0, i1, i2;
    ia64_bundle_t bundle;

    if (frame->exception_num == 11)
    {
	if (space->is_user_area (frame->iip))
	{
	    // Instruction may not be mapped in the DTLB
	    space->readmem (frame->iip, &bundle.raw64[0]);
	    space->readmem (addr_offset (frame->iip, sizeof (word_t)),
			    &bundle.raw64[1]);
	}
	else
	{
	    // Instruction should be mapped by kernel TR
	    bundle.raw64[0] = ((word_t *) frame->iip)[0];
	    bundle.raw64[1] = ((word_t *) frame->iip)[1];
	}

	i0 = bundle.slot (0);
	i1 = bundle.slot (1);
	i2 = bundle.slot (2);
    }

    /*
     * Check for single step traps.
     */
    if (frame->exception_num == 35 || frame->exception_num == 36)
    {
	addr_t ip = addr_offset (frame->iipa,
				 frame->isr.instruction_slot * 6);
	if (frame->exception_num == 35)
	    BSRC (ip);
	DISAS (ip);
	if (frame->exception_num == 35)
	{
	    ip = addr_offset (frame->iip, frame->ipsr.ri * 6);
	    BDST (ip);
	    DISAS (ip);
	}
	frame->ipsr.ss = 0;
	frame->ipsr.tb = 0;
	return true;
    }

    else if (frame->exception_num == 29)
    {
	addr_t ip = addr_offset (frame->iip, frame->ipsr.ri * 6);
	if (frame->isr.rwx == 1)
	    printf ("Debug exec fault @ %p [current=%t]\n", ip, current);
	else
	    printf ("Debug %s fault @ %p (data @ %p) [current=%t]\n",
		    frame->isr.rwx == 2 ? "write" :
		    frame->isr.rwx == 4 ? "read" : "read/write",
		    ip, frame->ifa, current);
	DISAS (ip);
	return true;
    }

    /*
     * Check if break instruction is a kdebug operation.  Kdebug
     * operations have the following format:
     *
     *   { .mlx
     *   (qp) break.m	<type>
     *   (qp) movl	r0 = <arg> ;;
     *   }
     */
    else if (frame->exception_num == 11 &&
	     bundle.get_template () == ia64_bundle_t::mlx_s3 &&
	     i0.m_nop.is_break () &&
	     i2.x_movl.is_movl () && i2.x_movl.reg () == 0)
    {
	switch (i0.m_nop.immediate ())
	{
	case 3:
	{
	    //
	    // enter_kdebug (string)
	    //

	    addr_t addr = (addr_t) i2.x_movl.immediate (i1);

	    if (! space->is_user_area (addr) && frame->ipsr.cpl != 0)
	    {
		printf (TXT_BRIGHT "--- KD# [Invalid string @ %p] ---"
			TXT_NORMAL "\n", addr);
	    }
	    else
	    {
		char c;
		printf (TXT_BRIGHT "--- KD# ");
		while (readmem (space, addr, &c) && (c != 0))
		{
		    putc (c);
		    addr = addr_offset (addr, 1);
		}
		printf (" ---" TXT_NORMAL "\n");
	    }

	    return true;
	}

	case 1:
	    //
	    // PrintChar (r14)
	    //
	    printf ("%c", frame->r14);
	    return false;

	case 2:
	    //
	    // r14 = GetChar_Blocked ()
	    //
	    frame->r14 = getc (true);
	    return false;

	case 4:
	    //
	    // r14 = GetChar ()
	    //
	    frame->r14 = getc (false);
	    return false;

	default:
	    printf (TXT_BRIGHT "--- Unknown KDB operation (%d) ---"
		    TXT_NORMAL "\n", i0.m_nop.immediate ());
	    return true;
	}
    }

    return true;
}


void kdb_t::post (void)
{
    ia64_exception_context_t * frame = (ia64_exception_context_t *) kdb_param;

    if (frame->exception_num == 29)
    {
	if (frame->isr.rwx == 1)
	    frame->ipsr.id = 1;
	else
	    frame->ipsr.dd = 1;
    }
}

