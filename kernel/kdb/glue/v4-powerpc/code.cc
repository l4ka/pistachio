/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	kdb/glue/v4-powerpc/code.cc
 * Description:	Test out code sequences
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
 * $Id: code.cc,v 1.2 2003/09/24 19:05:17 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>

#include INC_API(tcb.h)
#include INC_API(space.h)

DECLARE_CMD_GROUP( code );
DECLARE_CMD( cmd_code, arch, 'c', "code", "Code menu" );

DECLARE_CMD( cmd_tid_to_tcb, code, 't', "tid2tcb", "TID to TCB" );

CMD( cmd_code, cg )
{
    return code.interact( cg, "code" );
}

CMD( cmd_tid_to_tcb, cg )
{
    threadid_t tid;
    tcb_t *tcb;
    word_t loc;

    tid.set_global_id( 5, 1 );
    tcb = get_kernel_space()->get_tcb( tid );

    asm volatile (
	    "rlwinm %0, %1, 32 - (%2 - %3), (%2 - %3), 31 - %3"
	    :
	      "=r" (loc)
	    :
	      "r" (tid.get_raw()), "i" (L4_GLOBAL_VERSION_BITS), "i" (KTCB_BITS)
	    );
    loc += KTCB_AREA_START;

    printf( "tid: %x, location: %x, tcb: %x\n", tid.get_raw(), loc, tcb );

    return CMD_NOQUIT;
}
