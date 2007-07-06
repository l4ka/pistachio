/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     platform/pc99/mps.h
 * Description:   Intel multiprocessor specification
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
 * $Id: mps.h,v 1.2 2003/09/24 19:05:00 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__PC99__MPS_H__
#define __PLATFORM__PC99__MPS_H__

enum mps_bustype_t { 
    MP_BUS_UNKNOWN = 0,
    MP_BUS_ISA = 1,
    MP_BUS_EISA = 2,
    MP_BUS_PCI = 3,
    MP_BUS_MCA = 4
};


class mps_mpc_processor_t
{
public:
    u8_t type;
    u8_t apicid;
    u8_t apicver;
    u8_t cpuflag;
    u32_t cpufeature;
    u32_t featureflag;
    u32_t reserved[2];

public:
    bool is_available()
	{ return cpuflag & 1; }
    bool is_boot_cpu()
	{ return cpuflag & 2; }
};

class mps_mpc_bus_t
{
public:

    u8_t type;
    u8_t busid;
    u8_t bustype[6] __attribute__((packed));

public:
    mps_bustype_t get_bustype()
	{
	    if ((bustype[0] == 'I') && 
		(bustype[1] == 'S') &&
		(bustype[2] == 'A'))
		return MPS_BUS_ISA;
	    return MPS_BUS_UNKNOWN;
	}
};

class mps_mpc_ioapic_t
{
public:
    u8_t type;
    u8_t apicid;
    u8_t apicver;
    u8_t flags;
    addr_t addr;

public:
    bool is_usable()
	{ return flags & 1; }
};

class mps_mpc_intsrc_t
{
public:
    enum irqtype_t {
	INT = 0,
	NMI = 1,
	SMI = 2,
	ExtINT = 3
    };
    u8_t type;
    u8_t irqtype;
    u16_t irqflags;
    u8_t srcbus;
    u8_t srcbusirq;
    u8_t dstapic;
    u8_t dstirq;

public:
    irqtype_t get_irqtype()
	{ return (irqtype_t)irqtype; }
};


class mps_mp_config_table_t
{
public:
    enum entrytype_t
    {
	processor = 0,
	bus = 1,
	ioapic = 2,
	intsrc = 3,
	lintsrc = 4
    };
    
    u32_t signature;
    u32_t length;
    u8_t spec;
    u8_t checksum;
    char oem[8];
    char productid[12];
    addr_t oemptr;
    u32_t oemsize;
    u32_t entries;
    u32_t lapic;
    u32_t reserved;

public:
    bool is_valid()
	{ return signature == ('P'|('C'<<8)|('M'<<16)|('P'<<24)); }
};

class mps_mp_floating_t 
{
public:
    u32_t signature;
    mps_mp_config_table_t * config_table;
    u8_t length;
    u8_t specification;
    u8_t checksum;
    u8_t feature[5];

public:
    static mps_mp_floating_t * scan(addr_t start, addr_t end);
    bool is_valid() 
	{ return signature == (('_'<<24)|('P'<<16)|('M'<<8)|'_'); }
};

static inline mps_mp_floating_t * mps_mp_floating_t::scan(addr_t start, addr_t end)
{
    start = addr_mask(start, ~0xf);
    while (start < end)
    {
	if (((mps_mp_floating_t*)start)->is_valid())
	    return (mps_mp_floating_t*)start;
	start = addr_offset(start, 0x10);
    }
    return NULL;
}


#endif /* !__PLATFORM__PC99__MPS_H__ */
