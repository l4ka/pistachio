/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/rtas.cc
 * Description:   User-level interface to RTAS procedures
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
 * $Id: rtas.cc,v 1.3 2004/06/04 06:38:41 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(syscalls.h)
#include INC_ARCH(rtas.h)
#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT (SYSCALL_RTAS_CALL);


SYS_RTAS_CALL( word_t token, word_t nargs, word_t nret, word_t ptr )
{
    TRACEPOINT( SYSCALL_RTAS_CALL,
		printf( "SYS_RTAS_CALL: (token %d) (%d in, %d out) data: %p\n",
			token, nargs, nret, ptr ) );

    pgent_t * pg;
    pgent_t::pgsize_e pgsize;
    space_t * space = get_current_space();

    // invalid request - thread not privileged
    if ( !is_privileged_space( space ) )
	return -1ul;

    /* Check for valid data */
    if (! space->lookup_mapping( (addr_t)ptr, &pg, &pgsize) )
	return -1ul;
    if (! space->lookup_mapping( (addr_t)(ptr+16*8), &pg, &pgsize) )
	return -1ul;

    /* bounds check arguments */
    if ( (nargs > 16) ||  (nret > 16) || (nargs + nret > 16) )
	return -1ul;

    return get_rtas()->rtas_call( (word_t *)ptr, token, nargs, nret );
}

