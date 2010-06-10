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



class intctrl_t : public generic_intctrl_t
{
public:
    void init_arch();
    void init_cpu(int cpu);

    word_t get_number_irqs() 
	{ UNIMPLEMENTED(); return 0; }

    bool is_irq_available(word_t irq)
	{ UNIMPLEMENTED(); return false; }

    void mask(word_t irq)
	{ UNIMPLEMENTED(); }

    bool unmask(word_t irq)
	{ UNIMPLEMENTED(); return false; }

    bool is_masked(word_t irq)
	{ UNIMPLEMENTED(); return false; }

    bool is_pending(word_t irq)
	{ UNIMPLEMENTED(); return false; }

    void enable(word_t irq)
	{ UNIMPLEMENTED(); }

    void disable(word_t irq)
	{ UNIMPLEMENTED(); }

    bool is_enabled(word_t irq)
	{ UNIMPLEMENTED(); return false; }

    void set_cpu(word_t irq, word_t cpu)
	{ UNIMPLEMENTED(); }


    /* handler invoked on interrupt */
    void handle_irq(word_t cpu)
        { UNIMPLEMENTED(); }


    /* map routine provided by glue */
    void map()
	{ UNIMPLEMENTED(); }

    /* SMP support functions */
    void start_new_cpu(word_t cpu)
        { UNIMPLEMENTED(); }

    void send_ipi(word_t cpu)
	{ UNIMPLEMENTED(); }

    /* debug */
    void dump() 
	{ UNIMPLEMENTED(); }

private:


};

#endif /* !__PLATFORM__PPC44X__UIC_H__ */
