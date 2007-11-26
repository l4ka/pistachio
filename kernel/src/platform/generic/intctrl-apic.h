/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006-2007,  Karlsruhe University
 *                
 * File path:     platform/generic/intctrl-apic.h
 * Description:   Driver for APIC+IOAPIC
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
 * $Id: intctrl-apic.h,v 1.5 2006/10/19 22:57:36 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __PLATFORM__GENERIC__INTCTRL_APIC_H__
#define __PLATFORM__GENERIC__INTCTRL_APIC_H__

#ifndef __GENERIC__INTCTRL_H__
#error not for standalone inclusion
#endif

#include INC_PLAT(82093.h)
#include INC_ARCH(apic.h)
#include INC_ARCH(apic.h)
#include INC_ARCH(ioport.h)
#include <sync.h>
#include <linear_ptab.h>

#define NUM_REDIR_ENTRIES	(CONFIG_MAX_IOAPICS * I82093_NUM_IRQS)

class intctrl_t : public generic_intctrl_t 
{
private:
    class ioapic_t {
    public:
	void init(word_t id, addr_t paddr)
	    {
		this->id = id;
		i82093 = (i82093_t*)paddr;
		lock.init();
	    }

    public:
	word_t id;
	i82093_t *i82093;
	spinlock_t lock;
    };
    
    class ioapic_redir_table_t 
    {
    public:
	void init() { ioapic = NULL; } // mark entry invalid
	void set(ioapic_t * ioapic, word_t line) 
	    {
		this->ioapic = ioapic;
		this->line = line;
		this->pending = false;
	    }
	bool is_valid() { return this->ioapic != NULL; }

    public:
	ioapic_redir_t entry;
	ioapic_t* ioapic;
	word_t line;
	bool pending;
    };
    
    ioapic_t ioapics[CONFIG_MAX_IOAPICS];
    ioapic_redir_table_t redir[NUM_REDIR_ENTRIES];

    word_t num_intsources;
    word_t max_intsource;

    /* apic id handling */
    word_t num_ioapics;
    word_t num_cpus;
    spinlock_t idt_lock;

private:
    bool init_io_apic(word_t idx, word_t id, word_t irq_base, addr_t paddr);
    void init_local_apic();

    /** 
     * sets up the IDT and returns the IDT vector 
     */
    u8_t setup_idt_entry(word_t irq, u8_t prio);
    void free_idt_entry(word_t irq, u8_t vector);

    /**
     * write the cached redir entry into the APIC
     * may sync parts of the 64-bits or all
     */
    enum sync_redir_part_e {
	sync_low = 0,
	sync_high = 1,
	sync_all = 2
    };

    void sync_redir_entry(ioapic_redir_table_t * entry, 
			  sync_redir_part_e part = sync_low);

    

    void check_ioapic_shared(word_t id);
    
private: 
    bool   pmtimer_available;
    word_t pmtimer_ioport;
    
public:
    static local_apic_t<APIC_MAPPINGS_START> local_apic;

public:
 
    void init_arch();
    void init_cpu();
    
    word_t get_number_irqs();
    bool is_irq_available(word_t irq);

    void mask(word_t irq);
    bool unmask(word_t irq);
    bool is_masked(word_t irq);
    bool is_pending(word_t irq);
    
    void enable(word_t irq);
    void disable(word_t irq);
    bool is_enabled(word_t irq);

    void set_cpu(word_t irq, word_t cpu);

    static const word_t pmtimer_ticks = 3579545;
    static const word_t pmtimer_mask = 0xFFFFFF;
    
    word_t pmtimer_read() 
    {
	u32_t first, second;
 	
 	first = second = in_u32(pmtimer_ioport) & pmtimer_mask;
 	
 	while (first == second)
 	{
 	    second = in_u32(pmtimer_ioport) & pmtimer_mask;
 	    x86_pause();
 	}
 	return second; 
    }
    
    bool has_pmtimer() { return pmtimer_available; }
    void pmtimer_wait(const word_t ms) 
    { 
        /* Need to wait ms * pmtimer_ticks / 1000; */
        const word_t delta = (ms * pmtimer_ticks) / 1000;
        word_t start = pmtimer_read();
        
        if ((start + delta) >=  pmtimer_mask)
        {
            while (pmtimer_read() >= start)
                /* wait until overlap */;
        }
        
        while (pmtimer_read() < ((start + delta) & pmtimer_mask))
            /* wait */;	
	
    }
    
    /* handler invoked on interrupt */
    void handle_irq(word_t irq) __asm__("intctrl_t_handle_irq");
    
    friend class kdb_t;
};


#endif /* !__PLATFORM__GENERIC__INTCTRL_APIC_H__ */
