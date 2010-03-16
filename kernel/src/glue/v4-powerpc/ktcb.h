/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, Jan Stoess, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/ktcb.h
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
#ifndef __GLUE__V4_POWERPC__KTCB_H__
#define __GLUE__V4_POWERPC__KTCB_H__

#include INC_GLUE(ipc.h)

typedef bitmask_t<u16_t> ctrlxfer_mask_t;
typedef word_t ctrlxfer_regs_t[16];

class ppc_softhvm_t;
struct except_regs_t;
class tcb_t;

#if defined(CONFIG_X_PPC_SOFTHVM)
class softhvm_t
{
public:
    enum exit_reason_e {
	er_program,
	er_tlb,
	er_max
    };

    static msg_tag_t fault_tag(int exc, int untyped, bool internal)
	{
	    return msg_tag_t::tag (0, untyped, -((2UL + IPC_CTRLXFER_STDFAULTS + exc) << 4) | 
				   (internal  ? 1 << 3 : 0));
	}

    static msg_tag_t pagefault_tag(int exc, int untyped, bool read, bool write, bool exec)
	{
	    msg_tag_t tag = fault_tag(exc, untyped, false);
	    tag.x.label |= ( (read ? 1 << 2 : 0) |
			     (write ? 1 << 1 : 0) | 
			     (exec  ? 1 << 0 : 0) );
	    return tag;
	}

};
#endif

class arch_ktcb_t
{
public:
#if !defined(CONFIG_X_PPC_SOFTHVM)
    static const int fault_max = 0;
#else
    static const int fault_max = softhvm_t::er_max;
    ppc_softhvm_t *vm;
    void init_hvm(tcb_t *tcb);

    bool send_hvm_fault(softhvm_t::exit_reason_e exc,
			except_regs_t *frame, word_t instr, word_t param, bool internal);
    bool send_hvm_pagefault(softhvm_t::exit_reason_e exc, except_regs_t *frame,
                            word_t addr, word_t instr,
			    word_t tlb0, word_t tlb1, word_t tlb2, u8_t pid, u8_t idx,
			    bool read, bool write, bool execute);

    word_t get_powerpc_frameregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_powerpc_frameregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t powerpc_ctrlxfer_fpu(tcb_t *dst);
    word_t get_powerpc_fpuregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_powerpc_fpuregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);
    
    word_t get_powerpc_vmregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_powerpc_vmregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);
    word_t get_powerpc_tlbregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_powerpc_tlbregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);
    
#if defined(CONFIG_DEBUG)
    word_t get_ctrlxfer_reg(word_t id, word_t reg);
#endif

    static get_ctrlxfer_regs_t get_ctrlxfer_regs[arch_ctrlxfer_item_t::id_max];
    static set_ctrlxfer_regs_t set_ctrlxfer_regs[arch_ctrlxfer_item_t::id_max];
    
#endif
};

#endif /* !__GLUE__V4_POWERPC__KTCB_H__ */
