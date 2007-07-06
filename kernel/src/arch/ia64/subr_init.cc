/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     arch/ia64/subr_init.cc
 * Description:   Various helper function for initialization
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
 * $Id: subr_init.cc,v 1.6 2003/09/30 17:23:59 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_GLUE(hwspace.h)
#include INC_ARCH(psr.h)
#include INC_ARCH(pal.h)
#include INC_ARCH(ia64.h)


/**
 * Switch to physical mode, print string to console and enter infinite
 * loop.  Used for printing diagnostic messages if something goes
 * wrong before system is fully initialized.
 * 
 * @param str	String to print onto console
 */
void SECTION (".init")
ipanic (const char * str)
{
    psr_t psr = get_psr ();

    if (psr.it)
	ia64_switch_to_phys ();

    char * screen = (char *) ((1UL << 63) + 0xb8000);
    word_t cursor = 0;

    // Clear screen
    for (word_t i = 0; i < 24*160; i += 2)
    {
	screen[i] = ' ';
	screen[i+1] = 7;
    }

    // Output string
    for (char * s = (char *) (psr.it ? virt_to_phys (str) : str); *s; s++)
    {
	switch (*s)
	{
	case '\r':
	    cursor -= (cursor % 160);
	    break;
	case '\n':
	    cursor += (160 - (cursor % 160));
	    break;
	case '\t':
	    cursor += (8 - (cursor % 8));
	    break;
	case '\b':
	    cursor -= 2;
	    break;
	default:
	    screen[cursor++] = *s;
	    screen[cursor++] = 7;
	}
    }

    for (;;);
}

void SECTION (".init")
purge_complete_tc (void)
{
    pal_status_e status;
    pal_ptce_info_t info;

    if ((status = pal_ptce_info (&info)) != PAL_OK)
    {
	printf ("Error: PAL_PTCE_INFO => %d\n", (long) status);
	return;
    }

    word_t addr = info.base;
    for (word_t i = 0; i < info.count1; i++)
    {
	for (word_t j = 0; j < info.count2; j++)
	{
	    asm volatile ("ptc.e %0" :: "r" (addr));
	    ia64_srlz_d ();
	    addr += info.stride1;
	}
	addr += info.stride1;
    }
}
