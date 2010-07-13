/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      uic.h
 * Author:        Jan Stoess <stoess@kit.edu>
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
 ********************************************************************/
#ifndef __PLATFORM__PPC44X__UIC_H__
#define __PLATFORM__PPC44X__UIC_H__

#include <intctrl.h>
#include <sync.h>

//from Linux (arch/powerpc/include/asm/dcr-native.h)
#define mfdcr(rn) \
          ({      \
                  unsigned long rval; \
                  asm volatile("mfdcr %0,%1" : "=r"(rval) : "i"(rn)); \
                  rval; \
          })
#define mtdcr(rn, val) asm volatile("mtdcr %0,%1" : : "i"(rn), "r"(val))

/*
 * Universal Interrupt Controller register definitions. Each is a separate
 * DCR register.
 */

#define UIC0_DCR_BASE  0xc0
#define UIC1_DCR_BASE  0xd0
//FIXME: What is the correct base address for UIC2?

#define UIC0_SR        (UIC0_DCR_BASE+0x0)  /* UIC status                  */
#define UIC0_SRS       (UIC0_DCR_BASE+0x1)  /* UIC status register set     */
#define UIC0_ER        (UIC0_DCR_BASE+0x2)  /* UIC enable                  */
#define UIC0_CR        (UIC0_DCR_BASE+0x3)  /* UIC critical                */
#define UIC0_PR        (UIC0_DCR_BASE+0x4)  /* UIC polarity                */
#define UIC0_TR        (UIC0_DCR_BASE+0x5)  /* UIC triggering              */
#define UIC0_MSR       (UIC0_DCR_BASE+0x6)  /* UIC masked status           */
#define UIC0_VR        (UIC0_DCR_BASE+0x7)  /* UIC vector                  */
#define UIC0_VCR       (UIC0_DCR_BASE+0x8)  /* UIC vector configuration    */

#define UIC1_SR        (UIC1_DCR_BASE+0x0)  /* UIC status                  */
#define UIC1_SRS       (UIC1_DCR_BASE+0x1)  /* UIC status register set     */
#define UIC1_ER        (UIC1_DCR_BASE+0x2)  /* UIC enable                  */
#define UIC1_CR        (UIC1_DCR_BASE+0x3)  /* UIC critical                */
#define UIC1_PR        (UIC1_DCR_BASE+0x4)  /* UIC polarity                */
#define UIC1_TR        (UIC1_DCR_BASE+0x5)  /* UIC triggering              */
#define UIC1_MSR       (UIC1_DCR_BASE+0x6)  /* UIC masked status           */
#define UIC1_VR        (UIC1_DCR_BASE+0x7)  /* UIC vector                  */
#define UIC1_VCR       (UIC1_DCR_BASE+0x8)  /* UIC vector configuration    */

#if defined(PPC440EPx)
#define UIC2_SR        (UIC2_DCR_BASE+0x0)  /* UIC status                  */
#define UIC2_SRS       (UIC2_DCR_BASE+0x1)  /* UIC status register set     */
#define UIC2_ER        (UIC2_DCR_BASE+0x2)  /* UIC enable                  */
#define UIC2_CR        (UIC2_DCR_BASE+0x3)  /* UIC critical                */
#define UIC2_PR        (UIC2_DCR_BASE+0x4)  /* UIC polarity                */
#define UIC2_TR        (UIC2_DCR_BASE+0x5)  /* UIC triggering              */
#define UIC2_MSR       (UIC2_DCR_BASE+0x6)  /* UIC masked status           */
#define UIC2_VR        (UIC2_DCR_BASE+0x7)  /* UIC vector                  */
#define UIC2_VCR       (UIC2_DCR_BASE+0x8)  /* UIC vector configuration    */
#endif

#if defined(PPC440EPx)
#define INT_LEVEL_MAX 74   /* the UIC has 75 interrupt sources */
#define INT_LEVEL_MIN 0    /* 0-31 in UIC0, 32-63 in UIC1, 64-77 in UIC2     */
#else
#define INT_LEVEL_MAX 63   /* the UIC has 64 interrupt sources */
#define INT_LEVEL_MIN 0    /* 0-31 in UIC0, 32-63 in UIC1      */
#endif

