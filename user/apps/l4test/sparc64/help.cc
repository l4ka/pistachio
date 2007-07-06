/*********************************************************************
 *                
 * Copyright (C) 2003,  University of New South Wales
 *                
 * File path:     l4test/sparc64/help.cc
 * Description:   Helper finctions for SPARC v9
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
 * $Id: help.cc,v 1.3 2004/05/21 01:50:00 philipd Exp $
 *                
 ********************************************************************/

#include <l4/types.h>
#include <arch.h>

#include "../l4test.h"
#include "../assert.h"

/* setup an exreg 
 *
 * *sp == NULL: allocate a new stack
 * *sp != NULL: top of sp from an old exreg used, just refresh it!
 */
void
setup_exreg(L4_Word_t *ip, L4_Word_t *sp, void (*func)(void))
{
    L4_Word_t *stack;
    int max;

    /* allocate a stack, or perhaps not */
    max = STACK_PAGES * PAGE_SIZE / sizeof( L4_Word_t );
    if( *sp == NULL )
    {
	stack = (L4_Word_t*) get_pages( STACK_PAGES, 1 );
	assert( stack != NULL );

	stack = &stack[max-1];
    }
    else
	stack = (L4_Word_t*) *sp;

    /* set the return params */
    *sp = (L4_Word_t) stack;
    *ip = (L4_Word_t) func;
} // setup_exreg()

void *
code_addr(void *addr)
{
    return addr;
} // code_addr()

void
get_startup_values(void (*func)(void), L4_Word_t * ip, L4_Word_t * sp)
{
    // Calculate intial SP
    L4_Word_t stack = (L4_Word_t) get_pages (STACK_PAGES, 1);
    stack += STACK_PAGES * PAGE_SIZE;

    *ip = (L4_Word_t) func;
    *sp = stack;
} // get_startup_values()
