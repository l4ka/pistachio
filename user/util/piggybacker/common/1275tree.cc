/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/1275tree.cc
 * Description:	Builds a position independent copy of the Open Firmware
 * 		device tree.
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
 * $Id: 1275tree.cc,v 1.7 2005/01/19 14:00:09 cvansch Exp $
 *
 ***************************************************************************/

#include <l4/types.h>
#include <piggybacker/string.h>
#include <piggybacker/ieee1275.h>
#include <piggybacker/1275tree.h>
#include <piggybacker/io.h>

#define NAME_BUFSIZ	256

/****************************************************************************/

device_t *device_find( device_t *list, const char *name )
{
    while( list->handle )
    {
	if( !strcmp_of(list->name, name) )
	    return list;
	list = device_next( list );
    }
    return (device_t *)0;
}

device_t *device_find_handle( device_t *list, L4_Word32_t handle )
{
    while( list->handle )
    {
	if( list->handle == handle )
	    return list;
	list = device_next( list );
    }
    return (device_t *)0;
}

item_t *item_find( device_t *dev, const char *name )
{
    L4_Word_t i;
    item_t *item_name, *item_data;

    i = 0;
    item_name = item_first( dev );
    item_data = item_next( item_name );

    while( i < dev->prop_count )
    {
	if( !strcmp(item_name->data, name) )
	    return item_data;
	item_name = item_next( item_data );
	item_data = item_next( item_name );
	i++;
    }

    return (item_t *)0;
}

/****************************************************************************/

static void spill_node( device_t *dev, prom_handle_t node )
{
    item_t *item, *prev_item, *prop;
    int r;
    L4_Word_t start_offset;

    dev->prop_count = 0;
    dev->prop_size = 0;

    item = item_first( dev );
    start_offset = (L4_Word_t)item;

    /* Initialize name pointers. */
    prev_item = item;
    *prev_item->data = '\0';	/* Seed for the first lookup. */

    /* Get the first prop name. */
    r = prom_next_prop( node, prev_item->data, item->data );

    /* Process the properties. */
    while( r > 0 ) 
    {
	//puts( item->data );
	dev->prop_count++;
	prev_item = item;

	/* Set the length of the property name. */
	item->len = strlen( item->data ) + 1;	/* Include the null char. */

	/* Grab the property and its length. */
	prop = item_next( item );
	prop->len = prom_get_prop_len( node, item->data );
	if( prop->len < 0 )
	    prop->len = 0;
	else
	{
	    prom_get_prop( node, item->data, prop->data, prop->len );

	    if( (prop->len == 4) && 
		    !strcmp_of(dev->name, "/chosen") )
	    {
		// Convert instance handles into package handles.
		L4_Word32_t *handle = (L4_Word32_t *)&prop->data;
		*handle = (L4_Word_t)prom_instance_to_package( (void*)(L4_Word_t)*handle );
	    }
	}

	/* Move to the next item. */
	item = item_next( prop );
	/* Grab the next property name. */
	r = prom_next_prop( node, prev_item->data, item->data );
    }

    dev->prop_size = (L4_Word_t)item - start_offset;
}

/****************************************************************************/

static void shift_str( char *name, int shift_point )
    /* Open Firmware can be buggy and return strings that have null 
     * characters in the wrong places.
     */
{
    do {
	name[shift_point] = name[shift_point+1];
	shift_point++;
    } while( name[shift_point] );
}


L4_Word_t build_device_tree( char *spill )
{
    prom_handle_t next, node;
    int len;
    device_t *dev;

    node = prom_nav_tree( ROOT_PROM_HANDLE, "peer" );

    dev = device_first( spill );

    while( node != INVALID_PROM_HANDLE ) {
	/* Spill info about the current node. */
	len = prom_package_to_path( node, dev->name, NAME_BUFSIZ );
	if( len > 0 ) {
	    dev->name[len+1] = '\0';
	    dev->len = len + 1;	/* Include the null character. */
	    dev->handle = (L4_Word_t)node;
	    if( (int)strlen(dev->name) != len )
		shift_str( dev->name, strlen(dev->name) );
	    spill_node( dev, node );
	    dev = device_next( dev );
	}

	/* Look for children of the current node. */
	next = prom_nav_tree( node, "child" );
	if( next != INVALID_PROM_HANDLE ) {
	    node = next;
	    continue;
	}

	/* Look for siblings of the current node. */
	next = prom_nav_tree( node, "peer" );
	if( next != INVALID_PROM_HANDLE ) {
	    node = next;
	    continue;
	}

	/* Search for a parent with a sibling. */
	while( 1 ) {
    	    node = prom_nav_tree( node, "parent" );
    	    if( node == INVALID_PROM_HANDLE )
    		break;
	    next = prom_nav_tree( node, "peer" );	/* Parent's sibling. */
	    if( next != INVALID_PROM_HANDLE ) {
		node = next;
		break;
	    }
	}
    }

    /* Terminate the data structure with a null record. */
    dev->handle = dev->len = 0;
    dev->prop_count = dev->prop_size = 0;

    return (L4_Word_t)dev - (L4_Word_t)spill;
}

