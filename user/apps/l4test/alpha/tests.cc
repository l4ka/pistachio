/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     l4test/alpha/tests.cc
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
 * $Id: tests.cc,v 1.4 2003/11/03 08:01:40 sjw Exp $
 *                
 ********************************************************************/

#include <l4/ipc.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4io.h>
#include <config.h>

#include <arch.h>

#include "../l4test.h"
#include "../menu.h"
#include "../assert.h"

void sleeper(void)
{
    printf("Pre-sleep\n");
    L4_Sleep(L4_Never);
    printf("Post-sleep\n");

}

void fp_other(void)
{
    L4_ThreadId_t t;
    L4_Word_t res;
    double test asm("$f0");

    test = 1.0;
    for(int i = 0; i < 100000; i++) {
	test += 1.0;
    }

    L4_Wait(&t);
    printf("result == %f\n", test);
    L4_Call(t);
}

void fp_test(void)
{
    L4_ThreadId_t tid = create_thread(fp_other, false, false);

    double test asm("$f0") = 32.0;

    for(int i = 0; i < 100000; i++) {
	asm volatile ("addt $f0, $f31, $f0");
    }

    L4_Call(tid);
    printf("result == %f\n", test);
    kill_thread(tid);
}


void clock_test(void)
{
    L4_Word64_t clock1, clock2, clock3;

    clock1 = L4_SystemClock();
    clock2 = L4_SystemClock();

    L4_Sleep(L4_TimePeriod(1500)); /* milliseconds */

    clock3 = L4_SystemClock();

    printf("%ld %ld %ld\n", clock1, clock2, clock3);

}



/* the menu */
static struct menuitem menu_items[] = 
{
    { NULL, "return" },
    { sleeper,  "Sleeper" },
    { fp_test, "FP test"},
    { clock_test, "Clock test"},
};

static struct menu menu = 
{
    "Alpha Menu",
    0, 
    NUM_ITEMS(menu_items),
    menu_items
};


void arch_test(void)
{
    menu_input( &menu );
}

