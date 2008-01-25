/*********************************************************************
 *                
 * Copyright (C) 2002, 2004, 2007-2008,  Karlsruhe University
 *                
 * File path:     kdb/generic/console.cc
 * Description:   Generic console functionality
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
 * $Id: console.cc,v 1.5 2004/03/17 19:16:53 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <sync.h>
#include <kdb/kdb.h>
#include <kdb/cmd.h>
#include <kdb/console.h>

//#define CONFIG_SMP_OUTPUTPREFIX 1

void init_console (void)
{
    if (kdb_consoles[kdb_current_console].init)
	kdb_consoles[kdb_current_console].init ();
}


#if defined(CONFIG_SMP_OUTPUTPREFIX)
static bool use_cpuprefix = false;

DECLARE_CMD (cmd_toggle_cpuprefix, config, 'p', "cpuprefix",
	     "Toggle CPU prefix");

CMD (cmd_toggle_cpuprefix, cg)
{
    use_cpuprefix = ! use_cpuprefix;
    printf ("CPU output-prefix: %s\n", use_cpuprefix ? "on" : "off");
    return CMD_NOQUIT;
}
#endif

void SECTION(SEC_KDEBUG) putc (char c)
{
    kdb_console_t * cons = &kdb_consoles[kdb_current_console];


#if defined(CONFIG_SMP_OUTPUTPREFIX)
    static bool beginning_of_line = true;
    if (beginning_of_line)
    {
	if (use_cpuprefix)
	{
	    word_t cpuid = dbg_get_current_cpu ();
	    u16_t dbg_get_current_cpu();
	    cons->putc ('['); cons->putc ('C'); cons->putc ('P');
	    cons->putc ('U'); cons->putc (' ');
	    if (cpuid >= 10)
		cons->putc ('0' + ((cpuid / 10) % 10));
	    cons->putc ('0' + (cpuid % 10));
	    cons->putc (']'); cons->putc (' ');
	}
    }
#endif
    
    cons->putc (c);

}

char getc (bool block)
{
    return kdb_consoles[kdb_current_console].getc (block);
}

DECLARE_CMD (cmd_toggle_console, config, 'c', "console", "Toggle console");

CMD (cmd_toggle_console, cg)
{
    word_t newcons = kdb_current_console + 1;

    if (kdb_consoles[newcons].name == NULL)
	newcons = 0;

    printf ("Switch console: %s\n", kdb_consoles[newcons].name);
    kdb_current_console = newcons;

    if (kdb_consoles[kdb_current_console].init)
	kdb_consoles[kdb_current_console].init ();

    return CMD_NOQUIT;
}


