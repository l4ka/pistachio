/*********************************************************************
 *
 * Copyright (C) 2007,  Karlsruhe University
 *
 * File path:     glue/v4-ia32/hvm-space.h
 * Description:   Full Virtualization Extensions
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
#ifndef __GLUE__V4_X86__HVM_SPACE_H__
#define __GLUE__V4_X86__HVM_SPACE_H__



class tcb_t;
class space_t;
class kdb_t;


class x86_hvm_space_t {
public:
    /* Activate virtualization for this space. */
    bool is_active() { return active; }
    bool activate (space_t *space);

    /* Remember attached TCBs. */
    void enqueue_tcb (tcb_t *tcb, space_t *space);
    void dequeue_tcb (tcb_t *tcb, space_t *space);

    /* Handle unmapping on all attached VCPUs. */
    void handle_gphys_unmap (addr_t g_paddr, word_t log2size);

    /* Lookup a mapping in a VTLB. */
    bool lookup_gphys_addr (addr_t gvaddr, addr_t *gpaddr);
    
#if defined(CONFIG_DEBUG)
    tcb_t *get_tcb_list() { return tcb_list; }
#endif
    
private:
    /* Set virtualization mode according to space. */
    void set_hvm_mode (tcb_t *tcb, space_t *space);

private:
    bool        active;
    tcb_t 	*tcb_list;
    
    
};



#endif /* !__GLUE__V4_X86__HVM_SPACE_H__ */
