/*********************************************************************
 *                
 * Copyright (C) 2010,  Karlsruhe Institute of Technology
 *                
 * Filename:      uic.cc
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
#include <debug.h>
#include <kdb/tracepoints.h>

#include <lib.h>
#include INC_ARCH(string.h)

#include INC_PLAT(uic.h)
#include INC_PLAT(fdt.h)
#include INC_API(kernelinterface.h)

intctrl_t intctrl;

void SECTION (".init") intctrl_t::init_arch()
{
    fdt_t *fdt = get_dtree();
    fdt_header_t *hdr;
    fdt_property_t *prop;

    //Controller 0
    printf("Looking for interrupt controller 0... ");
    hdr = fdt->find_subtree("/interrupt-controller0");
    if (!hdr)
	panic("Couldn't find interrupt controller 0 in FDT\n");
    else
    	printf("found!\n");

    printf("Checking whether UIC0 is compatible... ");
    prop = fdt->find_property_node(hdr, "compatible");

    if ((!prop) || ((strcmp(prop->get_string(), "ibm,uic") != 0) && (strcmp(prop->get_string(), "ibm,uic-440gp") != 0)))
    	panic("UIC0: Couldn't find compatible node in FDT\ncompatibility string was %s",prop->get_string());
    else
    	printf("it is!\n");

    printf("Looking for UIC0's DCR base address... ");
    prop = fdt->find_property_node(hdr, "dcr-reg");
    if (!prop) // || prop->get_len() != 2 * sizeof(u32_t))
	panic("UIC0: Couldn't find 'dcr-reg' node in FDT (%p, %d)\n",
	      prop, prop->get_len());

    if (UIC0_DCR_BASE == prop->get_word(0))
    	printf("Found at 0x%x\n",UIC0_DCR_BASE);
    else
    	panic("UIC0 is not at its expected DCR base address!");

    if (prop->get_word(1) != 9)
    	panic("Invalid number of control registers found (%d)",prop->get_word(1));

    TRACE_INIT("UIC0: DCR base 0x%x, %d interrupts\n",
	       UIC0_DCR_BASE, UIC0_NUM_IRQS);

    //Controller 1
    printf("Looking for interrupt controller 1... ");
    hdr = fdt->find_subtree("/interrupt-controller1");
    if (!hdr)
    	panic("Couldn't find interrupt controller 1 in FDT\n");
    else
    	printf("found!\n");

    printf("Checking whether UIC1 is compatible... ");
    prop = fdt->find_property_node(hdr, "compatible");

    if ((!prop) || ((strcmp(prop->get_string(), "ibm,uic") != 0) && (strcmp(prop->get_string(), "ibm,uic-440gp") != 0)))
    	panic("UIC1: Couldn't find compatible node in FDT\ncompatibility string was %s",prop->get_string());
    else
    	printf("it is!\n");

    printf("Looking for UIC1's DCR base address... ");
    prop = fdt->find_property_node(hdr, "dcr-reg");
    if (!prop) // || prop->get_len() != 2 * sizeof(u32_t))
	panic("UIC1: Couldn't find 'dcr-reg' node in FDT (%p, %d)\n",
	      prop, prop->get_len());

    if (UIC1_DCR_BASE == prop->get_word(0))
    	printf("Found at 0x%x\n",UIC1_DCR_BASE);
    else
    	panic("UIC1 is not at its expected DCR base address!");

    if (prop->get_word(1) != 9)
    	panic("UIC1: Invalid number of control registers found (%d)",prop->get_word(1));

    prop = fdt->find_property_node(hdr, "interrupts");
    if (!prop)
		panic("UIC1: Couldn't determine daisychain interrupts");

    uic1_dchain_mask = 1 << (31 - prop->get_word(0));
    uic1_dchain_mask |= 1 << (31 - prop->get_word(2));

    TRACE_INIT("UIC1: DCR base 0x%x, %d interrupts\n",
	       UIC1_DCR_BASE, UIC1_NUM_IRQS);

    //Controller 2
#if defined(PPC440EPx)
    printf("Looking for interrupt controller 2... ");
    hdr = fdt->find_subtree("/interrupt-controller2");
    if (!hdr)
    	panic("Couldn't find interrupt controller 2 in FDT\n");
    else
    	printf("found!\n");

    printf("Checking whether UIC2 is compatible... ");
    prop = fdt->find_property_node(hdr, "compatible");

    if ((!prop) || ((strcmp(prop->get_string(), "ibm,uic") != 0) && (strcmp(prop->get_string(), "ibm,uic-440gp") != 0)))
    	panic("UIC2: Couldn't find compatible node in FDT\ncompatibility string was %s",prop->get_string());
    else
    	printf("it is!\n");

    printf("Looking for UIC2's DCR base address... ")
    prop = fdt->find_property_node(hdr, "dcr-reg");
    if (!prop) // || prop->get_len() != 2 * sizeof(u32_t))
	panic("UIC2: Couldn't find 'dcr-reg' node in FDT (%p, %d)\n",
	      prop, prop->get_len());

    if (UIC2_DCR_BASE == prop->get_word(0))
    	printf("Found at 0x%x\n",UIC2_DCR_BASE);
    else
    	panic("UIC2 is not at its expected DCR base address!")

    if (prop->get_word(1) != 9)
    	panic("UIC2: Invalid number of control registers found (%d)",prop->get_word(1));

    prop = fdt->find_property_node(hdr, "interrupts");
    if (!prop)
		panic("UIC2: Couldn't determine daisychain interrupts");

    uic2_dchain_mask = 1 << (31 - prop->get_word(0));
    uic2_dchain_mask |= 1 << (31 - prop->get_word(2));

    TRACE_INIT("UIC2: DCR base 0x%x, %d interrupts\n",
	       UIC2_DCR_BASE, UIC2_NUM_IRQS);

#endif //PPC440EPx

    init_controllers();
    // route all IRQs to CPU0
    memset(routing, 0, sizeof(routing));
}

void SECTION(".init") intctrl_t::init_cpu(int cpu)
{
#ifdef CONFIG_SMP
    ASSERT(cpu < 4);

    /* map IPIs */
    set_irq_routing(get_ipi_irq(cpu, 0), cpu);
    enable(get_ipi_irq(cpu, 0));
