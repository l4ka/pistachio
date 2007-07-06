/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/io.cc
 * Description:	Open Firmware i/o wrappers.
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
 * $Id: io.cc,v 1.14 2004/03/17 19:13:25 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>

#include <kdb/console.h>
#include INC_PLAT(1275tree.h)	// the device tree
#include INC_PLAT(ofppc.h)	// global symbols

#include INC_ARCH(string.h)
#include INC_API(space.h)	// to map devices

#include "of1275.h"
#include "ofppc.h"

#define PROM_NAME	"of1275"
#define COM_NAME	"com"

static void switch_console( const char *name )
{
    for( int i = 0; kdb_consoles[i].name; i++ )
	if( !strcmp(kdb_consoles[i].name, name) )
	{
	    kdb_current_console = i;
	    return;
	}
}

/****************************************************************************
 *
 *    Open Firmware console support
 *
 ****************************************************************************/

#if defined(CONFIG_KDB_CONS_OF1275)

void init_of1275_console( word_t entry )
{
    get_of1275_space()->init( 
	    (word_t)ofppc_stack_top(), (word_t)ofppc_stack_bottom() );
    get_of1275_ci()->init( entry );

    switch_console( PROM_NAME );
    printf( "Activated the Open Firmware console.\n" );
}

static void putc_of1275( char c )
{
    if( c == '\n' )
    {
	char nl[] = "\r\n";
	get_of1275_ci()->write( get_of1275_ci()->get_stdout(), nl, sizeof(nl) );
    }
    else
	get_of1275_ci()->write( get_of1275_ci()->get_stdout(), &c, 1 );
}

static char getc_of1275( bool block )
{
    char c;
    int cnt;

    do {
	cnt = get_of1275_ci()->read( get_of1275_ci()->get_stdin(), &c, 1 );
    } while( cnt == 0 );

    if( cnt != 1 )
	return 0;
    return c;
}

#endif /* CONFIG_KDB_CONS_OF1275 */

/****************************************************************************
 *
 *    psim com support
 *
 *    Consult gdb/sim/ppc/hw_com.c for details of this psuedo device.
 *
 ****************************************************************************/

#if defined(CONFIG_KDB_CONS_PSIM_COM)
static char *com_registers = NULL;

void init_psim_com_console( void )
{
    of1275_device_t *dev;
    char *alias;
    word_t *reg;
    word_t len;

    dev = get_of1275_tree()->find( "/aliases" );
    if( dev == NULL )
	return;
    if( !dev->get_prop("com", &alias, &len) )
	return;
 
    dev = get_of1275_tree()->find( alias );
    if( dev == NULL )
	return;
    if( !dev->get_prop("reg", (char **)&reg, &len) )
	return;

    if( (len != 3*sizeof(word_t)) || !reg[1] )
	return;

    com_registers = (char *)
	get_kernel_space()->map_device( (addr_t)reg[1], POWERPC_PAGE_SIZE, 
		false );

    if( com_registers )
    {
	com_registers[3] = 0;	// Some initialization ...
	switch_console( COM_NAME );
	printf( "Activated the psim com console.\n" );
    }
}

static void putc_com( char c )
{
    if( com_registers )
	com_registers[0] = c;
}

static char getc_com( bool block )
{
    if( com_registers )
	return com_registers[0];
    else
	return 0;
}

#endif	/* CONFIG_KDB_CONS_PSIM_COM */

/****************************************************************************
 *
 *    Console registration
 *
 ****************************************************************************/

kdb_console_t kdb_consoles[] = {
#if defined(CONFIG_KDB_CONS_OF1275)
    { PROM_NAME, NULL, putc_of1275, getc_of1275 },
#endif
#if defined(CONFIG_KDB_CONS_PSIM_COM)
    { COM_NAME, NULL, putc_com, getc_com },
#endif
    KDB_NULL_CONSOLE
};

word_t kdb_current_console = 0;

