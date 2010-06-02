/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     platform/ppc44x/bic.h
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
#ifndef __PLATFORM__PPC44X__BIC_H__
#define __PLATFORM__PPC44X__BIC_H__

#include <intctrl.h>
#include <sync.h>

#define BGP_MAX_CORE	4
#define BGP_MAX_GROUPS	15
#define BGP_MAX_IRQS	(BGP_MAX_GROUPS * 32)

class bgic_t 
{
public:
    class bgic_group_t 
    {
    public:
	volatile word_t status;		// status (read and write) 0
	volatile word_t rd_clr_status;	// status (read and clear) 4
	volatile word_t status_clr;	// status (write and clear)8
	volatile word_t status_set;	// status (write and set) c
	
	// 4 bits per IRQ
	volatile word_t target_irq[4];	// target selector 10-20
	volatile word_t noncrit_mask[BGP_MAX_CORE];	// mask 20-30
	volatile word_t crit_mask[BGP_MAX_CORE];	// mask 30-40
	volatile word_t mchk_mask[BGP_MAX_CORE];	// mask 40-50
	unsigned char __align[0x80 - 0x50];
    } __attribute__((packed));


    bgic_group_t groups[BGP_MAX_GROUPS];
    volatile word_t core_non_crit[BGP_MAX_CORE];
    volatile word_t core_crit[BGP_MAX_CORE];
    volatile word_t core_mchk[BGP_MAX_CORE];

private:
    const word_t irq_to_group(word_t hwirq)
	{ return (hwirq >> 5) & 0xf; }

    const word_t group_to_irq(word_t group)
	{ return group << 5; }

    const word_t irq_of_group(word_t hwirq)
	{ return hwirq & 0x1f; }

    const word_t get_group_mask(word_t hwirq)
	{ return 1U << (31 - irq_of_group(hwirq)); }

    void set_target(word_t hwirq, word_t target)
	{
	    word_t offset = ((7 - (hwirq & 0x7)) * 4);
	    volatile word_t *reg;
	    reg = &groups[irq_to_group(hwirq)].target_irq[irq_of_group(hwirq) / 8];
	    word_t val = *reg;
	    sync();
	    *reg = (val & ~(0xf << offset)) | ((target & 0xf) << offset);
	    sync();
	}

    word_t cpu_to_noncrit_target(word_t cpu)
	{ return 0x4 | cpu; }
    word_t cpu_to_crit_target(word_t cpu)
	{ return 0x8 | cpu; }
    word_t cpu_to_mcheck_target(word_t cpu)
	{ return 0xc | cpu; }

public:
    void mask_irq(word_t hwirq)
	{ set_target(hwirq, 0); }

    void unmask_irq(word_t hwirq, word_t cpu)
	{ set_target(hwirq, cpu_to_noncrit_target(cpu)); }

    bool is_masked(word_t hwirq)
	{
	    ASSERT(hwirq < BGP_MAX_IRQS);
	    word_t offset = ((7 - (hwirq & 0x7)) * 4);
	    word_t val = groups[irq_to_group(hwirq)].target_irq[irq_of_group(hwirq) / 8];
	    sync();
	    return val & (0xf << offset) == 0;
	}

    bool is_pending(word_t hwirq)
	{
	    ASSERT(hwirq < BGP_MAX_IRQS);
	    return groups[irq_to_group(hwirq)].status & (1 << (31-irq_of_group(hwirq)));
	}

    void raise_irq(word_t hwirq)
	{ 
	    ASSERT(hwirq < BGP_MAX_IRQS);
	    sync();
	    groups[irq_to_group(hwirq)].status_set = get_group_mask(hwirq); 
	}

    void ack_irq(word_t hwirq)
	{ 
	    ASSERT(hwirq < BGP_MAX_IRQS);
	    sync();
	    groups[irq_to_group(hwirq)].status_clr = get_group_mask(hwirq); 
	}

    void mask_and_clear_all()
	{
	    sync();
	    for (unsigned idx = 0; idx < BGP_MAX_GROUPS; idx++)
	    {
		groups[idx].target_irq[0] = 0;
		groups[idx].target_irq[1] = 0;
		groups[idx].target_irq[2] = 0;
		groups[idx].target_irq[3] = 0;
		groups[idx].status = 0;
		eieio();
	    }
	}

    int get_pending_irq(word_t core);
    void dump();

} __attribute__((packed));



class intctrl_t : public generic_intctrl_t
{
public:
    void init_arch();
    void init_cpu(int cpu);

    word_t get_number_irqs() 
	{ return num_irqs; }

    bool is_irq_available(word_t irq)
	{ return irq >= 32; }

    void mask(word_t irq)
	{ 
	    lock.lock();
	    ctrl->mask_irq(irq);
	    lock.unlock();
	}

    bool unmask(word_t irq)
	{
	    ASSERT(irq < BGP_MAX_IRQS);
	    lock.lock();
        ctrl->ack_irq(irq);
        bool pending = is_pending(irq);
        if (!pending)
            ctrl->unmask_irq(irq, get_irq_routing(irq));
        lock.unlock();
	    return pending; 
	}

    bool is_masked(word_t irq)
	{ return ctrl->is_masked(irq); }

    bool is_pending(word_t irq)
	{ return ctrl->is_pending(irq); }

    void enable(word_t irq)
	{ unmask(irq); }

    void disable(word_t irq)
	{ mask(irq); }

    bool is_enabled(word_t irq)
	{ return is_masked(irq); }

    void set_cpu(word_t irq, word_t cpu)
	{ 
	    set_irq_routing(irq, cpu);
	    if (!ctrl->is_masked(irq))
		ctrl->unmask_irq(irq, get_irq_routing(irq));
	}

    /* handler invoked on interrupt */
    void handle_irq(word_t cpu);

    /* map routine provided by glue */
    void map();

    /* SMP support functions */
    void start_new_cpu(word_t cpu);
    void send_ipi(word_t cpu);

    /* debug */
    void dump() 
	{ ctrl->dump(); }

private:
    bgic_t *ctrl;
    // we can route to 4 CPUs, and thus can encode 16 targets in a word
    u8_t routing[BGP_MAX_IRQS / 4]; // 4 IRQs per byte
    spinlock_t lock;
    word_t num_irqs;

    u64_t phys_addr;
    word_t mem_size;

    word_t get_irq_routing(word_t irq)
	{ 
	    word_t shift = (irq % 4) * 2;
	    return (routing[irq / 4] >> shift) & 3;
	}

    void set_irq_routing(word_t irq, word_t cpu)
	{ 
	    word_t shift = (irq % 4) * 2;
	    routing[irq / 4] = (routing[irq / 4] & ~(3 << shift)) | (cpu << shift);
	}

    word_t get_ipi_irq(word_t cpu, word_t ipi)
	{
	    return cpu * 8 + ipi;
	}

};

#endif /* !__PLATFORM__PPC44X__BIC_H__ */