#endif
}

word_t intctrl_t::init_controllers() {
	/*
	 * Initial Interrupt controller setup
	 */

	mtdcr(UIC0_SR, 0xFFFFFFFF);          /* clear all ints          */
	mtdcr(UIC1_SR, 0xFFFFFFFF);          /* clear all ints          */
#if defined(PPC440EPx)
	mtdcr(UIC2_SR, 0xFFFFFFFF);          /* clear all ints          */
#endif

	mtdcr(UIC0_ER, 0x00000000);          /* disable all ints        */
	mtdcr(UIC1_ER, 0x00000000);          /* disable all ints        */
#if defined(PPC440EPx)
	mtdcr(UIC2_ER, 0x00000000);          /* disable all ints        */
#endif

	mtdcr(UIC0_CR, UIC0_INTR_CRITICAL);  /* set critical ints       */
	mtdcr(UIC1_CR, UIC1_INTR_CRITICAL);  /* set critical ints       */
#if defined(PPC440EPx)
	mtdcr(UIC2_CR, UIC2_INTR_CRITICAL);  /* set critical ints       */
#endif

	mtdcr(UIC0_PR, UIC0_INTR_POLARITY);  /* set intr polarity       */
	mtdcr(UIC1_PR, UIC1_INTR_POLARITY);  /* set intr polarity       */
#if defined(PPC440EPx)
	mtdcr(UIC2_PR, UIC2_INTR_POLARITY);  /* set intr polarity       */
#endif

	mtdcr(UIC0_TR, UIC0_INTR_TRIGGER);   /* set level/edge trigger  */
	mtdcr(UIC1_TR, UIC1_INTR_TRIGGER);   /* set level/edge trigger  */
#if defined(PPC440EPx)
	mtdcr(UIC2_TR, UIC2_INTR_TRIGGER);   /* set level/edge trigger  */
#endif

	mtdcr(UIC0_VCR, 0x00000000);         /* set highest priority    */
	mtdcr(UIC1_VCR, 0x00000000);         /* set highest priority    */
#if defined(PPC440EPx)
	mtdcr(UIC2_VCR, 0x00000000);         /* set highest priority    */
#endif

	mtdcr(UIC0_SR, 0xFFFFFFFF);          /* clear all ints again    */
	mtdcr(UIC1_SR, 0xFFFFFFFF);          /* clear all ints again    */
#if defined(PPC440EPx)
	mtdcr(UIC2_SR, 0xFFFFFFFF);          /* clear all ints again    */
#endif

	return (0);
}