#define INT_LEVEL_UIC0_MIN   0    /* First interrupt level on UIC-0 */
#define INT_LEVEL_UIC0_MAX  31    /* Last interrupt level on UIC-0 */
#define UIC0_NUM_IRQS (INT_LEVEL_UIC0_MAX - INT_LEVEL_UIC0_MIN + 1)
#define INT_LEVEL_UIC1_MIN  32    /* First interrupt level on UIC-1 */
#define INT_LEVEL_UIC1_MAX  63    /* Last interrupt level on UIC-1 */
#define UIC1_NUM_IRQS (INT_LEVEL_UIC1_MAX - INT_LEVEL_UIC1_MIN + 1)
#if defined(PPC440EPx)
#define INT_LEVEL_UIC2_MIN  64    /* First interrupt level on UIC-2 */
#define INT_LEVEL_UIC2_MAX  74    /* Last interrupt level on UIC-2 */
#define UIC2_NUM_IRQS (INT_LEVEL_UIC2_MAX - INT_LEVEL_UIC2_MIN + 1)

#endif

#define INT_IS_CRT      (true)         /* Int. is critical (for handler) */
#define INT_IS_NOT_CRT  (!INT_IS_CRT)  /* Int. is not critical */


#define VEC_TO_BIT_SHIFT    31      /* max shift when setting bits */
#define VEC_TO_MSK_SHIFT    32      /* max shift when masking bits */

#define vecToUicBit(v)      (0x00000001 << (VEC_TO_BIT_SHIFT - (v)))
#define vecToUicMask(v)     (0xffffffff << (VEC_TO_MSK_SHIFT - (v)))

#define BGP_MAX_CORE	4
#define BGP_MAX_GROUPS	15
#define BGP_MAX_IRQS	(BGP_MAX_GROUPS * 32)

//all interrupts are non-critical for now
#define UIC0_INTR_CRITICAL 0x00000000
#define UIC1_INTR_CRITICAL 0x00000000
#if defined(PPC440EPx)
#define UIC2_INTR_CRITICAL 0x00000000
#endif

#define UIC0_INTR_POLARITY 0xffffffff
#define UIC1_INTR_POLARITY 0xffffffff
#if defined(PPC440EPx)
#define UIC2_INTR_POLARITY 0xffffffff
#endif

#define UIC0_INTR_TRIGGER 0x00000000
#define UIC1_INTR_TRIGGER 0x00000000
#if defined(PPC440EPx)
#define UIC2_INTR_TRIGGER 0x00000000
#endif

class intctrl_t : public generic_intctrl_t
{
public:
    void init_arch();
    void init_cpu(int cpu);

    word_t get_number_irqs() 
	{ return INT_LEVEL_MAX + 1; }

    bool is_irq_available(word_t irq)
	{ return (irq >= INT_LEVEL_MIN && irq <= INT_LEVEL_MAX); }

    void mask(word_t irq);

    bool unmask(word_t irq);

    bool is_masked(word_t irq);

    bool is_pending(word_t irq);

    void enable(word_t irq) {
    	if (unmask(irq))
    		::handle_interrupt(irq);
    }

    void disable(word_t irq)
	{ mask(irq); }

    bool is_enabled(word_t irq)
	{ return is_masked(irq); }

    void set_cpu(word_t irq, word_t cpu)
	{
	    set_irq_routing(irq, cpu);
	    if (!is_masked(irq))
		unmask(irq);
	}


    /* handler invoked on interrupt (left for compatibility)
     * TODO: Remove it?
     */
    void handle_irq(word_t cpu);

    /* map routine provided by glue */
    void map();

    /* SMP support functions */
    void start_new_cpu(word_t cpu);

    void send_ipi(word_t cpu);

    /* debug */
    void dump();

private:
    // we can route to 4 CPUs, and thus can encode 16 targets in a word
    u8_t routing[BGP_MAX_IRQS / 4]; // 4 IRQs per byte
    spinlock_t lock;
    word_t num_irqs;
    word_t mem_size;
    word_t uic1_dchain_mask;
#if defined(PPC440EPx)
    word_t uic2_dchain_mask;
#endif
    word_t init_controllers();
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
    //common interrupt handler, should not be called directly.
    void raise_irq(word_t irq);
};

#endif /* !__PLATFORM__PPC44X__UIC_H__ */
