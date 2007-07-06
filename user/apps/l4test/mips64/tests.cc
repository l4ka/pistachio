/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/mips64/tests.cc
 * Description:   Architecture dependent tests
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
 * $Id: tests.cc,v 1.3 2004/05/03 23:47:28 cvansch Exp $
 *                
 ********************************************************************/

#include "l4/thread.h"
#include <l4io.h>

#include "../l4test.h"
#include "../menu.h"
#include "../assert.h"

void fpu_test(void)
{
    L4_Word_t res = 0;
    L4_Word_t val = 0x12345678;

    printf("\nTesting FPU\n");

    asm volatile (
	"dmtc1	%1, $f9		;"
	"dmfc1	%0, $f9		;"
	: "=r" (res)
	: "r" (val)
	: "$f9"
    );

    print_result ("Register access", res == val);
}

void except_test(void)
{
    asm volatile (
	"li	$2, 400;"
	"syscall;	"
    );
}

/* the menu */
static struct menuitem menu_items[] = 
{
    { NULL, "return" },
    { fpu_test, "FPU test"},
    { except_test, "Exception test"},
};

static struct menu menu = 
{
    "MIPS64 Menu",
    0, 
    NUM_ITEMS(menu_items),
    menu_items
};


void arch_test(void)
{
    menu_input( &menu );
}