/*
 * UIC interrupt handler.
 * NOTE: This function does not implement any interrupt priorities.
 * Instead, all interrupts are immediately sent to the appropriate
 * handler thread when they arrive. If two interrupts are active
 * at the same time, the one with the lower number is served first.
 * There are also no critical interrupts for now.
 * Some time in the future, we might want to use the interrupt threads'
 * priorities here.
 */
void intctrl_t::handle_irq(word_t cpu)
{
    int   vector;
    word_t  uicmsr;     /* contents of Masked Status Register */
    word_t  uicer = 0;  /* contents of enable register */

    //Lock all interrupts while the handler is running.
    //TODO: Is this really necessary?
    lock.lock();
    /*
     * Get contents of UIC0_MSR.  This register is read-only
     * and relects the value of UIC0_SR ANDed with UIC0_ER.
     */

    uicmsr = mfdcr(UIC0_MSR);

    /*
     * Determine the interrupt source.
     */

    vector = count_leading_zeros(uicmsr);

    if ((vector == 30)|| (vector == 31)) /* dchain interrupt */
      {
      uicmsr = mfdcr(UIC1_MSR);
      vector = count_leading_zeros(uicmsr);
      vector+= INT_LEVEL_UIC1_MIN ;
      }
#if defined(PPC440EPx)
    if ((vector == 28)|| (vector == 29)) /* dchain interrupt */
      {
      uicmsr = mfdcr(UIC2_MSR);
      vector = count_leading_zeros(uicmsr);
      vector+= INT_LEVEL_UIC2_MIN ;
      }
#endif

#ifdef INCLUDE_WINDVIEW
    WV_EVT_INT_ENT(vector)
#endif

    if ( EXPECT_TRUE(vector <= INT_LEVEL_MAX) )
        {

        /*
         * The vector is valid. Which UIC is it for?
         */
#if defined(PPC440EPx)
        if (vector > INT_LEVEL_UIC1_MAX)
            {
            /* In UIC-2 */

            //Mask the current interrupt
        	uicer = mfdcr(UIC2_ER);
            uicer &= ~(1 << (31 - (vector - INT_LEVEL_UIC2_MIN)));
    		mtdcr(UIC2_ER, uicer );

            /* Clear the Status registers */

            mtdcr(UIC2_SR, (1 << (31 - (vector - INT_LEVEL_UIC2_MIN)))) ;
            mtdcr(UIC0_SR, uic2_dchain_Mask);    /* clear daisychain */

            }
        else if (vector > INT_LEVEL_UIC0_MAX)
#else
        if (vector > INT_LEVEL_UIC0_MAX)
#endif
            {

            /* In UIC-1 */

            //Mask the current interrupt
        	uicer = mfdcr(UIC1_ER);
            uicer &= ~(1 << (31 - (vector - INT_LEVEL_UIC1_MIN)));
    		mtdcr(UIC1_ER, uicer );

            /* Clear the Status registers */

            mtdcr(UIC1_SR, (1 << (31 - (vector - INT_LEVEL_UIC1_MIN)))) ;
            mtdcr(UIC0_SR, uic1_dchain_mask);    /* clear daisychain */
            }
        else
            {

            /* In UIC-0 */

            //Mask the current interrupt
        	uicer = mfdcr(UIC0_ER);
            uicer &= ~(1 << (31 - vector));
    		mtdcr(UIC0_ER, uicer );

            /* Clear the Status register. */
            mtdcr(UIC0_SR,(1 << (31 - vector))) ;
            }

        }
    else
        {
        /* This should never happen. Bail out. */
			panic("UIC: Invalid interrupt received!");
        }

    /*
     * The current interrupt is now masked, so we can allow other
     * interrupts from this point on.
     */
    lock.unlock();

    /*
     * Call the appropriate handler with the appropriate argument
     */

    ::handle_interrupt(vector);

}

