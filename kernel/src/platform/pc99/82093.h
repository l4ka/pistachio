/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     platform/pc99/82093.h
 * Description:   Driver for IO-APIC 82093
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
 * $Id: 82093.h,v 1.10 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__PC99__82093_H__
#define __PLATFORM__PC99__82093_H__

// the 82093 supports 24 IRQ lines
#define I82093_NUM_IRQS		24

typedef union {
    struct {
	u32_t version	: 8;
	u32_t		: 8;
	u32_t max_lvt	: 8;
	u32_t reserved0	: 8;
    } __attribute__((packed)) ver;
    u32_t raw;
} ioapic_version_t;
    
class ioapic_redir_t {
public:
    union {
	struct{
	    word_t vector			:  8;
	    word_t delivery_mode		:  3;
	    word_t dest_mode			:  1;
	    word_t delivery_status		:  1;
	    word_t polarity			:  1;
	    word_t irr				:  1;
	    word_t trigger_mode			:  1;
	    word_t mask				:  1;
	    word_t       			: 15;
	    union {
		struct {
		    word_t			: 24;
		    word_t physical_dest	:  4;
		    word_t			:  4;
		} __attribute__((packed)) physical;
	    
		struct {
		    word_t			: 24;
		    word_t logical_dest		:  8;
		} __attribute__((packed)) logical;
	    } dest;
	} x;
	u32_t raw[2];
    };
public:
    void set_fixed_hwirq(u32_t vector, bool low_active,
			 bool level_triggered, bool masked, 
			 u32_t apicid)
    {
	this->x.vector = vector;
	this->x.delivery_mode = 0;	// fixed
	this->x.dest_mode = 0;		// physical mode
	this->x.polarity = low_active ? 1 : 0;
	this->x.trigger_mode = level_triggered ? 1 : 0;
	this->x.mask = masked ? 1 : 0;
	this->x.dest.physical.physical_dest = apicid;
    }
    void set_phys_dest(u32_t apicid)
    { this->x.dest.physical.physical_dest = apicid; }
    u32_t get_phys_dest() 
    { return this->x.dest.physical.physical_dest; }

    void mask_irq()  { this->x.mask = 1; }
    void unmask_irq()	{ this->x.mask = 0; }
    bool is_masked_irq() {return (this->x.mask == 1); }    
    bool is_level_triggered()  { return (this->x.trigger_mode == 1); }
    bool is_edge_triggered()  { return (this->x.trigger_mode == 0); }
} __attribute__((packed));


class i82093_t {
    /* IOAPIC register ids */
    enum regno_t {
	IOAPIC_ID	=0x00,
	IOAPIC_VER	=0x01,
	IOAPIC_ARBID	=0x02,
	IOAPIC_REDIR0	=0x10
    };

    
private:
    volatile u32_t get(u32_t reg)
	{
	    *(__volatile__ u32_t*) this = reg;
 	    return *(__volatile__ u32_t*)(((word_t) this) + 0x10);
	    
	}

    void set(u32_t reg, u32_t val)
	{
	    *(__volatile__ u32_t*) this = reg;
	    *(__volatile__ u32_t*)(((word_t) this) + 0x10) = val;
	}

    volatile u32_t reread()
	{
	    return *(__volatile__ u32_t*)(((word_t) this) + 0x10);
	}

public:
    u8_t id() { return get(IOAPIC_ID) >> 24; };
    
    ioapic_version_t version() {
	return (ioapic_version_t) { raw : get(IOAPIC_VER) };
    }
    
    word_t num_irqs() { 
	return version().ver.max_lvt + 1;
    }
    
    /* VU: masking an IRQ on the IOAPIC only becomes active after
     * performing a read on the data register.  Therefore, when an IRQ
     * is masked we always perform the read assuming that masking/
     * unmasking is the operation performed most frequently */
	
    void set_redir_entry(word_t idx, ioapic_redir_t redir)
	{
	    ASSERT(idx < num_irqs());
	    set(0x11 + (idx * 2), redir.raw[1]);
	    set(0x10 + (idx * 2), redir.raw[0]);
	    if (redir.x.mask) reread();
	    
	}
    void set_redir_entry_low(word_t idx, ioapic_redir_t redir)
	{
	    ASSERT(idx < num_irqs());
	    set(0x10 + (idx * 2), redir.raw[0]);
	    if (redir.x.mask) reread();
	}
    
    void set_redir_entry_high(word_t idx, ioapic_redir_t redir)
	{
	    ASSERT(idx < num_irqs());
	    set(0x11 + (idx * 2), redir.raw[1]);
	}

    ioapic_redir_t get_redir_entry(word_t idx)
	{
	    ASSERT(idx < num_irqs());
	    ioapic_redir_t redir;
	    redir.raw[1] = get(0x11 + (idx * 2));
	    redir.raw[0] = get(0x10 + (idx * 2));
	    return redir;
	}
};

#endif /* !__PLATFORM__PC99__82093_H__ */
