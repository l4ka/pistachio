/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     util/kickstart/fdt.cc
 * Description:   
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
 * $Id$
 *                
 ********************************************************************/
#include "l4io.h"
#include "fdt.h"

static const char *indent = "                    ";

void fdt_t::dump()
{
    if (!is_valid())
	printf("Invalid FDT\n");

    int ilen = strlen(indent);

    int level = 0;
    fdt_node_t *node = get_root_node();

    do {
	if (node->is_begin_node()) 
	{
	    fdt_header_t* hdr = (fdt_header_t*)node;
	    printf("%s%s {\n", &indent[ilen - level * 2], 
		   level == 0 ? "/" : hdr->name);
	    level++;
	    node = get_next_node(hdr);
	} 
	else if (node->is_property_node()) 
	{
	    fdt_property_t* prop = (fdt_property_t*)node;
	    printf("%s%s\n", &indent[ilen - level * 2], prop->get_name(this));
	    node = get_next_node(prop);
	}	    
	else if (node->is_end_node()) 
	{
	    level--;
	    printf("%s}\n", &indent[ilen - level * 2]);
	    node++;
	}
	else 
	{
	    printf("unknown node type %d\n", node->tag);
	    break;
	}
    } while(level > 0);
}


fdt_header_t *fdt_t::find_subtree_node(fdt_node_t *node, char *name)
{
    int level = 0;
    do {
	if (node->is_begin_node())
	{
	    fdt_header_t* hdr = (fdt_header_t*)node;
	    if (strcmp(hdr->name, name) == 0 && level == 1)
		return hdr;
	    level++;
	    node = get_next_node(hdr);
	} 
	else if (node->is_property_node()) 
	{
	    fdt_property_t* prop = (fdt_property_t*)node;
	    node = get_next_node(prop);
	}	    
	else if (node->is_end_node()) 
	{
	    level--;
	    node++;
	}
	else 
	{
	    printf("unknown node type %d\n", node->tag);
	    break;
	}
    } while(level > 0);
    return 0;
}

fdt_property_t *fdt_t::find_property_node(fdt_node_t *node, char *name)
{
    int level = 0;
    do {
	if (node->is_begin_node())
	{
	    fdt_header_t* hdr = (fdt_header_t*)node;
	    level++;
	    node = get_next_node(hdr);
	} 
	else if (node->is_property_node()) 
	{
	    fdt_property_t* prop = (fdt_property_t*)node;
	    if (strcmp(prop->get_name(this), name) == 0 && level == 1)
		return prop;
	    node = get_next_node(prop);
	}	    
	else if (node->is_end_node()) 
	{
	    level--;
	    node++;
	}
	else 
	{
	    printf("unknown node type %d\n", node->tag);
	    break;
	}
    } while(level > 0);
    return 0;
}

fdt_property_t *fdt_t::find_property_node(char *path)
{
    fdt_node_t *node = get_root_node();
    char *next_path;

    /* remove trailing / */
    while(*path == '/')
	path++;

    for (;;)
    {
	next_path = strchr(path, '/');
	if (next_path != 0)
	{
	    *next_path = 0;
	    node = find_subtree_node(node, path);
	    *next_path = '/';
	    path = next_path + 1;
	    if (!node)
		return 0;
	}
	else
	    return find_property_node(node, path);
    } 
}

fdt_header_t *fdt_t::find_subtree(char *path)
{
    fdt_node_t *node = get_root_node();
    char *next_path;

    /* remove trailing / */
    while(*path == '/')
	path++;

    for (;;)
    {
	next_path = strchr(path, '/');
	if (next_path != 0)
	{
	    *next_path = 0;
	    node = find_subtree_node(node, path);
	    *next_path = '/';
	    path = next_path + 1;
	    if (!node)
		return 0;
	}
	else
	    return find_subtree_node(node, path);
    }
}
