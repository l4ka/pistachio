/*********************************************************************
 *                
 * Copyright (C) 2004,  Karlsruhe University
 *                
 * File path:     bench/pingpong/mips64.h
 * Description:   MIPS64 specific pingpong functions
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
 * $Id: mips64.h,v 1.2 2005/01/12 02:59:46 cvansch Exp $
 *                
 ********************************************************************/
#define HAVE_ARCH_IPC

L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_MsgTag_t temp;
    temp.raw = 0;
    temp.X.u = untyped;
    register L4_Word_t tag asm("$3") = temp.raw;
    register L4_Word_t to asm("$4") = dest.raw;
    register L4_Word_t from asm("$5") = dest.raw;
    register L4_Word_t timeouts asm("$6") = 0;

    asm volatile (
	    "li	    $2, -101	\n\r"
	    "syscall		\n\r"
	    : /* outputs */
	      "+r" (tag)
	    : /* inputs */
	      "r" (to), "r" (from), "r" (timeouts)
	    : /* clobbers */
		"$1", "$2",
		"$7", "$8", "$9", "$10", "$11", "$12",
		"$13", "$14", "$15", "$24", "$25", /*"$28",*/ "$31",
		"$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
		"memory"
	    );

    return tag;
}
