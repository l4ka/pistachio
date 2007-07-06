/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	lib/io/1275tree.h
 * Description:	Support for the canonical representation of the
 *              Open Firmware device tree created by the boot loader.
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
 * $Id: 1275tree.h,v 1.1 2004/01/16 11:23:56 joshua Exp $
 *
 ***************************************************************************/
#ifndef __USER__LIB__IO__1275TREE_H__
#define __USER__LIB__IO__1275TREE_H__

#include <l4/types.h>
#include <l4/kip.h>
#include <l4/sigma0.h>

#define OF1275_KIP_MAJOR_TYPE	0xe
#define OF1275_KIP_MINOR_TYPE	0xf
#define OF1275_KIP_TYPE	(OF1275_KIP_MAJOR_TYPE + (OF1275_KIP_MINOR_TYPE << 4))


L4_INLINE L4_Word_t of1275_align( L4_Word_t val )
{
    L4_Word_t size = sizeof(L4_Word_t);

    if( val % size )
	val = (val + size) & ~(size-1);
    return val;
}


class of1275_item_t
{
public:
    L4_Word_t len;
    char data[];

    of1275_item_t *next()
    {
	return (of1275_item_t *)
	    of1275_align( (L4_Word_t)this->data + this->len );
    }
};


class of1275_device_t
{
protected:
    L4_Word_t handle;
    L4_Word_t prop_count;
    L4_Word_t prop_size;
    L4_Word_t len;
    char name[];

    of1275_item_t *item_first()
    {
	return (of1275_item_t *)
	    of1275_align( (L4_Word_t)this->name + this->len );
    }

public:
    char *get_name()           { return this->name; }
    L4_Word_t get_handle()     { return this->handle; }
    L4_Word_t get_prop_count() { return this->prop_count; }

    bool is_valid() { return this->handle != 0; }

    bool get_prop( const char *prop_name, char **data, L4_Word_t *data_len );
    bool get_prop( L4_Word_t index, char **prop_name, char **data, L4_Word_t *data_len ); 
    int get_depth();

    bool get_prop( const char *prop_name, L4_Word_t *data )
    {
	L4_Word_t *ptr, prop_len;
	if( !this->get_prop(prop_name, (char **)&ptr, &prop_len) )
	    return false;
	if( prop_len != sizeof(*data) )
	    return false;
	*data = *ptr;
	return true;
    }

    of1275_device_t *next()
    {
	return (of1275_device_t *)
	    of1275_align( (L4_Word_t)this->name + this->len + this->prop_size );
    }
};


class of1275_tree_t
{
public:
    of1275_device_t *first()
    {
	return (of1275_device_t *)this;
    }

    of1275_device_t *find( const char *name );
    of1275_device_t *find_handle( L4_Word_t handle );
    of1275_device_t *find_device_type( const char *device_type );
    of1275_device_t *get_parent( of1275_device_t *dev );
};


/**
 *  get_of1275_tree_from_sigma0()
 *
 *  Searches the KIP's memory descriptors for the canonical form of
 *  the 1275 device tree installed by the boot loader.  It requests
 *  sigma0 to map the relevant pages into the current address space 1:1,
 *  and returns the device tree encapsulated as an of1275_tree_t *.
 *  The device tree is mapped 1:1 because the bootloader ensures
 *  that the device tree won't collide with any of the roottasks'
 *  initial memory layout.
 */
L4_INLINE of1275_tree_t * get_of1275_tree_from_sigma0()
{
    void *kip = L4_GetKernelInterface();
    L4_ThreadId_t sigma0 = L4_GlobalId(L4_ThreadIdUserBase(kip), 1);

    // Search for the memory descriptor for the 1275 tree.
    for( L4_Word_t i = 0; i < L4_NumMemoryDescriptors(kip); i++ )
    {
	L4_MemoryDesc_t *mdesc = L4_MemoryDesc( kip, i );
	if( L4_MemoryDescType(mdesc) == OF1275_KIP_TYPE )
	{
	    L4_Word_t start = L4_MemoryDescLow(mdesc);
	    L4_Word_t end = L4_MemoryDescHigh(mdesc) + 1;

	    if( L4_Myself() == sigma0 )
		return (of1275_tree_t *)start;

	    // Request mappings for the pages.
	    L4_Word_t pagesize = 4096;
	    while( start < end )
	    {
		L4_Fpage_t fpage = L4_Fpage( start, pagesize );
		fpage.X.rwx = L4_ReadWriteOnly;
		fpage = L4_Sigma0_GetPage( sigma0, fpage, fpage );
		if( L4_IsNilFpage(fpage) )
		    return 0;

		start += pagesize;
	    }
	    
	    return (of1275_tree_t *)L4_MemoryDescLow(mdesc);
	}
    }

    return 0;
}

#endif	/* __USER__LIB__IO__1275TREE_H__ */
