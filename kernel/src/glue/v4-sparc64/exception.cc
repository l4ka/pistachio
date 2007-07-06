/*********************************************************************
 *                
 * Copyright (C) 2004,  University of New South Wales
 *                
 * File path:     glue/v4-sparc64/exception.cc
 * Description:   Exception and high-level system call handling for SPARCv9
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
 * $Id: exception.cc,v 1.2 2004/02/22 23:16:03 philipd Exp $
 *                
 ********************************************************************/

#include INC_API(space.h)
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)
#include INC_ARCH(frame.h)
#include INC_ARCH(registers.h)

extern "C" void
send_exception(void) {
    tt_t tt;
    tpc_t tpc;
    tnpc_t tnpc;

    tt.get();
    tpc.get();
    tnpc.get();

    tcb_t* current = get_current_tcb();

    if(current->get_exception_handler().is_nilthread()) {
	TRACEF("Unhandled exception 0x%x @ %p in user thread %p\n", tt.tt,
	       tpc.tpc, current->get_global_id().get_raw());
	current->set_state(thread_state_t::halted);
	current->switch_to_idle();
    }

    /* setup exception IPC */
    word_t saved_mr[8];
    word_t saved_br0;
    msg_tag_t tag;

    for(int i = 0; i < 8; i++) {
	saved_mr[i] = current->get_mr(i);
    }
    saved_br0 = current->get_br(0);
    
    tag.set(0, 8, -5 << 4);
    current->set_mr(0, tag.raw);
    current->set_mr(1, tpc.tpc);
    current->set_mr(2, tnpc.tnpc);
    current->set_mr(3, (word_t)current->get_user_sp());
    current->set_mr(4, current->get_user_flags());
    current->set_mr(5, tt.tt);
    current->set_mr(6, current->get_local_id().get_raw());
    current->set_mr(7, (word_t)mmu_t::get_d_sfar());

    tag = current->do_ipc(current->get_exception_handler(),
			  current->get_exception_handler(),
			  timeout_t::never());

    if(!tag.is_error() && tag.get_untyped() >= 3) {
	tpc.tpc = current->get_mr(1);
	tnpc.tnpc = current->get_mr(2);
	current->set_user_sp((addr_t)current->get_mr(3));
	current->set_user_flags(current->get_mr(4));
	tpc.set();
	tnpc.set();
    } else {
	printf("Unable to deliver user exception: IPC error\n");
    }

    for(int i = 0; i < 8; i++) {
	current->set_mr(i, saved_mr[i]);
    }
    current->set_br(0, saved_br0);
}

extern "C" word_t
send_syscall_ipc(word_t o0, word_t o1, word_t o2, word_t o3, word_t o4, word_t o5)
{
    register word_t trap_number asm("g1");
    tpc_t tpc;
    tcb_t* current = get_current_tcb();

    tpc.get();

    if(current->get_exception_handler().is_nilthread()) {
	TRACEF("Unhandled syscall %x @ %p in user thread %p\n", trap_number,
	       tpc.tpc, current->get_global_id().get_raw());
	current->set_state(thread_state_t::halted);
	current->switch_to_idle();
    }

    /* setup exception IPC */
    msg_tag_t tag;

    tag.set(0, 10, -5 << 4);
    current->set_mr(0, tag.raw);
    current->set_mr(1, o0);
    current->set_mr(2, o1);
    current->set_mr(3, o2);
    current->set_mr(4, o3);
    current->set_mr(5, o4);
    current->set_mr(6, o5);
    current->set_mr(7, trap_number);
    current->set_mr(8, tpc.tpc);
    current->set_mr(9, (word_t)current->get_user_sp());
    current->set_mr(10, (word_t)current->get_user_flags());

    tag = current->do_ipc(current->get_exception_handler(),
			  current->get_exception_handler(),
			  timeout_t::never());

    if(!tag.is_error()) {
	word_t o0_r = current->get_mr(1);
	register word_t o1_r asm ("o1") = current->get_mr(2);
	register word_t o2_r asm ("o2") = current->get_mr(3);
	register word_t o3_r asm ("o3") = current->get_mr(4);
	register word_t o4_r asm ("o4") = current->get_mr(5);
	register word_t o5_r asm ("o5") = current->get_mr(6);
	
	asm volatile ("" :: "r" (o1_r), "r" (o2_r), "r" (o3_r), "r" (o4_r), "r" (o5_r));
	
	return o0_r;
    } else {
	printf("Unable to deliver user exception: IPC error\n");
	return 0;
    }
}

extern "C" word_t
sys_kernel_interface(void)
{
    space_t* space = get_current_space();

    word_t kip_location = (word_t)space->get_kip_page_area().get_base();
    register word_t o1 asm ("o1") = get_kip()->api_version;
    register word_t o2 asm ("o2") = get_kip()->api_flags;
    register word_t o3 asm ("o3") = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();

    asm volatile ("" :: "r" (o1), "r" (o2), "r" (o3));

    return kip_location;
}
