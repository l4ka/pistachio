/****************************************************************************
 *
 * Copyright (C) 2002-2003,  Karlsruhe University
 *
 * File path:	arch/powerpc64/1275tree.h
 * Description:	Macros and data types for enabling easy access to the 
 *		position-independent Open Firmware device tree.
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
 * $Id: 1275tree.h,v 1.4 2005/01/18 13:21:46 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__1275TREE_H__
#define __ARCH__POWERPC64__1275TREE_H__

#define OF1275_KIP_TYPE		0xe
#define OF1275_KIP_SUBTYPE	0xf

INLINE word_t of1275_align( word_t val )
{
    word_t size = sizeof(word_t);

    if( val % size )
	val = (val + size) & ~(size-1);
    return val;
}


class of1275_item_t
{
public:
    u32_t len;
    char data[];

    of1275_item_t *next()
    {
	return (of1275_item_t *)of1275_align( (word_t)this->data + this->len );
    }
};


class of1275_device_t
{
protected:
    u32_t handle;
    u32_t prop_count;
    u32_t prop_size;
    u32_t len;
    char name[];

    of1275_item_t *item_first()
    {
	return (of1275_item_t *)of1275_align( (word_t)this->name + this->len );
    }

public:
    char *get_name()	    { return this->name; }
    u32_t get_handle()	    { return this->handle; }
    u32_t get_prop_count()  { return this->prop_count; }

    bool is_valid() { return this->handle != 0; }

    bool get_prop( const char *name, char **data, u32_t *data_len );
    bool get_prop( word_t index, char **name, char **data, u32_t *data_len ); 
    int get_depth();

    bool get_prop( const char *name, word_t *data )
    {
	u32_t *ptr, len;
	if( !this->get_prop(name, (char **)&ptr, &len) )
	    return false;
	if( len != sizeof(*data) )
	    return false;
	*data = *ptr;
	return true;
    }

    of1275_device_t *next()
    {
	return (of1275_device_t *)
	    of1275_align( (word_t)this->name + this->len + this->prop_size );
    }
    of1275_device_t *next_by_type( const char *device_type );
};


class of1275_tree_t
{
protected:
    of1275_device_t *head;

public:
    void init( char *spill )
    {
	this->head = (of1275_device_t *)of1275_align( (word_t)spill );
    }

    of1275_device_t *first()
    {
	return this->head;
    }

    of1275_device_t *find( const char *name );
    of1275_device_t *find_handle( word_t handle );
    of1275_device_t *find_device_type( const char *device_type );
    of1275_device_t *get_parent( of1275_device_t *dev );
};


INLINE of1275_tree_t *get_of1275_tree()
{
    extern of1275_tree_t of1275_tree;
    return &of1275_tree;
}

class of1275_pci_address
{
public:
    u32_t a_hi;
    u32_t a_mid;
    u32_t a_lo;
} __attribute__((packed));


class of1275_pci_ranges
{
public:
    union {
	struct {
	    of1275_pci_address addr;
	    u32_t phys;
	    u32_t size_hi;
	} pci32;
	struct {
	    of1275_pci_address addr;
	    u32_t phys_hi;
	    u32_t phys_lo;
	    u32_t size_hi;
	    u32_t size_lo;
	} pci64;
    };
} __attribute__((packed));


class of1275_isa_reg_property
{
public:
    u32_t space;
    u32_t address;
    u32_t size;
} __attribute__((packed));


class of1275_pci_assigned_addresses
{
public:
    struct {
	of1275_pci_address addr;
	u32_t size_hi;
	u32_t size_lo;
    } pci;
} __attribute__((packed));


#endif	/* __ARCH__POWERPC64__1275TREE_H__ */
