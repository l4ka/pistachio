/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     platform/ppc44x/fdt.h
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
#ifndef __PLATFORM__PPC44X__FDT_H__
#define __PLATFORM__PPC44X__FDT_H__

#include INC_ARCH(string.h)

class fdt_reserve_entry_t
{
public:
    u64_t address;
    u64_t size;
};

class fdt_t;
class fdt_header_t;
class fdt_property_t;

enum {
    fdt_begin_node = 1,
    fdt_end_node = 2,
    fdt_property_node = 3,
};

class fdt_node_t
{
public:
    u32_t tag;

    bool is_begin_node()
	{ return tag == fdt_begin_node; }
    bool is_end_node()
	{ return tag == fdt_end_node; }
    bool is_property_node()
	{ return tag == fdt_property_node; }
};

class fdt_header_t : public fdt_node_t
{
public:
    char name[0];

    int get_size()
	{ return sizeof(fdt_header_t) + (strlen(name) + 4) & ~3; }
};

class fdt_t
{
public:
    bool is_valid()
	{ return magic == 0xd00dfeed; }
    word_t get_size()
	{ return size; }

    fdt_node_t *get_root_node()
	{ return (fdt_node_t*)((word_t)this + offset_dt_struct); }

    template <typename T> fdt_node_t *get_next_node(T *p)
	{ return (fdt_node_t*)((word_t)p + p->get_size()); }

    fdt_property_t *find_property_node(fdt_node_t *node, char *name);
    fdt_header_t *find_subtree_node(fdt_node_t *node, char *name);

    fdt_header_t *find_first_subtree_node(fdt_node_t *node)
	{ return next_subtree_node(node, false); }
    fdt_header_t *find_next_subtree_node(fdt_header_t *curr)
	{ return next_subtree_node(curr, true); }

    fdt_property_t *find_property_node(char *path);
    fdt_header_t *find_subtree(char *path);

    void dump();
private:
    fdt_header_t *next_subtree_node(fdt_node_t *node, bool cont);

public:
    u32_t magic;
    u32_t size;
    u32_t offset_dt_struct;	/* offset to structure */
    u32_t offset_dt_strings;	/* offset to strings */
    u32_t offset_mem_reserve_map; /* offset to memory map */
    u32_t version;
    u32_t last_compatible_version;
    u32_t boot_cpuid_phys;
    u32_t dt_string_size;
    u32_t dt_struct_size;
};

class fdt_property_t : public fdt_node_t
{
public:
    int get_size()
	{ return sizeof(fdt_property_t) + (len - 1 + 4) & ~3; }

    char *get_name(fdt_t *fdt)
	{ return ((char*)fdt) + fdt->offset_dt_strings + offset_name; }

    u32_t get_len()
	{ return len; }
    word_t get_word(int index)
	{ return data[index]; }
    u64_t get_u64(int index)
	{ return ((u64_t)data[index]) << 32 | ((u64_t)data[index + 1]); }
    char *get_string()
	{ return (char*)data; }

    u32_t len;
    u32_t offset_name;
    u32_t data[0];
};

typedef fdt_t dtree_t;
dtree_t *get_dtree();

#endif /* !__PLATFORM__PPC44X__FDT_H__ */