void intctrl_t::map()
{
    return; //UICs cannot be memory mapped
    //TODO: Should we panic here?
}

void intctrl_t::start_new_cpu(word_t cpu)
{
    ASSERT(cpu < 4);

    extern word_t secondary_release_reloc;
    secondary_release_reloc = cpu;
}

void intctrl_t::send_ipi(word_t cpu)
{
    //FIXME: nonexistant tracepoint?
	//TRACEPOINT(SMP_IPI, "irq %d cpu %d\n", get_ipi_irq(cpu, 0), cpu);
    raise_irq(get_ipi_irq(cpu, 0));
}

void intctrl_t::raise_irq(word_t irq)
{
    ASSERT(irq < INT_LEVEL_MAX);
	word_t intMask;

#if defined(PPC440EPx)
	if (irq > INT_LEVEL_UIC1_MAX) {       /* For UIC2 */
		irq-=INT_LEVEL_UIC2_MIN ;
		intMask = 1 << (31 - irq) ;
		lock.lock();
		mtdcr(UIC2_SRS,mfdcr(UIC2_SR) | intMask);
		lock.unlock();
	} else if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#else
	if (irq > INT_LEVEL_UIC0_MAX) {       /* For UIC1 */
#endif
		irq-=INT_LEVEL_UIC1_MIN ;
		intMask = 1 << (31 - irq) ;
		lock.lock();
		mtdcr(UIC1_SRS,mfdcr(UIC1_SR) | intMask);
		lock.unlock();
	} else {							/* For UIC0 */
		intMask = 1 << (31 - irq);
		lock.lock();
		mtdcr(UIC0_SRS,mfdcr(UIC0_SR) | intMask);
		lock.unlock();
	}
}


