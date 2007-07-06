/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  University of New South Wales
 *                
 * File path:     kdb/platform/ofsparc64/console.cc
 * Description:   Console setup for SPARC v9 OpenBoot (Open Firmware)
 *                systems.
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
 * $Id: console.cc,v 1.6 2004/06/04 05:23:05 philipd Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/console.h>

#if CONFIG_SPARC64_SAB82532
extern void serialB_init(void);
extern void serialB_putc(char c);
extern char serialB_getc(bool block);
#endif
#if CONFIG_SPARC64_Z8530
extern void z8530_init(void);
extern void z8530_putc(char c);
extern char z8530_getc(bool block);
#endif

extern kdb_console_t serialB_console;

word_t kdb_current_console = 0;

kdb_console_t kdb_consoles[] = {
#if CONFIG_SPARC64_SAB82532
    {"Serial Port B", &serialB_init, &serialB_putc, &serialB_getc},//serialB_console,
#elif CONFIG_SPARC64_Z8530
    {"Serial Port A", &z8530_init, &z8530_putc, &z8530_getc},//z8530_console,
#endif
    KDB_NULL_CONSOLE
};

#if CONFIG_KDB_BREAKIN
void kdebug_check_breakin(void) {
    if(kdb_consoles[kdb_current_console].getc(false) == 27) {
	enter_kdebug("KDB Breakin");
    }
}
#endif
