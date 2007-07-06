/*********************************************************************
 *                
 * Copyright (C) 2002,   University of New South Wales
 *                
 * File path:     glue/v4-tmplarch/thread.cc
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
 * $Id: thread.cc,v 1.9 2003/09/24 19:05:32 skoglund Exp $
 *                
 ********************************************************************/


#include <debug.h>
#include INC_API(tcb.h)
#include INC_ARCH(pal.h)
#include INC_ARCH(thread.h)
/**
 * Setup TCB to execute a function when switched to
 * @param func pointer to function
 *
 * The old stack state of the TCB does not matter.
 * The stack will look like the following:
 *
 * |----------------------------|
 * |     Interrupt context      |
 * |                            |
 * |----------------------------|
 * |                            |
 * |    alpha_return_to_user    |
 * |  notification stack frame  |
 * |                            |
 * |----------------------------|
 * |                            |
 * |           func             |
 * |  notification stack frame  |
 * |                            |
 * |----------------------------| <- ksp
 *
 */
void tcb_t::create_startup_stack (void (*func)())
{
    /* Re-init the stack */
    init_stack();

    notify(alpha_return_to_user);
    notify(func);

    alpha_context_t *ctx = (alpha_context_t *) get_alpha_context(this);

    /* Set user mode */
    ctx->ps |= PAL_PS_USER;

    /* Set first argument to be the UTCB */
    /* sjw (22/02/2003): Required? */
    ctx->a0 = get_local_id().get_raw();

    /* Initialise KTCB */
    alpha_pcb_t *pcb = &get_arch()->pcb;

    /* set PME bit */
//    pcb->flags = 1ull << 62;

    get_arch()->pcb_paddr = get_kernel_space()->translate(pcb);
    ASSERT(get_arch()->pcb_paddr);

    get_arch()->pcb.unique = (word_t) get_utcb_location();
    
}