void intctrl_t::mask(word_t irq)
{
	word_t intMask;

	if (irq > INT_LEVEL_MAX || irq < INT_LEVEL_MIN)
		printf("Tried to mask invalid interrupt!");


#if defined(PPC440EPx)
	if (irq > INT_LEVEL_UIC1_MAX)        /* For UIC2 */
		{
		irq-=INT_LEVEL_UIC2_MIN ;
		intMask = 1 << (31 - irq) ;


		/*
		 * disable the interrupt level.
		 * We must lock out interrupts to disable the hardware
		 * as the handler may crush what we just did in UIC_ER.
		 */

		lock.lock();                        /* lock interrupts */

		/* really disable interrupt */
		mtdcr(UIC2_ER, (~intMask) & mfdcr(UIC2_ER));

		mtdcr(UIC2_SR, intMask);       /* clear pending interrupts */
		mtdcr(UIC0_SR, uic2_dchain_mask);      /* clear dchained UIC1 ints */

		lock.unlock();                         /* re-enable interrupts */
		}
	else if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#else
	if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#endif
		{
		irq-=INT_LEVEL_UIC1_MIN ;
		intMask = 1 << (31 - irq) ;

		/*
		 * disable the interrupt level.
		 * We must lock out interrupts to disable the hardware
		 * as the handler may crush what we just did in UIC_ER.
		 */

		lock.lock();                        /* lock interrupts */

		/* really disable interrupt */
		mtdcr(UIC1_ER, (~intMask) & mfdcr(UIC1_ER));

		mtdcr(UIC1_SR, intMask);       /* clear pending interrupts */
		mtdcr(UIC0_SR, uic1_dchain_mask);      /* clear dchained UIC1 ints */

		lock.unlock();                         /* re-enable interrupts */

		}
	else
		{
		intMask = 1 << (31 - irq);

		/*
		 * disable the interrupt level
		 * We must lock out interrupts to disable the hardware
		 * as the handler may crush what we just did in UIC_ER.
		 */

		lock.lock();                        /* lock interrupts */

		/* really disable interrupt */
		mtdcr(UIC0_ER, (~intMask) & mfdcr(UIC0_ER));

		mtdcr(UIC0_SR, intMask);   /* clear pending interrupts */

		lock.unlock();                         /* re-enable interrupts */
		}

	return;
}

bool intctrl_t::unmask(word_t irq)
{
    word_t intMask;

    if (irq > INT_LEVEL_MAX || irq < INT_LEVEL_MIN)
        panic("Invalid Interrupt!");


#if defined(PPC440EPx)
    if (irq > INT_LEVEL_UIC1_MAX)        /* For UIC2 */
        {
        irq -= INT_LEVEL_UIC2_MIN ;
        intMask = 1 << (31 - irq) ;

        //Check if interrupt was already pending
        if ((mfdcr(UIC2_SR) & intMask) > 0) {
            mtdcr(UIC2_SR, intMask);        /* clear pending interrupts */
            mtdcr(UIC0_SR, uic2_dchain_mask);     /* clear pending dchain     */
            return true;
        } else {

			/*
			 * enable the interrupt level
			 * We must lock out interrupts to enable the hardware
			 * as the handler may crush what we just did in UIC_ER.
			 */

			lock.lock();                        /* lock interrupts */

			mtdcr(UIC2_ER, intMask | mfdcr(UIC2_ER));

			/* Enable dchain*/
			mtdcr(UIC0_ER, uic2_dchain_mask | mfdcr(UIC0_ER));

			lock.unlock();                         /* re-enable interrupts */
			return false;
        }
        }
    else if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#else
    if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#endif
        {
        irq -= INT_LEVEL_UIC1_MIN ;
        intMask = 1 << (31 - irq) ;

        //Check if interrupt was already pending
        if ((mfdcr(UIC1_SR) & intMask) > 0) {
        	mtdcr(UIC1_SR, intMask);        /* clear pending interrupts */
        	mtdcr(UIC0_SR, uic1_dchain_mask);     /* clear pending dchain     */
        	return true;
        } else {

			/*
			 * enable the interrupt level
			 * We must lock out interrupts to enable the hardware
			 * as the handler may crush what we just did in UIC_ER.
			 */

			lock.lock();                        /* lock interrupts */

			mtdcr(UIC1_ER, intMask | mfdcr(UIC1_ER));

			/* Enable dchain*/
			mtdcr(UIC0_ER, uic1_dchain_mask | mfdcr(UIC0_ER));

			lock.unlock();                         /* re-enable interrupts */
			return false;
        }
        }
    else
        {
        intMask = 1 << (31 - irq) ;

        //Check if interrupt was already pending
        if ((mfdcr(UIC0_SR) & intMask) > 0) {
        	mtdcr(UIC0_SR, intMask);        /* clear pending interrupts */
        	return true;
        } else {

			/*
			 * enable the interrupt level
			 * We must lock out interrupts to enable the hardware
			 * as the handler may crush what we just did in UIC_ER.
			 */

			lock.lock();                        /* lock interrupts */

			mtdcr(UIC0_ER, intMask | mfdcr(UIC0_ER));

			lock.unlock();                         /* re-enable interrupts */
			return false;
        }
        }

}

