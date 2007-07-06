/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	platform/ofppc/ofppc.cc
 * Description:	Dump info from the position-independent device tree.
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
 * $Id: ofppc.cc,v 1.5 2003/09/24 19:05:20 skoglund Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/input.h>

#include "ofppc.h"
#include "of1275.h"

#include INC_PLAT(1275tree.h)

DECLARE_CMD_GROUP( platform );
DECLARE_CMD( cmd_platform, root, 'P', "platform", "platform specifics" );
DECLARE_CMD( cmd_dump_1275tree, platform, 'd', "devtree", "Open Firmware device tree" );
DECLARE_CMD( cmd_dump_complete_tree, platform, 'D', "tottree", "Open Firmware device tree and properties" );
DECLARE_CMD( cmd_dump_translations, platform, 't', "translations", "Open Firmware device translations" );


CMD(cmd_platform, cg)
{
    return platform.interact( cg, "platform" );
}


CMD(cmd_dump_1275tree, cg)
    /* Print all of the device names.
     */
{
    of1275_device_t *dev;

    dev = get_of1275_tree()->first();
    if( !dev )
    {
	printf( "No device tree found.\n" );
	return CMD_NOQUIT;
    }

    int tot = 1;
    while( dev->is_valid() )
    {
	printf( "%x: %s\n", dev->get_handle(), dev->get_name() );
	dev = dev->next();

	if( !(tot % 23) )
	    if( get_choice("Continue?", "Continue/Quit", 'c') == 'q' )
		break;
	tot++;
    }

    return CMD_NOQUIT;
}

bool is_string( char *data, word_t data_len )
{
    return data[data_len-1] == '\0';
}

CMD(cmd_dump_complete_tree, cg)
    /* Print all device names, and their property names.
     * Try to print some of the property values by guessing their
     * types.
     */
{
    of1275_device_t *dev;

    dev = get_of1275_tree()->first();
    if( !dev )
    {
	printf( "No device tree found.\n" );
	return CMD_NOQUIT;
    }

    int tot = 1;
    bool quit = false;
    while( dev->is_valid() )
    {
	// The device name.
	printf( "%x: %s\n", dev->get_handle(), dev->get_name() );
	tot++;

	// Walk the properties.
	for( word_t prop = 0; prop < dev->get_prop_count(); prop++ )
	{
	    char *prop_name, *prop_data;
	    word_t prop_len;

	    // Print the property.
	    dev->get_prop( prop, &prop_name, &prop_data, &prop_len );
	    printf( "\t%s [%d]", prop_name, prop_len );
	    if( prop_len == 4 )
		printf( ": 0x%08x", *(word_t *)prop_data );
	    if( is_string(prop_data, prop_len) )
		printf( ": %s", prop_data );
	    printf( "\n" );
	    tot++;

	    // Prompt to continue dumping.
	    if( !(tot % 23) )
		if( get_choice("Continue?", "Continue/Quit", 'c') == 'q' )
		{
		    quit = true;
		    break;
		}
	    tot++;
	}
	if( quit )
	    break;

	dev = dev->next();

	// Prompt to continue dumping.
	if( !(tot % 23) )
	    if( get_choice("Continue?", "Continue/Quit", 'c') == 'q' )
		break;
    }

    return CMD_NOQUIT;
}


typedef struct
{
    word_t vaddr;
    word_t size;
    word_t paddr;
    word_t mode;
} of1275_map_t;

CMD(cmd_dump_translations, cg)
{
    of1275_device_t *dev;
    word_t handle;
    of1275_map_t *mappings;
    word_t len;

    dev = get_of1275_tree()->find( "/chosen" );
    if( !dev )
	goto abort;
    if( !dev->get_prop("mmu", &handle) )
	goto abort;

    dev = get_of1275_tree()->find_handle( handle );
    if( !dev )
	goto abort;
    if( !dev->get_prop( "translations", (char **)&mappings, &len) )
	goto abort;

    len = len / sizeof(of1275_map_t);
    for( word_t i = 0; i < len; i++ )
	printf( "paddr %p, vaddr %p, size %p, mode %4x\n",
		mappings[i].paddr, mappings[i].vaddr, mappings[i].size,
		mappings[i].mode );

    return CMD_NOQUIT;
abort:
    printf( "Unable to find translations\n" );
    return CMD_NOQUIT;
}


#if defined(CONFIG_KDB_CONS_OF1275)

of1275_space_t of1275_space;

void of1275_space_t::init( word_t stack_top, word_t stack_bottom )
{
    this->lock.init();

    this->of1275_stack_top = stack_top;
    this->of1275_stack_bottom = stack_bottom;

    this->of1275_ptab_loc = this->get_ptab_loc();
    this->get_segments( this->of1275_segments );
}

extern "C" word_t kdb_switch_space( 
	word_t of1275_htab, word_t *of1275_segments, 
	word_t old_htab, word_t *old_segments,
	word_t new_stack, word_t (*func)(void *), void *param );

word_t of1275_space_t::execute_of1275( word_t (*func)(void *), void *param )
{
    word_t result;
    word_t *sp;

    this->lock.lock();

    // Choose a stack.  Note: to avoid TLB faults, it is important that we 
    // use a stack mapped by a bat register.  It is also important to use a 
    // large stack with sufficient space for Open Firmware.  The boot stack
    // is mapped by a bat, and rather large.
    if( this->using_of1275_stack() )
	sp = NULL;
    else
    {
	sp = (word_t *)(this->of1275_stack_top-16);
	sp[0] = sp[1] = sp[2] = sp[3] = 0;
    }

    // Preserve the current address space settings.
    this->current_ptab_loc = this->get_ptab_loc();
    this->get_segments( this->current_segments );

    // Execute the function within the 1275 address space.
    result = kdb_switch_space( this->of1275_ptab_loc, this->of1275_segments,
	    this->current_ptab_loc, this->current_segments,
	    this->of1275_stack_top-16, func, param );

    this->lock.unlock();

    return result;
}

#endif	/* CONFIG_KDB_CONS_OF1275 */
