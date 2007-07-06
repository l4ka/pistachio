/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	arch/powwepc64/1275tree.cc
 * Description:	Functions which enable easy access to the position-independent
 * 		Open Firmware device tree.
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
 * $Id: 1275tree.cc,v 1.3 2005/01/18 13:32:35 cvansch Exp $
 *
 ***************************************************************************/

#include INC_ARCH(string.h)
#include INC_ARCH(1275tree.h)

of1275_tree_t of1275_tree;

int of1275_device_t::get_depth()
{
    int depth = 0;
    char *c = this->name;

    while( *c )
    {
	if( *c == '/' )
	    depth++;
	c++;
    }
    return depth;
}

#include <debug.h>
bool of1275_device_t::get_prop( const char *name, char **data, u32_t *data_len )
{
    of1275_item_t *item_name, *item_data;

    item_name = this->item_first();
    item_data = item_name->next();

    for( word_t i = 0; i < this->get_prop_count(); i++ )
    {
	if( !strcmp(item_name->data, name) )
	{
	    *data = item_data->data;
	    *data_len = item_data->len;
	    return true;
	}
	item_name = item_data->next();
	item_data = item_name->next();
    }

    return false;
}

bool of1275_device_t::get_prop( word_t index, 
	char **name, char **data, u32_t *data_len )
{
    of1275_item_t *item_name, *item_data;

    if( index >= this->get_prop_count() )
	return false;

    item_name = this->item_first();
    item_data = item_name->next();

    for( word_t i = 0; i < index; i++ )
    {
	item_name = item_data->next();
	item_data = item_name->next();
    }

    *name = item_name->data;
    *data = item_data->data;
    *data_len = item_data->len;
    return true;
}

of1275_device_t * of1275_tree_t::find( const char *name )
{
    of1275_device_t *dev = this->first();
    if( !dev )
	return NULL;

    while( dev->is_valid() )
    {
	if( !strcmp_of(dev->get_name(), name) )
	    return dev;
	dev = dev->next();
    }

    return NULL;
}

of1275_device_t * of1275_tree_t::find_handle( word_t handle )
{
    of1275_device_t *dev = this->first();
    if( !dev )
	return NULL;

    while( dev->is_valid() )
    {
	if( dev->get_handle() == handle )
	    return dev;
	dev = dev->next();
    }

    return NULL;
}

of1275_device_t * of1275_tree_t::get_parent( of1275_device_t *dev )
{
    char *slash = NULL;
    int cnt, depth;

    if( !dev || !this->first() )
	return NULL;

    // Do we have any parents?
    depth = dev->get_depth();
    if( depth <= 1 )
	return NULL;

    // Locate the last slash in the name.
    for( char *c = dev->get_name(); *c; c++ )
	if( *c == '/' )
	    slash = c;
    if( slash == NULL )
	return NULL;

    // Count the offset of the last slash.
    cnt = 0;
    for( char *c = dev->get_name(); c != slash; c++ )
	cnt++;

    // Search for the parent node.
    of1275_device_t *parent = this->first();
    while( parent->is_valid() )
    {
	if( !strncmp(parent->get_name(), dev->get_name(), cnt) )
	    if( parent->get_depth() == (depth-1) )
		return parent;
	parent = parent->next();
    }

    return NULL;
}

of1275_device_t * of1275_tree_t::find_device_type( const char *device_type )
{
    of1275_device_t *dev;
    u32_t len;
    char *type;

    dev = this->first();
    if( !dev )
	return NULL;

    while( dev->is_valid() )
    {
	if( dev->get_prop("device_type", &type, &len) )
	    if( !strcmp(type, device_type) )
		return dev;
	dev = dev->next();
    }

    return NULL;
}

of1275_device_t *of1275_device_t::next_by_type( const char *device_type )
{
    of1275_device_t *dev = this;
    u32_t len;
    char *type;

    if( !dev )
	return NULL;

    while( dev->is_valid() )
    {
	if( dev->get_prop("device_type", &type, &len) )
	    if( !strcmp(type, device_type) )
		return dev;
	dev = dev->next();
    }
    return NULL;
}
