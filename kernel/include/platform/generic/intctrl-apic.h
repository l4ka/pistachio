/*********************************************************************
 *                
 * Copyright (C) 2002-2004, 2006,  Karlsruhe University
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
#include <sync.h>
#include <linear_ptab.h>

#define NUM_REDIR_ENTRIES	(CONFIG_MAX_IOAPICS * I82093_NUM_IRQS)

class intctrl_t : public generic_intctrl_t 
{
private:
    class ioapic_redir_table_t 
    {
    public:
	void init() { id = 0; } // mark entry invalid
	void set(word_t id, word_t line) 
	    {
		this->id = id;
		this->line = line;
		this->pending = false;
	    }
	    
	bool is_valid() { return id != 0; }

    public:
	ioapic_redir_t entry;
	word_t id;
	word_t line;
	bool pending;
    };
    
    /* VU: ioapic_lock_t is used to synchronize access to IO-APICs on
     * multiple CPUs. If an IO-APIC is not shared, all interrupts are
     * routed to the same processor. Thus, concurrent access cannot occur
     * and we don't need locking. On IRQ re-routing we modify the shared bit.
     * Unlocks are always performed, since they don't cost. The advantage is
     * that we do not have to deal with shared locks and unshared unlocks.
     * In the UP case the implementation collapses to nothing.
     */
    class ioapic_lock_t
    {
    public:
#if defined(CONFIG_SMP)
	void init()	{ _lock.init(); shared = false; }
	void lock()	{ if (is_shared()) _lock.lock(); }
	void unlock ()	{ _lock.unlock(); }
	bool is_shared(){ return shared; }
	void set_shared(bool shared) { this->shared = shared; }
    private:
	bool shared;
	spinlock_t _lock;
#else /* ! CONFIG_SMP */
	void init()	{ }
	void lock()	{ }
	void unlock ()	{ }
	bool is_shared() { return false; }
	void set_shared(bool shared) { }
#endif /* ! CONFIG_SMP */
    };

    ioapic_lock_t ioapic_locks[CONFIG_MAX_IOAPICS];
    ioapic_redir_table_t redir[NUM_REDIR_ENTRIES];

    local_apic_t<APIC_MAPPINGS> local_apic;

    word_t num_intsources;
    word_t max_intsource;
    word_t ioapic_irq_base[CONFIG_MAX_IOAPICS];
    word_t ioapic_num_irqs[CONFIG_MAX_IOAPICS];

    /* apic id handling */
    word_t lapic_map;
    word_t ioapic_map;
    word_t num_cpus;

private:
    i82093_t * get_ioapic(word_t ioapic_id)
	{
	    ASSERT(ioapic_id > 0 && (ioapic_id < CONFIG_MAX_IOAPICS));
	    return (i82093_t*) (APIC_MAPPINGS + (ioapic_id * (page_size(APIC_PGENTSZ))));
	    
	}

    void init_io_apic(word_t ioapic_id, word_t irq_base);
    void init_local_apic();

    /** 
     * sets up the IDT and returns the IDT vector 
     */
    word_t setup_idt_entry(word_t irq, u8_t prio);

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
    
public:
    word_t get_lapic_map() { return lapic_map; }

public:
    void init_arch();
    void init_cpu();

    word_t get_number_irqs();
    bool is_irq_available(word_t irq);

    void mask(word_t irq);
    bool unmask(word_t irq);
    bool is_masked(word_t irq);
    
    void enable(word_t irq);
    void disable(word_t irq);
    bool is_enabled(word_t irq);

    void set_cpu(word_t irq, word_t cpu);

    /* handler invoked on interrupt */
    void handle_irq(word_t irq) __asm__("intctrl_t_handle_irq");

    friend class kdb_t;
};


#endif /* !__PLATFORM__GENERIC__INTCTRL_APIC_H__ */