bool intctrl_t::is_masked(word_t irq) {
	word_t intMask;
	word_t actualMask;

	if (irq > INT_LEVEL_MAX || irq < INT_LEVEL_MIN)
		printf("Tried to check invalid interrupt!");


#if defined(PPC440EPx)
	if (irq > INT_LEVEL_UIC1_MAX) {       /* For UIC2 */
		irq-=INT_LEVEL_UIC2_MIN ;
		intMask = 1 << (31 - irq) ;
		actualMask = mfdcr(UIC2_ER);
	} else if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#else
	if (irq > INT_LEVEL_UIC0_MAX) {       /* For UIC1 */
#endif
		irq-=INT_LEVEL_UIC1_MIN ;
		intMask = 1 << (31 - irq) ;
		actualMask = mfdcr(UIC1_ER);
	} else {							/* For UIC0 */
		intMask = 1 << (31 - irq);
		actualMask = mfdcr(UIC0_ER);
	}
	return (actualMask & intMask) == 0;

}

bool intctrl_t::is_pending(word_t irq) {
	word_t intMask;
	word_t actualMask;

	if (irq > INT_LEVEL_MAX || irq < INT_LEVEL_MIN)
		printf("Tried to check invalid interrupt!");


#if defined(PPC440EPx)
	if (irq > INT_LEVEL_UIC1_MAX) {       /* For UIC2 */
		irq-=INT_LEVEL_UIC2_MIN ;
		intMask = 1 << (31 - irq) ;
		actualMask = mfdcr(UIC2_SR);
	} else if (irq > INT_LEVEL_UIC0_MAX)        /* For UIC1 */
#else
	if (irq > INT_LEVEL_UIC0_MAX) {       /* For UIC1 */
#endif
		irq-=INT_LEVEL_UIC1_MIN ;
		intMask = 1 << (31 - irq) ;
		actualMask = mfdcr(UIC1_SR);
	} else {							/* For UIC0 */
		intMask = 1 << (31 - irq);
		actualMask = mfdcr(UIC0_SR);
	}
	return (actualMask & intMask) > 0;

}

void intctrl_t::dump() {
	lock.lock();
	printf("UIC0:\nSR: %08x\nER: %08x\nCR: %08x\nPR: %08x\nTR: %08x\nMSR: %08x\nVCR: %08x\nVR: %08x\n",
			mfdcr(UIC0_SR),mfdcr(UIC0_ER),mfdcr(UIC0_CR),mfdcr(UIC0_PR),
			mfdcr(UIC0_TR),mfdcr(UIC0_MSR),mfdcr(UIC0_VCR),mfdcr(UIC0_VR));
	printf("UIC1:\nSR: %08x\nER: %08x\nCR: %08x\nPR: %08x\nTR: %08x\nMSR: %08x\nVCR: %08x\nVR: %08x\n",
			mfdcr(UIC1_SR),mfdcr(UIC1_ER),mfdcr(UIC1_CR),mfdcr(UIC1_PR),
			mfdcr(UIC1_TR),mfdcr(UIC1_MSR),mfdcr(UIC1_VCR),mfdcr(UIC1_VR));
#if defined(PPC440EPx)
	printf("UIC2:\nSR: %08x\nER: %08x\nCR: %08x\nPR: %08x\nTR: %08x\nMSR: %08x\nVCR: %08x\nVR: %08x\n",
			mfdcr(UIC2_SR),mfdcr(UIC2_ER),mfdcr(UIC2_CR),mfdcr(UIC2_PR),
			mfdcr(UIC2_TR),mfdcr(UIC2_MSR),mfdcr(UIC2_VCR),mfdcr(UIC2_VR));
#endif
	lock.unlock();
}
