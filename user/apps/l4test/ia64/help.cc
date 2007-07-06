/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/ia64/help.cc
 * Description:   Helper functions for ia64
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
 * $Id: help.cc,v 1.6 2003/09/24 19:06:00 skoglund Exp $
 *                
 ********************************************************************/

#include <l4/types.h>
#include <config.h>

/* for the current arch */
#include <arch.h>

/* generic stuff */
#include "../l4test.h"
#include "../menu.h"
#include "../assert.h"

extern L4_Word_t _exreg_target;

/* setup an exreg 
 *
 * *sp == NULL: allocate a new stack
 * *sp != NULL: top of sp from an old exreg used, just refresh it!
 */
void 
setup_exreg( L4_Word_t *ip, L4_Word_t *sp, void (*func)(void) )
{
	L4_Word_t *stack, *hack_ip;
	int max;

	/* get the function address */
	hack_ip = (L4_Word_t*) func;

	/* work out the size in items */
	max = STACK_PAGES * PAGE_SIZE / sizeof( L4_Word_t );
	if( *sp == 0 )
	{
		stack = (L4_Word_t*) get_pages( STACK_PAGES, 1 );
		assert( stack != NULL );

		stack = &stack[max-2];
	}
	else
		stack = (L4_Word_t*) *sp;

	/* work out the size */
	stack[-1] = *hack_ip;      /* get the entry pt out */
	stack[-2] = (L4_Word_t) &stack[2-max]; /* RSE ptr */

	/* set their values */
	*ip = (L4_Word_t) &_exreg_target;
	*sp = (L4_Word_t) (stack - 2);
}

/* get the 1st word out of the function descriptor (stinkin gp!) */
void *
code_addr( void *addr )
{
	L4_Word_t *ptr;

	ptr = (L4_Word_t*) addr;
	return (void*) *ptr;
}



/*
 * Startup stub for setting up the appropriate GP, BSP and IP values.
 */
asm ("		.align	16				\n"
     "		.global _startup_stub			\n"
     "		.proc	_startup_stub			\n"
     "							\n"
     "_startup_stub:					\n"
     "		movl	gp = __gp			\n"
     "		ld8	r8 = [sp], 8			\n"
     "		;;					\n"
     "		ld8	r9 = [sp], 8			\n"
     "		;;					\n"
     "		mov	b0 = r9				\n"
     "		alloc	r10 = ar.pfs,0,0,0,0		\n"
     "		mov	ar.rsc = 0			\n"
     "		;;					\n"
     "		loadrs					\n"
     "		;;					\n"
     "		mov	ar.bspstore = r8		\n"
     "		;;					\n"
     "		mov	ar.rsc = 3			\n"
     "		br.sptk.many b0				\n"
     "							\n"
     "		.endp	_startup_stub			\n");

extern L4_Word_t _startup_stub;


void
get_startup_values (void (*func)(void), L4_Word_t * ip, L4_Word_t * sp)
{
    // Calculate intial SP and BSP values
    L4_Word_t regstack = (L4_Word_t) get_pages (STACK_PAGES, 1);
    L4_Word_t *stack = (L4_Word_t *) (regstack + STACK_PAGES * PAGE_SIZE);
    stack -= 2;

    // Push IP and BSP onto stack
    *--stack = *(L4_Word_t *) func;
    *--stack = regstack;

    *ip = (L4_Word_t) &_startup_stub;
    *sp = (L4_Word_t) stack;
}
