/*********************************************************************
 *                
 * Copyright (C) 2008-2009,  Karlsruhe University
 *                
 * File path:     glue/v4-x86/hvm.h
 * Description:   
 *                
 * @LICENSE@
 *                
 * $Id:$
 *                
 ********************************************************************/
#ifndef __GLUE__V4_X86__HVM_H__
#define __GLUE__V4_X86__HVM_H__

#include INC_GLUE_SA(hvm-vmx.h)

class arch_hvm_ktcb_t : public x86_svmx_hvm_t
{
public:
    /* Initialize/finalize. */
    bool is_hvm_enabled () 
        {return hvm_enabled; }
    bool enable_hvm ();
    void disable_hvm ();

    /* Send a virtualization IPC, wait for reply. */
    void send_hvm_fault	(word_t id, word_t qual, word_t insn_length, 
			 word_t addr_instr_info, bool internal);

    /* Manage guest-physical memory. */
    void handle_gphys_unmap (addr_t addr, word_t log2size)
        { 
            vtlb.flush_gphys (); 
        }


    /* Handle initial virtualization fault reply. */
    NORETURN void enter_hvm_loop ();
    
    /* Handle VM exit. */
    void handle_hvm_exit ();

    /* Get/set control registers. */
    word_t get_x86_hvm_cregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_cregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_hvm_dregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_dregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_hvm_segregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_segregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_hvm_nonregexc(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_nonregexc(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_hvm_execctrl(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_execctrl(word_t id, word_t mask, tcb_t *src, word_t &src_mr);

    word_t get_x86_hvm_otherregs(word_t id, word_t mask, tcb_t *dst, word_t &dst_mr);
    word_t set_x86_hvm_otherregs(word_t id, word_t mask, tcb_t *src, word_t &src_mr);


#if defined(CONFIG_DEBUG)
    word_t get_x86_hvm_ctrlxfer_reg(word_t id, word_t reg);
    void dump_hvm();
    void dump_hvm_ptab_entry(addr_t vaddr) 
        { vtlb.dump_ptab_entry(vaddr); }
#endif   

protected:
    void resume_hvm ();
    bool		hvm_enabled;		
    ringlist_t<tcb_t>   space_list;

    friend class x86_hvm_space_t;

};



#endif /* !__GLUE__V4_X86__HVM_H__ */
