/*********************************************************************
 *                
 * Copyright (C) 2002, 2004, 2008-2010,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/x32/ktcb.h
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
 * $Id: ktcb.h,v 1.4 2004/03/22 18:07:51 ud3 Exp $
 *                
 ********************************************************************/
#ifndef __GLUE_V4_X86__X32__KTCB_H__
#define __GLUE_V4_X86__X32__KTCB_H__

#include <tcb_layout.h>

#include INC_API(types.h)
#include INC_API(tcb.h)
#include INC_API_SCHED(ktcb.h)

#if defined(CONFIG_X_CTRLXFER_MSG)
#include INC_GLUE(ipc.h)
#endif

#if defined(CONFIG_X_X86_HVM)
#include INC_GLUE(hvm.h)
#define X86_CTRLXFER_FLAGMASK		(hvm_enabled ? (word_t) X86_HVM_EFLAGS_MASK : (word_t)  X86_USER_FLAGMASK)
#define X86_CTRLXFER_FAULT_MAX          vmcs_ei_reason_t::be_max
#else
#define X86_CTRLXFER_FLAGMASK		(word_t) (X86_USER_FLAGMASK)
#define X86_CTRLXFER_FAULT_MAX          0
class arch_hvm_ktcb_t 
{
};
#endif

typedef bitmask_t<u32_t> ctrlxfer_mask_t;

class arch_ktcb_t : public arch_hvm_ktcb_t
{
    /* TCB_START_MAKER */
    /* TCB_END_MARKER */

public:

#if defined(CONFIG_X_CTRLXFER_MSG)
    word_t get_x86_gpregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_gpregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_fpuregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr) { UNIMPLEMENTED(); return 0;}
    word_t set_x86_fpuregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr) { UNIMPLEMENTED(); return 0;}

    static get_ctrlxfer_regs_t get_ctrlxfer_regs[arch_ctrlxfer_item_t::id_max];
    static set_ctrlxfer_regs_t set_ctrlxfer_regs[arch_ctrlxfer_item_t::id_max];

    
#if defined(CONFIG_DEBUG)
    word_t get_ctrlxfer_reg(word_t id, word_t reg);
#endif

    static const word_t fault_max = X86_CTRLXFER_FAULT_MAX;

#endif /* defined(CONFIG_X_CTRLXFER_MSG) */

    
    friend class tcb_t;
    friend class x86_hvm_space_t;
};

#endif /* !__GLUE_V4_X86__X32__KTCB_H__ */
